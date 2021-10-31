/********************************************
 ** Copyright (C) 2018 OPPO Mobile Comm Corp. Ltd.
 ** ODM_HQ_EDIT
 ** File: hx8394f_hd_dsi_vdo_hlt_hsd_zal1890.c
 ** Description: Source file for LCD driver
 **          To Control LCD driver
 ** Version :1.0
 ** Date : 2019/1/17
 ** Author: Liyan@ODM_HQ.Multimedia.LCD
 ** ---------------- Revision History: --------------------------
 ** <version>    <date>          < author >              <desc>
 **  1.0           2019/1/17  Wangxianfei@ODM_HQ   Source file for LCD driver
 ********************************************/

#include "../inc/lcm_drv.h"
#include "../inc/lcm_gpio.h"

#ifndef BUILD_LK
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/regulator/consumer.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/i2c.h>
#include <linux/irq.h>
#include <linux/uaccess.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#endif
#ifdef BUILD_LK
#include <platform/upmu_common.h>
#include <platform/mt_gpio.h>
#include <platform/mt_i2c.h>
#include <platform/mt_pmic.h>
#include <string.h>
#elif defined(BUILD_UBOOT)
#include <asm/arch/mt_gpio.h>
#endif

#define LOG_TAG "LCM"

#ifdef BUILD_LK
#define LCM_LOGI(string, args...)  dprintf(ALWAYS, "[LK/"LOG_TAG"]"string, ##args)
#define LCM_LOGD(string, args...)  dprintf(INFO, "[LK/"LOG_TAG"]"string, ##args)
#else
#define LCM_LOGI(fmt, args...)  pr_notice("[KERNEL/"LOG_TAG"]"fmt, ##args)
#define LCM_LOGD(fmt, args...)  pr_debug("[KERNEL/"LOG_TAG"]"fmt, ##args)
#endif

static struct LCM_UTIL_FUNCS lcm_util;

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))
#define MDELAY(n)       (lcm_util.mdelay(n))
#define UDELAY(n)       (lcm_util.udelay(n))


/* --------------------------------------------------------------------------- */
/* Local Functions */
/* --------------------------------------------------------------------------- */

#define dsi_set_cmdq_V22(cmdq, cmd, count, ppara, force_update) \
	lcm_util.dsi_set_cmdq_V22(cmdq, cmd, count, ppara, force_update)
#define dsi_set_cmdq_V2(cmd, count, ppara, force_update) \
    lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update) \
        lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd) lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums) \
        lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd) \
      lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size) \
        lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)

#ifndef BUILD_LK
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/i2c.h>
#include <linux/irq.h>
#include <linux/uaccess.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#endif
/* --------------------------------------------------------------------------- */
/* Local Constants */
/* --------------------------------------------------------------------------- */
#define LCM_DSI_CMD_MODE                                    0
#define FRAME_WIDTH                                     (720)
#define FRAME_HEIGHT                                    (1560)

#define GPIO_LCM_ID_0 GPIO22
#define GPIO_LCM_ID_1 GPIO21
#define GPIO_LCM_ID_2 GPIO23
#define GPIO_LCD_LED_EN_N GPIO165
#define GPIO_LCD_LED_EN_P GPIO166


#define REGFLAG_DELAY       0xFFFC
#define REGFLAG_UDELAY  0xFFFB
#define REGFLAG_END_OF_TABLE    0xFFFD
#define REGFLAG_RESET_LOW   0xFFFE
#define REGFLAG_RESET_HIGH  0xFFFF

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif
//extern unsigned int esd_recovery_backlight_level;
struct LCM_setting_table {
	unsigned int cmd;
	unsigned char count;
	unsigned char para_list[64];
};

static struct LCM_setting_table lcm_suspend_setting[] = {
	{0x28, 0, {} },
	{REGFLAG_DELAY, 50, {} },
	{0x10, 0, {} },
	{REGFLAG_DELAY, 120, {} },
};
static unsigned char lcd_bias_id;

