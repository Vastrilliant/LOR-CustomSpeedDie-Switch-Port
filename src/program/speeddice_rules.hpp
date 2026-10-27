#pragma once

#include <string.h>

namespace sdrules {

    constexpr int kFactionEnemy = 0;
    constexpr int kFactionPlayer = 1;
    constexpr int kStageTypeInvitation = 0;
    constexpr int kStageTypeCreature = 1;

    struct StoryStyle {
        const char* story;
        const char* name;
        float r, g, b;
    };

    struct ChapterStyle {
        int chapter;
        const char* story;
        const char* name;
    };

    constexpr StoryStyle kStoryStyles[] = {
        { "Rats", "Rats", 0.7f, 0.5f, 0.4f },
        { "YunsOffice", "YunOffice", 0.7f, 0.7f, 0.7f },
        { "TheBrotherhoodofIron", "BrotherhoodofIron", 0.7f, 0.5f, 0.4f },
        { "HookOffice", "HookOffice", 0.7f, 0.5f, 0.4f },
        { "PierresMeatPies", "PierreBistro", 0.7f, 0.5f, 0.4f },
        { "StreetlightOffice", "StreetlightOffice", 0.4f, 0.5f, 0.7f },
        { "ZweiAssociation", "ZweiAssociation", 0.6f, 0.6f, 0.8f },
        { "MolarOffice", "MolarOffice", 0.8f, 0.8f, 0.8f },
        { "TheStrayDogs", "StrayDogs", 0.6f, 0.4f, 0.4f },
        { "TheCarnival", "TheCarnival", 0.7f, 0.7f, 0.7f },
        { "FullStopOffice", "FullstopOffice", 0.6f, 0.3f, 0.3f },
        { "DawnOffice", "DawnOffice", 0.8f, 0.3f, 0.0f },
        { "GazeOffice", "GazeOffice", 0.7f, 0.7f, 0.7f },
        { "Tomerry", "LoveTown", 1.0f, 0.3f, 0.7f },
        { "KurokumoClan", "KurokumoClan", 0.7f, 0.7f, 0.7f },
        { "TheMusiciansofBremen", "Bremen", 0.6f, 0.1f, 0.9f },
        { "ShiAssociation", "ShiAssociation", 0.6f, 0.2f, 0.1f },
        { "Puppeteer", "Puppeteer", 0.6f, 0.2f, 0.1f },
        { "TheEightOClockCircus", "Circus", 0.8f, 0.2f, 0.1f },
        { "Sweepers", "Sweeper", 0.7f, 0.7f, 0.7f },
        { "TheIndex", "Index", 0.8f, 0.8f, 0.8f },
        { "WarpCrew", "WarpCrew", 0.4f, 0.4f, 1.0f },
        { "SmilingFaces", "SmilingFace", 0.7f, 0.7f, 0.7f },
        { "CryingChild", "CryingChildren", 0.5f, 0.3f, 0.2f },
        { "LiuAssociation2", "LiuAssociation", 0.7f, 0.3f, 0.1f },
        { "LiuAssociation1", "LiuAssociation", 0.7f, 0.3f, 0.1f },
        { "TheThumb", "TheThumb", 0.7f, 0.4f, 0.4f },
        { "CaneOffice", "CaneOffice", 0.8f, 0.8f, 0.8f },
        { "TheBlueReverberation", "BlueReverberation", 0.3f, 0.4f, 1.0f },
        { "BlackSilence", "BlackSilence", 0.8f, 0.8f, 0.8f },
        { "Final", "TheHead", 0.8f, 0.65f, 0.0f },
        { "Roland", "BlackSilence", 0.8f, 0.8f, 0.8f },
        { "Argalia", "BlueReverberation", 0.3f, 0.4f, 1.0f },
        { "HeadArbiter", "TheHead", 0.8f, 0.65f, 0.0f },
        { "GambleMod", "GambleMod", 0.9f, 0.4f, 0.0f },
        { "MagicalGirl", "MagicalGirl", 0.6f, 0.1f, 0.9f },
    };

    constexpr ChapterStyle kChapterStyles[] = {
        { 1, "Chapter1", "Canard" },
        { 2, "Chapter2", "UrbanMyth" },
        { 3, "Chapter3", "UrbanLegend" },
        { 4, "Chapter4", "UrbanPlague" },
        { 5, "Chapter5", "UrbanNightmare" },
        { 6, "Chapter6", "StaroftheCity" },
        { 7, "Chapter7", "Impurity" },
    };

    constexpr const char* kBlueBandFloors[] = {
        "Malkuth", "Yesod", "Hod", "Netzach", "Tiphereth", "Gebura", "Chesed", "Binah", "Hokma", "Keter",
    };

    constexpr int kBlueBandFirstId = 70001;
    constexpr int kBlueBandCount = 10;

