/*
 * jz_sfc_ops.c is for sfc nor flash to set special function,
 * such as set_quad_mode, set_4byte_mode, and so on.
 *
 */

#include <errno.h>

#include <asm/arch/sfc_nand.h>
#include <asm/arch/sfc_params.h>

int get_status(struct sfc_flash *flash, int command, int len)
{
	struct sfc_transfer transfer;
	struct sfc_message message;
	struct cmd_info cmd;
	int ret;
	unsigned char buf[32];
	int i = 0;
	int val = 0;

	memset(buf, 0, sizeof(buf));
	sfc_message_init(&message);
	memset(&transfer, 0, sizeof(transfer));
	memset(&cmd, 0, sizeof(cmd));

	cmd.cmd = command;
	cmd.dataen = ENABLE;

	transfer.addr_len = 0;
	transfer.data_dummy_bits = 0;
	transfer.addr = 0;
	transfer.len = len;
	transfer.data = buf;
	transfer.cur_len = 0;
	transfer.ops_mode = CPU_OPS;

	transfer.sfc_mode = TM_STD_SPI;
	transfer.direction = GLB_TRAN_DIR_READ;
	transfer.cmd_info = &cmd;
	sfc_message_add_tail(&transfer, &message);


	ret = sfc_sync(flash->sfc, &message);
	if(ret) {
		printf("sfc_sync error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		ret=-EIO;
		return ret;
	}

	for(i = 0; i < len; i++) {
		val |= buf[i] << (i * 8);
	}
	return val;

}

/* do nothing to set quad mode, use cmd directly */
static int set_quad_mode_cmd(struct sfc_flash *flash)
{
	flash->quad_succeed = 1;
	return 0;
}

/* write nor flash status register QE bit to set quad mode */
static int set_quad_mode_reg(struct sfc_flash *flash)
{
	unsigned int data;
	int ret;
	struct sfc_transfer transfer[3];
	struct sfc_message message;
	struct cmd_info cmd[3];
	struct spi_nor_info *spi_nor_info;
	struct spi_nor_cmd_info *wr_en;
	struct spi_nor_st_info *quad_set;
	struct spi_nor_st_info *quad_get;
	struct spi_nor_st_info *busy;

	spi_nor_info = flash->g_nor_info;

	if ((&spi_nor_info->quad_get == NULL) || (&spi_nor_info->quad_set == NULL)) {
		printf( "quad info is null, use standard spi mode\n");
		return -1;
	}

	quad_set = &spi_nor_info->quad_set;
	quad_get = &spi_nor_info->quad_get;
	busy = &spi_nor_info->busy;
	data = (quad_set->val & quad_set->mask) << quad_set->bit_shift;
	wr_en = &spi_nor_info->wr_en;

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

	/* write ops */
	cmd[1].cmd = quad_set->cmd;
	cmd[1].dataen = ENABLE;
	transfer[1].len = quad_set->len;
	transfer[1].data = (unsigned char *)&data;
	transfer[1].data_dummy_bits = quad_set->dummy;
	transfer[1].ops_mode = CPU_OPS;
	transfer[1].sfc_mode = TM_STD_SPI;
	transfer[1].direction = GLB_TRAN_DIR_WRITE;
	transfer[1].cmd_info = &cmd[1];
	sfc_message_add_tail(&transfer[1], &message);

	cmd[2].cmd = quad_get->cmd;
	cmd[2].dataen = DISABLE;
	cmd[2].sta_exp = quad_get->val << quad_get->bit_shift;
	cmd[2].sta_msk = quad_get->mask << quad_get->bit_shift;
	transfer[2].data_dummy_bits = quad_get->dummy;
	transfer[2].cmd_info = &cmd[2];
	sfc_message_add_tail(&transfer[2], &message);

	ret = sfc_sync(flash->sfc, &message);
	if(ret) {
		printf("sfc_sync error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		ret=-EIO;
	} else {
		flash->quad_succeed = 1;
	}
	while(!(((get_status(flash, busy->cmd, busy->len) >> busy->bit_shift) & busy->mask) == busy->val));
	return ret;

}




static int write_enable(struct sfc_flash *flash)
{
	struct sfc_transfer transfer;
	struct sfc_message message;
	struct cmd_info cmd;
	int ret;
	struct spi_nor_info *spi_nor_info;
	struct spi_nor_cmd_info *wr_en;

	spi_nor_info = flash->g_nor_info;
	wr_en = &spi_nor_info->wr_en;

	sfc_message_init(&message);
	memset(&transfer, 0, sizeof(transfer));
	memset(&cmd, 0, sizeof(cmd));

	cmd.cmd = wr_en->cmd;
	cmd.dataen = DISABLE;
	transfer.cmd_info = &cmd;
	transfer.addr_len = wr_en->addr_nbyte;
	transfer.sfc_mode = wr_en->transfer_mode;
	transfer.data_dummy_bits = wr_en->dummy_byte;
	sfc_message_add_tail(&transfer, &message);

	ret = sfc_sync(flash->sfc, &message);
	if(ret) {
		printf("sfc_sync error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		ret=-EIO;
	}
	return ret;
}

static int enter_4byte(struct sfc_flash *flash)
{
	struct sfc_transfer transfer;
	struct sfc_message message;
	struct cmd_info cmd;
	int ret;
	struct spi_nor_info *spi_nor_info;
	struct spi_nor_cmd_info *en4byte;

	spi_nor_info = flash->g_nor_info;
	en4byte = &spi_nor_info->en4byte;
	sfc_message_init(&message);
	memset(&transfer, 0, sizeof(transfer));
	memset(&cmd, 0, sizeof(cmd));

	cmd.cmd = en4byte->cmd;
	cmd.dataen = DISABLE;
	transfer.cmd_info = &cmd;
	transfer.addr_len = en4byte->addr_nbyte;
	transfer.sfc_mode = en4byte->transfer_mode;
	transfer.data_dummy_bits = en4byte->dummy_byte;
	sfc_message_add_tail(&transfer, &message);

	ret = sfc_sync(flash->sfc, &message);
	if(ret) {
		printf("sfc_sync error ! %s %s %d\n",__FILE__,__func__,__LINE__);
		ret=-EIO;
	}
	return ret;

}


/* send 4byte command to enter 4byte mode */
static int set_4byte_mode_normal(struct sfc_flash *flash)
{
	int ret;
	ret = enter_4byte(flash);
	if (ret) {
		printf( "enter 4byte mode failed\n");
	}
	return ret;
}

/**
 * 1.send write enable command
 * 2.send 4byte command to enter 4byte mode
 **/
static int set_4byte_mode_wren(struct sfc_flash *flash)
{
	int ret;
	ret = write_enable(flash);
	if (ret) {
		printf( "enter 4byte mode failed\n");
		return ret;
	}

	ret = enter_4byte(flash);
	if (ret) {
		printf( "enter 4byte mode failed\n");
	}
	return ret;
}


struct spi_nor_flash_ops nor_flash_ops;

static int noop(struct sfc_flash *flash)
{
	return 0;
}

int sfc_nor_get_special_ops(struct sfc_flash *flash)
{
	struct spi_nor_info *spi_nor_info;

	spi_nor_info = flash->g_nor_info;

	switch (spi_nor_info->quad_ops_mode) {
	case 0:
		nor_flash_ops.set_quad_mode = set_quad_mode_cmd;
		break;
	case 1:
		nor_flash_ops.set_quad_mode = set_quad_mode_reg;
		break;
	default:
		nor_flash_ops.set_quad_mode = noop;
		break;
	}

	switch (spi_nor_info->addr_ops_mode) {
	case 0:
		nor_flash_ops.set_4byte_mode = set_4byte_mode_normal;
		break;
	case 1:
		nor_flash_ops.set_4byte_mode = set_4byte_mode_wren;
		break;
	default:
		nor_flash_ops.set_4byte_mode = noop;
		break;
	}

	flash->nor_flash_ops = &nor_flash_ops;

	return 0;
}

