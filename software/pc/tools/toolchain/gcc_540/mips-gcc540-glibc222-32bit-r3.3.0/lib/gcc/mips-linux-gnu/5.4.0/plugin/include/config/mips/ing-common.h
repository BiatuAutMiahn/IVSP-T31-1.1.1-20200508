/* Target macros for mips*-mti-linux* targets.
   Copyright (C) 2012-2015 Free Software Foundation, Inc.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3, or (at your option)
any later version.

GCC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING3.  If not see
<http://www.gnu.org/licenses/>.  */


/* Ingenic toolchains specail spec. */
#define MIPS_INGENIC_SELF_SPEC			\
  MIPS_INGENIC_ABS2008_SPEC,			\
  MIPS_INGENIC_NAN2008_SPEC

/* A spec condition that matches architectures should be targeted with
   o32 FP64 as default. */
#define MIPS_INGENIC_ARCH_O32_FP64_SPEC \
  "march=xburst2"

#define MIPS_INGENIC_O32_FP64_SPEC \
  "mfp64|mmsa|" MIPS_INGENIC_ARCH_O32_FP64_SPEC

/* Ingenic mfp64 supported arch, default support -mabs=2008 */
#define MIPS_INGENIC_ABS2008_SPEC					\
  "%{mabs*:;: %{mips32r2: %{" MIPS_INGENIC_O32_FP64_SPEC ": -mabs=2008}}}"

#undef MIPS_INGENIC_NAN2008_SPEC
/* Infer a -mnan=2008 setting from a -mips argument.  */
#define MIPS_INGENIC_NAN2008_SPEC \
  "%{mnan*:;" MIPS_INGENIC_O32_FP64_SPEC ":-mnan=2008}"

#define TARGET_ENDIAN_DEFAULT 0

/* TNODE: set ISA_LEVEL_SPEC %{march=xburst*: -mips32r2} in 
   macro MIPS_ISA_LEVEL_SPEC */
