#include "common.hpp"
#include "lib/util/sys/modules.hpp"
#include "lib/hook/trampoline.hpp"
#include <lib/log/logger_mgr.hpp>
#include <program/loggers.hpp>

namespace {
    constexpr uintptr_t kSpeedDiceUiInit = 0x233BF20;
    constexpr uintptr_t kSpeedDiceUiChangeSprite = 0x233B750;

    constexpr uintptr_t kFieldNormalFrame = 0xB8;

    constexpr uintptr_t kSpriteUtilLoadSprite = 0x1DBB8E0;
    constexpr uintptr_t kImageGetSprite = 0x24E55C0;
    constexpr uintptr_t kImageSetSprite = 0x24D56D0;
    constexpr uintptr_t kStreamingAssetsPath = 0x17E7490;
    constexpr uintptr_t kPtrToStringAnsi = 0x20EEF30;
    constexpr uintptr_t kStringConcat2 = 0x2F1B580;
    constexpr uintptr_t kFileExists = 0x2045D00;
    constexpr uintptr_t kGcHandleAlloc = 0x20EDF30;
    constexpr uintptr_t kGcHandleGetTarget = 0x20EDD10;

    constexpr const char* kRelativePath = "/SpeedDice/dice.png";

    struct Vector2 {
        float x, y;
    };

    struct GcHandle {
        intptr_t handle;
    };

    template<typename Sig>
    Sig* Fn(uintptr_t rva) {
        return reinterpret_cast<Sig*>(exl::util::GetMainModuleInfo().m_Total.m_Start + rva);
    }

    enum class LoadState { NotTried, Loaded, Failed };

    LoadState s_State = LoadState::NotTried;
    GcHandle s_Handle = {};
    int s_AppliedCount = 0;

    void LogManagedString(const char* tag, void* str) {
        if(str == nullptr) {
            Logging.Log("[LoRSpeedDice] %s<null>", tag);
            return;
        }
        const int len = *reinterpret_cast<const int*>(reinterpret_cast<uintptr_t>(str) + 0x10);
        const u16* chars = reinterpret_cast<const u16*>(reinterpret_cast<uintptr_t>(str) + 0x14);
        char buf[220];
        int n = len < 219 ? len : 219;
        for(int i = 0; i < n; i++)
            buf[i] = chars[i] < 128 ? static_cast<char>(chars[i]) : '?';
        buf[n] = 0;
        Logging.Log("[LoRSpeedDice] %s%s", tag, buf);
    }

    void TryLoad() {
        s_State = LoadState::Failed;

        void* dir = Fn<void*(const void*)>(kStreamingAssetsPath)(nullptr);
        void* tail = Fn<void*(intptr_t, const void*)>(kPtrToStringAnsi)(reinterpret_cast<intptr_t>(kRelativePath), nullptr);
        if(dir == nullptr || tail == nullptr) {
            Logging.Log("[LoRSpeedDice] could not build path string");
            return;
        }
        void* path = Fn<void*(void*, void*, const void*)>(kStringConcat2)(dir, tail, nullptr);
        LogManagedString("path: ", path);

        const bool exists = Fn<bool(void*, const void*)>(kFileExists)(path, nullptr);
        Logging.Log("[LoRSpeedDice] file exists: %d", exists ? 1 : 0);
        if(!exists)
            return;

        const Vector2 pivot = { 0.5f, 0.5f };
        void* sprite = Fn<void*(void*, Vector2, const void*)>(kSpriteUtilLoadSprite)(path, pivot, nullptr);
        if(sprite == nullptr) {
            Logging.Log("[LoRSpeedDice] SpriteUtil.LoadSprite returned null");
            return;
        }

        s_Handle = Fn<GcHandle(void*, const void*)>(kGcHandleAlloc)(sprite, nullptr);
        s_State = LoadState::Loaded;
        Logging.Log("[LoRSpeedDice] sprite loaded and pinned: %016lx", (unsigned long) sprite);
    }

    void ApplyFrameSprite(void* self) {
        if(self == nullptr)
            return;

        if(s_State == LoadState::NotTried)
            TryLoad();
        if(s_State != LoadState::Loaded)
            return;

        void* sprite = Fn<void*(GcHandle, const void*)>(kGcHandleGetTarget)(s_Handle, nullptr);
        if(sprite == nullptr)
            return;

        void* image = *reinterpret_cast<void* const*>(reinterpret_cast<uintptr_t>(self) + kFieldNormalFrame);
        if(image == nullptr)
            return;

        if(Fn<void*(void*, const void*)>(kImageGetSprite)(image, nullptr) == sprite)
            return;

        Fn<void(void*, void*, const void*)>(kImageSetSprite)(image, sprite, nullptr);

        if(s_AppliedCount < 8)
            Logging.Log("[LoRSpeedDice] frame sprite replaced on dice %016lx", (unsigned long) self);
        s_AppliedCount++;
    }

    HOOK_DEFINE_TRAMPOLINE(SpeedDiceUiInitHook) {
        static void Callback(void* self, int index, int diceMin, int diceFace, int faction, const void* method) {
            Orig(self, index, diceMin, diceFace, faction, method);
            ApplyFrameSprite(self);
        }
    };

    HOOK_DEFINE_TRAMPOLINE(SpeedDiceUiChangeSpriteHook) {
        static void Callback(void* self, int face, int value, const void* method) {
            Orig(self, face, value, method);
            ApplyFrameSprite(self);
        }
    };
}

extern "C" void exl_main(void* x0, void* x1) {
    (void)x0; (void)x1;

    Logging.Log("[LoRSpeedDice] stage 4: sprite replacement");

    exl::hook::Initialize();
    SpeedDiceUiInitHook::InstallAtOffset(kSpeedDiceUiInit);
    SpeedDiceUiChangeSpriteHook::InstallAtOffset(kSpeedDiceUiChangeSprite);

    Logging.Log("[LoRSpeedDice] hooks installed");
}

extern "C" void exl_exception_entry() {
    while (true) {}
}
