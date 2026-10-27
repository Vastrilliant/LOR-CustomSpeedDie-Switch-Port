#include "common.hpp"
#include "lib/util/sys/modules.hpp"
#include "lib/hook/trampoline.hpp"
#include <lib/log/logger_mgr.hpp>
#include <program/loggers.hpp>

namespace {
    constexpr uintptr_t kSpeedDiceUiInit = 0x233BF20;
    constexpr uintptr_t kSpeedDiceUiChangeSprite = 0x233B750;
    constexpr uintptr_t kGraphicSetColor = 0x24DE110;

    constexpr uintptr_t kFieldNormalFrame = 0xB8;
    constexpr uintptr_t kFieldLightFrame = 0xC0;

    struct Color {
        float r, g, b, a;
    };

    using SetColorFn = void (*)(void* graphic, Color value, const void* method);

    constexpr Color kRed = { 1.0f, 0.2f, 0.2f, 1.0f };

    int s_TintCount = 0;

    SetColorFn GetSetColor() {
        const auto& mainMod = exl::util::GetMainModuleInfo();
        return reinterpret_cast<SetColorFn>(mainMod.m_Total.m_Start + kGraphicSetColor);
    }

    void TintImage(void* self, uintptr_t fieldOffset) {
        void* image = *reinterpret_cast<void* const*>(reinterpret_cast<uintptr_t>(self) + fieldOffset);
        if(image == nullptr)
            return;
        GetSetColor()(image, kRed, nullptr);
    }

    void TintDice(void* self) {
        if(self == nullptr)
            return;
        TintImage(self, kFieldNormalFrame);
        TintImage(self, kFieldLightFrame);
        if(s_TintCount < 8) {
            Logging.Log("[LoRSpeedDice] tinted dice self=%016lx", (unsigned long) self);
        }
        s_TintCount++;
    }

    HOOK_DEFINE_TRAMPOLINE(SpeedDiceUiInitHook) {
        static void Callback(void* self, int index, int diceMin, int diceFace, int faction, const void* method) {
            Orig(self, index, diceMin, diceFace, faction, method);
            TintDice(self);
        }
    };

    HOOK_DEFINE_TRAMPOLINE(SpeedDiceUiChangeSpriteHook) {
        static void Callback(void* self, int face, int value, const void* method) {
            Orig(self, face, value, method);
            TintDice(self);
        }
    };
}

extern "C" void exl_main(void* x0, void* x1) {
    (void)x0; (void)x1;

    Logging.Log("[LoRSpeedDice] stage 3: red tint");

    exl::hook::Initialize();
    SpeedDiceUiInitHook::InstallAtOffset(kSpeedDiceUiInit);
    SpeedDiceUiChangeSpriteHook::InstallAtOffset(kSpeedDiceUiChangeSprite);

    Logging.Log("[LoRSpeedDice] hooks installed");
}

extern "C" void exl_exception_entry() {
    while (true) {}
}
