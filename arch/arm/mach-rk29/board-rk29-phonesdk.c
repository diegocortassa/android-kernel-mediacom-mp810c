/* arch/arm/mach-rk29/board-rk29-phonesdk.c
 *
 * Copyright (C) 2010 ROCKCHIP, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/spi/spi.h>
#include <linux/mmc/host.h>
#include <linux/android_pmem.h>
#include <linux/usb/android_composite.h>

#include <mach/hardware.h>
#include <asm/setup.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/flash.h>
#include <asm/hardware/gic.h>

#include <mach/iomux.h>
#include <mach/gpio.h>
#include <mach/irqs.h>
#include <mach/rk29_iomap.h>
#include <mach/board.h>
#include <mach/rk29_nand.h>
#include <mach/rk29_camera.h>                          /* ddl@rock-chips.com : camera support */
#include <media/soc_camera.h>                               /* ddl@rock-chips.com : camera support */
#include <mach/vpu_mem.h>
#include <mach/sram.h>
#include <mach/ddr.h>
#include <mach/cpufreq.h>

#include <linux/regulator/rk29-pwm-regulator.h>
#include <linux/regulator/machine.h>
#include <linux/mfd/wm831x/pdata.h>
#include <linux/mfd/wm831x/core.h>
#include <linux/mfd/wm831x/gpio.h>
#include <linux/mfd/wm8994/pdata.h>
#include <linux/mfd/wm8994/registers.h>

#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include <linux/i2c-gpio.h>
#include <linux/mpu.h>
#include "devices.h"


#if defined(CONFIG_MTK23D)
#include <linux/mtk23d.h>
#endif

#ifdef CONFIG_USE_GPIO_GENERATE_WAVE
#include "../../../drivers/testcode/gpio_wave.h"
#endif

#include "../../../drivers/headset_observe/rk_headset.h"
/*set touchscreen different type header*/
#if defined(CONFIG_TOUCHSCREEN_XPT2046_NORMAL_SPI)
#include "../../../drivers/input/touchscreen/xpt2046_ts.h"
#elif defined(CONFIG_TOUCHSCREEN_XPT2046_TSLIB_SPI)
#include "../../../drivers/input/touchscreen/xpt2046_tslib_ts.h"
#elif defined(CONFIG_TOUCHSCREEN_XPT2046_CBN_SPI)
#include "../../../drivers/input/touchscreen/xpt2046_cbn_ts.h"
#endif
#include "../../../drivers/misc/gps/rk29_gps.h"
#include "../../../drivers/serial/sc8800.h"
#ifdef CONFIG_VIDEO_RK29
/*---------------- Camera Sensor Macro Define Begin  ------------------------*/
/*---------------- Camera Sensor Configuration Macro Begin ------------------------*/
#define CONFIG_SENSOR_0 RK29_CAM_SENSOR_OV5642                      /* back camera sensor */
#define CONFIG_SENSOR_IIC_ADDR_0 	    0x78
#define CONFIG_SENSOR_IIC_ADAPTER_ID_0    1
#define CONFIG_SENSOR_POWER_PIN_0         INVALID_GPIO
#define CONFIG_SENSOR_RESET_PIN_0         INVALID_GPIO
#define CONFIG_SENSOR_POWERDN_PIN_0       RK29_PIN6_PB7
#define CONFIG_SENSOR_FALSH_PIN_0         INVALID_GPIO
#define CONFIG_SENSOR_POWERACTIVE_LEVEL_0 RK29_CAM_POWERACTIVE_L
#define CONFIG_SENSOR_RESETACTIVE_LEVEL_0 RK29_CAM_RESETACTIVE_L
#define CONFIG_SENSOR_POWERDNACTIVE_LEVEL_0 RK29_CAM_POWERDNACTIVE_H
#define CONFIG_SENSOR_FLASHACTIVE_LEVEL_0 RK29_CAM_FLASHACTIVE_L

#define CONFIG_SENSOR_1 RK29_CAM_SENSOR_OV2659                      /* front camera sensor */
#define CONFIG_SENSOR_IIC_ADDR_1 	    0x60
#define CONFIG_SENSOR_IIC_ADAPTER_ID_1    1
#define CONFIG_SENSOR_POWER_PIN_1         INVALID_GPIO
#define CONFIG_SENSOR_RESET_PIN_1         INVALID_GPIO
#define CONFIG_SENSOR_POWERDN_PIN_1       RK29_PIN5_PD7
#define CONFIG_SENSOR_FALSH_PIN_1         INVALID_GPIO
#define CONFIG_SENSOR_POWERACTIVE_LEVEL_1 RK29_CAM_POWERACTIVE_L
#define CONFIG_SENSOR_RESETACTIVE_LEVEL_1 RK29_CAM_RESETACTIVE_L
#define CONFIG_SENSOR_POWERDNACTIVE_LEVEL_1 RK29_CAM_POWERDNACTIVE_H
#define CONFIG_SENSOR_FLASHACTIVE_LEVEL_1 RK29_CAM_FLASHACTIVE_L
#endif  //#ifdef CONFIG_VIDEO_RK29
/*---------------- Camera Sensor Configuration Macro End------------------------*/
#include "../../../drivers/media/video/rk29_camera.c"
/*---------------- Camera Sensor Macro Define End  ------------------------*/


/* Set memory size of pmem */
#ifdef CONFIG_RK29_MEM_SIZE_M
#define SDRAM_SIZE          (CONFIG_RK29_MEM_SIZE_M * SZ_1M)
#else
#define SDRAM_SIZE          SZ_512M
#endif
#define PMEM_GPU_SIZE       SZ_64M
#define PMEM_UI_SIZE        SZ_32M
#define PMEM_VPU_SIZE       SZ_64M
#define PMEM_CAM_SIZE       PMEM_CAM_NECESSARY
#ifdef CONFIG_VIDEO_RK29_WORK_IPP
#define MEM_CAMIPP_SIZE     SZ_4M
#else
#define MEM_CAMIPP_SIZE     0
#endif
#define MEM_FB_SIZE         (3*SZ_2M)
#ifdef CONFIG_FB_WORK_IPP
#define MEM_FBIPP_SIZE      SZ_8M   //1920 x 1080 x 2 x 2  //RGB565 = x2;RGB888 = x4
#else
#define MEM_FBIPP_SIZE      0
#endif
#if SDRAM_SIZE > SZ_512M
#define PMEM_GPU_BASE       (RK29_SDRAM_PHYS + SZ_512M - PMEM_GPU_SIZE)
#else
#define PMEM_GPU_BASE       (RK29_SDRAM_PHYS + SDRAM_SIZE - PMEM_GPU_SIZE)
#endif
#define PMEM_UI_BASE        (PMEM_GPU_BASE - PMEM_UI_SIZE)
#define PMEM_VPU_BASE       (PMEM_UI_BASE - PMEM_VPU_SIZE)
#define PMEM_CAM_BASE       (PMEM_VPU_BASE - PMEM_CAM_SIZE)
#define MEM_CAMIPP_BASE     (PMEM_CAM_BASE - MEM_CAMIPP_SIZE)
#define MEM_FB_BASE         (MEM_CAMIPP_BASE - MEM_FB_SIZE)
#define MEM_FBIPP_BASE      (MEM_FB_BASE - MEM_FBIPP_SIZE)
#define LINUX_SIZE          (MEM_FBIPP_BASE - RK29_SDRAM_PHYS)

#define PREALLOC_WLAN_SEC_NUM           4
#define PREALLOC_WLAN_BUF_NUM           160
#define PREALLOC_WLAN_SECTION_HEADER    24

#define WLAN_SECTION_SIZE_0     (PREALLOC_WLAN_BUF_NUM * 128)
#define WLAN_SECTION_SIZE_1     (PREALLOC_WLAN_BUF_NUM * 128)
#define WLAN_SECTION_SIZE_2     (PREALLOC_WLAN_BUF_NUM * 512)
#define WLAN_SECTION_SIZE_3     (PREALLOC_WLAN_BUF_NUM * 1024)

#define WLAN_SKB_BUF_NUM        16

static struct sk_buff *wlan_static_skb[WLAN_SKB_BUF_NUM];

struct wifi_mem_prealloc {
        void *mem_ptr;
        unsigned long size;
};

extern struct sys_timer rk29_timer;

static int rk29_nand_io_init(void)
{
    return 0;
}

struct rk29_nand_platform_data rk29_nand_data = {
    .width      = 1,     /* data bus width in bytes */
    .hw_ecc     = 1,     /* hw ecc 0: soft ecc */
    .num_flash    = 1,
    .io_init   = rk29_nand_io_init,
};

#ifdef CONFIG_FB_RK29
/*****************************************************************************************
 * lcd  devices
 * author: zyw@rock-chips.com
 *****************************************************************************************/
//#ifdef  CONFIG_LCD_TD043MGEA1
#define LCD_RXD_PIN          RK29_PIN2_PC7
#define LCD_TXD_PIN          RK29_PIN2_PC6
#define LCD_CLK_PIN          RK29_PIN2_PC4
#define LCD_CS_PIN           RK29_PIN2_PC5
/*****************************************************************************************
* frame buffer  devices
* author: zyw@rock-chips.com
*****************************************************************************************/
#define FB_ID                       0
#define FB_DISPLAY_ON_PIN           INVALID_GPIO//RK29_PIN6_PD0
#define FB_LCD_STANDBY_PIN          INVALID_GPIO//RK29_PIN6_PD1
#define FB_LCD_CABC_EN_PIN          INVALID_GPIO//RK29_PIN6_PD2
#define FB_MCU_FMK_PIN              INVALID_GPIO

#define FB_DISPLAY_ON_VALUE         GPIO_HIGH
#define FB_LCD_STANDBY_VALUE        GPIO_HIGH

//#endif
static int rk29_lcd_io_init(void)
{
    int ret = 0;
    //printk("rk29_lcd_io_init\n");
    //ret = gpio_request(LCD_RXD_PIN, NULL);
    ret = gpio_request(LCD_TXD_PIN, NULL);
	ret = gpio_request(LCD_CLK_PIN, NULL);
	ret = gpio_request(LCD_CS_PIN, NULL);
	//rk29_mux_api_set(GPIO2C7_SPI1RXD_NAME,GPIO2H_GPIO2C7);
	rk29_mux_api_set(GPIO2C6_SPI1TXD_NAME,GPIO2H_GPIO2C6);
	rk29_mux_api_set(GPIO2C5_SPI1CSN0_NAME,GPIO2H_GPIO2C5);
	rk29_mux_api_set(GPIO2C4_SPI1CLK_NAME,GPIO2H_GPIO2C4);
    return ret;
}

static int rk29_lcd_io_deinit(void)
{
    int ret = 0;
    //printk("rk29_lcd_io_deinit\n");
    gpio_free(LCD_CS_PIN);
	gpio_free(LCD_CLK_PIN);
	gpio_free(LCD_TXD_PIN);
	//gpio_free(LCD_RXD_PIN);
	//rk29_mux_api_set(GPIO2C7_SPI1RXD_NAME,GPIO2H_SPI1_RXD);
	rk29_mux_api_set(GPIO2C6_SPI1TXD_NAME,GPIO2H_SPI1_TXD);
	rk29_mux_api_set(GPIO2C5_SPI1CSN0_NAME,GPIO2H_SPI1_CSN0);
	rk29_mux_api_set(GPIO2C4_SPI1CLK_NAME,GPIO2H_SPI1_CLK);
    return ret;
}

static struct rk29lcd_info rk29_lcd_info = {
    .txd_pin  = LCD_TXD_PIN,
    .clk_pin = LCD_CLK_PIN,
    .cs_pin = LCD_CS_PIN,
    .io_init   = rk29_lcd_io_init,
    .io_deinit = rk29_lcd_io_deinit,
};

int rk29_fb_io_enable(void)
{
    if(FB_DISPLAY_ON_PIN != INVALID_GPIO)
    {
        gpio_direction_output(FB_DISPLAY_ON_PIN, 0);
        gpio_set_value(FB_DISPLAY_ON_PIN, FB_DISPLAY_ON_VALUE);              
    }
    if(FB_LCD_STANDBY_PIN != INVALID_GPIO)
    {
        gpio_direction_output(FB_LCD_STANDBY_PIN, 0);
        gpio_set_value(FB_LCD_STANDBY_PIN, FB_LCD_STANDBY_VALUE);             
    }
    return 0;
}

int rk29_fb_io_disable(void)
{
    if(FB_DISPLAY_ON_PIN != INVALID_GPIO)
    {
        gpio_direction_output(FB_DISPLAY_ON_PIN, 0);
        gpio_set_value(FB_DISPLAY_ON_PIN, !FB_DISPLAY_ON_VALUE);              
    }
    if(FB_LCD_STANDBY_PIN != INVALID_GPIO)
    {
        gpio_direction_output(FB_LCD_STANDBY_PIN, 0);
        gpio_set_value(FB_LCD_STANDBY_PIN, !FB_LCD_STANDBY_VALUE);             
    }
    return 0;
}

static int rk29_fb_io_init(struct rk29_fb_setting_info *fb_setting)
{
    int ret = 0;
    if(fb_setting->mcu_fmk_en && (FB_MCU_FMK_PIN != INVALID_GPIO))
    {
        ret = gpio_request(FB_MCU_FMK_PIN, NULL);
        if(ret != 0)
        {
            gpio_free(FB_MCU_FMK_PIN);
            printk(">>>>>> FB_MCU_FMK_PIN gpio_request err \n ");
        }
        gpio_direction_input(FB_MCU_FMK_PIN);
    }
    if(fb_setting->disp_on_en && (FB_DISPLAY_ON_PIN != INVALID_GPIO))
    {
        ret = gpio_request(FB_DISPLAY_ON_PIN, NULL);
        if(ret != 0)
        {
            gpio_free(FB_DISPLAY_ON_PIN);
            printk(">>>>>> FB_DISPLAY_ON_PIN gpio_request err \n ");
        }
    }

    if(fb_setting->disp_on_en && (FB_LCD_STANDBY_PIN != INVALID_GPIO))
    {
        ret = gpio_request(FB_LCD_STANDBY_PIN, NULL);
        if(ret != 0)
        {
            gpio_free(FB_LCD_STANDBY_PIN);
            printk(">>>>>> FB_LCD_STANDBY_PIN gpio_request err \n ");
        }
    }

    if(FB_LCD_CABC_EN_PIN != INVALID_GPIO)
    {
        ret = gpio_request(FB_LCD_CABC_EN_PIN, NULL);
        if(ret != 0)
        {
            gpio_free(FB_LCD_CABC_EN_PIN);
            printk(">>>>>> FB_LCD_CABC_EN_PIN gpio_request err \n ");
        }
        gpio_direction_output(FB_LCD_CABC_EN_PIN, 0);
        gpio_set_value(FB_LCD_CABC_EN_PIN, GPIO_LOW);
    }
    rk29_fb_io_enable();   //enable it

    return ret;
}