//cmdline lcd_bias_id
static int __init bias_id(char *str)
{
	int id;

	if (get_option(&str , &id))
		lcd_bias_id = (unsigned char)id;
	return 0;
}
__setup("lcd_bias_id=", bias_id);
/* wangxianfei@ODM.HQ.Multimedia.LCM 2018/12/21 modified for backlight remapping*/
static int blmap_table[] = {
	36, 16,
	16, 22,
	17, 21,
	19, 20,
	19, 20,
	20, 17,
	22, 15,
	22, 14,
	24, 10,
	24, 8,
	26, 4,
	27, 0,
	29, 9,
	29, 9,
	30, 14,
	33, 25,
	34, 30,
	36, 44,
	37, 49,
	40, 65,
	40, 69,
	43, 88,
	46, 109,
	47, 112,
	50, 135,
	53, 161,
	53, 163,
	60, 220,
	60, 223,
	64, 257,
	63, 255,
	71, 334,
	71, 331,
	75, 375,
	80, 422,
	84, 473,
	89, 529,
	88, 518,
	99, 653,
	98, 640,
	103, 707,
	117, 878,
	115, 862,
	122, 947,
	128, 1039,
	135, 1138,
	132, 1102,
	149, 1355,
	157, 1478,
	166, 1611,
	163, 1563,
	183, 1900,
	180, 1844,
	203, 2232,
	199, 2169,
	209, 2344,
	236, 2821,
	232, 2742,
	243, 2958,
	255, 3188,
	268, 3433,
	282, 3696,
	317, 4405,
	176, 1560};

