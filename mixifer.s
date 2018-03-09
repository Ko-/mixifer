/* Optimized ARMv7-M assembly implementation of Mixifer. The CC0 license
 * applies to this code.
 *
 * Nov 2017.
*/
.syntax unified
.thumb

.macro gammapart s t
    orn \t, \s, \s, ror #16
    eor \t, \s, \t, ror #8
    and \t, \t, \s, ror #16
    eor \s, \t, \s, ror #24
.endm

.macro gamma s0 s1 s2 s3 s4 s5 s6 s7 t
    gammapart \s0, \t
    gammapart \s1, \t
    gammapart \s2, \t
    gammapart \s3, \t
    gammapart \s4, \t
    gammapart \s5, \t
    gammapart \s6, \t
    gammapart \s7, \t
.endm

.macro thetarho s0 s1 s2 s3 s4 s5 s6 s7 t0 t1 t2 t3 c
    //theta, calculate column parity
    eor \t0, \s0, \s2
    eor \t0, \s4
    eor \t0, \s6

    eor \t1, \s1, \s3
    eor \t1, \s5
    eor \t1, \s7

    //theta, calculate effect right side
    and \t2, \c, \t0, lsl #7
    and \t3, \t0, #0xfefefefe
    orr \t2, \t2, \t3, lsr #1

    eor \t2, \t1

    and \t3, \c, \t2, lsl #7
    and \t2, #0xfefefefe
    orr \t3, \t3, \t2, lsr #1

    eor \t3, \t0

    //theta, add effect right side
    eor \s1, \t3
    eor \s3, \t3
    eor \s5, \t3
    eor \s7, \t3

    //theta, calculate effect left side
    eor \t2, \c, \c, lsr #1 //0xc0c0c0c0
    and \t2, \t2, \t1, lsl #6
    and \t3, \t1, #0xfcfcfcfc
    orr \t2, \t2, \t3, lsr #2

    eor \t2, \t0
    eor \t2, \t1

    and \t3, \c, \t2, lsl #7
    and \t2, #0xfefefefe
    orr \t3, \t3, \t2, lsr #1

    //theta, add effect left side
    eor \t0, \s0, \t3
    eor \s2, \t3
    eor \s4, \t3
    eor \s6, \t3

    //rho
    ror \s6, #7
    ror \s7, #7
    ror \s0, \s1, #2
    ror \s1, \t0, #1
    ror \s2, #5
    ror \s3, #5
.endm

.macro round s0 s1 s2 s3 s4 s5 s6 s7 t0 t1 t2 t3 c rc i
    gamma \s0 \s1 \s2 \s3 \s4 \s5 \s6 \s7 \t0
    thetarho \s0 \s1 \s2 \s3 \s4 \s5 \s6 \s7 \t0 \t1 \t2 \t3 \c
    eor \s6, \s6, \rc, lsr #\i
.endm

.align 2
//void mixifer_permute(uint8_t *data)
.global mixifer_permute
.type   mixifer_permute,%function
mixifer_permute:

    //function prologue, preserve registers
    push {r0,r4-r12,r14}

    //load input
    ldmia r0, {r2-r9}

    //set constants
    mov r12, #0x80808080 //c
    movw r14, #0x5763 //rc
    movt r14, #0xF348 //rc

    //and go!
    round r2 r3 r4 r5 r6 r7 r8 r9 r0 r1 r10 r11 r12 r14 0
    round r8 r9 r2 r3 r4 r5 r6 r7 r0 r1 r10 r11 r12 r14 1
    round r6 r7 r8 r9 r2 r3 r4 r5 r0 r1 r10 r11 r12 r14 2
    round r4 r5 r6 r7 r8 r9 r2 r3 r0 r1 r10 r11 r12 r14 3
    round r2 r3 r4 r5 r6 r7 r8 r9 r0 r1 r10 r11 r12 r14 4
    round r8 r9 r2 r3 r4 r5 r6 r7 r0 r1 r10 r11 r12 r14 5
    round r6 r7 r8 r9 r2 r3 r4 r5 r0 r1 r10 r11 r12 r14 6
    round r4 r5 r6 r7 r8 r9 r2 r3 r0 r1 r10 r11 r12 r14 7
    round r2 r3 r4 r5 r6 r7 r8 r9 r0 r1 r10 r11 r12 r14 8
    round r8 r9 r2 r3 r4 r5 r6 r7 r0 r1 r10 r11 r12 r14 9
    round r6 r7 r8 r9 r2 r3 r4 r5 r0 r1 r10 r11 r12 r14 10
    round r4 r5 r6 r7 r8 r9 r2 r3 r0 r1 r10 r11 r12 r14 11
    round r2 r3 r4 r5 r6 r7 r8 r9 r0 r1 r10 r11 r12 r14 12
    round r8 r9 r2 r3 r4 r5 r6 r7 r0 r1 r10 r11 r12 r14 13
    round r6 r7 r8 r9 r2 r3 r4 r5 r0 r1 r10 r11 r12 r14 14
    round r4 r5 r6 r7 r8 r9 r2 r3 r0 r1 r10 r11 r12 r14 15

    //write output
    pop.w {r0}
    stmia r0, {r2-r9}
/*
    str r8, [r0, #0]
    str r9, [r0, #4]
    str r2, [r0, #8]
    str r3, [r0, #12]
    str r4, [r0, #16]
    str r5, [r0, #20]
    str r6, [r0, #24]
    str r7, [r0, #28]
*/

    //function epilogue, restore state
    pop {r4-r12,r14}
    bx lr