static struct rk29fb_info rk29_fb_info = {
    .fb_id   = FB_ID,
    .mcu_fmk_pin = FB_MCU_FMK_PIN,
    .lcd_info = &rk29_lcd_info,
    .io_init   = rk29_fb_io_init,
    .io_enable = rk29_fb_io_enable,
    .io_disable = rk29_fb_io_disable,
};

/* rk29 fb resource */
static struct resource rk29_fb_resource[] = {
	[0] = {
        .name  = "lcdc reg",
		.start = RK29_LCDC_PHYS,
		.end   = RK29_LCDC_PHYS + RK29_LCDC_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
	    .name  = "lcdc irq",
		.start = IRQ_LCDC,
		.end   = IRQ_LCDC,
		.flags = IORESOURCE_IRQ,
	},
	[2] = {
	    .name   = "win1 buf",
        .start  = MEM_FB_BASE,
        .end    = MEM_FB_BASE + MEM_FB_SIZE - 1,
        .flags  = IORESOURCE_MEM,
    },
    #ifdef CONFIG_FB_WORK_IPP
    [3] = {
	    .name   = "win1 ipp buf",
        .start  = MEM_FBIPP_BASE,
        .end    = MEM_FBIPP_BASE + MEM_FBIPP_SIZE - 1,
        .flags  = IORESOURCE_MEM,
    },
    #endif
};

/*platform_device*/
struct platform_device rk29_device_fb = {
	.name		  = "rk29-fb",
	.id		  = 4,
	.num_resources	  = ARRAY_SIZE(rk29_fb_resource),
	.resource	  = rk29_fb_resource,
	.dev            = {
		.platform_data  = &rk29_fb_info,
	}
};

struct platform_device rk29_device_dma_cpy = {
	.name		  = "dma_memcpy",
	.id		  = 4,

};

#endif

static struct android_pmem_platform_data android_pmem_pdata = {
	.name		= "pmem",
	.start		= PMEM_UI_BASE,
	.size		= PMEM_UI_SIZE,
	.no_allocator	= 0,
	.cached		= 1,
};

static struct platform_device android_pmem_device = {
	.name		= "android_pmem",
	.id		= 0,
	.dev		= {
		.platform_data = &android_pmem_pdata,
	},
};


static struct vpu_mem_platform_data vpu_mem_pdata = {
	.name		= "vpu_mem",
	.start		= PMEM_VPU_BASE,
	.size		= PMEM_VPU_SIZE,
	.cached		= 1,
};

static struct platform_device rk29_vpu_mem_device = {
	.name		= "vpu_mem",
	.id		    = 2,
	.dev		= {
	.platform_data = &vpu_mem_pdata,
	},
};

static struct platform_device rk29_v4l2_output_devce = {
	.name		= "rk29_vout",
};

/* HANNSTAR_P1003 touch I2C */
#if defined (CONFIG_HANNSTAR_P1003)
#define TOUCH_RESET_PIN RK29_PIN6_PC3
#define TOUCH_INT_PIN   RK29_PIN4_PD5

int p1003_init_platform_hw(void)
{
    if(gpio_request(TOUCH_RESET_PIN,NULL) != 0){
      gpio_free(TOUCH_RESET_PIN);
      printk("p1003_init_platform_hw gpio_request error\n");
      return -EIO;
    }

    if(gpio_request(TOUCH_INT_PIN,NULL) != 0){
      gpio_free(TOUCH_INT_PIN);
      printk("p1003_init_platform_hw gpio_request error\n");
      return -EIO;
    }
    gpio_pull_updown(TOUCH_INT_PIN, 1);
    gpio_direction_output(TOUCH_RESET_PIN, 0);
    msleep(500);
    gpio_set_value(TOUCH_RESET_PIN,GPIO_LOW);
    msleep(500);
    gpio_set_value(TOUCH_RESET_PIN,GPIO_HIGH);

    return 0;
}


struct p1003_platform_data p1003_info = {
  .model= 1003,
  .init_platform_hw= p1003_init_platform_hw,

};
#endif


#if defined(CONFIG_TOUCHSCREEN_GT801_IIC) 
#include "../../../drivers/input/touchscreen/gt801_ts.h"
#define GT801_GPIO_INT      RK29_PIN4_PD5
#define GT801_GPIO_RESET    RK29_PIN6_PC3
static struct gt801_platform_data gt801_info = {
	.model			= 801,
	.swap_xy		= 0,
	.x_min			= 0,
	.x_max			= 480,
	.y_min			= 0,
	.y_max			= 800,
	.gpio_reset     = GT801_GPIO_RESET,
	.gpio_reset_active_low = 0,
	.gpio_pendown		= GT801_GPIO_INT,
    .pendown_iomux_name = GPIO4D5_CPUTRACECTL_NAME,
    .resetpin_iomux_name = NULL,
    .pendown_iomux_mode = GPIO4H_GPIO4D5,
    .resetpin_iomux_mode = 0,
};
#endif


#if defined(CONFIG_TOUCHSCREEN_GT818_IIC)
#include "../../../drivers/input/touchscreen/gt818_ts.h"
#define GT818_GPIO_INT      RK29_PIN4_PD5
#define GT818_GPIO_RESET    RK29_PIN6_PC3
static struct gt818_platform_data gt818_info = {
	.model			= 818,
	.swap_xy		= 0,
	.x_min			= 0,
	.x_max			= 480,
	.y_min			= 0,
	.y_max			= 800,
	.gpio_reset     = GT818_GPIO_RESET,
	.gpio_reset_active_low = 0,
	.gpio_pendown		= GT818_GPIO_INT,
    .pendown_iomux_name = GPIO4D5_CPUTRACECTL_NAME,
    .resetpin_iomux_name = NULL,
    .pendown_iomux_mode = GPIO4H_GPIO4D5,
    .resetpin_iomux_mode = 0,
};
#endif

#if defined(CONFIG_TOUCHSCREEN_ILI2102_IIC) 
#include "../../../drivers/input/touchscreen/ili2102_ts.h"
#define GT801_GPIO_INT      RK29_PIN4_PD5
#define GT801_GPIO_RESET    RK29_PIN6_PC3
static struct ili2102_platform_data ili2102_info = {
	.model			= 2102,
	.swap_xy		= 0,
	.x_min			= 0,
	.x_max			= 481,
	.y_min			= 0,
	.y_max			= 801,
	.gpio_reset     = GT801_GPIO_RESET,
	.gpio_reset_active_low = 1,
	.gpio_pendown		= GT801_GPIO_INT,
    .pendown_iomux_name = GPIO4D5_CPUTRACECTL_NAME,
    .resetpin_iomux_name = NULL,
    .pendown_iomux_mode = GPIO4H_GPIO4D5,
    .resetpin_iomux_mode = 0,
};
#endif

/* EETI_EGALAX touch I2C */
#if defined (CONFIG_EETI_EGALAX)
#define TOUCH_RESET_PIN RK29_PIN6_PC3
#define TOUCH_INT_PIN   RK29_PIN4_PD5

static int EETI_EGALAX_init_platform_hw(void)
{
    if(gpio_request(TOUCH_RESET_PIN,NULL) != 0){
      gpio_free(TOUCH_RESET_PIN);
      printk("p1003_init_platform_hw gpio_request error\n");
      return -EIO;
    }

    if(gpio_request(TOUCH_INT_PIN,NULL) != 0){
      gpio_free(TOUCH_INT_PIN);
      printk("p1003_init_platform_hw gpio_request error\n");
      return -EIO;
    }
    gpio_pull_updown(TOUCH_INT_PIN, 1);
    gpio_direction_output(TOUCH_RESET_PIN, 0);
    msleep(500);
    gpio_set_value(TOUCH_RESET_PIN,GPIO_LOW);
    msleep(500);
    gpio_set_value(TOUCH_RESET_PIN,GPIO_HIGH);

    return 0;
}


static struct eeti_egalax_platform_data eeti_egalax_info = {
  .model= 1003,
  .init_platform_hw= EETI_EGALAX_init_platform_hw,

};
#endif

/*MMA8452 gsensor*/
#if defined (CONFIG_GS_MMA8452)
#define MMA8452_INT_PIN   RK29_PIN6_PC4

static int mma8452_init_platform_hw(void)
{

    if(gpio_request(MMA8452_INT_PIN,NULL) != 0){
      gpio_free(MMA8452_INT_PIN);
      printk("mma8452_init_platform_hw gpio_request error\n");
      return -EIO;
    }
    gpio_pull_updown(MMA8452_INT_PIN, 1);
    return 0;
}


static struct mma8452_platform_data mma8452_info = {
  .model= 8452,
  .swap_xy = 0,
  .init_platform_hw= mma8452_init_platform_hw,

};
#endif

#if defined (CONFIG_MPU_SENSORS_MPU3050)
/*mpu3050*/
static struct mpu3050_platform_data mpu3050_data = {
		.int_config = 0x10,
		//.orientation = { 1, 0, 0,0, -1, 0,0, 0, 1 },
		//.orientation = { 0, 1, 0,-1, 0, 0,0, 0, -1 },
		//.orientation = { -1, 0, 0,0, -1, 0,0, 0, -1 },
		//.orientation = { 0, 1, 0, -1, 0, 0, 0, 0, 1 },
		.orientation = { 1, 0, 0,0, 1, 0, 0, 0, 1 },
		.level_shifter = 0,
#if defined (CONFIG_MPU_SENSORS_KXTF9)
		.accel = {
#ifdef CONFIG_MPU_SENSORS_MPU3050_MODULE
				.get_slave_descr = NULL ,
#else
				.get_slave_descr = get_accel_slave_descr ,			
#endif
				.adapt_num = 0, // The i2c bus to which the mpu device is
				// connected
				//.irq = RK29_PIN6_PC4,
				.bus = EXT_SLAVE_BUS_SECONDARY,  //The secondary I2C of MPU
				.address = 0x0f,
				//.orientation = { 1, 0, 0,0, 1, 0,0, 0, 1 },
				//.orientation = { 0, -1, 0,-1, 0, 0,0, 0, -1 },
				//.orientation = { 0, 1, 0,1, 0, 0,0, 0, -1 },
				//.orientation = { 0, 1 ,0, -1 ,0, 0, 0, 0, 1 },
				.orientation = {1, 0, 0, 0, 1, 0, 0, 0, 1},
		},
#endif
#if defined (CONFIG_MPU_SENSORS_AK8975)
		.compass = {
#ifdef CONFIG_MPU_SENSORS_MPU3050_MODULE
				.get_slave_descr = NULL,/*ak5883_get_slave_descr,*/
#else
				.get_slave_descr = get_compass_slave_descr,
#endif						
				.adapt_num = 0, // The i2c bus to which the compass device is. 
				// It can be difference with mpu
				// connected
				//.irq = RK29_PIN6_PC5,
				.bus = EXT_SLAVE_BUS_PRIMARY,
				.address = 0x0d,
				//.orientation = { -1, 0, 0,0, -1, 0,0, 0, 1 },
				//.orientation = { 0, -1, 0,-1, 0, 0,0, 0, -1 },
				//.orientation = { 0, 1, 0,1, 0, 0,0, 0, -1 },
				//.orientation = { 0, -1, 0, 1, 0, 0, 0, 0, 1 },
				.orientation = {0, 1, 0, -1, 0, 0, 0, 0, 1},
		},
};
#endif
#endif

#if defined(CONFIG_GPIO_WM831X)
struct rk29_gpio_expander_info  wm831x_gpio_settinginfo[] = {
	{
		.gpio_num    		=WM831X_P01,// tp3
		.pin_type           = GPIO_OUT,
		.pin_value			=GPIO_HIGH,
	 },
	 
	 {
		.gpio_num    		=WM831X_P02,//tp4
		.pin_type           = GPIO_OUT,
		.pin_value			=GPIO_HIGH,
	 },
	 {
		.gpio_num    		=WM831X_P03,//tp2
		.pin_type           = GPIO_OUT,
		.pin_value			=GPIO_HIGH,
	 },
	 {
		.gpio_num    		=WM831X_P04,//tp1
		.pin_type           = GPIO_OUT,
		.pin_value			=GPIO_HIGH,
	 },
	 {
		.gpio_num    		=WM831X_P05,//tp1
		.pin_type           = GPIO_OUT,
		.pin_value			=GPIO_HIGH,
	 },
	 {
		.gpio_num    		=WM831X_P06,//tp1
		.pin_type           = GPIO_OUT,
		.pin_value			=GPIO_HIGH,
	 },
	 {
		.gpio_num    		=WM831X_P07,//tp1
		.pin_type           = GPIO_OUT,
		.pin_value			=GPIO_HIGH,
	 },
	 {
		.gpio_num    		=WM831X_P08,//tp1
		.pin_type           = GPIO_OUT,
		.pin_value			=GPIO_HIGH,
	 },
	 {
		.gpio_num    		=WM831X_P09,//tp1
		.pin_type           = GPIO_OUT,
		.pin_value			=GPIO_HIGH,
	 },
	 {
		.gpio_num    		=WM831X_P10,//tp1
		.pin_type           = GPIO_OUT,
		.pin_value			=GPIO_HIGH,
	 },
	 {
		.gpio_num    		=WM831X_P11,//tp1
		.pin_type           = GPIO_OUT,
		.pin_value			=GPIO_HIGH,
	 },	
	 {
		.gpio_num    		=WM831X_P12,
		.pin_type           = GPIO_OUT,
		.pin_value			=GPIO_HIGH,
	 },
};

#endif