static struct LCM_setting_table init_setting[] = {
        {0xB9, 3, {0xFF, 0x83, 0x94}},
        {0xB1, 10, {0x48, 0x18, 0x78, 0x09, 0x32, 0x44, 0x71, 0x31, 0x4D, 0x30}},
        {0xBA, 6, {0x62, 0x03, 0x68, 0x7C, 0xB2, 0xC0}},
        {0xB2, 5, {0x00, 0x80, 0x87, 0x0C, 0x07}},
        {0xB4, 21, {0x17, 0x60, 0x17, 0x60, 0x17, 0x60, 0x0C, 0x0C, 0x70, 0x55, 0x00, 0x3F, 0x01, 0x6B, 0x01, 0x6B, 0x01, 0x6B, 0x01, 0x0C, 0x7C}},
	{0xD3, 33, {0x10, 0x00, 0x04, 0x04, 0x3C, 0x1C, 0x00, 0x00, 0x32, 0x10, 0x09, 0x00, 0x09, 0x32, 0x16, 0x25, 0x06, 0x25, 0x32, 0x00, 0x00,
					0x00, 0x00, 0x37, 0x03, 0x0B, 0x0B, 0x37, 0x00, 0x00, 0x00, 0x0C, 0x40}},
        {0xD5, 44, {0x19, 0x19, 0x18, 0x18, 0x1B, 0x1B, 0x1A, 0x1A, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x20, 0x21, 0x18, 0x18, 0x18,
					0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x24, 0x25, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
					0x18, 0x18, 0x18, 0x18}},
        {0xD6, 44, {0x18, 0x18, 0x19, 0x19, 0x1B, 0x1B, 0x1A, 0x1A, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00, 0x25, 0x24, 0x18, 0x18, 0x18,
					0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x21, 0x20, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18}},
        {0xE0, 58, {0x00, 0x0E, 0x1E, 0x28, 0x2D, 0x32, 0x37, 0x37, 0x72, 0x83, 0x93, 0x91, 0x95, 0xA2, 0xA1, 0xA0, 0xA9, 0xA9, 0xA4, 0xAF, 0xBD,
					0x5C, 0x5A, 0x5E, 0x60, 0x67, 0x6B, 0x75, 0x7F, 0x00, 0x0E, 0x1D, 0x28, 0x2D, 0x32, 0x37, 0x37, 0x72, 0x83, 0x92,
					0x90, 0x94, 0xA1, 0xA0, 0xA0, 0xA9, 0xA9, 0xA4, 0xAF, 0xBD, 0x5C, 0x5A, 0x5E, 0x60, 0x67, 0x6B, 0x75, 0x7F}},
        {0xCC, 1, {0x0B}},
        {0xC0, 2, {0x1F, 0x31}},
        {0xB6, 2, {0x75, 0x75}},
        {0xD4, 1, {0x02}},
        {0xBD, 1, {0x01}},
        {0xB1, 1, {0x00}},
        {0xBD, 1, {0x00}},
        {0xD2, 1, {0xBB}},
        {0xC7, 2, {0x00, 0xC0}},
	{0x35, 1, {0x00}},
        {0xC6, 1, {0xC5}},                      //enable 12-bit PWM
        {0xBD, 1, {0x02}},                      //enable 12-bit PWM
        {0xB1, 5, {0x00, 0x00, 0x00, 0x00, 0x00}}, //enable 12-bit PWM
        {0xBD, 1, {0x00}},                     //enable 12-bit PWM
        {0x11, 0, {}},
        {REGFLAG_DELAY, 120, {}},
        {0x29, 0, {}},
        {REGFLAG_DELAY, 20, {}},
        {0x51, 2, {0x00, 0x00}}, //SET PWM DUTY=0%
        {REGFLAG_DELAY, 5, {}},
        {0xC9, 9, {0x03, 0x00, 0x24, 0x1E, 0xB1, 0x1E, 0x00, 0x91, 0x00}}, //PWM FRE=30KHZ ,CABC dimming on
        {REGFLAG_DELAY, 5, {}},
        {0x55, 1, {0x01}},
        {REGFLAG_DELAY, 5, {}},
        {0xCA, 9, {0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x22, 0x20}},  //CABC ON level=1
        {0x53,1, {0x2C}},  //manual dimming on

};

 static struct LCM_setting_table bl_level[] = {
    {0xB9, 0x03, {0xFF, 0x83, 0x94}},
    {0x51, 0x02, {0x00, 0x00}},
    {REGFLAG_END_OF_TABLE, 0x00, {}}
 };

 static struct LCM_setting_table lcm_cabc_enter_setting1[] = {
    {0xB9, 0x03, {0xFF, 0x83, 0x94}},
    {0xCA, 0x09, {0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x22, 0x20}},	//CABC level 10%
    {REGFLAG_END_OF_TABLE, 0x00, {}}
 };

 static struct LCM_setting_table lcm_cabc_enter_setting2[] = {
    {0xB9, 0x03, {0xFF, 0x83, 0x94}},
    {0xCA, 0x09, {0x2A, 0x2A, 0x2A, 0x28, 0x28, 0x28, 0x26, 0x26, 0x26}},	//CABC level 20%
    {REGFLAG_END_OF_TABLE, 0x00, {}}
 };


 static struct LCM_setting_table lcm_cabc_enter_setting3[] = {
     {0xB9, 0x03, {0xFF, 0x83, 0x94}},
     {0xCA, 0x09, {0x2D, 0x2D, 0x2D, 0x2D, 0x2B, 0x2B, 0x2B, 0x29, 0x29}},	//CABC level 25%
     {REGFLAG_END_OF_TABLE, 0x00, {}}
 };



 static struct LCM_setting_table lcm_cabc_exit_setting[] = {
     {0xB9, 0x03, {0xFF, 0x83, 0x94}},
     {0xCA, 0x09, {0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20}},	//CABC OFF
     {REGFLAG_END_OF_TABLE, 0x00, {}}
 };

static void push_table(void *cmdq, struct LCM_setting_table *table,
	unsigned int count, unsigned char force_update)
{
	unsigned int i, cmd;

	for (i = 0; i < count; i++) {
		cmd = table[i].cmd;

		switch (cmd) {

		case REGFLAG_DELAY:
			if (table[i].count <= 10)
				MDELAY(table[i].count);
			else
				MDELAY(table[i].count);
			break;

		case REGFLAG_UDELAY:
			UDELAY(table[i].count);
			break;

		case REGFLAG_END_OF_TABLE:
			break;

		default:
			dsi_set_cmdq_V22(cmdq, cmd, table[i].count,
				table[i].para_list, force_update);
		}
	}
}

/* --------------------------------------------------------------------------- */
/* LCM Driver Implementations */
/* --------------------------------------------------------------------------- */

static void lcm_set_util_funcs(const struct LCM_UTIL_FUNCS *util)
{
	memcpy(&lcm_util, util, sizeof(struct LCM_UTIL_FUNCS));
}

