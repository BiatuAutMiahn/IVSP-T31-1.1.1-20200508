cmd_/home1/toolchain/r2.3.3/obj/linux-2012.09-99-mips-linux-gnu-i686-pc-linux-gnu/tmp-install/include/linux/tc_ematch/.install := perl scripts/headers_install.pl /home1/toolchain/r2.3.3/obj/linux-2012.09-99-mips-linux-gnu-i686-pc-linux-gnu/include/linux/tc_ematch /home1/toolchain/r2.3.3/obj/linux-2012.09-99-mips-linux-gnu-i686-pc-linux-gnu/tmp-install/include/linux/tc_ematch mips tc_em_cmp.h tc_em_meta.h tc_em_nbyte.h tc_em_text.h; perl scripts/headers_install.pl /home1/toolchain/r2.3.3/obj/linux-2012.09-99-mips-linux-gnu-i686-pc-linux-gnu/include/linux/tc_ematch /home1/toolchain/r2.3.3/obj/linux-2012.09-99-mips-linux-gnu-i686-pc-linux-gnu/tmp-install/include/linux/tc_ematch mips ; perl scripts/headers_install.pl /home1/toolchain/r2.3.3/obj/linux-2012.09-99-mips-linux-gnu-i686-pc-linux-gnu/include/generated/linux/tc_ematch /home1/toolchain/r2.3.3/obj/linux-2012.09-99-mips-linux-gnu-i686-pc-linux-gnu/tmp-install/include/linux/tc_ematch mips ; for F in ; do echo "\#include <asm-generic/$$F>" > /home1/toolchain/r2.3.3/obj/linux-2012.09-99-mips-linux-gnu-i686-pc-linux-gnu/tmp-install/include/linux/tc_ematch/$$F; done; touch /home1/toolchain/r2.3.3/obj/linux-2012.09-99-mips-linux-gnu-i686-pc-linux-gnu/tmp-install/include/linux/tc_ematch/.install