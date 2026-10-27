#include "common.hpp"
#include "lib/util/sys/modules.hpp"
#include "lib/hook/trampoline.hpp"
#include <lib/log/logger_mgr.hpp>
#include <program/loggers.hpp>
#include <stdio.h>
#include <string.h>

namespace {
    constexpr uintptr_t kSpeedDiceUiInit = 0x233BF20;
    constexpr uintptr_t kSpeedDiceUiChangeSprite = 0x233B750;

    constexpr uintptr_t kFieldNormalFrame = 0xB8;
    constexpr uintptr_t kFieldLightFrame = 0xC0;
    constexpr uintptr_t kFieldTensNum = 0xD0;
    constexpr uintptr_t kFieldUnitsNum = 0xD8;
    constexpr uintptr_t kFieldRouletteImg = 0xE8;
    constexpr uintptr_t kFieldTxtSpeedRange = 0xF0;
    constexpr uintptr_t kFieldTxtSpeedMax = 0xF8;
    constexpr uintptr_t kFieldBreakedFrame = 0x108;
    constexpr uintptr_t kFieldBreakedLinearDodge = 0x110;
    constexpr uintptr_t kFieldLockedFrame = 0x120;
    constexpr uintptr_t kFieldLockedIcon = 0x128;
    constexpr uintptr_t kFieldHighlightFrame = 0x140;

    constexpr uintptr_t kSpriteUtilLoadSprite = 0x1DBB8E0;
    constexpr uintptr_t kSpriteCreate = 0x19C3210;
    constexpr uintptr_t kSpriteGetTexture = 0x19C1BD0;
    constexpr uintptr_t kTextureGetWidth = 0x2BE3930;
    constexpr uintptr_t kTextureGetHeight = 0x2BE39D0;
    constexpr uintptr_t kImageGetSprite = 0x24E55C0;
    constexpr uintptr_t kImageSetSprite = 0x24D56D0;
    constexpr uintptr_t kImageGetPixelsPerUnit = 0x24E64A0;
    constexpr uintptr_t kGraphicSetColor = 0x24DE110;
    constexpr uintptr_t kGraphicGetColor = 0x24DE100;
    constexpr uintptr_t kTmpTextSetColor = 0x2069670;
    constexpr uintptr_t kComponentGetGameObject = 0x19AE220;
    constexpr uintptr_t kComponentGetTransform = 0x19AE1D0;
    constexpr uintptr_t kGameObjectSetActive = 0x19AFD70;
    constexpr uintptr_t kTransformGetLocalPosition = 0x19BAB40;
    constexpr uintptr_t kTransformSetLocalPosition = 0x19BAC70;
    constexpr uintptr_t kTransformSetLocalScale = 0x19BD790;
    constexpr uintptr_t kStreamingAssetsPath = 0x17E7490;
    constexpr uintptr_t kPtrToStringAnsi = 0x20EEF30;
    constexpr uintptr_t kStringConcat2 = 0x2F1B580;
    constexpr uintptr_t kFileExists = 0x2045D00;
    constexpr uintptr_t kGcHandleAlloc = 0x20EDF30;
    constexpr uintptr_t kGcHandleGetTarget = 0x20EDD10;

    constexpr int kFactionPlayer = 1;
    constexpr int kInfiniteThreshold = 999;
    constexpr int kMaxBase = 256;
    constexpr int kMaxScaled = 512;

    struct Vector2 { float x, y; };
    struct Vector3 { float x, y, z; };
    struct Rect { float x, y, w, h; };
    struct Color { float r, g, b, a; };
    struct GcHandle { intptr_t handle; };

    struct BaseEntry {
        char name[96];
        bool tried;
        bool ok;
        GcHandle handle;
    };

    struct ScaledEntry {
        char name[96];
        float ppu;
        GcHandle handle;
    };

    BaseEntry s_Base[kMaxBase];
    int s_BaseCount = 0;
    ScaledEntry s_Scaled[kMaxScaled];
    int s_ScaledCount = 0;
    int s_LogBudget = 40;

    template<typename Sig>
    Sig* Fn(uintptr_t rva) {
        return reinterpret_cast<Sig*>(exl::util::GetMainModuleInfo().m_Total.m_Start + rva);
    }

    inline void* Field(void* self, uintptr_t off) {
        return *reinterpret_cast<void* const*>(reinterpret_cast<uintptr_t>(self) + off);
    }

    void* Deref(GcHandle& h) {
        return Fn<void*(GcHandle*, const void*)>(kGcHandleGetTarget)(&h, nullptr);
    }