#if defined(CONFIG_MFD_WM831X)
static struct wm831x *gWm831x;
int wm831x_pre_init(struct wm831x *parm)
{
	int ret;
	printk("%s\n", __FUNCTION__);
	gWm831x = parm;
	//ILIM = 900ma
	ret = wm831x_reg_read(parm, WM831X_POWER_STATE) & 0xffff;
	wm831x_reg_write(parm, WM831X_POWER_STATE, (ret&0xfff8) | 0x04);	
	
	//BATT_FET_ENA = 1
	wm831x_set_bits(parm, WM831X_RESET_CONTROL,0x1000,0x1000);
	ret = wm831x_reg_read(parm, WM831X_RESET_CONTROL) & 0xffff;
	printk("%s:WM831X_RESET_CONTROL=0x%x\n",__FUNCTION__,ret);
	
#if 0
	wm831x_set_bits(parm, WM831X_LDO_ENABLE, (1 << 3), 0);
	wm831x_set_bits(parm, WM831X_LDO_ENABLE, (1 << 7), 0);
	printk("%s:disable ldo4 and ldo8 because they are enabled in uboot\n",__FUNCTION__);
#endif	
	return 0;
}
int wm831x_post_init(struct wm831x *parm)
{
	struct regulator *dcdc;
	struct regulator *ldo;
	
	dcdc = regulator_get(NULL, "dcdc3");		// 1th IO
	regulator_set_voltage(dcdc,3000000,3000000);
	regulator_set_suspend_voltage(dcdc, 2800000);
	regulator_enable(dcdc);			
	printk("%s set dcdc3=%dmV end\n", __FUNCTION__, regulator_get_voltage(dcdc));
	regulator_put(dcdc);
	udelay(100);
	
	ldo = regulator_get(NULL, "ldo10");	// 1th modem IO
	regulator_set_voltage(ldo,3000000,3000000);
	regulator_set_suspend_voltage(ldo,3000000);
	regulator_enable(ldo);			
	printk("%s set ldo10=%dmV end\n", __FUNCTION__, regulator_get_voltage(ldo));
	regulator_put(ldo);
	udelay(100);
	
	dcdc = regulator_get(NULL, "dcdc2");	// 2th CORE
	regulator_set_voltage(dcdc,1300000,1300000);
	regulator_set_suspend_voltage(dcdc,1000000);
	regulator_enable(dcdc);				
	printk("%s set dcdc2=%dmV end\n", __FUNCTION__, regulator_get_voltage(dcdc));
	regulator_put(dcdc);
	udelay(100);
	
	dcdc = regulator_get(NULL, "dcdc1");	// 3th ddr
	regulator_set_voltage(dcdc,1800000,1800000);
	regulator_set_suspend_voltage(ldo, 1800000);
	regulator_enable(dcdc);
	printk("%s set dcdc1=%dmV end\n", __FUNCTION__, regulator_get_voltage(dcdc));	
	regulator_put(dcdc);
	udelay(100);
	
	ldo = regulator_get(NULL, "ldo1");		// 3th nand
	regulator_set_voltage(ldo,1800000,1800000);
	regulator_set_suspend_voltage(ldo,1800000);
	regulator_enable(ldo);			
	printk("%s set ldo1=%dmV end\n", __FUNCTION__, regulator_get_voltage(ldo));		
	regulator_put(ldo);
	udelay(100);
	
	ldo = regulator_get(NULL, "ldo4");		// 4th usb
	regulator_set_voltage(ldo,2500000,2500000);
	regulator_set_suspend_voltage(ldo,0000000);
	regulator_enable(ldo);	
	printk("%s set ldo4=%dmV end\n", __FUNCTION__, regulator_get_voltage(ldo));
	regulator_put(ldo);
	udelay(100);
	
	ldo = regulator_get(NULL, "ldo7");		// 5th usb
	regulator_set_voltage(ldo,3300000,3300000);
	regulator_set_suspend_voltage(ldo,3300000);
	regulator_enable(ldo);			
	printk("%s set ldo7=%dmV end\n", __FUNCTION__, regulator_get_voltage(ldo));
	regulator_put(ldo);
	udelay(100);
	
	dcdc = regulator_get(NULL, "dcdc4");	// backlight
	regulator_set_voltage(dcdc,20000000,20000000);
	regulator_set_suspend_voltage(dcdc, 20000000);
	regulator_enable(dcdc);	
	printk("%s set dcdc4=%dmV end\n", __FUNCTION__, regulator_get_voltage(dcdc));
	regulator_put(dcdc);
	udelay(100);
#if 1
	
	ldo = regulator_get(NULL, "ldo2");		//lcd
	regulator_set_voltage(ldo,3000000,3000000);
	regulator_set_suspend_voltage(ldo,3000000);
	regulator_enable(ldo);			
	printk("%s set ldo2=%dmV end\n", __FUNCTION__, regulator_get_voltage(ldo));
	regulator_put(ldo);


	ldo = regulator_get(NULL, "ldo5");		//tf
	regulator_set_voltage(ldo,3000000,3000000);
	regulator_set_suspend_voltage(ldo,3000000);
	regulator_enable(ldo);			
	printk("%s set ldo5=%dmV end\n", __FUNCTION__, regulator_get_voltage(ldo));
	regulator_put(ldo);

	ldo = regulator_get(NULL, "ldo6");		//camera
	regulator_set_voltage(ldo,1800000,1800000);
	regulator_set_suspend_voltage(ldo,1800000);
	regulator_enable(ldo);			
	printk("%s set ldo6=%dmV end\n", __FUNCTION__, regulator_get_voltage(ldo));
	regulator_put(ldo);



#if 0	
	ldo = regulator_get(NULL, "ldo3");		//sram
	regulator_set_voltage(ldo,1800000,1800000);
	regulator_set_suspend_voltage(ldo,1800000);
	regulator_enable(ldo);			
	printk("%s set ldo3=%dmV end\n", __FUNCTION__, regulator_get_voltage(ldo));
	regulator_put(ldo);	


	ldo = regulator_get(NULL, "ldo8");		//cmmb
	regulator_set_voltage(ldo,1200000,1200000);
	regulator_set_suspend_voltage(ldo,1200000);
	regulator_enable(ldo);			
	printk("%s set ldo8=%dmV end\n", __FUNCTION__, regulator_get_voltage(ldo));
	regulator_put(ldo);

	ldo = regulator_get(NULL, "ldo9");		//cmmb
	regulator_set_voltage(ldo,3000000,3000000);
	regulator_set_suspend_voltage(ldo,3000000);
	regulator_enable(ldo);			
	printk("%s set ldo9=%dmV end\n", __FUNCTION__, regulator_get_voltage(ldo));
	regulator_put(ldo);
#endif

#endif
	
	ldo = regulator_get(NULL, "ldo11");
	//regulator_enable(ldo);			
	printk("%s set ldo11=%dmV end\n", __FUNCTION__, regulator_get_voltage(ldo));	
	regulator_put(ldo);	
	
		
	return 0;
}

extern void wm831x_enter_sleep(void);
extern void wm831x_exit_sleep(void);

void pmu_wm831x_set_suspend_voltage(void)
{

}
EXPORT_SYMBOL_GPL(pmu_wm831x_set_suspend_voltage);

void pmu_wm831x_set_resume_voltage(void)
{

}
EXPORT_SYMBOL_GPL(pmu_wm831x_set_resume_voltage);

int wm831x_last_deinit(struct wm831x *parm)
{
	struct regulator* ldo;

	printk("%s\n", __FUNCTION__);

	ldo = regulator_get(NULL, "ldo1");
	regulator_disable(ldo);			
	regulator_put(ldo);
	
	ldo = regulator_get(NULL, "ldo2");
	regulator_disable(ldo);			
	regulator_put(ldo);
	
	ldo = regulator_get(NULL, "ldo3");
	regulator_disable(ldo);			
	regulator_put(ldo);
	
	ldo = regulator_get(NULL, "ldo4");
	//regulator_disable(ldo);	
	regulator_put(ldo);

	ldo = regulator_get(NULL, "ldo5");
	regulator_disable(ldo);			
	regulator_put(ldo);

	ldo = regulator_get(NULL, "ldo6");
	regulator_disable(ldo);			
	regulator_put(ldo);

	ldo = regulator_get(NULL, "ldo7");
	regulator_disable(ldo);			
	regulator_put(ldo);

	ldo = regulator_get(NULL, "ldo8");
	//regulator_disable(ldo);			
	regulator_put(ldo);

	ldo = regulator_get(NULL, "ldo9");
	regulator_disable(ldo);			
	regulator_put(ldo);

	ldo = regulator_get(NULL, "ldo10");
	regulator_disable(ldo);						
	regulator_put(ldo);

	return 0;
}

struct wm831x_backlight_pdata wm831x_backlight_platdata = {
	.isink = 1,     /** ISINK to use, 1 or 2 */
	.max_uA = 19484,    /** Maximum current to allow */
};

struct wm831x_backup_pdata wm831x_backup_platdata = {
	.charger_enable = 1,
	.no_constant_voltage = 0,  /** Disable constant voltage charging */
	.vlim = 3100,   /** Voltage limit in milivolts */
	.ilim = 300,   /** Current limit in microamps */
};

struct wm831x_battery_pdata wm831x_battery_platdata = {
	.enable = 1,         /** Enable charging */
	.fast_enable = 1,    /** Enable fast charging */
	.off_mask = 1,       /** Mask OFF while charging */
	.trickle_ilim = 200,   /** Trickle charge current limit, in mA */
	.vsel = 4200,           /** Target voltage, in mV */
	.eoc_iterm = 90,      /** End of trickle charge current, in mA */
	.fast_ilim = 500,      /** Fast charge current limit, in mA */
	.timeout = 240,        /** Charge cycle timeout, in minutes */
	.syslo = 3300,    /* syslo threshold, in mV*/
	.sysok = 3500,    /* sysko threshold, in mV*/
};

struct wm831x_status_pdata wm831x_status_platdata[WM831X_MAX_STATUS] = {
	{
	.default_src = WM831X_STATUS_OTP,
	.name = "wm831x_status0",
	.default_trigger = "wm831x_otp",
	},
	{
	.default_src = WM831X_STATUS_POWER,
	.name = "wm831x_status1",
	.default_trigger = "wm831x_power",
	},	
};


static struct regulator_consumer_supply dcdc1_consumers[] = {
	{
		.supply = "dcdc1",
	}
};
static struct regulator_consumer_supply dcdc2_consumers[] = {
	{
		.supply = "dcdc2",
	},
	{
		.supply = "vcore",
	}
};
static struct regulator_consumer_supply dcdc3_consumers[] = {
	{
		.supply = "dcdc3",
	}
};
static struct regulator_consumer_supply dcdc4_consumers[] = {
	{
		.supply = "dcdc4",
	}
};
static struct regulator_consumer_supply epe1_consumers[] = {
	{
		.supply = "epe1",
	}
};
static struct regulator_consumer_supply epe2_consumers[] = {
	{
		.supply = "epe2",
	}
};
static struct regulator_consumer_supply ldo1_consumers[] = {
	{
		.supply = "ldo1",
	}
};
static struct regulator_consumer_supply ldo2_consumers[] = {
	{
		.supply = "ldo2",
	}
};
static struct regulator_consumer_supply ldo3_consumers[] = {
	{
		.supply = "ldo3",
	}
};
static struct regulator_consumer_supply ldo4_consumers[] = {
	{
		.supply = "ldo4",
	}
};
static struct regulator_consumer_supply ldo5_consumers[] = {
	{
		.supply = "ldo5",
	}
};
static struct regulator_consumer_supply ldo6_consumers[] = {
	{
		.supply = "ldo6",
	}
};
static struct regulator_consumer_supply ldo7_consumers[] = {
	{
		.supply = "ldo7",
	}
};
static struct regulator_consumer_supply ldo8_consumers[] = {
	{
		.supply = "ldo8",
	}
};
static struct regulator_consumer_supply ldo9_consumers[] = {
	{
		.supply = "ldo9",
	}
};
static struct regulator_consumer_supply ldo10_consumers[] = {
	{
		.supply = "ldo10",
	}
};
static struct regulator_consumer_supply ldo11_consumers[] = {
	{
		.supply = "ldo11",
	}
};
static struct regulator_consumer_supply isink1_consumers[] = {
	{
		.supply = "isink1",
	}
};
static struct regulator_consumer_supply isink2_consumers[] = {
	{
		.supply = "isink2",
	}
};

struct regulator_init_data wm831x_regulator_init_dcdc[WM831X_MAX_DCDC] = {
	{
		.constraints = {
			.name = "DCDC1",
			.min_uV = 600000,
			.max_uV = 1800000,//0.6-1.8V
			.apply_uV = true,		
			.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE,		
		},
		.num_consumer_supplies = ARRAY_SIZE(dcdc1_consumers),
		.consumer_supplies = dcdc1_consumers,
	},
	{
		.constraints = {
			.name = "DCDC2",
			.min_uV = 600000,
			.max_uV = 1800000,//0.6-1.8V
			.apply_uV = true,		
			.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE,		
		},
		.num_consumer_supplies = ARRAY_SIZE(dcdc2_consumers),
		.consumer_supplies = dcdc2_consumers,
	},
	{
		.constraints = {
			.name = "DCDC3",
			.min_uV = 850000,
			.max_uV = 3400000,//0.85-3.4V
			.apply_uV = true,		
			.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE,		
		},
		.num_consumer_supplies = ARRAY_SIZE(dcdc3_consumers),
		.consumer_supplies = dcdc3_consumers,
	},
	{
		.constraints = {
			.name = "DCDC4",
			.min_uV = 00000000,
			.max_uV = 30000000,//30V/40mA
			.apply_uV = true,		
			.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE,		
		},
		.num_consumer_supplies = ARRAY_SIZE(dcdc4_consumers),
		.consumer_supplies = dcdc4_consumers,
	},
	
};
struct regulator_init_data wm831x_regulator_init_epe[WM831X_MAX_EPE] = {
	{
		.constraints = {
			.name = "EPE1",
			.min_uV = 1200000,
			.max_uV = 3000000,
			.apply_uV = true,		
			.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE,		
		},
		.num_consumer_supplies = ARRAY_SIZE(epe1_consumers),
		.consumer_supplies = epe1_consumers,
	},
	{
		.constraints = {
			.name = "EPE2",
			.min_uV = 1200000,
			.max_uV = 3000000,
			.apply_uV = true,		
			.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE,		
		},
		.num_consumer_supplies = ARRAY_SIZE(epe2_consumers),
		.consumer_supplies = epe2_consumers,
	},
};