.macro invgammapart s t
    bic \t, \s, \s, ror #16
    eor \t, \s, \t, ror #24
    bic \t, \t, \s, ror #16
    eor \s, \t, \s, ror #8
.endm

.macro invgamma s0 s1 s2 s3 s4 s5 s6 s7 t
    invgammapart \s0, \t
    invgammapart \s1, \t
    invgammapart \s2, \t
    invgammapart \s3, \t
    invgammapart \s4, \t
    invgammapart \s5, \t
    invgammapart \s6, \t
    invgammapart \s7, \t
.endm

.macro invrhotheta s0 s1 s2 s3 s4 s5 s6 s7 t0 t1 t2 t3 c
    //invrho
    ror \t0, \s2, #30
    ror \s2, \s3, #31

    //theta, calculate column parity
    eor \s3, \s2, \s0, ror #25
    eor \s3, \s3, \s4, ror #27
    eor \s3, \s6

    eor \t1, \t0, \s1, ror #25
    eor \t1, \t1, \s5, ror #27
    eor \t1, \s7

    //theta, calculate effect right side
    and \t2, \c, \s3, lsl #7
    and \t3, \s3, #0xfefefefe
    orr \t2, \t2, \t3, lsr #1

    eor \t2, \t1

    and \t3, \c, \t2, lsl #7
    and \t2, #0xfefefefe
    orr \t3, \t3, \t2, lsr #1

    eor \t3, \s3

    //theta, add effect right side
    eor \s1, \t3, \s1, ror #25
    eor \t0, \t3
    eor \s5, \t3, \s5, ror #27
    eor \s7, \t3

    //theta, calculate effect left side
    eor \t2, \c, \c, lsr #1 //0xc0c0c0c0
    and \t2, \t2, \t1, lsl #6
    and \t3, \t1, #0xfcfcfcfc
    orr \t2, \t2, \t3, lsr #2

    eor \t2, \s3
    eor \t2, \t1

    and \t3, \c, \t2, lsl #7
    and \t2, #0xfefefefe
    orr \t3, \t3, \t2, lsr #1

    //theta, add effect left side
    eor \s0, \t3, \s0, ror #25
    eor \s2, \t3
    eor \s4, \t3, \s4, ror #27
    eor \s6, \t3

    mov \s3, \t0
.endm

.macro invround s0 s1 s2 s3 s4 s5 s6 s7 t0 t1 t2 t3 c rc i
    eor \s0, \s0, \rc, lsr #\i
    invrhotheta \s0 \s1 \s2 \s3 \s4 \s5 \s6 \s7 \t0 \t1 \t2 \t3 \c
    invgamma \s0 \s1 \s2 \s3 \s4 \s5 \s6 \s7 \t0
.endm

.align 2
//void mixifer_invpermute(uint8_t *data)
.global mixifer_invpermute
.type   mixifer_invpermute,%function
mixifer_invpermute:

    //function prologue, preserve registers
    push {r0,r4-r12,r14}

    //load input
    ldmia r0, {r2-r9}

    //set constants
    mov r12, #0x80808080 //c
    movw r14, #0x5763 //rc
    movt r14, #0xF348 //rc

    //and go!
    invround r2 r3 r4 r5 r6 r7 r8 r9 r0 r1 r10 r11 r12 r14 15
    invround r4 r5 r6 r7 r8 r9 r2 r3 r0 r1 r10 r11 r12 r14 14
    invround r6 r7 r8 r9 r2 r3 r4 r5 r0 r1 r10 r11 r12 r14 13
    invround r8 r9 r2 r3 r4 r5 r6 r7 r0 r1 r10 r11 r12 r14 12
    invround r2 r3 r4 r5 r6 r7 r8 r9 r0 r1 r10 r11 r12 r14 11
    invround r4 r5 r6 r7 r8 r9 r2 r3 r0 r1 r10 r11 r12 r14 10
    invround r6 r7 r8 r9 r2 r3 r4 r5 r0 r1 r10 r11 r12 r14 9
    invround r8 r9 r2 r3 r4 r5 r6 r7 r0 r1 r10 r11 r12 r14 8
    invround r2 r3 r4 r5 r6 r7 r8 r9 r0 r1 r10 r11 r12 r14 7
    invround r4 r5 r6 r7 r8 r9 r2 r3 r0 r1 r10 r11 r12 r14 6
    invround r6 r7 r8 r9 r2 r3 r4 r5 r0 r1 r10 r11 r12 r14 5
    invround r8 r9 r2 r3 r4 r5 r6 r7 r0 r1 r10 r11 r12 r14 4
    invround r2 r3 r4 r5 r6 r7 r8 r9 r0 r1 r10 r11 r12 r14 3
    invround r4 r5 r6 r7 r8 r9 r2 r3 r0 r1 r10 r11 r12 r14 2
    invround r6 r7 r8 r9 r2 r3 r4 r5 r0 r1 r10 r11 r12 r14 1
    invround r8 r9 r2 r3 r4 r5 r6 r7 r0 r1 r10 r11 r12 r14 0

    //write output
    pop.w {r0}
    stmia r0, {r2-r9}
/*
    str r4, [r0, #0]
    str r5, [r0, #4]
    str r6, [r0, #8]
    str r7, [r0, #12]
    str r8, [r0, #16]
    str r9, [r0, #20]
    str r2, [r0, #24]
    str r3, [r0, #28]
*/

    //function epilogue, restore state
    pop {r4-r12,r14}
    bx lr