    void* LoadBaseSprite(const char* name) {
        BaseEntry* entry = nullptr;
        for(int i = 0; i < s_BaseCount; i++) {
            if(strcmp(s_Base[i].name, name) == 0) {
                entry = &s_Base[i];
                break;
            }
        }
        if(entry == nullptr) {
            if(s_BaseCount >= kMaxBase)
                return nullptr;
            entry = &s_Base[s_BaseCount++];
            snprintf(entry->name, sizeof(entry->name), "%s", name);
            entry->tried = false;
            entry->ok = false;
        }
        if(entry->tried)
            return entry->ok ? Deref(entry->handle) : nullptr;

        entry->tried = true;

        char rel[160];
        snprintf(rel, sizeof(rel), "/SpeedDice/%s.png", name);

        void* dir = Fn<void*(const void*)>(kStreamingAssetsPath)(nullptr);
        void* tail = Fn<void*(intptr_t, const void*)>(kPtrToStringAnsi)(reinterpret_cast<intptr_t>(rel), nullptr);
        if(dir == nullptr || tail == nullptr)
            return nullptr;
        void* path = Fn<void*(void*, void*, const void*)>(kStringConcat2)(dir, tail, nullptr);
        if(path == nullptr)
            return nullptr;

        if(!Fn<bool(void*, const void*)>(kFileExists)(path, nullptr)) {
            if(s_LogBudget-- > 0)
                Logging.Log("[LoRSpeedDice] missing: %s", rel);
            return nullptr;
        }

        const Vector2 pivot = { 0.5f, 0.5f };
        void* sprite = Fn<void*(void*, Vector2, const void*)>(kSpriteUtilLoadSprite)(path, pivot, nullptr);
        if(sprite == nullptr) {
            if(s_LogBudget-- > 0)
                Logging.Log("[LoRSpeedDice] load failed: %s", rel);
            return nullptr;
        }

        entry->handle = Fn<GcHandle(void*, const void*)>(kGcHandleAlloc)(sprite, nullptr);
        entry->ok = true;
        if(s_LogBudget-- > 0)
            Logging.Log("[LoRSpeedDice] loaded: %s", rel);
        return sprite;
    }

    void* GetScaledSprite(const char* name, void* image) {
        if(image == nullptr)
            return nullptr;

        const float ppu = Fn<float(void*, const void*)>(kImageGetPixelsPerUnit)(image, nullptr);

        for(int i = 0; i < s_ScaledCount; i++) {
            if(s_Scaled[i].ppu == ppu && strcmp(s_Scaled[i].name, name) == 0)
                return Deref(s_Scaled[i].handle);
        }

        void* baseSprite = LoadBaseSprite(name);
        if(baseSprite == nullptr)
            return nullptr;
        void* texture = Fn<void*(void*, const void*)>(kSpriteGetTexture)(baseSprite, nullptr);
        if(texture == nullptr)
            return nullptr;

        const int w = Fn<int(void*, const void*)>(kTextureGetWidth)(texture, nullptr);
        const int h = Fn<int(void*, const void*)>(kTextureGetHeight)(texture, nullptr);
        const Rect rect = { 0.0f, 0.0f, static_cast<float>(w), static_cast<float>(h) };
        const Vector2 pivot = { 0.5f, 0.5f };

        void* sprite = Fn<void*(void*, Rect, Vector2, float, uint32_t, int, const void*)>(kSpriteCreate)(texture, rect, pivot, ppu, 0, 0, nullptr);
        if(sprite == nullptr)
            return nullptr;

        if(s_ScaledCount < kMaxScaled) {
            ScaledEntry& e = s_Scaled[s_ScaledCount++];
            snprintf(e.name, sizeof(e.name), "%s", name);
            e.ppu = ppu;
            e.handle = Fn<GcHandle(void*, const void*)>(kGcHandleAlloc)(sprite, nullptr);
        }
        return sprite;
    }

    bool SetImageSprite(void* image, const char* name) {
        void* sprite = GetScaledSprite(name, image);
        if(sprite == nullptr)
            return false;
        if(Fn<void*(void*, const void*)>(kImageGetSprite)(image, nullptr) == sprite)
            return true;
        Fn<void(void*, void*, const void*)>(kImageSetSprite)(image, sprite, nullptr);
        return true;
    }

    void SetGraphicColor(void* graphic, Color c) {
        if(graphic == nullptr)
            return;
        Fn<void(void*, Color, const void*)>(kGraphicSetColor)(graphic, c, nullptr);
    }

    void SetTmpColor(void* tmp, Color c) {
        if(tmp == nullptr)
            return;
        Fn<void(void*, Color, const void*)>(kTmpTextSetColor)(tmp, c, nullptr);
    }

