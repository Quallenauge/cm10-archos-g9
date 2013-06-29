/*
 *  Copyright (C) 2009-2012 Texas Instruments, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*! \brief PROLOG is a macro which saves your context on the stack. To reference the
 * next stack variable, add (sN-s0+2)*4, ie PROLOG r0,r12 will move the sp - (14*4)
 */
.macro PROLOG, s0, sN
    stmfd   sp!, {\s0-\sN, lr}
.endm

/*! \brief EPILOG is a macro which restores your context on the stack. This will
 * also modify the PC.
 */
.macro EPILOG, s0, sN
    ldmfd   sp!, {\s0-\sN, pc}
.endm

/**
 * This macro changes RGB888 pixels into RGB565 pixles in NEON
 */
.macro RGB888_TO_RGB565 db, dg, dr, qb, qg, qr, qp
    vshr.u8     \db, \db, #3
    vshr.u8     \dg, \dg, #2
    vshr.u8     \dr, \dr, #3
    vmovl.u8    \qb, \db
    vmovl.u8    \qg, \dg
    vmovl.u8    \qr, \dr
    vshl.u16    \qg, \qg, #5
    vshl.u16    \qr, \qr, #11
    vorr.u16    \qp, \qb, \qg
    vorr.u16    \qp, \qp, \qr
.endm

/* These are the 64x Constants for Y,Cb,Cr to R,G,B in BT601 Space. */
.macro FILL_YCBCR_TO_RGB_BT601 yk, gk2, gk1, rk, bk, coff, yoff
    vmov.i16    \yk,    #74     /* 1.164 * 64 */
    vmov.i16    \gk2,   #25     /* 0.391 * 64 */
    vmov.i16    \gk1,   #52     /* 0.813 * 64 */
    vmov.i16    \rk,    #102    /* 1.596 * 64 */
    vmov.i16    \bk,    #129    /* 2.018 * 64 */
    vmov.u8     \coff,  #128    /* 128 */
    vmov.u8     \yoff,  #16     /* 16 */
.endm

/* These are the 64x constants for R,G,B to Y,Cb,Cr in BT601 Space. */
.macro FILL_RGB_TO_YCBCR_BT601 r0, r1, r2, g0, g1, g2, b1, b2
    vmov.i16    \r0,    #16     /* R Coef */
    vmov.i16    \r1,    #32
    vmov.i16    \r2,    #6
    vmov.i16    \g0,    #9      /* (G Coef) */
    vmov.i16    \g1,    #19
    vmov.i16    \g2,    #28     /* (G & B Coef) */
    vmov.i16    \b1,    #24     /* (B Coef) */
    vmov.i16    \b2,    #5
.endm

/* These are the 64x Constants for Y,Cb,Cr to R,G,B in BT601 Space. */
.macro FILL_YCBCR_TO_RGB_BT601_Q13 yk, gk2, gk1, rk, bk, tmp
    vmov.i16    \yk,    #0x003F /* 1.164 * 8192 =  9535 = 0x253F */
    vmov.i16    \tmp,   #0x2500
    vorr.u16    \yk, \tmp, \yk
    vmov.i16    \gk2,   #0x0083 /* 0.391 * 8192 =  3203 = 0x0C83 */
    vmov.i16    \tmp,   #0x0C00
    vorr.u16    \gk2, \tmp, \gk2
    vmov.i16    \gk1,   #0x0004 /* 0.813 * 8192 =  6660 = 0x1A04 */
    vmov.i16    \tmp,   #0x1A00
    vorr.u16    \gk1, \tmp, \gk1
    vmov.i16    \rk,    #0x0012 /* 1.596 * 8192 = 13074 = 0x3312 */
    vmov.i16    \tmp,   #0x3300
    vorr.u16    \rk, \tmp, \rk
    vmov.i16    \bk,    #0x0093 /* 2.018 * 8192 = 16531 = 0x4093 */
    vmov.i16    \tmp,   #0x4000
    vorr.u16    \bk, \tmp, \bk
.endm