static void lcm_get_params(struct LCM_PARAMS *params)
{
	memset(params, 0, sizeof(struct LCM_PARAMS));

	params->type = LCM_TYPE_DSI;
	params->width = FRAME_WIDTH;
	params->height = FRAME_HEIGHT;
	params->physical_width = 65;
	params->physical_height = 140;

	#if (LCM_DSI_CMD_MODE)
	params->dsi.mode = CMD_MODE;
	params->dsi.switch_mode = SYNC_PULSE_VDO_MODE;
	#else
	params->dsi.mode = SYNC_PULSE_VDO_MODE;
	params->dsi.switch_mode = CMD_MODE;
	#endif
	params->dsi.switch_mode_enable = 0;

	/* DSI */
	/* Command mode setting */
	params->dsi.LANE_NUM = LCM_THREE_LANE;
	/* The following defined the fomat for data coming from LCD engine. */
	params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq = LCM_DSI_TRANS_SEQ_MSB_FIRST;
	params->dsi.data_format.padding = LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format = LCM_DSI_FORMAT_RGB888;

	/* Highly depends on LCD driver capability. */
	params->dsi.packet_size = 256;
	/* video mode timing */

	params->dsi.PS = LCM_PACKED_PS_24BIT_RGB888;

	params->dsi.vertical_sync_active = 8;
	params->dsi.vertical_backporch = 20;
	params->dsi.vertical_frontporch = 20;
//	params->dsi.vertical_frontporch_for_low_power = 540;
	params->dsi.vertical_active_line = FRAME_HEIGHT;

	params->dsi.horizontal_sync_active = 8;
	params->dsi.horizontal_backporch = 46;
	params->dsi.horizontal_frontporch = 46;
	params->dsi.horizontal_active_pixel = FRAME_WIDTH;
	params->dsi.ssc_disable = 0; /*SSC config enable*/
	params->dsi.ssc_range = 4; /*SSC range*/
	params->dsi.PLL_CLOCK = 338; /* this value must be in MTK suggested table */

	params->dsi.cont_clock = 1;
	params->dsi.CLK_TRAIL = 8;
	params->dsi.clk_lp_per_line_enable = 0;
	params->dsi.esd_check_enable = 1;
	params->dsi.customization_esd_check_enable = 1;
	params->dsi.lcm_esd_check_table[0].cmd = 0x09;
	params->dsi.lcm_esd_check_table[0].count = 3;
	params->dsi.lcm_esd_check_table[0].para_list[0] = 0x80;
	params->dsi.lcm_esd_check_table[0].para_list[1] = 0x73;
	params->dsi.lcm_esd_check_table[0].para_list[2] = 0x04;

	params->dsi.lcm_esd_check_table[1].cmd = 0xd9;
	params->dsi.lcm_esd_check_table[1].count = 1;
	params->dsi.lcm_esd_check_table[1].para_list[0] = 0x80;
#ifdef CONFIG_MTK_ROUND_CORNER_SUPPORT
	params->round_corner_en = 1;
	params->full_content = 1;
	params->corner_pattern_width = 720;
	params->corner_pattern_height = 89;
	params->corner_pattern_height_bot = 82;
#endif
/* wangxianfei@ODM.HQ.Multimedia.LCM 2018/12/21 modified for backlight remapping*/
	params->blmap = blmap_table;
	params->blmap_size = sizeof(blmap_table)/sizeof(blmap_table[0]);
	params->brightness_max = 2047;
	params->brightness_min = 6;
}
extern void lcm_set_pwr(int val);
extern void lcm_set_pwr_n(int val);
static void lcm_resume_power(void);
static void lcm_init_power(void)/*esd recover will call it */
{
	LCM_LOGI("%s: enter\n", __func__);
	lcm_resume_power();
	LCM_LOGI("%s: exit\n", __func__);
}

