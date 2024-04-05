#ifndef __H6_FD655H__
#define __H6_FD655H__

#include <linux/delay.h>
#include <linux/of_gpio.h>
#include <linux/io.h>

#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/of.h>
#include <linux/pinctrl/consumer.h>
#include <linux/fb.h>
#include <linux/time.h>

typedef unsigned char   u_int8;
typedef unsigned short  u_int16;	
typedef unsigned long 	u_int32;



#if 0
#define pr_dbg(args...) printk(KERN_ALERT "FD655: " args)
#else
#define pr_dbg(args...)
#endif

#define pr_error(args...) printk(KERN_ALERT "FD655: " args)

#ifndef CONFIG_OF
#define CONFIG_OF
#endif



#define MOD_NAME_CLK       "fd655"
#define MOD_NAME_DAT       "fd655_dat"
#define DEV_NAME           "fd655_dev"


#define 	FD655_DELAY_1us						udelay(4)					    	/* 延时时间 >1us*/
/* **************************************用户不需要修改*********************************************** */
/* **************写入FD628延时部分（具体含义看Datasheet）********************** */
#define 	FD655_DELAY_LOW		     	FD628_DELAY_1us                     		        /* 时钟低电平时间 >500ns*/
#define		FD655_DELAY_HIGH     	 	FD628_DELAY_1us 	   										 				/* 时钟高电平时间 >500ns*/
#define  	FD655_DELAY_BUF		 		 	FD628_DELAY_1us	                     				  	/* 通信结束到下一次通信开始的间隔 >1us*/
#define  	FD655_DELAY_STB					FD628_DELAY_1us





typedef struct _tag_fd655_dev{
	int clk_pin;    
	int dat_pin;
	char wbuf[5];
	struct class sysfs;
//	struct class cls;
         int vplay_flag;
         int vpause_flag;
         struct notifier_block notifier;
}FD655_DEV;

#define LEDMAPNUM 64

 /** Character conversion of digital tube display code*/
typedef struct _led_bitmap
{
	u_int8 character;
	u_int8 bitmap;
} led_bitmap;

/** Character conversion of digital tube display code array*/
static const led_bitmap LED_decode_tab[LEDMAPNUM] = 
{
#if 1
	{'0', 0x3F}, {'1', 0x06}, {'2', 0x5B}, {'3', 0x4F},
	{'4', 0x66}, {'5', 0x6D}, {'6', 0x7D}, {'7', 0x07},
	{'8', 0x7F}, {'9', 0x6F}, {'a', 0x77}, {'A', 0x77},
	{'b', 0x7C}, {'B', 0x7C}, {'c', 0x58}, {'C', 0x39},
	{'d', 0x5E}, {'D', 0x5E}, {'e', 0x79}, {'E', 0x79},
	{'f', 0x71}, {'F', 0x71}, {'I', 0X60}, {'i', 0x60},
	{'L', 0x38}, {'l', 0x38}, {'r', 0x38}, {'R', 0x38},
	{'n', 0x54}, {'N', 0x37}, {'O', 0x3F}, {'o', 0x5c},
	{'p', 0xf3}, {'P', 0x38}, {'S', 0x6D}, {'s', 0x6d},
	{'y', 0x6e}, {'Y', 0x6E}, {'_', 0x08}, {0,   0x3F}, 
	{1,   0x06}, {2,   0x5B}, {3,   0x4F}, {4,   0x66}, 
	{5,   0x6D}, {6,   0x7D}, {7,   0x07}, {8,   0x7F}, 
	{9,   0x6F}, {'!', 0X01}, {'@', 0X02}, {'#', 0X04},
	{'$', 0X08}, {':', 0X10}, {'^', 0X20}, {'&', 0X40},
	{0xC5,0X00}, {0x3b,0x18}, {0xc4,0x08}, {0x3c,0x14},
	{0xc3,0x04}, {0x3d,0x1c}, {0xc2,0x0c}, {'t',0x78}
#else
    {'0', 0x3F}, {'1', 0x30}, {'2', 0x5B}, {'3', 0x79},
	{'4', 0x74}, {'5', 0x6D}, {'6', 0x6F}, {'7', 0x38},
	{'8', 0x7F}, {'9', 0x7D}, {'a', 0x77}, {'A', 0x77},
	{'b', 0x67}, {'B', 0x67}, {'c', 0x58}, {'C', 0x39},
	{'d', 0x5E}, {'D', 0x5E}, {'e', 0x79}, {'E', 0x79},
	{'f', 0x71}, {'F', 0x71}, {'o', 0x63}, {'O', 0x63}, 
    {'t', 0x47}, {'T', 0x47}, {':', 0X10}, {0,   0x3F}, 
	{1,   0x30}, {2,   0x5B}, {3,   0x79}, {4,   0x74}, 
	{5,   0x6D}, {6,   0x6F}, {7,   0x38}, {8,   0x7F}, 
	{9,   0x7D}

#endif 	
};//BCD码字映射



/* ********************************************************************************************* */
// 设置系统参数地址	655
#define READKEY 	0x4f        // 读键
#define SET     	0x48        // 设置参数

//设置系统参数命令
//该命令的输出字节1  为01001000B，即48H；输出字节2为[SLEEP][INTENS]000[KEYB][DISP]B。  
//SET 地址下的命令
#define DISP		0x01		// 开启显示位
#define SLEEP		0x80		// 睡眠控制位
#define KEYB		0x02		// 开启按键扫描位

#define INTENS0		0x00		// 默认亮度 开启内部限流，占空比为4/4

#define INTENS1		0x20		// 1级亮度	开启内部限流，占空比为1/4
#define INTENS2		0x40		// 2级亮度	开启内部限流，占空比为2/4

#define INTENS3		0x60		// 3级亮度	关闭内部限流，占空比为4/4 
								//在3.3V电源电压时可以根据显示亮度需求关闭内部限流以增加驱动电流。

//客户可根据自己需要修改亮度以及按键与显示
#define FD655SYSON 	( DISP|KEYB |INTENS0 ) 		//开启显示键盘默认亮度

//#define DIG1		0x68		// 数码管位0显示
//#define DIG2		0x6a		// 数码管位1显示
//#define DIG3		0x6c		// 数码管位2显示
//#define DIG4		0x6e		// 数码管位3显示

#define DIG1		0x6e
#define DIG2		0x6c
#define DIG3		0x6a
#define DIG4		0x68
#define DIG5		0x66



void FD655_Command( u_int8 cmd ,FD655_DEV *dev)	;

void FD655_Disp(u_int8 address ,u_int8 dat,FD655_DEV *dev);

void LedShow( u_int8 *acFPStr, FD655_DEV *dev);



























#endif
