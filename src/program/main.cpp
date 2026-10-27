#include "common.hpp"
#include "lib/util/sys/modules.hpp"
#include "lib/hook/trampoline.hpp"
#include <lib/log/logger_mgr.hpp>
#include <program/loggers.hpp>
#include <stdio.h>
#include <string.h>
#include "speeddice_rules.hpp"

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
    constexpr uintptr_t kGcHandleFree = 0x20EE000;

    constexpr uintptr_t kStageControllerInitCommon = 0x1CFAAD0;
    constexpr uintptr_t kStageControllerStartBattle = 0x1CFBF10;
    constexpr uintptr_t kStageControllerApplyLibrarianCardPhase = 0x1D06410;
    constexpr uintptr_t kStageClassInfoGetId = 0x1A41830;
    constexpr uintptr_t kStageClassInfoGetStoryType = 0x1A53550;
    constexpr uintptr_t kSpecialStageIdsIsBlueBand = 0x1A534C0;
    constexpr uintptr_t kSpecialStageIdsIsKeterFinal = 0x1A534E0;
    constexpr uintptr_t kLorIdOpEqualityInt = 0x1DA4280;
    constexpr uintptr_t kBattleUnitModelGetUnitData = 0x1EEF510;
    constexpr uintptr_t kBattleUnitModelIsBreakLifeZero = 0x1EF0990;
    constexpr uintptr_t kUnitDataModelGetDefaultBook = 0x1D26D40;
    constexpr uintptr_t kUnitDataModelGetEnemyUnitId = 0x1D26F70;
    constexpr uintptr_t kBookModelGetBookClassInfoId = 0x20AC560;
    constexpr uintptr_t kBufListDetailHasAssimilation = 0x1B3E2F0;
    constexpr uintptr_t kBattleUnitBufIsDestroyed = 0x1E25390;
    constexpr uintptr_t kBattleDiceCardModelGetId = 0x2446BE0;
    constexpr uintptr_t kBattleObjectManagerGetInstance = 0x28807A0;
    constexpr uintptr_t kBattleObjectManagerGetAliveListFaction = 0x2885570;
    constexpr uintptr_t kSpeedDiceSetterGetSpeedDiceByIndex = 0x233B5B0;

    constexpr uintptr_t kFieldView = 0x98;
    constexpr uintptr_t kViewModel = 0x28;
    constexpr uintptr_t kModelView = 0x50;
    constexpr uintptr_t kViewSpeedDiceSetter = 0x1C0;
    constexpr uintptr_t kModelCardSlotDetail = 0x100;
    constexpr uintptr_t kModelBufListDetail = 0x108;
    constexpr uintptr_t kModelPassiveDetail = 0x128;
    constexpr uintptr_t kCardSlotDetailCardAry = 0x30;
    constexpr uintptr_t kPlayingCardCard = 0x28;
    constexpr uintptr_t kBufListDetailBufList = 0x18;
    constexpr uintptr_t kPassiveDetailPassiveList = 0x18;
    constexpr uintptr_t kPassiveAbilityDestroyed = 0x18;
    constexpr uintptr_t kUnitBattleDataUnitData = 0x10;
    constexpr uintptr_t kUnitDataIsSephirah = 0xB8;
    constexpr uintptr_t kStageControllerStageType = 0x18;
    constexpr uintptr_t kStageControllerCurrentFloor = 0x20;
    constexpr uintptr_t kStageControllerCurrentWave = 0x24;
    constexpr uintptr_t kStageControllerStageModel = 0x10;
    constexpr uintptr_t kStageModelClassInfo = 0x10;
    constexpr uintptr_t kStageClassInfoChapter = 0x48;
    constexpr uintptr_t kLorIdId = 0x10;
    constexpr uintptr_t kListItems = 0x10;
    constexpr uintptr_t kListSize = 0x18;
    constexpr uintptr_t kArrayData = 0x20;
    constexpr uintptr_t kClassName = 0x10;
    constexpr uintptr_t kClassParent = 0x58;
    constexpr uintptr_t kStringLength = 0x10;
    constexpr uintptr_t kStringChars = 0x14;

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
    GcHandle s_StageHandle;
    bool s_StageHandleValid = false;
    void* s_StageRaw = nullptr;

    template<typename Sig>
    Sig* Fn(uintptr_t rva) {
        return reinterpret_cast<Sig*>(exl::util::GetMainModuleInfo().m_Total.m_Start + rva);
    }

    inline void* Field(void* self, uintptr_t off) {
        if(self == nullptr)
            return nullptr;
        return *reinterpret_cast<void* const*>(reinterpret_cast<uintptr_t>(self) + off);
    }

    inline int FieldInt(void* self, uintptr_t off) {
        if(self == nullptr)
            return 0;
        return *reinterpret_cast<const int*>(reinterpret_cast<uintptr_t>(self) + off);
    }

    inline bool FieldBool(void* self, uintptr_t off) {
        if(self == nullptr)
            return false;
        return *reinterpret_cast<const uint8_t*>(reinterpret_cast<uintptr_t>(self) + off) != 0;
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

    bool StringEquals(void* str, const char* ascii) {
        if(str == nullptr)
            return false;
        const int length = FieldInt(str, kStringLength);
        const char16_t* chars = reinterpret_cast<const char16_t*>(reinterpret_cast<uintptr_t>(str) + kStringChars);
        int i = 0;
        for(; ascii[i] != 0; i++) {
            if(i >= length || chars[i] != static_cast<char16_t>(static_cast<unsigned char>(ascii[i])))
                return false;
        }
        return i == length;
    }

    int ListCount(void* list) {
        return FieldInt(list, kListSize);
    }

    void* ListAt(void* list, int index) {
        if(list == nullptr || index < 0 || index >= ListCount(list))
            return nullptr;
        void* items = Field(list, kListItems);
        if(items == nullptr)
            return nullptr;
        return *reinterpret_cast<void* const*>(reinterpret_cast<uintptr_t>(items) + kArrayData + sizeof(void*) * static_cast<size_t>(index));
    }

    bool IsInstanceOf(void* obj, const char* className) {
        if(obj == nullptr)
            return false;
        void* klass = *reinterpret_cast<void* const*>(obj);
        for(; klass != nullptr; klass = Field(klass, kClassParent)) {
            const char* name = *reinterpret_cast<const char* const*>(reinterpret_cast<uintptr_t>(klass) + kClassName);
            if(name != nullptr && strcmp(name, className) == 0)
                return true;
        }
        return false;
    }

    bool LorIdIs(void* lorId, int value) {
        if(lorId == nullptr)
            return false;
        return Fn<bool(void*, int, const void*)>(kLorIdOpEqualityInt)(lorId, value, nullptr);
    }

    void* UnitDataOf(void* model) {
        if(model == nullptr)
            return nullptr;
        void* battleData = Fn<void*(void*, const void*)>(kBattleUnitModelGetUnitData)(model, nullptr);
        return Field(battleData, kUnitBattleDataUnitData);
    }

    bool EnemyUnitIdIs(void* unit, int value) {
        void* unitData = UnitDataOf(unit);
        if(unitData == nullptr)
            return false;
        return LorIdIs(Fn<void*(void*, const void*)>(kUnitDataModelGetEnemyUnitId)(unitData, nullptr), value);
    }

    void* StoryTypeOf(void* stage) {
        void* classInfo = Field(Field(stage, kStageControllerStageModel), kStageModelClassInfo);
        if(classInfo == nullptr)
            return nullptr;
        return Fn<void*(void*, const void*)>(kStageClassInfoGetStoryType)(classInfo, nullptr);
    }

    void* CurrentStage() {
        if(!s_StageHandleValid)
            return nullptr;
        return Deref(s_StageHandle);
    }

    void CacheStage(void* stage) {
        if(stage == nullptr || (s_StageHandleValid && stage == s_StageRaw))
            return;
        if(s_StageHandleValid)
            Fn<void(GcHandle*, const void*)>(kGcHandleFree)(&s_StageHandle, nullptr);
        s_StageHandle = Fn<GcHandle(void*, const void*)>(kGcHandleAlloc)(stage, nullptr);
        s_StageHandleValid = true;
        s_StageRaw = stage;
    }

    struct GameEnv {
        void* ui;
        int factionValue;
        void* stage;
        void* classInfo;
        void* model;
        void* unitData;
        void* story;
        bool storyFetched;

        GameEnv(void* uiIn, int faction, void* stageIn)
            : ui(uiIn), factionValue(faction), stage(stageIn), story(nullptr), storyFetched(false) {
            classInfo = Field(Field(stage, kStageControllerStageModel), kStageModelClassInfo);
            model = Field(Field(ui, kFieldView), kViewModel);
            unitData = UnitDataOf(model);
        }

        void* stageId() {
            if(classInfo == nullptr)
                return nullptr;
            return Fn<void*(void*, const void*)>(kStageClassInfoGetId)(classInfo, nullptr);
        }

        void* defaultBook() {
            if(unitData == nullptr)
                return nullptr;
            return Fn<void*(void*, const void*)>(kUnitDataModelGetDefaultBook)(unitData, nullptr);
        }

        int faction() { return factionValue; }
        int wave() { return FieldInt(stage, kStageControllerCurrentWave); }
        int floor() { return FieldInt(stage, kStageControllerCurrentFloor); }
        int stageType() { return FieldInt(stage, kStageControllerStageType); }
        int chapter() { return FieldInt(classInfo, kStageClassInfoChapter); }
        int stageIdValue() { return FieldInt(stageId(), kLorIdId); }
        bool stageIdIs(int value) { return LorIdIs(stageId(), value); }

        bool keterFinal() {
            void* id = stageId();
            if(id == nullptr)
                return false;
            return Fn<bool(void*, const void*)>(kSpecialStageIdsIsKeterFinal)(id, nullptr);
        }

        bool blueBand(int id) {
            return Fn<bool(int, const void*)>(kSpecialStageIdsIsBlueBand)(id, nullptr);
        }

        bool storyIs(const char* value) {
            if(!storyFetched) {
                story = StoryTypeOf(stage);
                storyFetched = true;
            }
            return StringEquals(story, value);
        }

        bool hasAssimilation() {
            void* bufs = Field(model, kModelBufListDetail);
            if(bufs == nullptr)
                return false;
            return Fn<bool(void*, const void*)>(kBufListDetailHasAssimilation)(bufs, nullptr);
        }

        bool hasBuf(const char* className) {
            void* list = Field(Field(model, kModelBufListDetail), kBufListDetailBufList);
            const int count = ListCount(list);
            for(int i = 0; i < count; i++) {
                void* buf = ListAt(list, i);
                if(buf == nullptr || !IsInstanceOf(buf, className))
                    continue;
                if(!Fn<bool(void*, const void*)>(kBattleUnitBufIsDestroyed)(buf, nullptr))
                    return true;
            }
            return false;
        }

        bool hasPassive(const char* className) {
            void* list = Field(Field(model, kModelPassiveDetail), kPassiveDetailPassiveList);
            const int count = ListCount(list);
            for(int i = 0; i < count; i++) {
                void* passive = ListAt(list, i);
                if(passive == nullptr || !IsInstanceOf(passive, className))
                    continue;
                if(!FieldBool(passive, kPassiveAbilityDestroyed))
                    return true;
            }
            return false;
        }

        bool isSephirah() { return FieldBool(unitData, kUnitDataIsSephirah); }
        bool bookValid() { return defaultBook() != nullptr; }

        bool defaultBookIs(int value) {
            void* book = defaultBook();
            if(book == nullptr)
                return false;
            return LorIdIs(Fn<void*(void*, const void*)>(kBookModelGetBookClassInfoId)(book, nullptr), value);
        }

        bool enemyUnitIs(int value) {
            if(unitData == nullptr)
                return false;
            return LorIdIs(Fn<void*(void*, const void*)>(kUnitDataModelGetEnemyUnitId)(unitData, nullptr), value);
        }
    };

    void ApplyDiceStyle(void* ui, int faction) {
        if(ui == nullptr)
            return;
        void* stage = CurrentStage();
        if(stage == nullptr) {
            if(s_LogBudget-- > 0)
                Logging.Log("[LoRSpeedDice] stage controller not captured yet");
            return;
        }
        GameEnv env(ui, faction, stage);
        sdrules::Decide(env, [ui](const char* name, const char* suffix, float r, float g, float b) {
            ChangeSpeedDiceColor(name, suffix, Color{ r, g, b, 1.0f }, ui);
        });
    }

    int FindCardIndex(void* cardAry, const int* ids, int idCount) {
        const int count = ListCount(cardAry);
        for(int i = 0; i < count; i++) {
            void* card = Field(ListAt(cardAry, i), kPlayingCardCard);
            if(card == nullptr)
                continue;
            void* cardId = Fn<void*(void*, const void*)>(kBattleDiceCardModelGetId)(card, nullptr);
            for(int k = 0; k < idCount; k++) {
                if(LorIdIs(cardId, ids[k]))
                    return i;
            }
        }
        return -1;
    }

    void StyleDiceAtCard(void* unit, const int* ids, int idCount, const char* name, const char* suffix, Color color) {
        void* cardAry = Field(Field(unit, kModelCardSlotDetail), kCardSlotDetailCardAry);
        const int index = FindCardIndex(cardAry, ids, idCount);
        if(index < 0)
            return;
        void* setter = Field(Field(unit, kModelView), kViewSpeedDiceSetter);
        if(setter == nullptr)
            return;
        void* dice = Fn<void*(void*, int, const void*)>(kSpeedDiceSetterGetSpeedDiceByIndex)(setter, index, nullptr);
        if(dice == nullptr)
            return;
        ChangeSpeedDiceColor(name, suffix, color, dice);
    }

    void ApplyCardPhaseStyles(void* stage) {
        void* story = StoryTypeOf(stage);
        if(story == nullptr)
            return;

        const bool shyao = StringEquals(story, "Shyao");
        const bool index2 = !shyao && StringEquals(story, "TheIndex2");
        const bool hana = !shyao && !index2 && StringEquals(story, "HanaAssociation");
        if(!shyao && !index2 && !hana)
            return;

        void* manager = Fn<void*(const void*)>(kBattleObjectManagerGetInstance)(nullptr);
        if(manager == nullptr)
            return;
        void* units = Fn<void*(void*, int, const void*)>(kBattleObjectManagerGetAliveListFaction)(manager, sdrules::kFactionEnemy, nullptr);
        const int unitCount = ListCount(units);

        if(shyao) {
            static const int ids[] = { 610012, 610013 };
            for(int i = 0; i < unitCount; i++) {
                void* unit = ListAt(units, i);
                if(unit == nullptr)
                    continue;
                if(!EnemyUnitIdIs(unit, 50038))
                    continue;
                if(Fn<bool(void*, const void*)>(kBattleUnitModelIsBreakLifeZero)(unit, nullptr))
                    return;
                StyleDiceAtCard(unit, ids, 2, "Xiao", "ReversedScale", Color{ 0.7f, 0.3f, 0.1f, 1.0f });
                return;
            }
            return;
        }

        for(int i = 0; i < unitCount; i++) {
            void* unit = ListAt(units, i);
            if(unit == nullptr)
                continue;
            if(index2) {
                static const int ids[] = { 605001, 605002, 605003 };
                StyleDiceAtCard(unit, ids, 3, "Index", "ProxiesSpecial", Color{ 0.3f, 0.5f, 0.8f, 1.0f });
            } else {
                static const int ids[] = { 701001 };
                StyleDiceAtCard(unit, ids, 1, "Hana", nullptr, Color{ 0.8f, 0.8f, 0.8f, 1.0f });
            }
        }
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
            ApplyDiceStyle(self, faction);
        }
    };

    HOOK_DEFINE_TRAMPOLINE(SpeedDiceUiChangeSpriteHook) {
        static void Callback(void* self, int face, int value, const void* method) {
            Orig(self, face, value, method);
            ApplyInfinite(self, value);
        }
    };

    HOOK_DEFINE_TRAMPOLINE(StageInitCommonHook) {
        static void Callback(void* self, void* stageInfo, bool isRebattle, const void* method) {
            CacheStage(self);
            Orig(self, stageInfo, isRebattle, method);
        }
    };

    HOOK_DEFINE_TRAMPOLINE(StageStartBattleHook) {
        static void Callback(void* self, const void* method) {
            CacheStage(self);
            Orig(self, method);
        }
    };

    HOOK_DEFINE_TRAMPOLINE(StageApplyLibrarianCardPhaseHook) {
        static void Callback(void* self, float deltaTime, const void* method) {
            Orig(self, deltaTime, method);
            CacheStage(self);
            ApplyCardPhaseStyles(self);
        }
    };
}

extern "C" void exl_main(void* x0, void* x1) {
    (void)x0; (void)x1;

    Logging.Log("[LoRSpeedDice] stage 6: decision tree");

    exl::hook::Initialize();
    StageInitCommonHook::InstallAtOffset(kStageControllerInitCommon);
    StageStartBattleHook::InstallAtOffset(kStageControllerStartBattle);
    StageApplyLibrarianCardPhaseHook::InstallAtOffset(kStageControllerApplyLibrarianCardPhase);
    SpeedDiceUiInitHook::InstallAtOffset(kSpeedDiceUiInit);
    SpeedDiceUiChangeSpriteHook::InstallAtOffset(kSpeedDiceUiChangeSprite);

    Logging.Log("[LoRSpeedDice] hooks installed");
}

extern "C" void exl_exception_entry() {
    while (true) {}
}
