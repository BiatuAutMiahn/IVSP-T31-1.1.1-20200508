cmd_/home1/toolchain/r2.3.3/obj/linux-2012.09-99-mips-linux-gnu-i686-pc-linux-gnu/tmp-install/include/linux/dvb/.install := perl scripts/headers_install.pl /home1/toolchain/r2.3.3/obj/linux-2012.09-99-mips-linux-gnu-i686-pc-linux-gnu/include/linux/dvb /home1/toolchain/r2.3.3/obj/linux-2012.09-99-mips-linux-gnu-i686-pc-linux-gnu/tmp-install/include/linux/dvb mips audio.h ca.h dmx.h frontend.h net.h osd.h version.h video.h; perl scripts/headers_install.pl /home1/toolchain/r2.3.3/obj/linux-2012.09-99-mips-linux-gnu-i686-pc-linux-gnu/include/linux/dvb /home1/toolchain/r2.3.3/obj/linux-2012.09-99-mips-linux-gnu-i686-pc-linux-gnu/tmp-install/include/linux/dvb mips ; perl scripts/headers_install.pl /home1/toolchain/r2.3.3/obj/linux-2012.09-99-mips-linux-gnu-i686-pc-linux-gnu/include/generated/linux/dvb /home1/toolchain/r2.3.3/obj/linux-2012.09-99-mips-linux-gnu-i686-pc-linux-gnu/tmp-install/include/linux/dvb mips ; for F in ; do echo "\#include <asm-generic/$$F>" > /home1/toolchain/r2.3.3/obj/linux-2012.09-99-mips-linux-gnu-i686-pc-linux-gnu/tmp-install/include/linux/dvb/$$F; done; touch /home1/toolchain/r2.3.3/obj/linux-2012.09-99-mips-linux-gnu-i686-pc-linux-gnu/tmp-install/include/linux/dvb/.install