struct regulator_init_data wm831x_regulator_init_ldo[WM831X_MAX_LDO] = {
	{
		.constraints = {
			.name = "LDO1",
			.min_uV = 900000,
			.max_uV = 3300000,
			.apply_uV = true,		
			.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE,		
		},
		.num_consumer_supplies = ARRAY_SIZE(ldo1_consumers),
		.consumer_supplies = ldo1_consumers,
	},
	{
		.constraints = {
			.name = "LDO2",
			.min_uV = 900000,
			.max_uV = 3300000,
			.apply_uV = true,		
			.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE,		
		},
		.num_consumer_supplies = ARRAY_SIZE(ldo2_consumers),
		.consumer_supplies = ldo2_consumers,
	},
	{
		.constraints = {
			.name = "LDO3",
			.min_uV = 900000,
			.max_uV = 3300000,
			.apply_uV = true,		
			.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE,		
		},
		.num_consumer_supplies = ARRAY_SIZE(ldo3_consumers),
		.consumer_supplies = ldo3_consumers,
	},
	{
		.constraints = {
			.name = "LDO4",
			.min_uV = 900000,
			.max_uV = 3300000,
			.apply_uV = true,		
			.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE,		
		},
		.num_consumer_supplies = ARRAY_SIZE(ldo4_consumers),
		.consumer_supplies = ldo4_consumers,
	},
	{
		.constraints = {
			.name = "LDO5",
			.min_uV = 900000,
			.max_uV = 3300000,
			.apply_uV = true,		
			.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE,		
		},
		.num_consumer_supplies = ARRAY_SIZE(ldo5_consumers),
		.consumer_supplies = ldo5_consumers,
	},
	{
		.constraints = {
			.name = "LDO6",
			.min_uV = 900000,
			.max_uV = 3300000,
			.apply_uV = true,		
			.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE,		
		},
		.num_consumer_supplies = ARRAY_SIZE(ldo6_consumers),
		.consumer_supplies = ldo6_consumers,
	},
	{
		.constraints = {
			.name = "LDO7",
			.min_uV = 1000000,
			.max_uV = 3500000,
			.apply_uV = true,		
			.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE,		
		},
		.num_consumer_supplies = ARRAY_SIZE(ldo7_consumers),
		.consumer_supplies = ldo7_consumers,
	},
	{
		.constraints = {
			.name = "LDO8",
			.min_uV = 1000000,
			.max_uV = 3500000,
			.apply_uV = true,		
			.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE,		
		},
		.num_consumer_supplies = ARRAY_SIZE(ldo8_consumers),
		.consumer_supplies = ldo8_consumers,
	},
	{
		.constraints = {
			.name = "LDO9",
			.min_uV = 1000000,
			.max_uV = 3500000,
			.apply_uV = true,		
			.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE,		
		},
		.num_consumer_supplies = ARRAY_SIZE(ldo9_consumers),
		.consumer_supplies = ldo9_consumers,
	},
	{
		.constraints = {
			.name = "LDO10",
			.min_uV = 1000000,
			.max_uV = 3500000,
			.apply_uV = true,		
			.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE,		
		},
		.num_consumer_supplies = ARRAY_SIZE(ldo10_consumers),
		.consumer_supplies = ldo10_consumers,
	},
	{
		.constraints = {
			.name = "LDO11",
			.min_uV = 1200000,
			.max_uV = 3000000,
			.apply_uV = true,		
			.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE,		
		},
		.num_consumer_supplies = ARRAY_SIZE(ldo11_consumers),
		.consumer_supplies = ldo11_consumers,
	},
};

struct regulator_init_data wm831x_regulator_init_isink[WM831X_MAX_ISINK] = {
	{
		.constraints = {
			.name = "ISINK1",
			.min_uA = 00000,
			.max_uA = 40000,
			.always_on = true,
			.apply_uV = true,		
			.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_CURRENT,		
		},
		.num_consumer_supplies = ARRAY_SIZE(isink1_consumers),
		.consumer_supplies = isink1_consumers,
	},
	{
		.constraints = {
			.name = "ISINK2",
			.min_uA = 0000000,
			.max_uA = 0000000,
			.apply_uV = false,		
			.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_CURRENT,		
		},
		.num_consumer_supplies = ARRAY_SIZE(isink2_consumers),
		.consumer_supplies = isink2_consumers,
	},
};

static int wm831x_checkrange(int start,int num,int val)
{   
	if((val<(start+num))&&(val>=start))
		return 0;
	else 
		return -1;
}

static int wm831x_init_pin_type(struct wm831x *wm831x)
{
#if 1
	struct wm831x_pdata *pdata = wm831x->dev->platform_data;
	struct rk29_gpio_expander_info *wm831x_gpio_settinginfo;
	uint16_t offset = 0;
	uint16_t wm831x_settingpin_num = 0;
	uint16_t ret = 0;
	int i = 0;

	if(wm831x)
	{
		wm831x_gpio_settinginfo=pdata->settinginfo;
		if(wm831x_gpio_settinginfo)
		{
			wm831x_settingpin_num = pdata->settinginfolen;
			for(i=0;i<wm831x_settingpin_num;i++)
			{
				if(!wm831x_checkrange(pdata->gpio_base,pdata->gpio_pin_num,wm831x_gpio_settinginfo[i].gpio_num))
				{
					offset = wm831x_gpio_settinginfo[i].gpio_num - pdata->gpio_base;

					if(wm831x_gpio_settinginfo[i].pin_type==GPIO_IN)
					{
						wm831x_set_bits(wm831x,(WM831X_GPIO1_CONTROL+offset), WM831X_GPN_DIR_MASK|WM831X_GPN_TRI_MASK, 1<<WM831X_GPN_DIR_SHIFT|1<<WM831X_GPN_TRI_SHIFT);
					}
					else
					{
						wm831x_set_bits(wm831x,(WM831X_GPIO1_CONTROL+offset), WM831X_GPN_DIR_MASK|WM831X_GPN_TRI_MASK, 1<<WM831X_GPN_TRI_SHIFT);
						if(wm831x_gpio_settinginfo[i].pin_value==GPIO_HIGH)
						{
							wm831x_set_bits(wm831x, WM831X_GPIO_LEVEL, (1 << offset),(1 << offset));
						}
						else
						{
							wm831x_set_bits(wm831x, WM831X_GPIO_LEVEL, (1 << offset),(0 << offset));
						}
					}
					
				}
			}
		}
	}

	for(i=0;i<pdata->gpio_pin_num;i++)
	{
		wm831x_set_bits(wm831x,(WM831X_GPIO1_CONTROL+i), 
			WM831X_GPN_PULL_MASK|WM831X_GPN_POL_MASK|WM831X_GPN_OD_MASK|WM831X_GPN_TRI_MASK, 
			1<<WM831X_GPN_POL_SHIFT|1<<WM831X_GPN_TRI_SHIFT);
		ret =  wm831x_reg_read(wm831x, WM831X_GPIO1_CONTROL+i);
		printk("Gpio%d Pin Configuration = %x\n",i,ret);
	}
#endif
	return 0;
}

/*
 * GPIO Buttons
 */
#if defined(CONFIG_KEYBOARD_WM831X_GPIO)
static struct wm831x_gpio_keys_button wm831x_gpio_buttons[] = {
{	
	.code		= KEY_MEDIA,
	.gpio		= TCA6424_P21,
	.active_low	= 1,
	.desc		= "media",
	.wakeup		= 0,
	.debounce_interval = 120,
},
{
    .code= KEY_VOLUMEUP,
		.gpio= WM831X_P05,
		.active_low= 1,
		.desc= "volume_up",
		.wakeup= 0,
},
{
		.code= KEY_CAMERA,
		.gpio= WM831X_P06,
		.active_low= 1,
		.desc= "camera",
		.wakeup= 0,
},
{
		.code= KEY_VOLUMEDOWN,
		.gpio= WM831X_P07,
		.active_low= 1,
		.desc= "volume_down",
		.wakeup= 0,
},
{
		.code= KEY_END,
		.gpio= WM831X_P09,
		.active_low= 1,
		.desc= "enter",
		.wakeup= 0,
},
{
		.code= KEY_MENU,
		.gpio= WM831X_P10,
		.active_low= 1,
		.desc= "menu",
		.wakeup= 0,
},
{
		.code= KEY_SEND,
		.gpio= WM831X_P11,
		.active_low= 1,
		.desc= "esc",
		.wakeup= 0,
},
{
		.code= KEY_BACK,
		.gpio= WM831X_P12,
		.active_low= 1,
		.desc= "home",
		.wakeup= 0,		    	
},
};

struct wm831x_gpio_keys_pdata wm831x_gpio_keys_platdata = {
	.buttons	= wm831x_gpio_buttons,
	.nbuttons	= ARRAY_SIZE(wm831x_gpio_buttons),
};

#endif
struct wm831x_pdata wm831x_platdata = {
	/** Called before subdevices are set up */
	.pre_init= wm831x_pre_init,
	/** Called after subdevices are set up */
	.post_init = wm831x_post_init,
	/** Called before subdevices are power down */
	.last_deinit = wm831x_last_deinit,
	
#if defined(CONFIG_GPIO_WM831X)
	.gpio_base=WM831X_GPIO_EXPANDER_BASE,
	.gpio_pin_num=WM831X_TOTOL_GPIO_NUM,
	.settinginfo=wm831x_gpio_settinginfo,
	.settinginfolen=ARRAY_SIZE(wm831x_gpio_settinginfo),
	.pin_type_init = wm831x_init_pin_type,
	.irq_base= NR_AIC_IRQS + 7*NUM_GROUP,
#endif

	.backlight = &wm831x_backlight_platdata,

	.backup = &wm831x_backup_platdata,
	
	.battery = &wm831x_battery_platdata,
	//.wm831x_touch_pdata = NULL,
	//.watchdog = NULL,
	
#if defined(CONFIG_KEYBOARD_WM831X_GPIO)	
	.gpio_keys = &wm831x_gpio_keys_platdata,
#endif

	/** LED1 = 0 and so on */
	.status = {&wm831x_status_platdata[0], &wm831x_status_platdata[1]},
	
	/** DCDC1 = 0 and so on */
	.dcdc = {&wm831x_regulator_init_dcdc[0], &wm831x_regulator_init_dcdc[1], &wm831x_regulator_init_dcdc[2], &wm831x_regulator_init_dcdc[3]},

	/** EPE1 = 0 and so on */
	.epe = {&wm831x_regulator_init_epe[0], &wm831x_regulator_init_epe[1]},

	/** LDO1 = 0 and so on */
	.ldo = {&wm831x_regulator_init_ldo[0], &wm831x_regulator_init_ldo[1], &wm831x_regulator_init_ldo[2], &wm831x_regulator_init_ldo[3],
			&wm831x_regulator_init_ldo[4], &wm831x_regulator_init_ldo[5], &wm831x_regulator_init_ldo[6], &wm831x_regulator_init_ldo[7],
			&wm831x_regulator_init_ldo[8], &wm831x_regulator_init_ldo[9], &wm831x_regulator_init_ldo[10]},

	/** ISINK1 = 0 and so on*/
	.isink = {&wm831x_regulator_init_isink[0], &wm831x_regulator_init_isink[1]},
};
#endif



#if defined(CONFIG_RK29_GPS)

#define 	RK29_GPS_POWER_PIN 		RK29_PIN6_PB2
#define 	RK29_GPS_RESET_PIN	  	RK29_PIN6_PC1

int rk29_gps_power_up(void)
{	
	printk("%s \n", __FUNCTION__);  

    gpio_request(RK29_GPS_POWER_PIN, NULL);    
	gpio_direction_output(RK29_GPS_POWER_PIN, GPIO_HIGH);	

	return 0;
}

int rk29_gps_power_down(void)
{	
	printk("%s \n", __FUNCTION__);	

    gpio_request(RK29_GPS_POWER_PIN, NULL);
	gpio_direction_output(RK29_GPS_POWER_PIN, GPIO_LOW);		

	return 0;
}

int rk29_gps_reset_set(int level)
{
	gpio_request(RK29_GPS_RESET_PIN, NULL);
	if (level)
		gpio_direction_output(RK29_GPS_RESET_PIN, GPIO_HIGH);
	else
		gpio_direction_output(RK29_GPS_RESET_PIN, GPIO_LOW);

	return 0;
}

struct rk29_gps_data rk29_gps_info = {	
	.power_up = rk29_gps_power_up,	
	.power_down = rk29_gps_power_down,	
	.reset = rk29_gps_reset_set,
	.uart_id = 3,
};

struct platform_device rk29_device_gps = {
	.name = "rk29_gps",
	.id = -1,		
	.dev		= {
	.platform_data = &rk29_gps_info,	
		}    	    
	};
#endif

/*****************************************************************************************
 * wm8994  codec
 * author: qjb@rock-chips.com
 *****************************************************************************************/
struct wm8994_pdata wm8994_platdata = {	
	.BB_input_diff = 0,
	.phone_pad = 0,

	.PA_control_pin = 0,	
	.Power_EN_Pin = RK29_PIN5_PA1,

	.speaker_incall_vol = 0,
	.speaker_incall_mic_vol = -9,
	.speaker_normal_vol = 6,
	.earpiece_incall_vol = 0,
	.headset_incall_vol = 6,
	.headset_incall_mic_vol = -6,
	.headset_normal_vol = -6,
	.BT_incall_vol = 0,
	.BT_incall_mic_vol = 0,
	.recorder_vol = 20,
	
};


#ifdef CONFIG_RK_HEADSET_DET
#define HEADSET_GPIO RK29_PIN4_PD2
struct rk_headset_pdata rk_headset_info = {
	.Headset_gpio		= RK29_PIN4_PD2,
	.headset_in_type= HEADSET_IN_HIGH,
	.Hook_gpio = RK29_PIN4_PD1,//Detection Headset--Must be set
	.hook_key_code = KEY_MEDIA,
};

struct platform_device rk_device_headset = {
		.name	= "rk_headsetdet",
		.id 	= 0,
		.dev    = {
		    .platform_data = &rk_headset_info,
		}
};
#endif

#if defined(CONFIG_GS_L3G4200D)

#include <linux/l3g4200d.h>
#define L3G4200D_INT_PIN  RK29_PIN5_PA3

static int l3g4200d_init_platform_hw(void)
{
	if (gpio_request(L3G4200D_INT_PIN, NULL) != 0) {
		gpio_free(L3G4200D_INT_PIN);
		printk("%s: request l3g4200d int pin error\n", __func__);
		return -EIO;
	}
	gpio_pull_updown(L3G4200D_INT_PIN, 1);
	return 0;
}

static struct l3g4200d_platform_data l3g4200d_info = {
	.fs_range = 1,
	
	.axis_map_x = 0,
	.axis_map_y = 1,
	.axis_map_z = 2,

	.negate_x = 1,
	.negate_y = 1,
	.negate_z = 0,

	.init = l3g4200d_init_platform_hw,
};

#endif