    template<class E, class F>
    void DecidePlayer(E& e, F& emit) {
        if(e.hasAssimilation() && !e.hasPassive("PassiveAbility_250422")) {
            emit("Abnormalities", nullptr, 0.7f, 0.5f, 0.4f);
            return;
        }

        switch(e.floor()) {
            case 1:
                emit("Malkuth", nullptr, 0.7f, 0.5f, 0.2f);
                break;
            case 2:
                emit("Yesod", nullptr, 0.8f, 0.8f, 1.0f);
                break;
            case 3:
                emit("Hod", nullptr, 1.0f, 0.4f, 0.0f);
                break;
            case 4:
                emit("Netzach", nullptr, 0.3f, 1.0f, 0.3f);
                break;
            case 5:
                emit("Tiphereth", nullptr, 1.0f, 0.9f, 0.01f);
                break;
            case 6:
                if((e.hasPassive("PassiveAbility_250122") || e.hasPassive("PassiveAbility_250022")) && e.isSephirah())
                    emit("RedMist", e.hasBuf("BattleUnitBuf_RedMistEgo") ? "EGO" : nullptr, 0.9f, 0.4f, 0.4f);
                else
                    emit("Gebura", nullptr, 0.8f, 0.4f, 0.4f);
                break;
            case 7:
                emit("Chesed", nullptr, 0.3f, 0.5f, 1.0f);
                break;
            case 8:
                emit("Binah", nullptr, 0.8f, 0.65f, 0.0f);
                break;
            case 9:
                emit("Hokma", nullptr, 0.7f, 0.7f, 0.7f);
                break;
            case 10:
                if(e.hasPassive("PassiveAbility_10012") && e.isSephirah())
                    emit("BlackSilence", nullptr, 0.8f, 0.8f, 0.8f);
                else
                    emit("Keter", nullptr, 0.9f, 0.9f, 0.9f);
                break;
            default:
                break;
        }

        if(e.stageIdIs(60007)) {
            if(!e.bookValid())
                return;
            if(e.defaultBookIs(10))
                emit("BlackSilence", nullptr, 0.8f, 0.8f, 0.8f);
            else if(e.defaultBookIs(6))
                emit("RedMist", e.hasBuf("BattleUnitBuf_RedMistEgo") ? "EGO" : nullptr, 0.9f, 0.4f, 0.4f);
            else
                emit("Binah", nullptr, 0.8f, 0.65f, 0.0f);
            return;
        }

        if(e.stageIdIs(210005) || e.keterFinal()) {
            if(e.isSephirah())
                emit("Angela", nullptr, 0.1f, 0.7f, 0.8f);
            else
                emit("Keter", nullptr, 0.9f, 0.9f, 0.9f);
            return;
        }

        if(e.storyIs("Roland") || e.storyIs("Argalia")) {
            if(!e.bookValid())
                return;
            if(e.defaultBookIs(9100501))
                emit("Angela", nullptr, 0.1f, 0.7f, 0.8f);
            else
                emit("BlackSilence", nullptr, 0.8f, 0.8f, 0.8f);
        }
    }