/* This takes a dN register and duplicates the bottom bytes */
/* d0 = [1,2,3,4,0,0,0,0] */
/* d0 = [1,1,2,2,3,3,4,4] */
.macro DUPLICATE_EACH_BYTE reg, tmp
    vmov.i8     \tmp,\reg
    vzip.i8     \reg,\tmp
.endm

/*! \brief This loads 16 macropixels (32 total pixels) by 2 rows and creates a 16x1 average.
 * \d0-\d7 row 1
 * \d8-\d15 row 2
 * The loads should be done in an interleaved order.
 * ie: d0 = u0, d1 = y0, d2 = v0, d3 = y1
 * Stores must be done in 2.16 order over \d0, \d1
 * Stores must be done in 2.16 order over \d4, \d5
 */
.macro uyvy_32x2_average d0, d1, d2, d3, d4, d5, d6, d7, d8, d9, d10, d11, d12, d13, d14, d15
    vhadd.u8    \d0, \d0, \d8  /* Average U's */
    vhadd.u8    \d1, \d1, \d9  /* Average Y0's */
    vhadd.u8    \d2, \d2, \d10 /* Average V's */
    vhadd.u8    \d3, \d3, \d11 /* Average Y1's */
    vhadd.u8    \d4, \d4, \d12 /* Average U's */
    vhadd.u8    \d5, \d5, \d13 /* Average Y0's */
    vhadd.u8    \d6, \d6, \d14 /* Average V's */
    vhadd.u8    \d7, \d7, \d15 /* Average Y1's */
    /* Now the two lines are averaged. */

    /* Average the adjacent pixels. */
    vtrn.8      \d0, \d2       /* \d0 = [u0, v0, u2, v2, u4, v4, u6, v6] */
                               /* \d2 = [u1, v1, u3, v3, u5, v5, u7, v7] */
    vhadd.u8    \d0, \d0, \d2  /* Average U/V's */
    vhadd.u8    \d1, \d1, \d3  /* Average Y's */
    vtrn.8      \d0, \d1

    vtrn.8      \d4, \d6       /* same for other half */
    vhadd.u8    \d4, \d4, \d6  /* Average U/V's */
    vhadd.u8    \d5, \d5, \d7  /* Average Y's */
    vtrn.8      \d4, \d5
.endm

/*.equ L1_LINE_SIZE, 64*/
/*.equ L2_LINE_SIZE, 64*/

.equ L1_LINE_SIZE, 32
.equ L2_LINE_SIZE, 32

/* Preloads 256 bytes of contigous data addresses into the L2 cache */
.macro preload_L2_1x256bytes address
    pld     [\address, #(L2_LINE_SIZE * 0)]
    pld     [\address, #(L2_LINE_SIZE * 1)]
    pld     [\address, #(L2_LINE_SIZE * 2)]
    pld     [\address, #(L2_LINE_SIZE * 3)]
.endm


/* Preloads 4 lines of 64 bytes of data into the L2 cache */
.macro preload_L2_4x64bytes address, offset, tmp
    pld     [\address]
    pld     [\address, \offset]
    mov     \tmp,      \offset, lsl #1
    pld     [\address, \tmp]
    add     \tmp,      \tmp,    \offset
    pld     [\address, \tmp]
.endm


/* Preloads 4 lines of 64 bytes of data into the L2 cache, aligned to 64 bytes */
/* @note address will be modified */
.macro preload_L2_4x64bytes_aligned address, offset, tmp
    mvn     \tmp,      #0x2F
    and     \address,  \address, \tmp
    pld     [\address]
    pld     [\address, \offset]
    mov     \tmp,      \offset, lsl #1
    pld     [\address, \tmp]
    add     \tmp,      \tmp,    \offset
    pld     [\address, \tmp]
.endm


/* VREV64 Reverses all bytes in a 64 bit value (dN) */
.macro REVERSE s0, s1
    vrev64.32 \s0, \s1
    vrev32.16 \s0, \s1
    vrev16.8  \s0, \s1
.endm
