cmd_/home/toolchains/r2.3.3/obj/linux-2012.09-99-mips-linux-gnu-i686-pc-linux-gnu/tmp-install/include/linux/hsi/.install := perl scripts/headers_install.pl /home/toolchains/r2.3.3/obj/linux-2012.09-99-mips-linux-gnu-i686-pc-linux-gnu/include/linux/hsi /home/toolchains/r2.3.3/obj/linux-2012.09-99-mips-linux-gnu-i686-pc-linux-gnu/tmp-install/include/linux/hsi mips hsi_char.h; perl scripts/headers_install.pl /home/toolchains/r2.3.3/obj/linux-2012.09-99-mips-linux-gnu-i686-pc-linux-gnu/include/linux/hsi /home/toolchains/r2.3.3/obj/linux-2012.09-99-mips-linux-gnu-i686-pc-linux-gnu/tmp-install/include/linux/hsi mips ; perl scripts/headers_install.pl /home/toolchains/r2.3.3/obj/linux-2012.09-99-mips-linux-gnu-i686-pc-linux-gnu/include/generated/linux/hsi /home/toolchains/r2.3.3/obj/linux-2012.09-99-mips-linux-gnu-i686-pc-linux-gnu/tmp-install/include/linux/hsi mips ; for F in ; do echo "\#include <asm-generic/$$F>" > /home/toolchains/r2.3.3/obj/linux-2012.09-99-mips-linux-gnu-i686-pc-linux-gnu/tmp-install/include/linux/hsi/$$F; done; touch /home/toolchains/r2.3.3/obj/linux-2012.09-99-mips-linux-gnu-i686-pc-linux-gnu/tmp-install/include/linux/hsi/.install