    template<class E, class F>
    void DecideEnemyInvitation(E& e, F& emit) {
        const int stageId = e.stageIdValue();

        if(e.blueBand(stageId)) {
            const int k = stageId - kBlueBandFirstId;
            if(k >= 0 && k < kBlueBandCount)
                emit("BlueReverberation", kBlueBandFloors[k], 0.3f, 0.4f, 1.0f);
            return;
        }

        if(stageId >= 60004 && stageId <= 60006) {
            emit("BlueReverberation", nullptr, 0.3f, 0.4f, 1.0f);
            return;
        }

        for(const StoryStyle& s : kStoryStyles) {
            if(e.storyIs(s.story)) {
                emit(s.name, nullptr, s.r, s.g, s.b);
                return;
            }
        }

        if(e.storyIs("WedgeOffice")) {
            if(e.wave() == 1) {
                if(e.enemyUnitIs(30027))
                    emit("DawnOffice", nullptr, 0.8f, 0.3f, 0.0f);
                else
                    emit("WedgeOffice", nullptr, 0.6f, 0.5f, 0.01f);
            } else {
                emit("PhilipEGO", nullptr, 0.9f, 0.3f, 0.0f);
            }
            return;
        }

        if(e.storyIs("TheIndex2")) {
            if(e.enemyUnitIs(51001) || e.enemyUnitIs(51002))
                emit("Index", nullptr, 0.3f, 0.5f, 0.8f);
            else if(e.hasBuf("BattleUnitBuf_indexRelease"))
                emit("Index", "Proxies", 0.3f, 0.5f, 0.8f);
            else
                emit("Index", "ProxiesChained", 0.3f, 0.5f, 0.8f);
            return;
        }

        if(e.storyIs("TheRedMist")) {
            emit("RedMist", e.hasBuf("BattleUnitBuf_RedMistEgo") ? "EGO" : nullptr, 0.9f, 0.1f, 0.1f);
            return;
        }

        if(e.storyIs("RCorp1")) {
            if(e.wave() == 1)
                emit("RRR", "Reindeer", 0.9f, 0.4f, 0.0f);
            else if(e.wave() == 2)
                emit("RRR", "Rhino", 0.9f, 0.4f, 0.0f);
            else if(e.enemyUnitIs(50023))
                emit("RRR", nullptr, 0.9f, 0.4f, 0.0f);
            else
                emit("RRR", "Rabbit", 0.9f, 0.4f, 0.0f);
            return;
        }

        if(e.storyIs("RCorp2")) {
            if(e.wave() == 1)
                emit("RRR", "Reindeer", 0.9f, 0.4f, 0.0f);
            else if(e.wave() == 2)
                emit("RRR", "Rhino", 0.9f, 0.4f, 0.0f);
            else if(e.wave() == 3)
                emit("RRR", "Rabbit", 0.9f, 0.4f, 0.0f);
            else if(e.enemyUnitIs(50032))
                emit("RRR", "Rabbit", 0.9f, 0.4f, 0.0f);
            else if(e.enemyUnitIs(50033))
                emit("RRR", "Rhino", 0.9f, 0.4f, 0.0f);
            else if(e.enemyUnitIs(50034))
                emit("RRR", "Reindeer", 0.9f, 0.4f, 0.0f);
            else
                emit("RRR", nullptr, 0.9f, 0.4f, 0.0f);
            return;
        }

        if(e.storyIs("ThePurpleTear")) {
            if(e.hasBuf("BattleUnitBuf_purpleSlash"))
                emit("PurpleTear", "Slash", 0.6f, 0.1f, 0.9f);
            else if(e.hasBuf("BattleUnitBuf_purplePenetrate"))
                emit("PurpleTear", "Pierce", 0.6f, 0.1f, 0.9f);
            else if(e.hasBuf("BattleUnitBuf_purpleHit"))
                emit("PurpleTear", "Blunt", 0.6f, 0.1f, 0.9f);
            else if(e.hasBuf("BattleUnitBuf_purpleDefense"))
                emit("PurpleTear", "Guard", 0.6f, 0.1f, 0.9f);
            else
                emit("PurpleTear", nullptr, 0.6f, 0.1f, 0.9f);
            return;
        }

        if(e.storyIs("Shyao")) {
            if(e.wave() == 1)
                emit("LiuAssociation", nullptr, 0.7f, 0.3f, 0.1f);
            else if(e.wave() == 2)
                emit("Xiao", nullptr, 0.7f, 0.3f, 0.1f);
            return;
        }

        if(e.storyIs("Yan")) {
            if(e.enemyUnitIs(50051))
                emit("Yan", "Main", 0.1f, 0.7f, 0.9f);
            else
                emit("Yan", nullptr, 0.1f, 0.7f, 0.9f);
            return;
        }

        if(e.storyIs("HanaAssociation")) {
            if(e.hasBuf("BattleUnitBuf_hana1"))
                emit("Hana", "Geon", 0.8f, 0.8f, 0.8f);
            else if(e.hasBuf("BattleUnitBuf_hana2"))
                emit("Hana", "Gon", 0.8f, 0.8f, 0.8f);
            else if(e.hasBuf("BattleUnitBuf_hana3"))
                emit("Hana", "Gam", 0.8f, 0.8f, 0.8f);
            else if(e.hasBuf("BattleUnitBuf_hana4"))
                emit("Hana", "Ri", 0.8f, 0.8f, 0.8f);
            else
                emit("Hana", nullptr, 0.8f, 0.8f, 0.8f);
            return;
        }

        const int chapter = e.chapter();
        for(const ChapterStyle& c : kChapterStyles) {
            if(chapter == c.chapter && e.storyIs(c.story)) {
                emit(c.name, nullptr, 0.7f, 0.4f, 0.2f);
                return;
            }
        }
    }

    template<class E, class F>
    void DecideEnemy(E& e, F& emit) {
        if(e.stageIdIs(210005) || e.keterFinal()) {
            emit("LobotomyCorporation", nullptr, 1.0f, 0.4f, 0.4f);
            return;
        }

        const int stageType = e.stageType();
        if(stageType == kStageTypeCreature) {
            emit("Abnormalities", nullptr, 0.7f, 0.5f, 0.4f);
            return;
        }
        if(stageType != kStageTypeInvitation)
            return;

        DecideEnemyInvitation(e, emit);
    }

    template<class E, class F>
    void Decide(E& e, F&& emit) {
        const int faction = e.faction();
        if(faction == kFactionPlayer)
            DecidePlayer(e, emit);
        else if(faction == kFactionEnemy)
            DecideEnemy(e, emit);
    }

}