static void lcm_suspend_power(void)
{
	LCM_LOGI("%s: enter\n", __func__);
	lcm_set_pwr(0);
	lcm_set_pwr_n(0);
	LCM_LOGI("%s: exit\n", __func__);
}
extern int sm5109_read_bytes(u8 addr, void *data);
extern int sm5109_write_bytes(unsigned char addr, unsigned char value);
static void lcm_resume_power(void)
{
	char buf[2]={0};
	lcm_set_pwr(1);
	UDELAY(50);
	//SM5109 reg[03] default value=0x03 NT50358CG reg[03]=0x33
	//printk("read 0x00=%d, 0x01=%d,lcd_bias_id=%d\n",buf[0],buf[1],lcd_bias_id);
	if(lcd_bias_id==0x03){
		sm5109_write_bytes(0x00, 0x34);//6v avdd SM5109
		sm5109_write_bytes(0x01, 0x14);//-6v avee
	}else{
		sm5109_write_bytes(0x00, 0x14);//6v avdd NT50358CG
		sm5109_write_bytes(0x01, 0x14);//-6v avee
	}
	MDELAY(3);//powertiming
	lcm_set_pwr_n(1);
	sm5109_read_bytes(0x00,&buf[0]);
	sm5109_read_bytes(0x01,&buf[1]);

	LCM_LOGI("%s: read 0x00=%d, 0x01=%d exit\n", __func__,buf[0],buf[1]);
}


static void lcm_init_lcm(void)
{
	LCM_LOGI("%s: enter\n", __func__);

	SET_RESET_PIN(1);
	MDELAY(5);
	SET_RESET_PIN(0);
	MDELAY(5);
	SET_RESET_PIN(1);
	MDELAY(50);

	push_table(NULL, init_setting, sizeof(init_setting) / sizeof(struct LCM_setting_table), 1);
	LCM_LOGI("%s: exit\n", __func__);
}

static void lcm_suspend(void)
{
	push_table(NULL, lcm_suspend_setting, sizeof(lcm_suspend_setting) / sizeof(struct LCM_setting_table), 1);

}

static void lcm_resume(void)
{
	lcm_init_lcm();
}

static unsigned int lcm_compare_id(void)
{
	LCM_LOGI("%s: enter\n", __func__);
	return 0;
}

static void lcm_setbacklight_cmdq(void *handle, unsigned int level)//255 0xf f //0x0f 0xf0
{
//	LCM_LOGI(" %s,backlight: level = %d\n", __func__, level);
	bl_level[1].para_list[0]=(level&0x7ff)>>7;/*get hight 4bit*/
	bl_level[1].para_list[1]=(level<<1)&0xfe;/*get low 7bit*/
	push_table(handle, bl_level,
		sizeof(bl_level) / sizeof(struct LCM_setting_table), 1);
//	esd_recovery_backlight_level=level;
}

static void lcm_set_cabc_mode_cmdq(void *handle, unsigned int level)
{
	printk("%s [lcd] cabc_mode is %d \n", __func__, level);

	if (1==level){
		push_table(handle, lcm_cabc_enter_setting1, sizeof(lcm_cabc_enter_setting1) / sizeof(struct LCM_setting_table), 1);
	}else if(2==level){
		push_table(handle, lcm_cabc_enter_setting2, sizeof(lcm_cabc_enter_setting2) / sizeof(struct LCM_setting_table), 1);
	}else if(3==level){
		push_table(handle, lcm_cabc_enter_setting3, sizeof(lcm_cabc_enter_setting3) / sizeof(struct LCM_setting_table), 1);
	} else {
		push_table(handle, lcm_cabc_exit_setting, sizeof(lcm_cabc_exit_setting) / sizeof(struct LCM_setting_table), 1);
	}
}

struct LCM_DRIVER hx8394f_hd_dsi_vdo_hlt_hsd_zal1890_lcm_drv = {
	.name = "hx8394f_hd_dsi_vdo_hlt_hsd_zal1890",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params = lcm_get_params,
	.init = lcm_init_lcm,
	.suspend = lcm_suspend,
	.resume = lcm_resume,
	.compare_id = lcm_compare_id,
	.init_power = lcm_init_power,
	.resume_power = lcm_resume_power,
	.suspend_power = lcm_suspend_power,
	.set_cabc_mode_cmdq = lcm_set_cabc_mode_cmdq,
	.set_backlight_cmdq = lcm_setbacklight_cmdq,
};
