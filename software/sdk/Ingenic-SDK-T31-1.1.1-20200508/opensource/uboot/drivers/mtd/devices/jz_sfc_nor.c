#include <common.h>
#include <malloc.h>
#include <ubi_uboot.h>
#include <asm/io.h>

#include <asm/arch/sfc_params.h>
#include <asm/arch/sfc.h>
#include <asm/arch/spi_nor.h>
#include "jz_sfc_common.h"


struct sfc_flash *flash = NULL;
struct burner_params params;

struct mini_spi_nor_info mini_params;

#ifdef CONFIG_BURNER
unsigned int burn_mode = 0;
#endif

int sfc_nor_reset()
{
	struct sfc_transfer transfer[1];
	struct sfc_message message;
	struct cmd_info cmd[1];
	int ret;

	sfc_message_init(&message);
	memset(&transfer, 0, sizeof(transfer));
	memset(&cmd, 0, sizeof(cmd));

	cmd[0].cmd = SPINOR_OP_RSTEN;
	cmd[0].dataen = DISABLE;
	transfer[0].cmd_info = &cmd[0];
	transfer[0].sfc_mode = 0;
	sfc_message_add_tail(&transfer[0], &message);
	ret = sfc_sync(flash->sfc, &message);
	if(ret) {
		printf("sfc_sync error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		ret=-EIO;
	}

	cmd[0].cmd = SPINOR_OP_RST;
	cmd[0].dataen = DISABLE;
	transfer[0].cmd_info = &cmd[0];
	transfer[0].sfc_mode = 0;
	sfc_message_add_tail(&transfer[0], &message);

	ret = sfc_sync(flash->sfc, &message);
	if(ret) {
		printf("sfc_sync error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		ret=-EIO;
	}

	udelay(100);
	return ret;
}

unsigned int sfc_nor_read_id(unsigned int command, unsigned int addr, int addr_len, unsigned int len, int dummy_byte)
{

	struct sfc_transfer transfer;
	struct sfc_message message;
	struct cmd_info cmd;
	int ret;
	unsigned int chip_id = 0;

	sfc_message_init(&message);
	memset(&transfer, 0, sizeof(transfer));
	memset(&cmd, 0, sizeof(cmd));

	cmd.cmd = command;
	cmd.dataen = ENABLE;

	transfer.addr_len = addr_len;
	transfer.data_dummy_bits = dummy_byte;
	transfer.addr = addr;
	transfer.len = len;
	transfer.data =(unsigned char *)&chip_id;
	transfer.ops_mode = CPU_OPS;
	transfer.sfc_mode = TM_STD_SPI;
	transfer.direction = GLB_TRAN_DIR_READ;
	transfer.cmd_info = &cmd;
	sfc_message_add_tail(&transfer, &message);

	ret = sfc_sync(flash->sfc, &message);
	if(ret) {
		printf("sfc_sync error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		ret=-EIO;
	}

	chip_id =  chip_id & 0x00ffffff;
	chip_id = ((chip_id & 0xff) << 16) | (((chip_id >> 8) & 0xff) << 8) | ((chip_id >> 16) & 0xff);
	return chip_id;
}

unsigned int get_norflash_id()
{
	unsigned int id_len = 3;
	unsigned int addr = 0;
	unsigned int addr_len = 0;
	unsigned int dummy = 0;

	return sfc_nor_read_id(SPINOR_OP_RDID, addr, addr_len, id_len, dummy);
}

static int get_current_operate_cmd()
{

	struct spi_nor_info *spi_nor_info;
	struct spi_nor_cmd_info *read_standard;
	struct spi_nor_cmd_info *read_quad;
	struct spi_nor_cmd_info *write_standard;
	struct spi_nor_cmd_info *write_quad;
	struct spi_nor_cmd_info *cur_r_cmd;
	struct spi_nor_cmd_info *cur_w_cmd;

	spi_nor_info = flash->g_nor_info;
	read_standard = &spi_nor_info->read_standard;
	read_quad = &spi_nor_info->read_quad;
	write_standard = &spi_nor_info->write_standard;
	write_quad = &spi_nor_info->write_quad;

	if (flash->quad_succeed){
		cur_r_cmd = read_quad;
		cur_w_cmd = write_quad;
	} else {
		cur_r_cmd = read_standard;
		cur_w_cmd = write_standard;
	}

	flash->cur_r_cmd = cur_r_cmd;
	flash->cur_w_cmd = cur_w_cmd;

	return 0;
}

static unsigned int sfc_nor_read_params(unsigned int addr, unsigned char *buf, unsigned int len)
{
	unsigned char command;
	int dummy_byte;
	int addr_size;
	int transfer_mode;
	struct sfc_transfer transfer;
	struct sfc_message message;
	struct cmd_info cmd;
	int ret;

	sfc_message_init(&message);
	memset(&transfer, 0, sizeof(transfer));
	memset(&cmd, 0, sizeof(cmd));

	cmd.cmd = SPINOR_OP_READ;
	cmd.dataen = ENABLE;
	transfer.addr_len = DEF_ADDR_LEN;
	transfer.addr = addr;
	transfer.len = len;
	transfer.data = buf;
	transfer.ops_mode = CPU_OPS;
	transfer.sfc_mode = TM_STD_SPI;
	transfer.direction = GLB_TRAN_DIR_READ;
	transfer.cmd_info = &cmd;
	sfc_message_add_tail(&transfer, &message);

	ret = sfc_sync(flash->sfc, &message);
	if(ret) {
		printf("sfc_sync error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		ret=-EIO;
	}

	return message.actual_length;
}


static unsigned int sfc_do_read(unsigned int addr, unsigned char *buf, unsigned int len)
{
	unsigned char command;
	int dummy_byte;
	int addr_size;
	int transfer_mode;
	struct sfc_transfer transfer;
	struct sfc_message message;
	struct cmd_info cmd;
	int ret;

	command = flash->cur_r_cmd->cmd;
	dummy_byte = flash->cur_r_cmd->dummy_byte;
	addr_size = flash->cur_r_cmd->addr_nbyte;
	transfer_mode = flash->cur_r_cmd->transfer_mode;

	sfc_message_init(&message);
	memset(&transfer, 0, sizeof(transfer));
	memset(&cmd, 0, sizeof(cmd));

	cmd.cmd = command;
	cmd.dataen = ENABLE;
	transfer.addr_len = addr_size;
	transfer.data_dummy_bits = dummy_byte;
	transfer.addr = addr;
	transfer.len = len;
	transfer.data = buf;
	transfer.ops_mode = CPU_OPS;
	transfer.sfc_mode = transfer_mode;
	transfer.direction = GLB_TRAN_DIR_READ;
	transfer.cmd_info = &cmd;
	sfc_message_add_tail(&transfer, &message);

	ret = sfc_sync(flash->sfc, &message);
	if(ret) {
		printf("sfc_sync error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		ret=-EIO;
	}

	return message.actual_length;
}
static unsigned  int sfc_do_write(unsigned int addr, unsigned int len, const unsigned char *buf)
{
	unsigned char command;
	int dummy_byte;
	int transfer_mode;
	int addr_size;
	struct sfc_transfer transfer[3];
	struct sfc_message message;
	struct cmd_info cmd[3];
	int ret;
	struct spi_nor_info *spi_nor_info;
	struct spi_nor_cmd_info *wr_en;
	struct spi_nor_st_info *busy;

	spi_nor_info = flash->g_nor_info;
	wr_en = &spi_nor_info->wr_en;
	busy = &spi_nor_info->busy;

	command = flash->cur_w_cmd->cmd;
	dummy_byte = flash->cur_w_cmd->dummy_byte;
	addr_size = flash->cur_w_cmd->addr_nbyte;
	transfer_mode = flash->cur_w_cmd->transfer_mode;

	sfc_message_init(&message);
	memset(&transfer, 0, sizeof(transfer));
	memset(&cmd, 0, sizeof(cmd));

	/* write enable */
	cmd[0].cmd = wr_en->cmd;
	cmd[0].dataen = DISABLE;
	transfer[0].cmd_info = &cmd[0];
	transfer[0].addr_len = wr_en->addr_nbyte;
	transfer[0].sfc_mode = transfer_mode;
	transfer[0].data_dummy_bits = wr_en->dummy_byte;
	sfc_message_add_tail(&transfer[0], &message);

	/* write ops */
	cmd[1].cmd = command;
	cmd[1].dataen = ENABLE;
	transfer[1].addr = addr;
	transfer[1].addr_len = addr_size;
	transfer[1].len = len;
	transfer[1].data_dummy_bits = dummy_byte;
	transfer[1].data = buf;
	transfer[1].ops_mode = CPU_OPS;
	transfer[1].sfc_mode = transfer_mode;
	transfer[1].direction = GLB_TRAN_DIR_WRITE;
	transfer[1].cmd_info = &cmd[1];
	sfc_message_add_tail(&transfer[1], &message);

	cmd[2].cmd = busy->cmd;
	cmd[2].dataen = DISABLE;
	cmd[2].sta_exp = busy->val << busy->bit_shift;
	cmd[2].sta_msk = busy->mask << busy->bit_shift;
	transfer[2].cmd_info = &cmd[2];
	sfc_message_add_tail(&transfer[2], &message);

	ret = sfc_sync(flash->sfc, &message);
	if(ret) {
		printf("sfc_sync error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		ret=-EIO;
	}

	return message.actual_length;
}

static int sfc_do_erase(uint32_t addr)
{
	struct sfc_transfer transfer[3];
	struct sfc_message message;
	struct cmd_info cmd[3];
	int ret;

	struct spi_nor_info *spi_nor_info;
	struct spi_nor_cmd_info *sector_erase;
	struct spi_nor_cmd_info *wr_en;
	struct spi_nor_st_info *busy;
	int addr_size;


	spi_nor_info = flash->g_nor_info;
	sector_erase = &spi_nor_info->sector_erase;
	wr_en = &spi_nor_info->wr_en;
	busy = &spi_nor_info->busy;
	addr_size = sector_erase->addr_nbyte;

	sfc_message_init(&message);
	memset(&transfer, 0, sizeof(transfer));
	memset(&cmd, 0, sizeof(cmd));

	/* write enable */
	cmd[0].cmd = wr_en->cmd;
	cmd[0].dataen = DISABLE;
	transfer[0].cmd_info = &cmd[0];
	transfer[0].addr_len = wr_en->addr_nbyte;
	transfer[0].sfc_mode = wr_en->transfer_mode;
	transfer[0].data_dummy_bits = wr_en->dummy_byte;
	sfc_message_add_tail(&transfer[0], &message);

	/* erase ops */
	cmd[1].cmd = sector_erase->cmd;
	cmd[1].dataen = DISABLE;
	transfer[1].addr_len = addr_size;
	transfer[1].data_dummy_bits = sector_erase->dummy_byte;
	transfer[1].addr = addr;
	transfer[1].sfc_mode = TM_STD_SPI;
	transfer[1].direction = GLB_TRAN_DIR_WRITE;
	transfer[1].cmd_info = &cmd[1];
	sfc_message_add_tail(&transfer[1], &message);

	cmd[2].cmd = busy->cmd;
	cmd[2].dataen = DISABLE;
	cmd[2].sta_exp = busy->val << busy->bit_shift;
	cmd[2].sta_msk = busy->mask << busy->bit_shift;
	transfer[2].cmd_info = &cmd[2];
	sfc_message_add_tail(&transfer[2], &message);

	ret = sfc_sync(flash->sfc, &message);
	if(ret) {
		printf("sfc_sync error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		ret=-EIO;
	}

	return ret;
}

static int sfc_read(unsigned int from, unsigned int len, unsigned char *buf)
{
	int tmp_len = 0, current_len = 0;

	while(len) {
		tmp_len = sfc_do_read((unsigned int)from + current_len, &buf[current_len], len);
		current_len += tmp_len;
		len -= tmp_len;
	}

	return current_len;
}


int sfc_nor_read(unsigned int from, unsigned int len, unsigned char *buf)
{

#ifndef CONFIG_BURNER
	int i;
	if(flash->norflash_partitions->num_partition_info && (flash->norflash_partitions->num_partition_info != 0xffffffff)) {
		for(i = 0; i < flash->norflash_partitions->num_partition_info; i++){
			if(from >= flash->norflash_partitions->nor_partition[i].offset && \
					from < (flash->norflash_partitions->nor_partition[i].offset + \
						flash->norflash_partitions->nor_partition[i].size) && \
					(flash->norflash_partitions->nor_partition[i].mask_flags & NORFLASH_PART_WO)){
				printf("the partiton can't read,please check the partition RW mode\n");
				return 0;
			}
		}
	}
#endif

	sfc_read(from, len, buf);
	return 0;
}

int sfc_nor_page_write(unsigned int to, unsigned int len, unsigned char *buf)
{
	unsigned int page_offset, actual_len;
	int ret;
	struct spi_nor_info *spi_nor_info;
	int writesize;

	spi_nor_info = flash->g_nor_info;
	writesize = spi_nor_info->page_size;

#ifndef CONFIG_BURNER
	int i;
	if(flash->norflash_partitions->num_partition_info && (flash->norflash_partitions->num_partition_info != 0xffffffff)) {
		for(i = 0; i < flash->norflash_partitions->num_partition_info; i++){
			if(to >= flash->norflash_partitions->nor_partition[i].offset && \
					to < (flash->norflash_partitions->nor_partition[i].offset + \
						flash->norflash_partitions->nor_partition[i].size) && \
					(flash->norflash_partitions->nor_partition[i].mask_flags & NORFLASH_PART_RO)){
				printf("the partiton can't write,please check the partition RW mode\n");
				return 0;
			}
		}
	}
#endif

	page_offset = to & (spi_nor_info->page_size - 1);
	/* do all the bytes fit onto one page? */
	if (page_offset + len <= spi_nor_info->page_size) {
		ret = sfc_do_write(to, len, buf);
	} else {
		u32 i;

		/* the size of data remaining on the first page */
		actual_len = spi_nor_info->page_size - page_offset;
		ret = sfc_do_write(to, actual_len, buf);

		/* write everything in flash->page_size chunks */
		for (i = actual_len; i < len; i += writesize) {
			actual_len = len - i;
			if (actual_len >= writesize)
				actual_len = writesize;
			ret = sfc_do_write(to + i, actual_len, buf + i);
		}
	}
	return ret;

}


#ifdef CONFIG_BURNER

static struct legacy_params g_legacy_params;
struct legacy_params *params_compatibility()
{
	int val, mask, bit_shift;
	struct legacy_params *p = &g_legacy_params;

	memset(p, 0, sizeof(struct legacy_params));

	p->magic = params.magic;
	p->version = 1;

	memcpy(p->nor_params.name, params.spi_nor_info.name, sizeof(SIZEOF_NAME));
	p->nor_params.pagesize = params.spi_nor_info.page_size;
	p->nor_params.sectorsize = 4096;
	p->nor_params.chipsize = params.spi_nor_info.chip_size;
	p->nor_params.erasesize = params.spi_nor_info.erase_size;
	p->nor_params.id = params.spi_nor_info.id;

	if (p->nor_params.chipsize > 0x1000000 ) {
		p->nor_params.addrsize = 4;
	} else {
		p->nor_params.addrsize = 3;
	}

	p->nor_params.block_info.cmd_blockerase = params.spi_nor_info.sector_erase.cmd;
	p->nor_params.quad_mode.dummy_byte = params.spi_nor_info.read_quad.dummy_byte;
	p->nor_params.quad_mode.RDSR_CMD = params.spi_nor_info.quad_get.cmd;
	p->nor_params.quad_mode.WRSR_CMD = params.spi_nor_info.quad_set.cmd;

	val = params.spi_nor_info.quad_get.val;
	mask = params.spi_nor_info.quad_get.mask;
	bit_shift = params.spi_nor_info.quad_get.bit_shift;
	p->nor_params.quad_mode.RDSR_DATA = (val & mask) << bit_shift;
	p->nor_params.quad_mode.RD_DATA_SIZE = params.spi_nor_info.quad_set.bit_shift / 8 + 1;

	val = params.spi_nor_info.quad_set.val;
	mask = params.spi_nor_info.quad_set.mask;
	bit_shift = params.spi_nor_info.quad_set.bit_shift;
	p->nor_params.quad_mode.WRSR_DATA = (val & mask) << bit_shift;
	p->nor_params.quad_mode.WD_DATA_SIZE = params.spi_nor_info.quad_set.bit_shift / 8 + 1;

	p->nor_params.quad_mode.cmd_read = params.spi_nor_info.read_quad.cmd;
	p->nor_params.quad_mode.sfc_mode = params.spi_nor_info.read_quad.transfer_mode;

	memcpy(&p->norflash_partitions, &params.norflash_partitions, sizeof(struct norflash_partitions));

	return p;
}
#endif


int sfc_nor_write(unsigned int to, unsigned int len, unsigned char *buf)
{
	int ret = 0;
	ret = sfc_nor_page_write(to, len, buf);

	return 0;
}

int sfc_nor_erase(unsigned int addr, unsigned int len)
{
	int ret;
	uint32_t end;
	int erasesize;

	struct spi_nor_info *spi_nor_info;

	spi_nor_info = flash->g_nor_info;
	erasesize = spi_nor_info->erase_size;

#ifndef CONFIG_BURNER
	int i;
	if(flash->norflash_partitions->num_partition_info && (flash->norflash_partitions->num_partition_info != 0xffffffff)) {
		for(i = 0; i < flash->norflash_partitions->num_partition_info; i++){
			if(addr >= flash->norflash_partitions->nor_partition[i].offset && \
					addr < (flash->norflash_partitions->nor_partition[i].offset + \
						flash->norflash_partitions->nor_partition[i].size) && \
					(flash->norflash_partitions->nor_partition[i].mask_flags & NORFLASH_PART_RO)){
				printf("the partiton can't erase,please check the partition RW mode\n");
				return 0;
			}
		}
	}
#endif
	if(len % erasesize != 0){
		len = len - (len % erasesize) + erasesize;
	}

	end = addr + len;
	while (addr < end){
		ret = sfc_do_erase(addr);
		if (ret) {
			printf("erase error !\n");
			return ret;
		}
		addr += erasesize;
	}

	return 0;
}

int sfc_nor_do_special_func()
{
	int tchsh;
	int tslch;
	int tshsl_rd;
	int tshsl_wr;
	struct spi_nor_info *spi_nor_info;

	spi_nor_info = flash->g_nor_info;

	tchsh = spi_nor_info->tCHSH;
	tslch = spi_nor_info->tSLCH;
	tshsl_rd = spi_nor_info->tSHSL_RD;
	tshsl_wr = spi_nor_info->tSHSL_WR;
	set_flash_timing(flash->sfc, tchsh, tslch, tshsl_rd, tshsl_wr);

	sfc_nor_get_special_ops(flash);

	flash->quad_succeed = 0;
#ifndef CONFIG_BURNER
	if (params.uk_quad) {
#else
	if (burn_mode) {
#endif
		if (flash->nor_flash_ops->set_quad_mode) {
			flash->nor_flash_ops->set_quad_mode(flash);
		}
		if (flash->quad_succeed)
			printf("nor flash quad mode is set, now use quad mode!\n");
	}

	/* if nor flash size is greater than 16M, use 4byte mode */
	if(spi_nor_info->chip_size > 0x1000000) {
		if (flash->nor_flash_ops->set_4byte_mode) {
			flash->nor_flash_ops->set_4byte_mode(flash);
		}
	}

	get_current_operate_cmd(flash);
}


int sfc_nor_flash_init()
{
	int ret = 0;
	int chip_id = 0;
	struct spi_nor_info *spi_nor_info;
	int i;

	flash = malloc(sizeof(struct sfc_flash));
	if (!flash) {
		printf("ERROR: %s %d kzalloc() error !\n",__func__,__LINE__);
		return -1;
	}
	memset(flash, 0, sizeof(struct sfc_flash));

	flash->sfc = sfc_res_init(CONFIG_SFC_NOR_RATE);

	sfc_nor_reset();

#ifndef CONFIG_BURNER
	set_flash_timing(flash->sfc, DEF_TCHSH, DEF_TSLCH, DEF_TSHSL_R, DEF_TSHSL_W);
	sfc_nor_read_params(CONFIG_SPIFLASH_PART_OFFSET, &params, sizeof(struct burner_params));
	printf("params.magic : 0x%x   params.version : 0x%x\n", params.magic, params.version);
	if((params.magic != NOR_MAGIC) || (params.version != NOR_VERSION)) {
		printf("sfc nor read params error\n");
		return -1;
	}

	flash->g_nor_info = &(params.spi_nor_info);
	flash->norflash_partitions = &params.norflash_partitions;

	sfc_nor_do_special_func();

#else
	flash->g_nor_info = malloc(sizeof(struct spi_nor_info));
	flash->norflash_partitions = malloc(sizeof(struct norflash_partitions));
#endif
	return 0;

}

int jz_sfc_chip_erase()
{
	printf("chip erasing...\n");
	struct sfc_transfer transfer[3];
	struct sfc_message message;
	struct cmd_info cmd[3];
	int ret;

	struct spi_nor_info *spi_nor_info;
	struct spi_nor_cmd_info *wr_en;
	struct spi_nor_st_info *busy;


	spi_nor_info = flash->g_nor_info;
	wr_en = &spi_nor_info->wr_en;
	busy = &spi_nor_info->busy;

	sfc_message_init(&message);
	memset(&transfer, 0, sizeof(transfer));
	memset(&cmd, 0, sizeof(cmd));

	/* write enable */
	cmd[0].cmd = wr_en->cmd;
	cmd[0].dataen = DISABLE;
	transfer[0].cmd_info = &cmd[0];
	transfer[0].addr_len = wr_en->addr_nbyte;
	transfer[0].sfc_mode = wr_en->transfer_mode;
	transfer[0].data_dummy_bits = wr_en->dummy_byte;
	sfc_message_add_tail(&transfer[0], &message);

	/* erase ops */
	cmd[1].cmd = spi_nor_info->chip_erase_cmd;
	cmd[1].dataen = DISABLE;
	transfer[1].sfc_mode = TM_STD_SPI;
	transfer[1].direction = GLB_TRAN_DIR_WRITE;
	transfer[1].cmd_info = &cmd[1];
	sfc_message_add_tail(&transfer[1], &message);

	cmd[2].cmd = busy->cmd;
	cmd[2].dataen = DISABLE;
	cmd[2].sta_exp = busy->val << busy->bit_shift;
	cmd[2].sta_msk = busy->mask << busy->bit_shift;
	transfer[2].cmd_info = &cmd[2];
	sfc_message_add_tail(&transfer[2], &message);

	ret = sfc_sync(flash->sfc, &message);
	if (ret) {
		printf("sfc_sync error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		ret=-EIO;
	}

	return ret;
}

#ifdef CONFIG_BURNER
struct nor_partition *get_partition_index(u32 offset, u32 length, int *pt_index)
{
	int i;
	struct spi_nor_info *spi_nor_info;

	spi_nor_info = flash->g_nor_info;

	for(i = 0; i < flash->norflash_partitions->num_partition_info; i++){
		if(offset >= flash->norflash_partitions->nor_partition[i].offset && \
				(offset + length) <= (flash->norflash_partitions->nor_partition[i].offset + \
					flash->norflash_partitions->nor_partition[i].size)){
			*pt_index = i;
			break;
		}else if(offset >= flash->norflash_partitions->nor_partition[i].offset && \
				offset < (spi_nor_info->chip_size) && \
				(flash->norflash_partitions->nor_partition[i].size == 0xffffffff)){ /*size == -1*/
			flash->norflash_partitions->nor_partition[i].size=spi_nor_info->chip_size-flash->norflash_partitions->nor_partition[i].offset;
			*pt_index = i;
			break;
		}
	}
	if(i >= flash->norflash_partitions->num_partition_info){
		*pt_index = -1;
		printf("partition size not align with write transfer size \n");
		return NULL;
	}
	return &flash->norflash_partitions->nor_partition[i];
}

int check_offset(u32 offset,u32 length)
{
	int i;

	for(i = 1; i < flash->norflash_partitions->num_partition_info; i++){
		if(offset < flash->norflash_partitions->nor_partition[i].offset && \
				offset > (flash->norflash_partitions->nor_partition[i-1].offset + \
					flash->norflash_partitions->nor_partition[i-1].size)){
			break;
		}
	}
	if(i >= flash->norflash_partitions->num_partition_info){
		return i;
	}else if((offset+length) > flash->norflash_partitions->nor_partition[i].offset ){
		return -1;
	}else{
		return i;
	};
}

#ifdef SFC_NOR_CLONER_DEBUG
static void dump_cloner_params()
{
	struct spi_nor_info *spi_nor_info;

	spi_nor_info = &params.spi_nor_info;

	printf("name=%s\n", spi_nor_info->name);
	printf("id=0x%x\n", spi_nor_info->id);

	printf("read_standard->cmd=0x%x\n",		spi_nor_info->read_standard.cmd);
	printf("read_standard->dummy=0x%x\n",		spi_nor_info->read_standard.dummy_byte);
	printf("read_standard->addr_nbyte=0x%x\n",	spi_nor_info->read_standard.addr_nbyte);
	printf("read_standard->transfer_mode=0x%x\n",	spi_nor_info->read_standard.transfer_mode);

	printf("read_quad->cmd=0x%x\n",			spi_nor_info->read_quad.cmd);
	printf("read_quad->dummy=0x%x\n",		spi_nor_info->read_quad.dummy_byte);
	printf("read_quad->addr_nbyte=0x%x\n",		spi_nor_info->read_quad.addr_nbyte);
	printf("read_quad->transfer_mode=0x%x\n",	spi_nor_info->read_quad.transfer_mode);

	printf("write_standard->cmd=0x%x\n",		spi_nor_info->write_standard.cmd);
	printf("write_standard->dummy=0x%x\n",		spi_nor_info->write_standard.dummy_byte);
	printf("write_standard->addr_nbyte=0x%x\n",	spi_nor_info->write_standard.addr_nbyte);
	printf("write_standard->transfer_mode=0x%x\n",	spi_nor_info->write_standard.transfer_mode);

	printf("write_quad->cmd=0x%x\n",		spi_nor_info->write_quad.cmd);
	printf("write_quad->dummy=0x%x\n",		spi_nor_info->write_quad.dummy_byte);
	printf("write_quad->addr_nbyte=0x%x\n",		spi_nor_info->write_quad.addr_nbyte);
	printf("write_quad->transfer_mode=0x%x\n",	spi_nor_info->write_quad.transfer_mode);

	printf("sector_erase->cmd=0x%x\n",		spi_nor_info->sector_erase.cmd);
	printf("sector_erase->dummy=0x%x\n",		spi_nor_info->sector_erase.dummy_byte);
	printf("sector_erase->addr_nbyte=0x%x\n",	spi_nor_info->sector_erase.addr_nbyte);
	printf("sector_erase->transfer_mode=0x%x\n",	spi_nor_info->sector_erase.transfer_mode);

	printf("wr_en->cmd=0x%x\n",		spi_nor_info->wr_en.cmd);
	printf("wr_en->dummy=0x%x\n",		spi_nor_info->wr_en.dummy_byte);
	printf("wr_en->addr_nbyte=0x%x\n",	spi_nor_info->wr_en.addr_nbyte);
	printf("wr_en->transfer_mode=0x%x\n",	spi_nor_info->wr_en.transfer_mode);

	printf("en4byte->cmd=0x%x\n",		spi_nor_info->en4byte.cmd);
	printf("en4byte->dummy=0x%x\n",		spi_nor_info->en4byte.dummy_byte);
	printf("en4byte->addr_nbyte=0x%x\n",	spi_nor_info->en4byte.addr_nbyte);
	printf("en4byte->transfer_mode=0x%x\n",	spi_nor_info->en4byte.transfer_mode);

	printf("quad_set->cmd=0x%x\n",		spi_nor_info->quad_set.cmd);
	printf("quad_set->bit_shift=0x%x\n",		spi_nor_info->quad_set.bit_shift);
	printf("quad_set->mask=0x%x\n",		spi_nor_info->quad_set.mask);
	printf("quad_set->val=0x%x\n",		spi_nor_info->quad_set.val);
	printf("quad_set->len=0x%x\n",		spi_nor_info->quad_set.len);
	printf("quad_set->dummy=0x%x\n",	spi_nor_info->quad_set.dummy);

	printf("quad_get->cmd=0x%x\n",		spi_nor_info->quad_get.cmd);
	printf("quad_get->bit_shift=0x%x\n",		spi_nor_info->quad_get.bit_shift);
	printf("quad_get->mask=0x%x\n",		spi_nor_info->quad_get.mask);
	printf("quad_get->val=0x%x\n",		spi_nor_info->quad_get.val);
	printf("quad_get->len=0x%x\n",		spi_nor_info->quad_get.len);
	printf("quad_get->dummy=0x%x\n",	spi_nor_info->quad_get.dummy);

	printf("busy->cmd=0x%x\n",		spi_nor_info->busy.cmd);
	printf("busy->bit_shift=0x%x\n",		spi_nor_info->busy.bit_shift);
	printf("busy->mask=0x%x\n",		spi_nor_info->busy.mask);
	printf("busy->val=0x%x\n",		spi_nor_info->busy.val);
	printf("busy->len=0x%x\n",		spi_nor_info->busy.len);
	printf("busy->dummy=0x%x\n",		spi_nor_info->busy.dummy);

	printf("quad_ops_mode=%d\n",	spi_nor_info->quad_ops_mode);
	printf("addr_ops_mode=%d\n",	spi_nor_info->addr_ops_mode);

	printf("tCHSH=%d\n",	spi_nor_info->tCHSH);
	printf("tSLCH=%d\n",	spi_nor_info->tSLCH);
	printf("tSHSL_RD=%d\n", spi_nor_info->tSHSL_RD);
	printf("tSHSL_WR=%d\n", spi_nor_info->tSHSL_WR);

	printf("chip_size=%d\n",	spi_nor_info->chip_size);
	printf("page_size=%d\n",	spi_nor_info->page_size);
	printf("erase_size=%d\n",	spi_nor_info->erase_size);

	printf("chip_erase_cmd=0x%x\n",	spi_nor_info->chip_erase_cmd);
}
static void dump_mini_cloner_params()
{
	struct mini_spi_nor_info *spi_nor_info;

	spi_nor_info = &mini_params;

	printf("mini_name=%s\n", spi_nor_info->name);
	printf("mini_name=%x\n", spi_nor_info->name);
	printf("mini_id=0x%x\n", spi_nor_info->id);

	printf("mini_read_standard->cmd=0x%x\n",		spi_nor_info->read_standard.cmd);
	printf("mini_read_standard->dummy=0x%x\n",		spi_nor_info->read_standard.dummy_byte);
	printf("mini_read_standard->addr_nbyte=0x%x\n",	spi_nor_info->read_standard.addr_nbyte);
	printf("mini_read_standard->transfer_mode=0x%x\n",	spi_nor_info->read_standard.transfer_mode);

	printf("mini_read_quad->cmd=0x%x\n",			spi_nor_info->read_quad.cmd);
	printf("mini_read_quad->dummy=0x%x\n",		spi_nor_info->read_quad.dummy_byte);
	printf("mini_read_quad->addr_nbyte=0x%x\n",		spi_nor_info->read_quad.addr_nbyte);
	printf("mini_read_quad->transfer_mode=0x%x\n",	spi_nor_info->read_quad.transfer_mode);

	printf("mini_wr_en->cmd=0x%x\n",		spi_nor_info->wr_en.cmd);
	printf("mini_wr_en->dummy=0x%x\n",		spi_nor_info->wr_en.dummy_byte);
	printf("mini_wr_en->addr_nbyte=0x%x\n",	spi_nor_info->wr_en.addr_nbyte);
	printf("mini_wr_en->transfer_mode=0x%x\n",	spi_nor_info->wr_en.transfer_mode);

	printf("mini_en4byte->cmd=0x%x\n",		spi_nor_info->en4byte.cmd);
	printf("mini_en4byte->dummy=0x%x\n",		spi_nor_info->en4byte.dummy_byte);
	printf("mini_en4byte->addr_nbyte=0x%x\n",	spi_nor_info->en4byte.addr_nbyte);
	printf("mini_en4byte->transfer_mode=0x%x\n",	spi_nor_info->en4byte.transfer_mode);

	printf("mini_quad_set->cmd=0x%x\n",		spi_nor_info->quad_set.cmd);
	printf("mini_quad_set->bit_shift=0x%x\n",		spi_nor_info->quad_set.bit_shift);
	printf("mini_quad_set->mask=0x%x\n",		spi_nor_info->quad_set.mask);
	printf("mini_quad_set->val=0x%x\n",		spi_nor_info->quad_set.val);
	printf("mini_quad_set->len=0x%x\n",		spi_nor_info->quad_set.len);
	printf("mini_quad_set->dummy=0x%x\n",	spi_nor_info->quad_set.dummy);

	printf("mini_quad_get->cmd=0x%x\n",		spi_nor_info->quad_get.cmd);
	printf("mini_quad_get->bit_shift=0x%x\n",		spi_nor_info->quad_get.bit_shift);
	printf("mini_quad_get->mask=0x%x\n",		spi_nor_info->quad_get.mask);
	printf("mini_quad_get->val=0x%x\n",		spi_nor_info->quad_get.val);
	printf("mini_quad_get->len=0x%x\n",		spi_nor_info->quad_get.len);
	printf("mini_quad_get->dummy=0x%x\n",	spi_nor_info->quad_get.dummy);

	printf("mini_busy->cmd=0x%x\n",		spi_nor_info->busy.cmd);
	printf("mini_busy->bit_shift=0x%x\n",		spi_nor_info->busy.bit_shift);
	printf("mini_busy->mask=0x%x\n",		spi_nor_info->busy.mask);
	printf("mini_busy->val=0x%x\n",		spi_nor_info->busy.val);
	printf("mini_busy->len=0x%x\n",		spi_nor_info->busy.len);
	printf("mini_busy->dummy=0x%x\n",		spi_nor_info->busy.dummy);

	printf("mini_quad_ops_mode=%d\n",	spi_nor_info->quad_ops_mode);
	printf("addr_ops_mode=%d\n",	spi_nor_info->addr_ops_mode);

	printf("mini_chip_size=%d\n",	spi_nor_info->chip_size);
	printf("mini_page_size=%d\n",	spi_nor_info->page_size);
	printf("mini_erase_size=%d\n",	spi_nor_info->erase_size);
	printf("mini_quad_mode=%d\n",	spi_nor_info->spl_quad);

}
#endif


int norflash_get_params_from_burner(unsigned char *addr)
{
	unsigned int chip_id ,chipnum,i;
	struct spi_nor_info *spi_nor_info;
	struct mini_spi_nor_info *mini_spi_nor_info;
	unsigned int id_len = 3;
	unsigned int id_addr = 0;
	unsigned int id_addr_len = 0;
	unsigned int dummy = 0;
	struct spiflash_info *spiflash_info;

	spiflash_info = (struct spiflash_info *)addr;

	chip_id = sfc_nor_read_id(SPINOR_OP_RDID, id_addr, id_addr_len, id_len, dummy);
	printf("spi nor flash chip_id is : %x\n", chip_id);

	memcpy(&params, spiflash_info, sizeof(struct burner_params));
	memcpy(&mini_params, &spiflash_info->mini_spi_nor_info, sizeof(struct mini_spi_nor_info));
	burn_mode = spiflash_info->b_quad;


#ifdef SFC_NOR_CLONER_DEBUG
	dump_cloner_params();
	dump_mini_cloner_params();
	printf("fs_erase_size=%d\n", params.fs_erase_size);
	printf("uk_quad=%d\n", params.uk_quad);
	printf("burner_quad_mode=%d\n",spiflash_info->b_quad);
#endif

	if (!memcmp(&params.spi_nor_info, 0, sizeof(struct spi_nor_info))) {
		printf("unsupport nor flash, no params in burner\n");
		return -1;
	}
	if (!memcmp(&mini_params, 0, sizeof(struct mini_spi_nor_info))) {
		printf("unsupport nor flash, no mini params in burner\n");
		return -1;
	}


	memcpy(flash->g_nor_info, &params.spi_nor_info, sizeof(struct spi_nor_info));
	memcpy(flash->norflash_partitions, &params.norflash_partitions, sizeof(struct norflash_partitions));
	sfc_nor_do_special_func();

#ifdef SFC_NOR_CLONER_DEBUG
	printf("partition num=%d\n", flash->norflash_partitions->num_partition_info);
	for (i = 0; i < flash->norflash_partitions->num_partition_info; i++) {
		printf("p[%d].name=%s\n", i, flash->norflash_partitions->nor_partition[i].name);
		printf("p[%d].size=%x\n", i, flash->norflash_partitions->nor_partition[i].size);
		printf("p[%d].offset=%x\n", i, flash->norflash_partitions->nor_partition[i].offset);
	}
#endif
	return 0;
}



#endif
