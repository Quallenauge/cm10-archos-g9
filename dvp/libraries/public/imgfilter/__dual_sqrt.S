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

.text
.arch armv7-a
.fpu neon

.global __dual_sqrti_array
.global __dual_sqrtf_array

.include "imgfilter.inc"

__dual_sqrti_array:
    PROLOG      r0, r12
__dual_sqrti_outer_loop:
    vld1.32     {d0}, [r0]!
    vcvt.F32.S32 d0, d0
    vmov.F32    d1, d0
    SQRTF       d0, d1, d2, d3
    vcvt.S32.F32 d0, d0
    vst1.32     {d0}, [r2]!
    subs        r1, r1, #2
    bgt         __dual_sqrti_outer_loop
    EPILOG      r0, r12

__dual_sqrtf_array:
    PROLOG      r0, r12
__dual_sqrtf_outer_loop:
    vld1.32     {d0}, [r0]!
    SQRTF       d0, d1, d2, d3
    vst1.32     {d0}, [r2]!
    subs        r1, r1, #2
    bgt         __dual_sqrtf_outer_loop
    EPILOG      r0, r12

.end