/*****************************************************************************************
 * i2c devices
 * author: kfx@rock-chips.com
*****************************************************************************************/
static int rk29_i2c0_io_init(void)
{
#ifdef CONFIG_RK29_I2C0_CONTROLLER
	rk29_mux_api_set(GPIO2B7_I2C0SCL_NAME, GPIO2L_I2C0_SCL);
	rk29_mux_api_set(GPIO2B6_I2C0SDA_NAME, GPIO2L_I2C0_SDA);
#else
	rk29_mux_api_set(GPIO2B7_I2C0SCL_NAME, GPIO2L_GPIO2B7);
	rk29_mux_api_set(GPIO2B6_I2C0SDA_NAME, GPIO2L_GPIO2B6);
#endif
	return 0;
}

static int rk29_i2c1_io_init(void)
{
#ifdef CONFIG_RK29_I2C1_CONTROLLER
	rk29_mux_api_set(GPIO1A7_I2C1SCL_NAME, GPIO1L_I2C1_SCL);
	rk29_mux_api_set(GPIO1A6_I2C1SDA_NAME, GPIO1L_I2C1_SDA);
#else
	rk29_mux_api_set(GPIO1A7_I2C1SCL_NAME, GPIO1L_GPIO1A7);
	rk29_mux_api_set(GPIO1A6_I2C1SDA_NAME, GPIO1L_GPIO1A6);
#endif
	return 0;
}
static int rk29_i2c2_io_init(void)
{
#ifdef CONFIG_RK29_I2C2_CONTROLLER
	rk29_mux_api_set(GPIO5D4_I2C2SCL_NAME, GPIO5H_I2C2_SCL);
	rk29_mux_api_set(GPIO5D3_I2C2SDA_NAME, GPIO5H_I2C2_SDA);
#else
	rk29_mux_api_set(GPIO5D4_I2C2SCL_NAME, GPIO5H_GPIO5D4);
	rk29_mux_api_set(GPIO5D3_I2C2SDA_NAME, GPIO5H_GPIO5D3);
#endif
	return 0;
}

static int rk29_i2c3_io_init(void)
{
#ifdef CONFIG_RK29_I2C3_CONTROLLER
	rk29_mux_api_set(GPIO2B5_UART3RTSN_I2C3SCL_NAME, GPIO2L_I2C3_SCL);
	rk29_mux_api_set(GPIO2B4_UART3CTSN_I2C3SDA_NAME, GPIO2L_I2C3_SDA);
#else
	rk29_mux_api_set(GPIO2B5_UART3RTSN_I2C3SCL_NAME, GPIO2L_GPIO2B5);
	rk29_mux_api_set(GPIO2B4_UART3CTSN_I2C3SDA_NAME, GPIO2L_GPIO2B4);
#endif
	return 0;
}
#ifdef CONFIG_RK29_I2C0_CONTROLLER
struct rk29_i2c_platform_data default_i2c0_data = {
	.bus_num    = 0,
	.flags      = 0,
	.slave_addr = 0xff,
	.scl_rate  = 400*1000,
	.mode 		= I2C_MODE_IRQ,
	.io_init = rk29_i2c0_io_init,
};
#else
struct i2c_gpio_platform_data default_i2c0_data = {
       .sda_pin = RK29_PIN2_PB6,
       .scl_pin = RK29_PIN2_PB7,
       .udelay = 5, // clk = 500/udelay = 100Khz
       .timeout = 100,//msecs_to_jiffies(200),
       .bus_num    = 0,
       .io_init = rk29_i2c0_io_init,
};
#endif
#ifdef CONFIG_RK29_I2C1_CONTROLLER
struct rk29_i2c_platform_data default_i2c1_data = {
	.bus_num    = 1,
	.flags      = 0,
	.slave_addr = 0xff,
	.scl_rate  = 400*1000,
	.mode 		= I2C_MODE_IRQ,
	.io_init = rk29_i2c1_io_init,
};
#else
struct i2c_gpio_platform_data default_i2c1_data = {
       .sda_pin = RK29_PIN1_PA6,
       .scl_pin = RK29_PIN1_PA7,
       .udelay = 5, // clk = 500/udelay = 100Khz
       .timeout = 100,//msecs_to_jiffies(200),
       .bus_num    = 1,
       .io_init = rk29_i2c1_io_init,
};
#endif
#ifdef CONFIG_RK29_I2C2_CONTROLLER
struct rk29_i2c_platform_data default_i2c2_data = {
	.bus_num    = 2,
	.flags      = 0,
	.slave_addr = 0xff,
	.scl_rate  = 400*1000,
	.mode 		= I2C_MODE_IRQ,
	.io_init = rk29_i2c2_io_init,
};
#else
struct i2c_gpio_platform_data default_i2c2_data = {
       .sda_pin = RK29_PIN5_PD3,
       .scl_pin = RK29_PIN5_PD4,
       .udelay = 5, // clk = 500/udelay = 100Khz
       .timeout = 100,//msecs_to_jiffies(200),
       .bus_num    = 2,
       .io_init = rk29_i2c2_io_init,
};
#endif
#ifdef CONFIG_RK29_I2C3_CONTROLLER
struct rk29_i2c_platform_data default_i2c3_data = {
	.bus_num    = 3,
	.flags      = 0,
	.slave_addr = 0xff,
	.scl_rate  = 400*1000,
	.mode 		= I2C_MODE_IRQ,
	.io_init = rk29_i2c3_io_init,
};
#else
struct i2c_gpio_platform_data default_i2c3_data = {
       .sda_pin = RK29_PIN5_PB5,
       .scl_pin = RK29_PIN5_PB4,
       .udelay = 5, // clk = 500/udelay = 100Khz
       .timeout = 100,//msecs_to_jiffies(200),
       .bus_num    = 3,
       .io_init = rk29_i2c3_io_init,
};
#endif

#if defined (CONFIG_ANX7150)
struct hdmi_platform_data anx7150_data = {
	//.io_init = anx7150_io_init,
};
#endif
#ifdef CONFIG_I2C0_RK29
static struct i2c_board_info __initdata board_i2c0_devices[] = {
#if defined (CONFIG_RK1000_CONTROL)
	{
		.type    		= "rk1000_control",
		.addr           = 0x40,
		.flags			= 0,
	},
#endif
#if defined (CONFIG_SND_SOC_RK1000)
	{
		.type    		= "rk1000_i2c_codec",
		.addr           = 0x60,
		.flags			= 0,
	},
#endif
#if defined (CONFIG_SND_SOC_WM8900)
	{
		.type    		= "wm8900",
		.addr           = 0x1A,
		.flags			= 0,
	},
#endif
#if defined (CONFIG_SND_SOC_WM8994)
	{
		.type    		= "wm8994",
		.addr           = 0x1a,
		.flags			= 0,
//	#if defined(CONFIG_MFD_WM8994)	
		.platform_data  = &wm8994_platdata,	
//	#endif	
	},
#endif
#if defined (CONFIG_BATTERY_STC3100)
	{
		.type    		= "stc3100",
		.addr           = 0x70,
		.flags			= 0,
	},
#endif
#if defined (CONFIG_BATTERY_BQ27510)
	{
		.type    		= "bq27510",
		.addr           = 0x55,
		.flags			= 0,
	},
#endif
#if defined (CONFIG_RTC_HYM8563)
	{
		.type    		= "rtc_hym8563",
		.addr           = 0x51,
		.flags			= 0,
		.irq            = RK29_PIN0_PA1,
	},
#endif
#if defined (CONFIG_GS_MMA8452)
    {
      .type           = "gs_mma8452",
      .addr           = 0x1c,
      .flags          = 0,
      .irq            = MMA8452_INT_PIN,
      .platform_data  = &mma8452_info,
    },
#endif
#if defined (CONFIG_COMPASS_AK8973)
	{
		.type    		= "ak8973",
		.addr           = 0x1d,
		.flags			= 0,
		.irq			= RK29_PIN6_PC5,
	},
#endif
#if defined (CONFIG_COMPASS_AK8975)
	{
		.type    		= "ak8975",
		.addr           = 0x0d,
		.flags			= 0,
		.irq			= RK29_PIN6_PC5,
	},
#endif
#if defined (CONFIG_INPUT_LPSENSOR_ISL29028)
	{
		.type           = "isl29028",
		.addr           = 0x44,
		.flags          = 0,
		.irq            = RK29_PIN4_PD3,
	},
#endif
#if defined (CONFIG_ANX7150)
    {
		.type           = "anx7150",
        .addr           = 0x39,             //0x39, 0x3d
        .flags          = 0,
        .irq            = RK29_PIN2_PA3,
		.platform_data  = &anx7150_data,
    },
#endif
#if defined (CONFIG_GS_L3G4200D)
	{
		.type           = "gs_l3g4200d",
		.addr           = 0x69,
		.flags          = 0,
		.irq            = L3G4200D_INT_PIN,
		.platform_data  = &l3g4200d_info,
	},
#endif
#if defined (CONFIG_MPU_SENSORS_MPU3050) 
	{
		.type			= "mpu3050",
		.addr			= 0x68,
		.flags			= 0,
		.irq			= RK29_PIN4_PC4,
		.platform_data	= &mpu3050_data,
	},
#endif
};
#endif

#ifdef CONFIG_I2C1_RK29
static struct i2c_board_info __initdata board_i2c1_devices[] = {
#if defined (CONFIG_RK1000_CONTROL1)
	{
		.type			= "rk1000_control",
		.addr			= 0x40,
		.flags			= 0,
	},
#endif

};
#endif

#ifdef CONFIG_I2C2_RK29
static struct i2c_board_info __initdata board_i2c2_devices[] = {
#if defined (CONFIG_TOUCHSCREEN_GT801_IIC)
{
	.type           = "gt801_ts",
	.addr           = 0x55,
	.flags          = 0,
	.irq            = RK29_PIN4_PD5,
	.platform_data = &gt801_info,
},	
#endif

#if defined (CONFIG_TOUCHSCREEN_GT818_IIC)
{
	.type           = "gt818_ts",
	.addr           = 0x5d,
	.flags          = 0,
	.irq            = RK29_PIN4_PD5,
	.platform_data = &gt818_info,
},
#endif

#if defined (CONFIG_TOUCHSCREEN_ILI2102_IIC)
{
	.type           = "ili2102_ts",
	.addr           = 0x41,
	.flags          = I2C_M_NEED_DELAY,
	.udelay      = 600,
	.irq            = RK29_PIN4_PD5,
	.platform_data = &ili2102_info,
},	
#endif

#if defined (CONFIG_MFD_WM831X_I2C)
{
	.type           = "wm8310",
	.addr           = 0x34,
	.flags          = 0,
	.irq            = RK29_PIN4_PD0,
	.platform_data = &wm831x_platdata,
},	
#endif
#if defined (CONFIG_HANNSTAR_P1003)
    {
      .type           = "p1003_touch",
      .addr           = 0x04,
      .flags          = 0,
      .irq            = RK29_PIN0_PA2,
      .platform_data  = &p1003_info,
    },
#endif
#if defined (CONFIG_EETI_EGALAX)
    {
      .type           = "egalax_i2c",
      .addr           = 0x04,
      .flags          = 0,
      .irq            = RK29_PIN4_PD5,
      .platform_data  = &eeti_egalax_info,
    },
#endif
};
#endif

#ifdef CONFIG_I2C3_RK29
static struct i2c_board_info __initdata board_i2c3_devices[] = {
};
#endif

/*****************************************************************************************
 * camera  devices
 * author: ddl@rock-chips.com
 *****************************************************************************************/
#ifdef CONFIG_VIDEO_RK29 
#define CONFIG_SENSOR_POWER_IOCTL_USR      0
#define CONFIG_SENSOR_RESET_IOCTL_USR      0
#define CONFIG_SENSOR_POWERDOWN_IOCTL_USR      0
#define CONFIG_SENSOR_FLASH_IOCTL_USR      0

#if CONFIG_SENSOR_POWER_IOCTL_USR
static int sensor_power_usr_cb (struct rk29camera_gpio_res *res,int on)
{
    #error "CONFIG_SENSOR_POWER_IOCTL_USR is 1, sensor_power_usr_cb function must be writed!!";
}
#endif

#if CONFIG_SENSOR_RESET_IOCTL_USR
static int sensor_reset_usr_cb (struct rk29camera_gpio_res *res,int on)
{
    #error "CONFIG_SENSOR_RESET_IOCTL_USR is 1, sensor_reset_usr_cb function must be writed!!";
}
#endif

#if CONFIG_SENSOR_POWERDOWN_IOCTL_USR
static int sensor_powerdown_usr_cb (struct rk29camera_gpio_res *res,int on)
{
    #error "CONFIG_SENSOR_POWERDOWN_IOCTL_USR is 1, sensor_powerdown_usr_cb function must be writed!!";
}
#endif

#if CONFIG_SENSOR_FLASH_IOCTL_USR
static int sensor_flash_usr_cb (struct rk29camera_gpio_res *res,int on)
{
    #error "CONFIG_SENSOR_FLASH_IOCTL_USR is 1, sensor_flash_usr_cb function must be writed!!";
}
#endif

static struct rk29camera_platform_ioctl_cb  sensor_ioctl_cb = {
    #if CONFIG_SENSOR_POWER_IOCTL_USR
    .sensor_power_cb = sensor_power_usr_cb,
    #else
    .sensor_power_cb = NULL,
    #endif

    #if CONFIG_SENSOR_RESET_IOCTL_USR
    .sensor_reset_cb = sensor_reset_usr_cb,
    #else
    .sensor_reset_cb = NULL,
    #endif

    #if CONFIG_SENSOR_POWERDOWN_IOCTL_USR
    .sensor_powerdown_cb = sensor_powerdown_usr_cb,
    #else
    .sensor_powerdown_cb = NULL,
    #endif

    #if CONFIG_SENSOR_FLASH_IOCTL_USR
    .sensor_flash_cb = sensor_flash_usr_cb,
    #else
    .sensor_flash_cb = NULL,
    #endif
};
#include "../../../drivers/media/video/rk29_camera.c"
#endif

/*****************************************************************************************
 * backlight  devices
 * author: nzy@rock-chips.com
 *****************************************************************************************/
#ifdef CONFIG_BACKLIGHT_RK29_BL
 /*
 GPIO1B5_PWM0_NAME,       GPIO1L_PWM0
 GPIO5D2_PWM1_UART1SIRIN_NAME,  GPIO5H_PWM1
 GPIO2A3_SDMMC0WRITEPRT_PWM2_NAME,   GPIO2L_PWM2
 GPIO1A5_EMMCPWREN_PWM3_NAME,     GPIO1L_PWM3
 */

#define PWM_ID            0
#define PWM_MUX_NAME      GPIO1B5_PWM0_NAME
#define PWM_MUX_MODE      GPIO1L_PWM0
#define PWM_MUX_MODE_GPIO GPIO1L_GPIO1B5
#define PWM_EFFECT_VALUE  1

