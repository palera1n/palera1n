#define _PTHREAD_STRUCT_DIRECT_STACKADDR_OFFSET   -48
#define _PTHREAD_STRUCT_DIRECT_STACKBOTTOM_OFFSET -40

.align 4
_thread_chkstk_darwin:
        .globl ___chkstk_darwin
___chkstk_darwin: // %w9/x9 == alloca size
        stp     x10, x11, [sp, #-16]

        // validate that the frame pointer is on our stack (no alt stack)
        mrs     x10, TPIDRRO_EL0

        // (%sp - pthread_self()->stackaddr) > 0 ?
#if defined(__ARM64_ARCH_8_32__)
        ubfx    x9, x9, #0, #32
        ldur    w11, [x10, _PTHREAD_STRUCT_DIRECT_STACKADDR_OFFSET]
#else
        ldur    x11, [x10, _PTHREAD_STRUCT_DIRECT_STACKADDR_OFFSET]
#endif
        subs    x11, sp, x11
        b.hs    Lprobe

        // %sp <= pthread_self()->stackbottom ?
#if defined(__ARM64_ARCH_8_32__)
        ldur    w11, [x10, _PTHREAD_STRUCT_DIRECT_STACKBOTTOM_OFFSET]
#else
        ldur    x11, [x10, _PTHREAD_STRUCT_DIRECT_STACKBOTTOM_OFFSET]
#endif
        mov     x10, sp
        cmp     x10, x11
        b.ls    Lprobe

        // %sp - (uintptr_t)%x9 < pthread_self()->stackbottom ?
        subs    x10, x10, x9
        b.lo    Lcrash
        cmp     x10, x11
        b.lo    Lcrash

Lexit:
        ldp     x10, x11, [sp, #-16]
        ret

Lcrash:
        // POSIX mandates that stack overflow crashes with SIGSEGV
        // so load an address in the guard page and dereference it
        //
        // x11 contains pthread_self()->stackbottom already
        ldr     x11, [x11, #-8]
        // if main_thread caused stack growth with setrlimit()
        // fall into Lprobe and eventually cause SIGSEGV.

Lprobe:
        mov     x10, sp
        cmp     x9, #0x1000
        b.lo    Lend
Lloop:
        sub     x10, x10, #0x1000
        ldr     x11, [x10]
        sub     x9, x9, #0x1000
        cmp     x9, #0x1000
        b.hi    Lloop
Lend:
        sub     x10, x10, x9
        ldr     x11, [x10]
        b       Lexit