    void ChangeSpeedDiceColor(const char* name, const char* suffix, Color color, void* ui) {
        if(ui == nullptr)
            return;

        char normal[96];
        char glow[96];
        char hovered[96];
        if(suffix != nullptr && suffix[0] != 0)
            snprintf(normal, sizeof(normal), "%s_%s", name, suffix);
        else
            snprintf(normal, sizeof(normal), "%s", name);
        snprintf(glow, sizeof(glow), "%s_Glow", name);
        snprintf(hovered, sizeof(hovered), "%s_Hovered", name);

        SetImageSprite(Field(ui, kFieldNormalFrame), normal);
        SetImageSprite(Field(ui, kFieldLightFrame), glow);
        SetImageSprite(Field(ui, kFieldHighlightFrame), hovered);

        SetGraphicColor(Field(ui, kFieldTxtSpeedRange), color);
        SetGraphicColor(Field(ui, kFieldRouletteImg), color);
        SetTmpColor(Field(ui, kFieldTxtSpeedMax), color);
        SetGraphicColor(Field(ui, kFieldTensNum), color);
        SetGraphicColor(Field(ui, kFieldUnitsNum), color);

        Color dim = color;
        dim.a -= 0.6f;
        SetGraphicColor(Field(ui, kFieldBreakedFrame), dim);
        SetGraphicColor(Field(ui, kFieldLockedFrame), dim);
        SetGraphicColor(Field(ui, kFieldLockedIcon), dim);
        SetGraphicColor(Field(ui, kFieldBreakedLinearDodge), dim);
    }

    void ApplyDefault(void* ui, int faction) {
        if(faction == kFactionPlayer)
            ChangeSpeedDiceColor("Hokma", nullptr, Color{ 0.7f, 0.7f, 0.7f, 1.0f }, ui);
        else
            ChangeSpeedDiceColor("Abnormalities", nullptr, Color{ 0.7f, 0.5f, 0.4f, 1.0f }, ui);
    }

    void ApplyInfinite(void* ui, int value) {
        if(ui == nullptr || value < kInfiniteThreshold)
            return;

        void* tens = Field(ui, kFieldTensNum);
        void* units = Field(ui, kFieldUnitsNum);
        void* maxText = Field(ui, kFieldTxtSpeedMax);
        if(tens == nullptr)
            return;

        void* sprite = GetScaledSprite("Infinite", tens);
        if(sprite == nullptr)
            return;
        if(Fn<void*(void*, const void*)>(kImageGetSprite)(tens, nullptr) == sprite)
            return;

        if(maxText != nullptr) {
            void* maxGo = Fn<void*(void*, const void*)>(kComponentGetGameObject)(maxText, nullptr);
            if(maxGo != nullptr)
                Fn<void(void*, bool, const void*)>(kGameObjectSetActive)(maxGo, false, nullptr);
        }

        Fn<void(void*, void*, const void*)>(kImageSetSprite)(tens, sprite, nullptr);

        void* tf = Fn<void*(void*, const void*)>(kComponentGetTransform)(tens, nullptr);
        if(tf != nullptr) {
            Vector3 pos = Fn<Vector3(void*, const void*)>(kTransformGetLocalPosition)(tf, nullptr);
            pos.x += 13.0f;
            Fn<void(void*, Vector3, const void*)>(kTransformSetLocalPosition)(tf, pos, nullptr);
            Fn<void(void*, Vector3, const void*)>(kTransformSetLocalScale)(tf, Vector3{ 0.95f, 0.85f, 0.0f }, nullptr);
        }

        if(units != nullptr) {
            Color c = Fn<Color(void*, const void*)>(kGraphicGetColor)(units, nullptr);
            SetGraphicColor(tens, c);
        }

        void* tensGo = Fn<void*(void*, const void*)>(kComponentGetGameObject)(tens, nullptr);
        if(tensGo != nullptr)
            Fn<void(void*, bool, const void*)>(kGameObjectSetActive)(tensGo, true, nullptr);
    }

    HOOK_DEFINE_TRAMPOLINE(SpeedDiceUiInitHook) {
        static void Callback(void* self, int index, int diceMin, int diceFace, int faction, const void* method) {
            Orig(self, index, diceMin, diceFace, faction, method);
            ApplyDefault(self, faction);
        }
    };

    HOOK_DEFINE_TRAMPOLINE(SpeedDiceUiChangeSpriteHook) {
        static void Callback(void* self, int face, int value, const void* method) {
            Orig(self, face, value, method);
            ApplyInfinite(self, value);
        }
    };
}

extern "C" void exl_main(void* x0, void* x1) {
    (void)x0; (void)x1;

    Logging.Log("[LoRSpeedDice] stage 5: full frame set");

    exl::hook::Initialize();
    SpeedDiceUiInitHook::InstallAtOffset(kSpeedDiceUiInit);
    SpeedDiceUiChangeSpriteHook::InstallAtOffset(kSpeedDiceUiChangeSprite);

    Logging.Log("[LoRSpeedDice] hooks installed");
}

extern "C" void exl_exception_entry() {
    while (true) {}
}