//#define LCD_DISP_ON_PIN

#ifdef  LCD_DISP_ON_PIN
#define BL_EN_MUX_NAME    GPIOF34_UART3_SEL_NAME
#define BL_EN_MUX_MODE    IOMUXB_GPIO1_B34

#define BL_EN_PIN         GPIO0L_GPIO0A5
#define BL_EN_VALUE       GPIO_HIGH
#endif
static int rk29_backlight_io_init(void)
{
    int ret = 0;

    rk29_mux_api_set(PWM_MUX_NAME, PWM_MUX_MODE);
	#ifdef  LCD_DISP_ON_PIN
    rk29_mux_api_set(BL_EN_MUX_NAME, BL_EN_MUX_MODE);

    ret = gpio_request(BL_EN_PIN, NULL);
    if(ret != 0)
    {
        gpio_free(BL_EN_PIN);
    }

    gpio_direction_output(BL_EN_PIN, 0);
    gpio_set_value(BL_EN_PIN, BL_EN_VALUE);
	#endif
    return ret;
}

static int rk29_backlight_io_deinit(void)
{
    int ret = 0;
    #ifdef  LCD_DISP_ON_PIN
    gpio_free(BL_EN_PIN);
    #endif
    rk29_mux_api_set(PWM_MUX_NAME, PWM_MUX_MODE_GPIO);
    return ret;
}
struct rk29_bl_info rk29_bl_info = {
    .pwm_id   = PWM_ID,
    .bl_ref   = PWM_EFFECT_VALUE,
    .io_init   = rk29_backlight_io_init,
    .io_deinit = rk29_backlight_io_deinit,
};
#endif
/*****************************************************************************************
* pwm voltage regulator devices
******************************************************************************************/
#if defined (CONFIG_RK29_PWM_REGULATOR)

#define REGULATOR_PWM_ID					2
#define REGULATOR_PWM_MUX_NAME      		GPIO2A3_SDMMC0WRITEPRT_PWM2_NAME
#define REGULATOR_PWM_MUX_MODE      					GPIO2L_PWM2
#define REGULATOR_PWM_MUX_MODE_GPIO 				GPIO2L_GPIO2A3
#define REGULATOR_PWM_GPIO				RK29_PIN2_PA3

static struct regulator_consumer_supply pwm_consumers[] = {
	{
		.supply = "vcore",
	}
};

static struct regulator_init_data rk29_pwm_regulator_data = {
	.constraints = {
		.name = "PWM2",
		.min_uV =  950000,
		.max_uV = 1400000,
		.apply_uV = 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE,
	},
	.num_consumer_supplies = ARRAY_SIZE(pwm_consumers),
	.consumer_supplies = pwm_consumers,
};

static struct pwm_platform_data rk29_regulator_pwm_platform_data = {
	.pwm_id = REGULATOR_PWM_ID,
	.pwm_gpio = REGULATOR_PWM_GPIO,
	//.pwm_iomux_name[] = REGULATOR_PWM_MUX_NAME;
	.pwm_iomux_name = REGULATOR_PWM_MUX_NAME,
	.pwm_iomux_pwm = REGULATOR_PWM_MUX_MODE,
	.pwm_iomux_gpio = REGULATOR_PWM_MUX_MODE_GPIO,
	.init_data  = &rk29_pwm_regulator_data,
};

static struct platform_device rk29_device_pwm_regulator = {
	.name = "pwm-voltage-regulator",
	.id   = -1,
	.dev  = {
		.platform_data = &rk29_regulator_pwm_platform_data,
	},
};

#endif


#if defined(CONFIG_MTK23D)
static int mtk23d_io_init(void)
{
	
	return 0;
}

static int mtk23d_io_deinit(void)
{
	
	return 0;
}
 
struct rk2818_23d_data rk2818_23d_info = {
	.io_init = mtk23d_io_init,
  .io_deinit = mtk23d_io_deinit,
	.bp_power = RK29_PIN0_PA0,
	.bp_power_active_low = 0,
	.bp_reset = RK29_PIN0_PA1,
	.bp_reset_active_low = 1,
	.bp_statue = RK29_PIN0_PA2,//input  high bp sleep;
	.ap_statue = RK29_PIN0_PA4,//output high ap sleep;
	.ap_bp_wakeup = RK29_PIN0_PA3, //output AP wake up BP used rising edge;
	.bp_ap_wakeup = 0,//input BP wake up AP
};
struct platform_device rk2818_device_mtk23d = {	
        .name = "mtk23d",	
    	.id = -1,	
	.dev		= {
		.platform_data = &rk2818_23d_info,
	}    	
    };
#endif


/*****************************************************************************************
 * SDMMC devices
*****************************************************************************************/
#ifdef CONFIG_SDMMC0_RK29
static int rk29_sdmmc0_cfg_gpio(void)
{
	rk29_mux_api_set(GPIO1D1_SDMMC0CMD_NAME, GPIO1H_SDMMC0_CMD);
	rk29_mux_api_set(GPIO1D0_SDMMC0CLKOUT_NAME, GPIO1H_SDMMC0_CLKOUT);
	rk29_mux_api_set(GPIO1D2_SDMMC0DATA0_NAME, GPIO1H_SDMMC0_DATA0);
	rk29_mux_api_set(GPIO1D3_SDMMC0DATA1_NAME, GPIO1H_SDMMC0_DATA1);
	rk29_mux_api_set(GPIO1D4_SDMMC0DATA2_NAME, GPIO1H_SDMMC0_DATA2);
	rk29_mux_api_set(GPIO1D5_SDMMC0DATA3_NAME, GPIO1H_SDMMC0_DATA3);
#ifdef CONFIG_SDMMC_RK29_OLD	
	rk29_mux_api_set(GPIO2A2_SDMMC0DETECTN_NAME, GPIO2L_GPIO2A2);
#else
  rk29_mux_api_set(GPIO2A2_SDMMC0DETECTN_NAME, GPIO2L_SDMMC0_DETECT_N);//Modifyed by xbw.
#endif	
	rk29_mux_api_set(GPIO5D5_SDMMC0PWREN_NAME, GPIO5H_GPIO5D5);   ///GPIO5H_SDMMC0_PWR_EN);  ///GPIO5H_GPIO5D5);
	gpio_request(RK29_PIN5_PD5,"sdmmc");
	gpio_set_value(RK29_PIN5_PD5,GPIO_HIGH);
	mdelay(100);
	gpio_set_value(RK29_PIN5_PD5,GPIO_LOW);
	return 0;
}

#define CONFIG_SDMMC0_USE_DMA
struct rk29_sdmmc_platform_data default_sdmmc0_data = {
	.host_ocr_avail = (MMC_VDD_25_26|MMC_VDD_26_27|MMC_VDD_27_28|MMC_VDD_28_29|MMC_VDD_29_30|
					   MMC_VDD_30_31|MMC_VDD_31_32|MMC_VDD_32_33|
					   MMC_VDD_33_34|MMC_VDD_34_35| MMC_VDD_35_36),
	.host_caps 	= (MMC_CAP_4_BIT_DATA|MMC_CAP_MMC_HIGHSPEED|MMC_CAP_SD_HIGHSPEED),
	.io_init = rk29_sdmmc0_cfg_gpio,
	.dma_name = "sd_mmc",
#ifdef CONFIG_SDMMC0_USE_DMA
	.use_dma  = 1,
#else
	.use_dma = 0,
#endif
	.detect_irq = RK29_PIN2_PA2, // INVALID_GPIO
	.enable_sd_wakeup = 0,
};
#endif
#ifdef CONFIG_SDMMC1_RK29
#define CONFIG_SDMMC1_USE_DMA
static int rk29_sdmmc1_cfg_gpio(void)
{
	rk29_mux_api_set(GPIO1C2_SDMMC1CMD_NAME, GPIO1H_SDMMC1_CMD);
	rk29_mux_api_set(GPIO1C7_SDMMC1CLKOUT_NAME, GPIO1H_SDMMC1_CLKOUT);
	rk29_mux_api_set(GPIO1C3_SDMMC1DATA0_NAME, GPIO1H_SDMMC1_DATA0);
	rk29_mux_api_set(GPIO1C4_SDMMC1DATA1_NAME, GPIO1H_SDMMC1_DATA1);
	rk29_mux_api_set(GPIO1C5_SDMMC1DATA2_NAME, GPIO1H_SDMMC1_DATA2);
	rk29_mux_api_set(GPIO1C6_SDMMC1DATA3_NAME, GPIO1H_SDMMC1_DATA3);
	//rk29_mux_api_set(GPIO1C0_UART0CTSN_SDMMC1DETECTN_NAME, GPIO1H_SDMMC1_DETECT_N);
	return 0;
}

#ifdef CONFIG_WIFI_CONTROL_FUNC
static int rk29sdk_wifi_status(struct device *dev);
static int rk29sdk_wifi_status_register(void (*callback)(int card_presend, void *dev_id), void *dev_id);
#endif

#define RK29SDK_WIFI_SDIO_CARD_DETECT_N    RK29_PIN1_PD6

struct rk29_sdmmc_platform_data default_sdmmc1_data = {
	.host_ocr_avail = (MMC_VDD_25_26|MMC_VDD_26_27|MMC_VDD_27_28|MMC_VDD_28_29|
					   MMC_VDD_29_30|MMC_VDD_30_31|MMC_VDD_31_32|
					   MMC_VDD_32_33|MMC_VDD_33_34),
	.host_caps 	= (MMC_CAP_4_BIT_DATA|MMC_CAP_SDIO_IRQ|
				   MMC_CAP_MMC_HIGHSPEED|MMC_CAP_SD_HIGHSPEED),
	.io_init = rk29_sdmmc1_cfg_gpio,
	.dma_name = "sdio",
#ifdef CONFIG_SDMMC1_USE_DMA
	.use_dma  = 1,
#else
	.use_dma = 0,
#endif
#ifdef CONFIG_WIFI_CONTROL_FUNC
        .status = rk29sdk_wifi_status,
        .register_status_notify = rk29sdk_wifi_status_register,
#endif
#if 0
        .detect_irq = RK29SDK_WIFI_SDIO_CARD_DETECT_N,
#endif
};
#endif

#ifdef CONFIG_WIFI_CONTROL_FUNC
#define RK29SDK_WIFI_BT_GPIO_POWER_N       RK29_PIN5_PD6
#define RK29SDK_WIFI_GPIO_RESET_N          RK29_PIN6_PC0
#define RK29SDK_BT_GPIO_RESET_N            RK29_PIN6_PC7

static int rk29sdk_wifi_cd = 0;   /* wifi virtual 'card detect' status */
static void (*wifi_status_cb)(int card_present, void *dev_id);
static void *wifi_status_cb_devid;
int rk29sdk_wifi_power_state = 0;
int rk29sdk_bt_power_state = 0;

static int rk29sdk_wifi_status(struct device *dev)
{
        return rk29sdk_wifi_cd;
}

static int rk29sdk_wifi_status_register(void (*callback)(int card_present, void *dev_id), void *dev_id)
{
        if(wifi_status_cb)
                return -EAGAIN;
        wifi_status_cb = callback;
        wifi_status_cb_devid = dev_id;
        return 0;
}

static int rk29sdk_wifi_bt_gpio_control_init(void)
{
    if (gpio_request(RK29SDK_WIFI_BT_GPIO_POWER_N, "wifi_bt_power")) {
           pr_info("%s: request wifi_bt power gpio failed\n", __func__);
           return -1;
    }

    if (gpio_request(RK29SDK_WIFI_GPIO_RESET_N, "wifi reset")) {
           pr_info("%s: request wifi reset gpio failed\n", __func__);
           gpio_free(RK29SDK_WIFI_BT_GPIO_POWER_N);
           return -1;
    }

    if (gpio_request(RK29SDK_BT_GPIO_RESET_N, "bt reset")) {
          pr_info("%s: request bt reset gpio failed\n", __func__);
          gpio_free(RK29SDK_WIFI_GPIO_RESET_N);
          return -1;
    }

    gpio_direction_output(RK29SDK_WIFI_BT_GPIO_POWER_N, GPIO_LOW);
    gpio_direction_output(RK29SDK_WIFI_GPIO_RESET_N,    GPIO_LOW);
    gpio_direction_output(RK29SDK_BT_GPIO_RESET_N,      GPIO_LOW);

    pr_info("%s: init finished\n",__func__);

    return 0;
}

static int rk29sdk_wifi_power(int on)
{
        pr_info("%s: %d\n", __func__, on);
        if (on){
                gpio_set_value(RK29SDK_WIFI_BT_GPIO_POWER_N, GPIO_HIGH);
                gpio_set_value(RK29SDK_WIFI_GPIO_RESET_N, GPIO_HIGH);
                mdelay(100);
                pr_info("wifi turn on power\n");
        }else{
                if (!rk29sdk_bt_power_state){
                        gpio_set_value(RK29SDK_WIFI_BT_GPIO_POWER_N, GPIO_LOW);
                        mdelay(100);
                        pr_info("wifi shut off power\n");
                }else
                {
                        pr_info("wifi shouldn't shut off power, bt is using it!\n");
                }
                gpio_set_value(RK29SDK_WIFI_GPIO_RESET_N, GPIO_LOW);

        }

        rk29sdk_wifi_power_state = on;
        return 0;
}

static int rk29sdk_wifi_reset_state;
static int rk29sdk_wifi_reset(int on)
{
        pr_info("%s: %d\n", __func__, on);
        gpio_set_value(RK29SDK_WIFI_GPIO_RESET_N, on);
        mdelay(100);
        rk29sdk_wifi_reset_state = on;
        return 0;
}

int rk29sdk_wifi_set_carddetect(int val)
{
        pr_info("%s:%d\n", __func__, val);
        rk29sdk_wifi_cd = val;
        if (wifi_status_cb){
                wifi_status_cb(val, wifi_status_cb_devid);
        }else {
                pr_warning("%s, nobody to notify\n", __func__);
        }
        return 0;
}
EXPORT_SYMBOL(rk29sdk_wifi_set_carddetect);

static struct wifi_mem_prealloc wifi_mem_array[PREALLOC_WLAN_SEC_NUM] = {
        {NULL, (WLAN_SECTION_SIZE_0 + PREALLOC_WLAN_SECTION_HEADER)},
        {NULL, (WLAN_SECTION_SIZE_1 + PREALLOC_WLAN_SECTION_HEADER)},
        {NULL, (WLAN_SECTION_SIZE_2 + PREALLOC_WLAN_SECTION_HEADER)},
        {NULL, (WLAN_SECTION_SIZE_3 + PREALLOC_WLAN_SECTION_HEADER)}
};

