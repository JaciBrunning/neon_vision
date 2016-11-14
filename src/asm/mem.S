.global memcpy_threshold_asm

memcpy_threshold_asm:
    push {fp}
    add fp, sp, #0  // Push new Stack Frame

    // Arg 0: r0: Destination Address
    // Arg 1: r1: Source Address
    // Arg 2: r2: Count
    // Arg 3: r3: Threshold Minimum

    lsr r2, #4       // Shift count right by 4 bits (divide by 16, 16 bytes = 128 bits)
    vdup.8 q3, r3    // Write threshold to q3
    _loop:
        // Load into Q0
        vld1.64 d0, [r1]!
        vld1.64 d1, [r1]!

        vcgt.u8 q1, q0, q3  // Will set all to 1 if Greater
        vand.u8 q2, q1, q0  // AND the new set, result is in q2

        // Load out of Q2
        vst1.64 d4, [r0]!
        vst1.64 d5, [r0]!

        sub r2, r2, #1
        cmp r2, #0
        bgt _loop

    sub sp, fp, #0  // Pop our Stack Frame
    pop {fp}
    bx lr