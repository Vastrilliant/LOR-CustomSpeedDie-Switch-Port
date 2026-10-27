#include "common.hpp"
#include "lib/util/sys/cur_proc_handle.hpp"
#include "lib/util/sys/modules.hpp"
#include "lib/util/sys/mem_layout.hpp"
#include <lib/log/logger_mgr.hpp>
#include <program/loggers.hpp>

namespace {

    __attribute__((section(".text.aliasprobe")))
    alignas(PAGE_SIZE)
    const volatile u8 s_ProbePage[PAGE_SIZE] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88 };

    void ProbeAlias(const char* name, uintptr_t ro, bool writeTest) {
        const uintptr_t alignedRo = ALIGN_DOWN(ro, PAGE_SIZE);
        const size_t size = PAGE_SIZE;

        void* rw = virtmemFindAslr(size, 0);
        if(rw == nullptr) {
            Logging.Log(EXL_LOG_PREFIX "[probe %s] no free aslr space", name);
            return;
        }
        auto* reserve = virtmemAddReservation(rw, size);
        auto handle = exl::util::proc_handle::Get();

        Result rc = svcMapProcessMemory(rw, handle, alignedRo, size);
        Logging.Log(EXL_LOG_PREFIX "[probe %s] map ro=%016lx rw=%016lx rc=%08x", name, (unsigned long) alignedRo, (unsigned long) rw, (unsigned) rc);
        if(R_FAILED(rc)) {
            virtmemRemoveReservation(reserve);
            return;
        }

        const u8* roBytes = reinterpret_cast<const u8*>(alignedRo);
        const u8* rwBytes = reinterpret_cast<const u8*>(rw);

        size_t first = 0;
        while(first < size && roBytes[first] == rwBytes[first])
            first++;
        size_t rwZero = 0;
        for(size_t i = 0; i < size; i++)
            if(rwBytes[i] == 0)
                rwZero++;

        Logging.Log(EXL_LOG_PREFIX "[probe %s] first diff=%lx (== 1000 means identical), zero bytes in rw=%lx", name, (unsigned long) first, (unsigned long) rwZero);
        Logging.Log(EXL_LOG_PREFIX "[probe %s] ro[0..7]=%02x %02x %02x %02x %02x %02x %02x %02x", name,
            roBytes[0], roBytes[1], roBytes[2], roBytes[3], roBytes[4], roBytes[5], roBytes[6], roBytes[7]);
        Logging.Log(EXL_LOG_PREFIX "[probe %s] rw[0..7]=%02x %02x %02x %02x %02x %02x %02x %02x", name,
            rwBytes[0], rwBytes[1], rwBytes[2], rwBytes[3], rwBytes[4], rwBytes[5], rwBytes[6], rwBytes[7]);

        if(writeTest) {
            volatile u8* w = reinterpret_cast<volatile u8*>(rw);
            const volatile u8* r = reinterpret_cast<const volatile u8*>(alignedRo);
            w[0x80] = 0xA5;
            Logging.Log(EXL_LOG_PREFIX "[probe %s] write-through: wrote A5 via rw, ro sees %02x, rw sees %02x", name, (unsigned) r[0x80], (unsigned) w[0x80]);
        }

        rc = svcUnmapProcessMemory(rw, handle, alignedRo, size);
        Logging.Log(EXL_LOG_PREFIX "[probe %s] unmap rc=%08x", name, (unsigned) rc);
        virtmemRemoveReservation(reserve);
    }
}

extern "C" void exl_main(void* x0, void* x1) {
    (void)x0; (void)x1;

    Logging.Log("[LoRSpeedDice] plugin loaded OK (stage 1: alias probe)");

    const auto& mainMod = exl::util::GetMainModuleInfo();
    ProbeAlias("own-text", reinterpret_cast<uintptr_t>(&s_ProbePage[0]), true);
    ProbeAlias("main-text", mainMod.m_Text.m_Start, false);
    ProbeAlias("main-rodata", mainMod.m_Rodata.m_Start, false);

    Logging.Log("[LoRSpeedDice] probe finished");
}

extern "C" void exl_exception_entry() {
    while (true) {}
}
