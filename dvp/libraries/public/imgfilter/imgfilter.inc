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

/* .equ IMGFILTER_NOT_OPTIMIZED 0 */

/** PROLOG is a macro which saves your context on the stack. To reference the
 * next stack variable, add (sN-s0+2)*4, ie PROLOG r0,r12 will move the sp - (14*4)
 */
.macro PROLOG, s0, sN
    stmfd   sp!, {\s0-\sN, lr}
.endm

.macro EPILOG, s0, sN
    ldmfd   sp!, {\s0-\sN, pc}
.endm

/** This macro performs a float sqrt on the t0 register. t1-t3 are temps. */
.macro SQRTF    t0, t1, t2, t3
    vmov.F32    \t1, \t0
    vrsqrte.F32 \t0, \t0
    vmul.F32    \t2, \t0, \t1
    vrsqrts.F32 \t3, \t2, \t0
    vmul.F32    \t0, \t0, \t3
    vmul.F32    \t2, \t0, \t1
    vrsqrts.F32 \t3, \t2, \t0
    vmul.F32    \t0, \t0, \t3
    vrecpe.F32  \t1, \t0
    vrecps.F32  \t2, \t1, \t0
    vmul.F32    \t1, \t1, \t2
    vrecps.F32  \t2, \t1, \t0
    vmul.F32    \t0, \t1, \t2
.endm

/* For Cortex A8 */
/*.equ L1_LINE_SIZE, 64*/
/*.equ L2_LINE_SIZE, 64*/

/* For Cortex A9 */
.equ L1_LINE_SIZE, 32
.equ L2_LINE_SIZE, 32

/* 0 is "on", anything else is "off" */
.equ SUPPORT_NONMULTIPLE_LINES, 1
.equ SUPPORT_EDGES, 0