static void *rk29sdk_mem_prealloc(int section, unsigned long size)
{
        if (section == PREALLOC_WLAN_SEC_NUM)
                return wlan_static_skb;

        if ((section < 0) || (section > PREALLOC_WLAN_SEC_NUM))
                return NULL;

        if (wifi_mem_array[section].size < size)
                return NULL;

        return wifi_mem_array[section].mem_ptr;
}

int __init rk29sdk_init_wifi_mem(void)
{
        int i;
        int j;

        for (i = 0 ; i < WLAN_SKB_BUF_NUM ; i++) {
                wlan_static_skb[i] = dev_alloc_skb(
                                ((i < (WLAN_SKB_BUF_NUM / 2)) ? 4096 : 8192));

                if (!wlan_static_skb[i])
                        goto err_skb_alloc;
        }

        for (i = 0 ; i < PREALLOC_WLAN_SEC_NUM ; i++) {
                wifi_mem_array[i].mem_ptr =
                                kmalloc(wifi_mem_array[i].size, GFP_KERNEL);

                if (!wifi_mem_array[i].mem_ptr)
                        goto err_mem_alloc;
        }
        return 0;

 err_mem_alloc:
        pr_err("Failed to mem_alloc for WLAN\n");
        for (j = 0 ; j < i ; j++)
                kfree(wifi_mem_array[j].mem_ptr);

        i = WLAN_SKB_BUF_NUM;

 err_skb_alloc:
        pr_err("Failed to skb_alloc for WLAN\n");
        for (j = 0 ; j < i ; j++)
                dev_kfree_skb(wlan_static_skb[j]);

        return -ENOMEM;
}

static struct wifi_platform_data rk29sdk_wifi_control = {
        .set_power = rk29sdk_wifi_power,
        .set_reset = rk29sdk_wifi_reset,
        .set_carddetect = rk29sdk_wifi_set_carddetect,
        .mem_prealloc   = rk29sdk_mem_prealloc,
};
static struct platform_device rk29sdk_wifi_device = {
        .name = "bcm4329_wlan",
        .id = 1,
        .dev = {
                .platform_data = &rk29sdk_wifi_control,
         },
};
#endif


/* bluetooth rfkill device */
static struct platform_device rk29sdk_rfkill = {
        .name = "rk29sdk_rfkill",
        .id = -1,
};


#ifdef CONFIG_VIVANTE
static struct resource resources_gpu[] = {
    [0] = {
		.name 	= "gpu_irq",
        .start 	= IRQ_GPU,
        .end    = IRQ_GPU,
        .flags  = IORESOURCE_IRQ,
    },
    [1] = {
		.name = "gpu_base",
        .start  = RK29_GPU_PHYS,
        .end    = RK29_GPU_PHYS + RK29_GPU_SIZE - 1,
        .flags  = IORESOURCE_MEM,
    },
    [2] = {
		.name = "gpu_mem",
        .start  = PMEM_GPU_BASE,
        .end    = PMEM_GPU_BASE + PMEM_GPU_SIZE - 1,
        .flags  = IORESOURCE_MEM,
    },
};
static struct platform_device rk29_device_gpu = {
    .name             = "galcore",
    .id               = 0,
    .num_resources    = ARRAY_SIZE(resources_gpu),
    .resource         = resources_gpu,
};
#endif
#ifdef CONFIG_KEYS_RK29
extern struct rk29_keys_platform_data rk29_keys_pdata;
static struct platform_device rk29_device_keys = {
	.name		= "rk29-keypad",
	.id		= -1,
	.dev		= {
		.platform_data	= &rk29_keys_pdata,
	},
};
#endif

#ifdef CONFIG_USE_GPIO_GENERATE_WAVE
static struct gpio_wave_platform_data gpio_wave_pdata = {
	.gpio = RK29_PIN0_PA0,
	.Htime = 2000,
	.Ltime = 300,
	.Dvalue = GPIO_HIGH,
};
static struct platform_device gpio_wave_device = {
	.name		= "gpio_wave",
	.id 	= -1,
	.dev		= {
		.platform_data	= &gpio_wave_pdata,
	},
};
#endif

static void __init rk29_board_iomux_init(void)
{
	int err;
	#ifdef CONFIG_RK29_PWM_REGULATOR
	rk29_mux_api_set(REGULATOR_PWM_MUX_NAME,REGULATOR_PWM_MUX_MODE);
	#endif
	rk29_mux_api_set(GPIO4C0_RMIICLKOUT_RMIICLKIN_NAME,GPIO4H_GPIO4C0);

/****************************clock change********************************************/
	err = gpio_request(RK29_PIN4_PC0, "clk27M_control");
	if (err) {
		gpio_free(RK29_PIN4_PC0);
		printk("-------request RK29_PIN4_PC0 fail--------\n");
		return -1;
	}
	//phy power down
	gpio_direction_output(RK29_PIN4_PC0, GPIO_LOW);// 27M  32K
	gpio_set_value(RK29_PIN4_PC0, GPIO_LOW);

	rk29_mux_api_set(GPIO4C5_RMIICSRDVALID_MIIRXDVALID_NAME,GPIO4H_GPIO4C5);

	err = gpio_request(RK29_PIN4_PC5, "clk24M_control");
	if (err) {
		gpio_free(RK29_PIN4_PC5);
		printk("-------request RK29_PIN4_PC5 fail--------\n");
		return -1;
	}
	//phy power down
	gpio_direction_output(RK29_PIN4_PC5, GPIO_LOW);// control 24M
	gpio_set_value(RK29_PIN4_PC5, GPIO_LOW);
/*******************************************************************/


}

// For phone,just a disk only, add by phc,20110816
#ifdef CONFIG_USB_ANDROID
struct usb_mass_storage_platform_data phone_mass_storage_pdata = {
	.nluns		= 1,  
	.vendor		= "RockChip",
	.product	= "rk29 sdk",
	.release	= 0x0100,
};

//static 
struct platform_device phone_usb_mass_storage_device = {
	.name	= "usb_mass_storage",
	.id	= -1,
	.dev	= {
		.platform_data = &phone_mass_storage_pdata,
	},
};
#endif


static struct platform_device *devices[] __initdata = {

#ifdef CONFIG_RK29_WATCHDOG
	&rk29_device_wdt,
#endif

#ifdef CONFIG_UART1_RK29
	&rk29_device_uart1,
#endif
#ifdef CONFIG_UART0_RK29
	&rk29_device_uart0,
#endif
#ifdef CONFIG_UART2_RK29
	&rk29_device_uart2,
#endif
#ifdef CONFIG_UART3_RK29
	&rk29_device_uart3,
#endif

#ifdef CONFIG_RK29_PWM_REGULATOR
	&rk29_device_pwm_regulator,
#endif
#ifdef CONFIG_SPIM0_RK29
    &rk29xx_device_spi0m,
#endif
#ifdef CONFIG_SPIM1_RK29
    &rk29xx_device_spi1m,
#endif
#ifdef CONFIG_ADC_RK29
	&rk29_device_adc,
#endif
#ifdef CONFIG_I2C0_RK29
	&rk29_device_i2c0,
#endif
#ifdef CONFIG_I2C1_RK29
	&rk29_device_i2c1,
#endif
#ifdef CONFIG_I2C2_RK29
	&rk29_device_i2c2,
#endif
#ifdef CONFIG_I2C3_RK29
	&rk29_device_i2c3,
#endif

#ifdef CONFIG_SND_RK29_SOC_I2S_2CH
        &rk29_device_iis_2ch,
#endif
#ifdef CONFIG_SND_RK29_SOC_I2S_8CH
        &rk29_device_iis_8ch,
#endif

#ifdef CONFIG_KEYS_RK29
	&rk29_device_keys,
#endif
#ifdef CONFIG_USE_GPIO_GENERATE_WAVE
	&gpio_wave_device,
#endif
#ifdef CONFIG_SDMMC0_RK29
	&rk29_device_sdmmc0,
#endif
#ifdef CONFIG_SDMMC1_RK29
	&rk29_device_sdmmc1,
#endif

#ifdef CONFIG_MTD_NAND_RK29XX
	&rk29xx_device_nand,
#endif

#ifdef CONFIG_WIFI_CONTROL_FUNC
        &rk29sdk_wifi_device,
#endif

#ifdef CONFIG_BT
        &rk29sdk_rfkill,
#endif

#if defined(CONFIG_MTK23D)
	&rk2818_device_mtk23d,
#endif

#ifdef CONFIG_MTD_NAND_RK29
	&rk29_device_nand,
#endif

#ifdef CONFIG_FB_RK29
	&rk29_device_fb,
	&rk29_device_dma_cpy,
#endif
#ifdef CONFIG_BACKLIGHT_RK29_BL
	&rk29_device_backlight,
#endif
#ifdef CONFIG_RK29_VMAC
	&rk29_device_vmac,
#endif
#ifdef CONFIG_VIVANTE
	&rk29_device_gpu,
#endif
#ifdef CONFIG_VIDEO_RK29
 	&rk29_device_camera,      /* ddl@rock-chips.com : camera support  */
 	#if (CONFIG_SENSOR_IIC_ADDR_0 != 0x00)
 	&rk29_soc_camera_pdrv_0,
 	#endif
 	&rk29_soc_camera_pdrv_1,
 	&android_pmem_cam_device,
#endif
	&android_pmem_device,
	&rk29_vpu_mem_device,
#ifdef CONFIG_USB20_OTG
	&rk29_device_usb20_otg,
#endif
#ifdef CONFIG_USB20_HOST
	&rk29_device_usb20_host,
#endif
#ifdef CONFIG_USB11_HOST
	&rk29_device_usb11_host,
#endif
#ifdef CONFIG_USB_ANDROID
	&android_usb_device,
	&phone_usb_mass_storage_device,
#endif
#ifdef CONFIG_RK29_IPP
	&rk29_device_ipp,
#endif
#ifdef CONFIG_VIDEO_RK29XX_VOUT
	&rk29_v4l2_output_devce,
#endif
#ifdef CONFIG_RK_HEADSET_DET
    &rk_device_headset,
#endif
#ifdef CONFIG_RK29_GPS
	&rk29_device_gps,
#endif
};

#ifdef CONFIG_RK29_VMAC
/*****************************************************************************************
 * vmac devices
 * author: lyx@rock-chips.com
 *****************************************************************************************/
static int rk29_vmac_register_set(void)
{
	//config rk29 vmac as rmii, 100MHz
	u32 value= readl(RK29_GRF_BASE + 0xbc);
	value = (value & 0xfff7ff) | (0x400);
	writel(value, RK29_GRF_BASE + 0xbc);
	return 0;
}

static int rk29_rmii_io_init(void)
{
	int err;

	//phy power gpio
	err = gpio_request(RK29_PIN6_PB0, "phy_power_en");
	if (err) {
		gpio_free(RK29_PIN6_PB0);
		printk("-------request RK29_PIN6_PB0 fail--------\n");
		return -1;
	}
	//phy power down
	gpio_direction_output(RK29_PIN6_PB0, GPIO_LOW);
	gpio_set_value(RK29_PIN6_PB0, GPIO_LOW);

	return 0;
}

static int rk29_rmii_io_deinit(void)
{
	//phy power down
	gpio_direction_output(RK29_PIN6_PB0, GPIO_LOW);
	gpio_set_value(RK29_PIN6_PB0, GPIO_LOW);
	//free
	gpio_free(RK29_PIN6_PB0);
	return 0;
}

static int rk29_rmii_power_control(int enable)
{
	if (enable) {
		//enable phy power
		gpio_direction_output(RK29_PIN6_PB0, GPIO_HIGH);
		gpio_set_value(RK29_PIN6_PB0, GPIO_HIGH);
	}
	else {
		gpio_direction_output(RK29_PIN6_PB0, GPIO_LOW);
		gpio_set_value(RK29_PIN6_PB0, GPIO_LOW);
	}
	return 0;
}

struct rk29_vmac_platform_data rk29_vmac_pdata = {
	.vmac_register_set = rk29_vmac_register_set,
	.rmii_io_init = rk29_rmii_io_init,
	.rmii_io_deinit = rk29_rmii_io_deinit,
	.rmii_power_control = rk29_rmii_power_control,
};
#endif

/*****************************************************************************************
 * spi devices
 * author: cmc@rock-chips.com
 *****************************************************************************************/
#define SPI_CHIPSELECT_NUM 2
static struct spi_cs_gpio rk29xx_spi0_cs_gpios[SPI_CHIPSELECT_NUM] = {
    {
		.name = "spi0 cs0",
		.cs_gpio = RK29_PIN2_PC1,
		.cs_iomux_name = GPIO2C1_SPI0CSN0_NAME,
		.cs_iomux_mode = GPIO2H_SPI0_CSN0,
	},
	{
		.name = "spi0 cs1",
		.cs_gpio = RK29_PIN1_PA4,
		.cs_iomux_name = GPIO1A4_EMMCWRITEPRT_SPI0CS1_NAME,//if no iomux,set it NULL
		.cs_iomux_mode = GPIO1L_SPI0_CSN1,
	}
};

static struct spi_cs_gpio rk29xx_spi1_cs_gpios[SPI_CHIPSELECT_NUM] = {
    {
		.name = "spi1 cs0",
		.cs_gpio = RK29_PIN2_PC5,
		.cs_iomux_name = GPIO2C5_SPI1CSN0_NAME,
		.cs_iomux_mode = GPIO2H_SPI1_CSN0,
	},
	{
		.name = "spi1 cs1",
		.cs_gpio = RK29_PIN1_PA3,
		.cs_iomux_name = GPIO1A3_EMMCDETECTN_SPI1CS1_NAME,//if no iomux,set it NULL
		.cs_iomux_mode = GPIO1L_SPI1_CSN1,
	}
};

static int spi_io_init(struct spi_cs_gpio *cs_gpios, int cs_num)
{
#if 1
		int i;
		if (cs_gpios) {
			for (i=0; i<cs_num; i++) {
				rk29_mux_api_set(cs_gpios[i].cs_iomux_name, cs_gpios[i].cs_iomux_mode);
			}
		}
#endif
	return 0;
}

static int spi_io_deinit(struct spi_cs_gpio *cs_gpios, int cs_num)
{
	return 0;
}

static int spi_io_fix_leakage_bug(void)
{
#if 0
	gpio_direction_output(RK29_PIN2_PC1, GPIO_LOW);
#endif
	return 0;
}

static int spi_io_resume_leakage_bug(void)
{
#if 0
	gpio_direction_output(RK29_PIN2_PC1, GPIO_HIGH);
#endif
	return 0;
}

