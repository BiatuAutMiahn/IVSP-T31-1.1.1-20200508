cmd_/home/toolchains/r2.3.3/obj/linux-2012.09-99-mips-linux-gnu-i686-pc-linux-gnu/tmp-install/include/linux/netfilter_bridge/.install := perl scripts/headers_install.pl /home/toolchains/r2.3.3/obj/linux-2012.09-99-mips-linux-gnu-i686-pc-linux-gnu/include/linux/netfilter_bridge /home/toolchains/r2.3.3/obj/linux-2012.09-99-mips-linux-gnu-i686-pc-linux-gnu/tmp-install/include/linux/netfilter_bridge mips ebt_802_3.h ebt_among.h ebt_arp.h ebt_arpreply.h ebt_ip.h ebt_ip6.h ebt_limit.h ebt_log.h ebt_mark_m.h ebt_mark_t.h ebt_nat.h ebt_nflog.h ebt_pkttype.h ebt_redirect.h ebt_stp.h ebt_ulog.h ebt_vlan.h ebtables.h; perl scripts/headers_install.pl /home/toolchains/r2.3.3/obj/linux-2012.09-99-mips-linux-gnu-i686-pc-linux-gnu/include/linux/netfilter_bridge /home/toolchains/r2.3.3/obj/linux-2012.09-99-mips-linux-gnu-i686-pc-linux-gnu/tmp-install/include/linux/netfilter_bridge mips ; perl scripts/headers_install.pl /home/toolchains/r2.3.3/obj/linux-2012.09-99-mips-linux-gnu-i686-pc-linux-gnu/include/generated/linux/netfilter_bridge /home/toolchains/r2.3.3/obj/linux-2012.09-99-mips-linux-gnu-i686-pc-linux-gnu/tmp-install/include/linux/netfilter_bridge mips ; for F in ; do echo "\#include <asm-generic/$$F>" > /home/toolchains/r2.3.3/obj/linux-2012.09-99-mips-linux-gnu-i686-pc-linux-gnu/tmp-install/include/linux/netfilter_bridge/$$F; done; touch /home/toolchains/r2.3.3/obj/linux-2012.09-99-mips-linux-gnu-i686-pc-linux-gnu/tmp-install/include/linux/netfilter_bridge/.install