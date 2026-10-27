#include "common.hpp"
#include "lib/util/sys/modules.hpp"
#include "lib/hook/trampoline.hpp"
#include "lib/patch/code_patcher.hpp"
#include <lib/log/logger_mgr.hpp>
#include <program/loggers.hpp>

namespace {
    constexpr uintptr_t kSpeedDiceUiInit = 0x233BF20;
    constexpr int kMaxLoggedCalls = 24;

    int s_CallCount = 0;

    HOOK_DEFINE_TRAMPOLINE(SpeedDiceUiInitHook) {
        static void Callback(void* self, int index, int diceMin, int diceFace, int faction, const void* method) {
            if(s_CallCount < kMaxLoggedCalls) {
                Logging.Log("[LoRSpeedDice] SpeedDiceUI.Init #%d self=%016lx index=%d min=%d face=%d faction=%d",
                    s_CallCount, (unsigned long) self, index, diceMin, diceFace, faction);
            }
            s_CallCount++;
            Orig(self, index, diceMin, diceFace, faction, method);
        }
    };

    bool RewriteSameWord(uintptr_t offset) {
        const auto& mainMod = exl::util::GetMainModuleInfo();
        const uintptr_t addr = mainMod.m_Total.m_Start + offset;
        const u32 before = *reinterpret_cast<const volatile u32*>(addr);

        exl::patch::CodePatcher patcher(offset);
        patcher.Write<u32>(before);

        const u32 after = *reinterpret_cast<const volatile u32*>(addr);
        Logging.Log("[LoRSpeedDice] self-write at +%lx: before=%08x after=%08x -> %s",
            (unsigned long) offset, (unsigned) before, (unsigned) after, before == after ? "OK" : "MISMATCH");
        return before == after;
    }
}

extern "C" void exl_main(void* x0, void* x1) {
    (void)x0; (void)x1;

    Logging.Log("[LoRSpeedDice] stage 2: plugin loaded");

    const auto& mainMod = exl::util::GetMainModuleInfo();
    const uintptr_t initAddr = mainMod.m_Total.m_Start + kSpeedDiceUiInit;
    Logging.Log("[LoRSpeedDice] main base=%016lx Init addr=%016lx first word=%08x",
        (unsigned long) mainMod.m_Total.m_Start, (unsigned long) initAddr,
        (unsigned) *reinterpret_cast<const volatile u32*>(initAddr));

    if(!RewriteSameWord(kSpeedDiceUiInit)) {
        Logging.Log("[LoRSpeedDice] direct write did not stick, aborting before hook");
        return;
    }

    exl::hook::Initialize();
    SpeedDiceUiInitHook::InstallAtOffset(kSpeedDiceUiInit);
    Logging.Log("[LoRSpeedDice] hook installed on SpeedDiceUI.Init");
}

extern "C" void exl_exception_entry() {
    while (true) {}
}