struct rk29xx_spi_platform_data rk29xx_spi0_platdata = {
	.num_chipselect = SPI_CHIPSELECT_NUM,
	.chipselect_gpios = rk29xx_spi0_cs_gpios,
	.io_init = spi_io_init,
	.io_deinit = spi_io_deinit,
	.io_fix_leakage_bug = spi_io_fix_leakage_bug,
	.io_resume_leakage_bug = spi_io_resume_leakage_bug,
};

struct rk29xx_spi_platform_data rk29xx_spi1_platdata = {
	.num_chipselect = SPI_CHIPSELECT_NUM,
	.chipselect_gpios = rk29xx_spi1_cs_gpios,
	.io_init = spi_io_init,
	.io_deinit = spi_io_deinit,
	.io_fix_leakage_bug = spi_io_fix_leakage_bug,
	.io_resume_leakage_bug = spi_io_resume_leakage_bug,
};

/*****************************************************************************************
 * xpt2046 touch panel
 * author: hhb@rock-chips.com
 *****************************************************************************************/
#if defined(CONFIG_TOUCHSCREEN_XPT2046_NORMAL_SPI) || defined(CONFIG_TOUCHSCREEN_XPT2046_TSLIB_SPI)
#define XPT2046_GPIO_INT           RK29_PIN4_PD5 //中断???#define DEBOUNCE_REPTIME  3

static struct xpt2046_platform_data xpt2046_info = {
	.model			= 2046,
	.keep_vref_on 	= 1,
	.swap_xy		= 0,
	.debounce_max		= 7,
	.debounce_rep		= DEBOUNCE_REPTIME,
	.debounce_tol		= 20,
	.gpio_pendown		= XPT2046_GPIO_INT,
	.pendown_iomux_name = GPIO4D5_CPUTRACECTL_NAME,	
	.pendown_iomux_mode = GPIO4H_GPIO4D5,	
	.touch_virtualkey_length = 60,
	.penirq_recheck_delay_usecs = 1,
#if defined(CONFIG_TOUCHSCREEN_480X800)
	.x_min			= 0,
	.x_max			= 480,
	.y_min			= 0,
	.y_max			= 800,
	.touch_ad_top = 3940,
	.touch_ad_bottom = 310,
	.touch_ad_left = 3772,
	.touch_ad_right = 340,
#elif defined(CONFIG_TOUCHSCREEN_800X480)
	.x_min			= 0,
	.x_max			= 800,
	.y_min			= 0,
	.y_max			= 480,
	.touch_ad_top = 2447,
	.touch_ad_bottom = 207,
	.touch_ad_left = 5938,
	.touch_ad_right = 153,
#elif defined(CONFIG_TOUCHSCREEN_320X480)
	.x_min			= 0,
	.x_max			= 320,
	.y_min			= 0,
	.y_max			= 480,
	.touch_ad_top = 3166,
	.touch_ad_bottom = 256,
	.touch_ad_left = 3658,
	.touch_ad_right = 380,
#endif	
};
#elif defined(CONFIG_TOUCHSCREEN_XPT2046_CBN_SPI)
static struct xpt2046_platform_data xpt2046_info = {
	.model			= 2046,
	.keep_vref_on 	= 1,
	.swap_xy		= 0,
	.debounce_max		= 7,
	.debounce_rep		= DEBOUNCE_REPTIME,
	.debounce_tol		= 20,
	.gpio_pendown		= XPT2046_GPIO_INT,
	.pendown_iomux_name = GPIO4D5_CPUTRACECTL_NAME,	
	.pendown_iomux_mode = GPIO4H_GPIO4D5,	
	.touch_virtualkey_length = 60,
	.penirq_recheck_delay_usecs = 1,
	
#if defined(CONFIG_TOUCHSCREEN_480X800)
	.x_min			= 0,
	.x_max			= 480,
	.y_min			= 0,
	.y_max			= 800,
	.screen_x = { 70,  410, 70, 410, 240},
	.screen_y = { 50, 50,  740, 740, 400},
	.uncali_x_default = {  3267,  831, 3139, 715, 1845 },
	.uncali_y_default = { 3638,  3664, 564,  591, 2087 },
#elif defined(CONFIG_TOUCHSCREEN_800X480)
	.x_min			= 0,
	.x_max			= 800,
	.y_min			= 0,
	.y_max			= 480,
	.screen_x[5] = { 50, 750,  50, 750, 400};
  	.screen_y[5] = { 40,  40, 440, 440, 240};
	.uncali_x_default[5] = { 438,  565, 3507,  3631, 2105 };
	.uncali_y_default[5] = {  3756,  489, 3792, 534, 2159 };
#elif defined(CONFIG_TOUCHSCREEN_320X480)
	.x_min			= 0,
	.x_max			= 320,
	.y_min			= 0,
	.y_max			= 480,
	.screen_x[5] = { 50, 270,  50, 270, 160}; 
	.screen_y[5] = { 40,  40, 440, 440, 240}; 
	.uncali_x_default[5] = { 812,  3341, 851,  3371, 2183 };
	.uncali_y_default[5] = {  442,  435, 3193, 3195, 2004 };
#endif	
};
#endif

#if defined(CONFIG_SERIAL_SC8800)
static struct plat_sc8800 sc8800_plat_data = {
	.slav_rts_pin = RK29_PIN4_PD4,
	.slav_rdy_pin = RK29_PIN4_PD1,
	.master_rts_pin = RK29_PIN4_PD2,
	.master_rdy_pin = RK29_PIN4_PD3,
	//.poll_time = 100,
};
#endif

static struct spi_board_info board_spi_devices[] = {
#if defined(CONFIG_TOUCHSCREEN_XPT2046_SPI)
	{
		.modalias	= "xpt2046_ts",
		.chip_select	= 0,// 2,
		.max_speed_hz	= 125 * 1000 * 26,/* (max sample rate @ 3V) * (cmd + data + overhead) */
		.bus_num	= 0,
		.irq = XPT2046_GPIO_INT,
		.platform_data = &xpt2046_info,
	},
#endif

#if defined(CONFIG_MFD_WM831X_SPI)
	{
		.modalias	= "wm8310",
		.chip_select	= 1,
		.max_speed_hz	= 1*1000*1000,
		.bus_num	= 1,
		.irq            = RK29_PIN4_PD0,
		.platform_data = &wm831x_platdata,
	},
#endif
#if defined(CONFIG_SERIAL_SC8800)
	{
		.modalias  = "sc8800",
		.bus_num = 0,
		.platform_data = &sc8800_plat_data,
		.max_speed_hz  = 12*1000*1000,
		.chip_select   = 0,
	},
#endif
};


/**********************************************************************************************
 *
 * The virtual keys for android "back", "home", "menu", "search", these four keys are touch key
 * on the touch screen panel. (added by hhb@rock-chips.com 2011.03.31)
 *
 ***********************************************************************************************/
static ssize_t rk29xx_virtual_keys_show(struct kobject *kobj,
			struct kobj_attribute *attr, char *buf)
{
#if (defined(CONFIG_TOUCHSCREEN_XPT2046_SPI) && defined(CONFIG_TOUCHSCREEN_480X800)) \
	|| defined(CONFIG_TOUCHSCREEN_HX8520_IIC) || defined(CONFIG_TOUCHSCREEN_GT801_IIC)
	/* center: x: home: 50, menu: 184, back: 315, search 435, y: 830*/
    /* centerx;centery;width;height; */
	return sprintf(buf,
		__stringify(EV_KEY) ":" __stringify(KEY_BACK)	    ":315:815:120:50"     //":50:830:98:50"  //":210:796:98:50"
		":" __stringify(EV_KEY) ":" __stringify(KEY_MENU)   ":85:815:88:50"   // ":184:830:120:50"  // ":435:796:120:50"
		":" __stringify(EV_KEY) ":" __stringify(KEY_HOME)   ":184:815:100:50"   //":315:830:100:50"  //":320:796:100:50"
		":" __stringify(EV_KEY) ":" __stringify(KEY_SEARCH) ":435:815:88:50"   //":50:815:98:50"    //   //":85:796:88:50"
		"\n");
#endif
	return 0;
}

static struct kobj_attribute rk29xx_virtual_keys_attr = {
	.attr = {
#if defined(CONFIG_TOUCHSCREEN_XPT2046_SPI)
		.name = "virtualkeys.xpt2046-touchscreen",
#elif defined(CONFIG_TOUCHSCREEN_HX8520_IIC)
        .name = "virtualkeys.hx8520-touchscreen",
#elif defined(CONFIG_TOUCHSCREEN_GT801_IIC)
		.name = "virtualkeys.gt801-touchscreen",
#elif defined(CONFIG_TOUCHSCREEN_ILI2102_IIC)
		.name = "virtualkeys.ili2102-touchscreen",		
#else
		.name = "virtualkeys",
#endif
		.mode = S_IRUGO,
	},
	.show = &rk29xx_virtual_keys_show,
};

static struct attribute *rk29xx_properties_attrs[] = {
	&rk29xx_virtual_keys_attr.attr,
	NULL
};

static struct attribute_group rk29xx_properties_attr_group = {
	.attrs = rk29xx_properties_attrs,
};
static int rk29xx_virtual_keys_init(void)
{
	int ret;
	struct kobject *properties_kobj;
	printk("rk29xx_virtual_keys_init \n");
	properties_kobj = kobject_create_and_add("board_properties", NULL);
	if (properties_kobj)
		ret = sysfs_create_group(properties_kobj,
				&rk29xx_properties_attr_group);
	if (!properties_kobj || ret)
	{
		pr_err("failed to create board_properties\n");
	}
	return ret;
}


static void __init rk29_gic_init_irq(void)
{
	gic_dist_init(0, (void __iomem *)RK29_GICPERI_BASE, 32);
	gic_cpu_init(0, (void __iomem *)RK29_GICCPU_BASE);
}

static void __init machine_rk29_init_irq(void)
{
	rk29_gic_init_irq();
	rk29_gpio_init();
}

#define POWER_ON_PIN RK29_PIN4_PA4

static void rk29_pm_power_restart(void)
{
	printk("%s,line=%d\n",__FUNCTION__,__LINE__);
	mdelay(2);
#if defined(CONFIG_MFD_WM831X)
	wm831x_device_restart(gWm831x);
#endif

}

static void rk29_pm_power_off(void)
{
	printk(KERN_ERR "rk29_pm_power_off start...\n");
	gpio_direction_output(POWER_ON_PIN, GPIO_LOW);
#if defined(CONFIG_MFD_WM831X)
	if(wm831x_read_usb(gWm831x))
	rk29_pm_power_restart();	//if charging then restart
	else
	wm831x_device_shutdown(gWm831x);//else shutdown
#endif
	while (1);
}

static struct cpufreq_frequency_table freq_table[] =
{
	{ .index = 1200000, .frequency =  408000 },
	{ .index = 1200000, .frequency =  600000 },
	{ .index = 1200000, .frequency =  816000 },
	{ .index = 1350000, .frequency = 1008000 },
	{ .frequency = CPUFREQ_TABLE_END },
};

static void __init machine_rk29_board_init(void)
{
	rk29_board_iomux_init();
    
	gpio_request(POWER_ON_PIN,"poweronpin");
	gpio_set_value(POWER_ON_PIN, GPIO_HIGH);
	gpio_direction_output(POWER_ON_PIN, GPIO_HIGH);
	pm_power_off = rk29_pm_power_off;
	//arm_pm_restart = rk29_pm_power_restart;

	board_update_cpufreq_table(freq_table);

	platform_add_devices(devices, ARRAY_SIZE(devices));
#ifdef CONFIG_I2C0_RK29
	i2c_register_board_info(default_i2c0_data.bus_num, board_i2c0_devices,
			ARRAY_SIZE(board_i2c0_devices));
#endif
#ifdef CONFIG_I2C1_RK29
	i2c_register_board_info(default_i2c1_data.bus_num, board_i2c1_devices,
			ARRAY_SIZE(board_i2c1_devices));
#endif
#ifdef CONFIG_I2C2_RK29
	i2c_register_board_info(default_i2c2_data.bus_num, board_i2c2_devices,
			ARRAY_SIZE(board_i2c2_devices));
#endif
#ifdef CONFIG_I2C3_RK29
	i2c_register_board_info(default_i2c3_data.bus_num, board_i2c3_devices,
			ARRAY_SIZE(board_i2c3_devices));
#endif

	spi_register_board_info(board_spi_devices, ARRAY_SIZE(board_spi_devices));

#ifdef CONFIG_WIFI_CONTROL_FUNC
	rk29sdk_wifi_bt_gpio_control_init();
	rk29sdk_init_wifi_mem();
#endif

#if (defined(CONFIG_TOUCHSCREEN_XPT2046_SPI) && defined(CONFIG_TOUCHSCREEN_480X800)) \
	|| defined(CONFIG_TOUCHSCREEN_HX8520_IIC) || defined(CONFIG_TOUCHSCREEN_GT801_IIC)
	rk29xx_virtual_keys_init();
#endif

}

static void __init machine_rk29_fixup(struct machine_desc *desc, struct tag *tags,
					char **cmdline, struct meminfo *mi)
{
	mi->nr_banks = 1;
	mi->bank[0].start = RK29_SDRAM_PHYS;
	mi->bank[0].node = PHYS_TO_NID(RK29_SDRAM_PHYS);
	mi->bank[0].size = LINUX_SIZE;
#if SDRAM_SIZE > SZ_512M
	mi->nr_banks = 2;
	mi->bank[1].start = RK29_SDRAM_PHYS + SZ_512M;
	mi->bank[1].size = SDRAM_SIZE - SZ_512M;
#endif
}

static void __init machine_rk29_mapio(void)
{
	rk29_map_common_io();
	rk29_setup_early_printk();
	rk29_sram_init();
	rk29_clock_init2(periph_pll_96mhz, codec_pll_300mhz, false);
	rk29_iomux_init();
	ddr_init(DDR_TYPE, DDR_FREQ);
}

MACHINE_START(RK29, "RK29board")
	/* UART for LL DEBUG */
	.phys_io	= RK29_UART1_PHYS & 0xfff00000,
	.io_pg_offst	= ((RK29_UART1_BASE) >> 18) & 0xfffc,
	.boot_params	= RK29_SDRAM_PHYS + 0x88000,
	.fixup		= machine_rk29_fixup,
	.map_io		= machine_rk29_mapio,
	.init_irq	= machine_rk29_init_irq,
	.init_machine	= machine_rk29_board_init,
	.timer		= &rk29_timer,
MACHINE_END
