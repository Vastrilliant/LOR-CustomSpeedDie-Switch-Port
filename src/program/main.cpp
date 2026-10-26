// Stage 0: testing
#include <cstddef>

static void dbg(const char* s) {
    size_t n = 0;
    while (s[n]) ++n;
    register const char* x0 asm("x0") = s;
    register size_t x1 asm("x1") = n;
    asm volatile("svc #0x27" : "+r"(x0) : "r"(x1) : "memory");
}

extern "C" void exl_main(void* x0, void* x1) {
    (void)x0; (void)x1;
    dbg("[LoRSpeedDice] plugin loaded OK (stage 0)");
}

extern "C" void exl_exception_entry() {
    while (true) {}
}
