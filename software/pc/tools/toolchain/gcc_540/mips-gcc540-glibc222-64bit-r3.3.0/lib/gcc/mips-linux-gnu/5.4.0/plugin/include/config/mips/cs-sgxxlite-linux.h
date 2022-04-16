/* MIPS SourceryG++ GNU/Linux Configuration.
   Copyright (C) 2008, 2011
   Free Software Foundation, Inc.

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

#undef SUBTARGET_CC1_SPEC
#define SUBTARGET_CC1_SPEC \
  "%{profile:-p}" \
  "%{muclibc:%{fstack-protector:%e-fstack-protector cannot be used with -muclibc}}"

/* Override linux64.h to default to O32.  */
#undef DRIVER_SELF_SPECS
#define DRIVER_SELF_SPECS \
  /* Make sure a -mips option is present.  This helps us to pick	\
     the right multilib, and also makes the later specs easier		\
     to write.  */							\
  MIPS_ISA_LEVEL_SPEC,							\
									\
  /* Infer the default float setting from -march.  */			\
  MIPS_ARCH_FLOAT_SPEC,							\
									\
  /* Infer the -msynci setting from -march if not explicitly set.  */	\
  MIPS_ISA_SYNCI_SPEC,							\
									\
  /* If no ABI option is specified, infer one from the ISA level	\
     or -mgp setting.  */						\
  "%{!mabi=*: %{" MIPS_32BIT_OPTION_SPEC ": -mabi=32;: -mabi=n32}}",	\
									\
  /* If no FP ABI option is specified, infer one from the		\
     ABI/ISA level unless there is a conflicting option.  */		\
  "%{!msoft-float: %{!msingle-float: %{!mfp*: %{mabi=32: %{"		\
  MIPS_INGENIC_ARCH_O32_FP64_SPEC ": -mfp64}}}}}",			\
									\
  /* Ingenic specail spec. */						\
  MIPS_INGENIC_SELF_SPEC,						\
									\
  /* Base SPECs.  */							\
  BASE_DRIVER_SELF_SPECS						\
									\
  /* Use the standard linux specs for everything else.  */		\
  LINUX_DRIVER_SELF_SPECS


/* We do not need to provide an explicit big-endian multilib.  */
#undef MULTILIB_DEFAULTS
#define MULTILIB_DEFAULTS \
  { "EB", "mabi=32" }

/* The various C libraries each have their own subdirectory.  */
/* For mfp64  sysroot default with -mnan=2008 */
#undef SYSROOT_SUFFIX_SPEC
#define SYSROOT_SUFFIX_SPEC				\
  "%{muclibc:/uclibc}"					\
  "%{mmicromips:/micromips;mips16:/mips16}"		\
  "%{mfp64:/mfp64;mmsa:/mfp64}"				\
  "%{msoft-float:/soft-float}"				\
  "%{!mmsa:%{!mfp64:%{!mips32r6:%{!mips64r6:%{mnan=2008:/nan2008}}}}}"

#undef SYSROOT_HEADERS_SUFFIX_SPEC
#define SYSROOT_HEADERS_SUFFIX_SPEC \
  "%{muclibc:/uclibc}"

#undef STARTFILE_PREFIX_SPEC
#define STARTFILE_PREFIX_SPEC				\
  "%{mabi=32: /usr/local/lib/ /lib/ /usr/lib/} 		\
   %{mabi=64: /usr/local/lib64/ /lib64/ /usr/lib64/}"

