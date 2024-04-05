/*
 *
 * FD655 Driver
 *
 * Copyright (C) 2015 Fdhisi, Inc.
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/of_gpio.h>
#include <linux/ioport.h>
#include <linux/ioctl.h>
#include <linux/device.h>

#include <linux/errno.h>
#include <linux/mutex.h>


#include <linux/miscdevice.h>
#include <linux/fs.h>

#include <linux/fcntl.h>
#include <linux/poll.h>
//#include <linux/mfd/acx00-mfd.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/rwsem.h>
#include <linux/timer.h>
#include <linux/gpio.h>
#include <linux/err.h>
#include <linux/ctype.h>
#include <linux/sysfs.h>
#include <linux/device.h>
#include <linux/io.h>


#include <linux/pinctrl/consumer.h>
#include "fd655.h"


static FD655_DEV *pdata = NULL;
static int pvr = 0;
static struct class *fd655_sw_class;
//static int led_flag = 1;    
/** 
 * @brief   转换字符为数码管的显示码
 * @param   cTemp 待转换为显示码的字符
 * @return  显示码值,8位无符号
 * @note    码值见BCD_decode_tab[LEDMAPNUM]，如遇到无法转换的字符返回0  
 */ 
static u_int8 Led_Get_Code(char cTemp)
{
	u_int8 i, bitmap=0x00;

	for(i=0; i<LEDMAPNUM; i++)
	{
		if(LED_decode_tab[i].character == cTemp)
		{
			bitmap = LED_decode_tab[i].bitmap;
			break;
		}
	}

	return bitmap;
}

/*
 * struct gpio_config - gpio config info
 * @gpio:      gpio global index, must be unique
 * @mul_sel:   multi sel val: 0 - input, 1 - output.
 * @pull:      pull val: 0 - pull up/down disable, 1 - pull up
 * @drv_level: driver level val: 0 - level 0, 1 - level 1
 * @data:      data val: 0 - low, 1 - high, only valid when mul_sel is input/output
 */
struct gpio_config {
	u32	data;
	u32	gpio;
	u32	mul_sel;
	u32	pull;
	u32	drv_level;
};
/*******************************************************************************
*函数名	： FD655_Start()；
*参数	   	:  无		 																	   
*功能描述 	:  FD655操作起始
*函数说明 	： 
*返回值	：  无																   
******************************************************************************/
static void FD655_Start( FD655_DEV *dev )		 	// 操作起始
{
//	FD655_SDA_D_OUT;				 /* 设置SDA为输出方向 */
//	FD655_SCL_D_OUT;				 /* 设置SCL为输出方向 */	
//	FD655_SDA_SET;  				/*发送起始条件的数据信号*/
    gpio_direction_output(dev->dat_pin, 1);
//	FD655_SCL_SET;
	gpio_direction_output(dev->clk_pin, 1);
	FD655_DELAY_1us;
//	FD655_SDA_CLR;					  /*发送起始信号*/
    gpio_direction_output(dev->dat_pin, 0);
	FD655_DELAY_1us;      
//	FD655_SCL_CLR;					  /*钳住I2C总线，准备发送或接收数据 */
    gpio_direction_output(dev->clk_pin, 0);
}					     

/*******************************************************************************
*函数名	： FD655_Stop()；
*参数	   	:  无		 																	   
*功能描述 	:  FD655操作结束
*函数说明 	： 
*返回值	：  无																   
******************************************************************************/
static void FD655_Stop( FD655_DEV *dev )				  // 操作结束
{
//	FD655_SDA_D_OUT;					 /* 设置SDA为输出方向 */	
//	FD655_SCL_D_OUT;				 	/* 设置SCL为输出方向 */	
//	FD655_SDA_CLR;
    gpio_direction_output(dev->dat_pin, 0);
	FD655_DELAY_1us;
//	FD655_SCL_SET;
    gpio_direction_output(dev->clk_pin, 1);
	FD655_DELAY_1us;
//	FD655_SDA_SET;						 /*发送I2C总线结束信号*/
    gpio_direction_output(dev->dat_pin, 1);
    FD655_DELAY_1us;
}

/*******************************************************************************
*函数名	： FD655_Writebyte( uchar dat)；
*参数	   	:  dat,一个字节数据写入		 																	   
*功能描述 	:  FD655写入一个字节
*函数说明 	： 
*返回值	：  无																   
******************************************************************************/
static void FD655_Writebyte(u_int8 dat,FD655_DEV *dev)		 
{
	u_int8 i;
	for( i = 0; i != 8; i++ )
	{
		if( dat & 0x80 ) 
		{
		   // FD655_SDA_SET;
		   gpio_direction_output(dev->dat_pin, 1);
		}
		else 
		{
		   // FD655_SDA_CLR;
		   gpio_direction_output(dev->dat_pin, 0);
		}
		FD655_DELAY_1us;
	//	FD655_SCL_SET;
	    gpio_direction_output(dev->clk_pin, 1);
		dat <<= 1;
		FD655_DELAY_1us;  // 可选延时
	//	FD655_SCL_CLR;
	    gpio_direction_output(dev->clk_pin, 0);
	}
	//FD655_SDA_SET;
	gpio_direction_output(dev->dat_pin, 1);
//	gpio_direction_output(dev->dat_pin);
	FD655_DELAY_1us;
	//FD655_SCL_SET;
	gpio_direction_output(dev->clk_pin, 1);
	FD655_DELAY_1us;
	//FD655_SCL_CLR;
	gpio_direction_output(dev->clk_pin, 0);
}
#if 0
/*******************************************************************************
*函数名	：  uchar FD655_Readbyte( )；
*参数	   	:  	无	 																	   
*功能描述 	:   CPU读取FD655的一个字节
*函数说明 	：  
*返回值	：  dat, 读取一个字节														
******************************************************************************/
static u_int8 FD655_Readbyte(FD655_DEV *dev)				  
{
	u_int8 dat,i;
	//FD655_SDA_D_IN;
	dat = 0;
	for( i = 0; i != 8; i++ )
	{
		FD655_DELAY_1us;  // 可选延时
		//FD655_SCL_SET;
		gpio_direction_output(dev->clk_pin, 1);
		FD655_DELAY_1us;  // 可选延时
		dat <<= 1;
		//if( FD655_SDA_IN ) 
		if( gpio_get_value(dev->dat_pin) ) 
			dat++;
		//FD655_SCL_CLR;
		gpio_direction_output(dev->clk_pin, 0);
	}
//	FD655_SDA_D_OUT;
//	FD655_SDA_SET;
    gpio_direction_output(dev->dat_pin, 1);
	FD655_DELAY_1us;
//	FD655_SCL_SET;
    gpio_direction_output(dev->clk_pin, 1);
	FD655_DELAY_1us;
//	FD655_SCL_CLR;
    gpio_direction_output(dev->clk_pin, 0);
	return dat;
}
#endif
/*******************************************************************************
*函数名	： FD655_Command( uchar cmd)；
*参数	   	:  cmd, 控制命令，具体命令请参见FD655.H		 																	   
*功能描述 	:  对FD655进行控制，
*函数说明 	： 对其亮度，睡眠，按键显示开启等功能操作
*返回值	：  无																   
******************************************************************************/
void FD655_Command( u_int8 cmd ,FD655_DEV *dev)		  		
{									
	FD655_Start(dev);
	FD655_Writebyte(SET,dev);
	FD655_Writebyte(cmd,dev);
	FD655_Stop(dev);	
}

/*******************************************************************************
*函数名	： FD655_Disp( uchar address,uchar dat)；
*参数	   	:  address, 位地址，dat,段数据		 																	   
*功能描述 	:  对FD655单个位进行操作显示函数
*函数说明 	： add 68,6a,6c,6e 66对应DIG1-DIG4以及DIG5 dat表示显示的段码数据共阴
*返回值	：  无																   
******************************************************************************/
void FD655_Disp(u_int8 address ,u_int8 dat,FD655_DEV *dev)
{
	FD655_Start(dev);
	FD655_Writebyte(address,dev);
	FD655_Writebyte(dat,dev);
	FD655_Stop(dev);
}
#if 0

/*******************************************************************************
*函数名	： uchar FD655_KeyScan()；
*参数	   	:  add, 位地址，dat,段数据		 																	   
*功能描述 	:  FD655按键扫描函数
*函数说明 	： 如若没按键 ，返回0 ，如果有按键，返回按键值
*返回值	：  keytemp , 返回按键代码																   
******************************************************************************/
uchar FD655_KeyScan(FD655_DEV *dev)   			
{	
	u_int8 keytemp;
	FD655_Start(dev);
	FD655_Writebyte( READKEY ,dev);
	keytemp=FD655_Readbyte(dev);
	FD655_Stop(dev);
	if((keytemp&0x40)==0)
	{	
		keytemp=0;
	}
	return keytemp;	
}

#endif

/*******************************************************************************
*函数名	： LedShow(uchar *acFPStr)；
*参数	   	:  无		 																	   
*功能描述 	:  输入4位ASCII码让数码管显示
*函数说明 	： 如输入"OPEN"	，则数码管既可显示OPEN
*返回值	：  无																   
******************************************************************************/
void LedShow( u_int8 *acFPStr, FD655_DEV *dev)
{
	u_int8 i, iLenth;
	u_int8 dat[5]={0};
	if( strcmp(acFPStr, "") == 0 )
	{
		return;
	}
	iLenth = strlen(acFPStr);
	if(iLenth>5)
		iLenth = 5;
	
	for(i=0; i<iLenth; i++)
	{
		dat[i] = Led_Get_Code(acFPStr[i]);
	}
	//Send display data
	
	FD655_Disp(DIG1,dat[0],dev);
	FD655_Disp(DIG2,dat[1],dev);
	FD655_Disp(DIG3,dat[2],dev);
	FD655_Disp(DIG4,dat[3],dev);
	FD655_Disp(DIG5,dat[4],dev);
}







static int fd655_dev_open(struct inode *inode, struct file *file)
{
    
    file->private_data = pdata;
	FD655_Command(FD655SYSON,pdata);
//	pr_dbg("fd655_dev_open now.............................\r\n");
    return 0;
}

static int fd655_dev_release(struct inode *inode, struct file *file)
{
	file->private_data = NULL;
	//modify for display longer

	// FD655_Command(~FD655SYSON,pdata);
					//FD655_Command(FD655SYSON,pdata);
					FD655_Command(0x00,pdata);
	pr_error("succes to close  fd655_dev in release 1027.............\n");

	/*pr_dbg("try not to close  fd655_dev in release.............\n");*/
    return 0;
}
/*static long fd655_dev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	pr_dbg("fd655_dev_ioctl");
	int ret = 0;
	ret = __get_user(cmd, (int __user *) arg);
		switch((int)arg)  
		{  
			case 0:  
			{  		
				pr_dbg("case 0 sunvell-1025.\n");
				FD655_Command(0x00,pdata);
				break;  
			}  
			default:  
			{  
				return -ENOTTY;  
			}  
		}  
	return 1;  
}*/


static ssize_t  fd655_dev_write(struct file *filp, const char __user *buf,		size_t count, loff_t *f_pos)
{

		FD655_DEV *dev;
		unsigned long	missing;
		size_t			status = 0, i = 0;
		char data[5] = {0};
		char tmp[5]= {0};
		dev = filp->private_data;
		if (count >5)		
			count = 5;
		missing = copy_from_user(data, buf, count);
		pr_dbg("write buf: %s \r\n",data);
		if (missing == 0)
		{	
			for(i=0; i<count; i++)
			{
				tmp[i] = Led_Get_Code(data[i]);
				pr_dbg("Led_Get_Code buf: %x \r\n",tmp[i]);
			}
			
			FD655_Disp(DIG1,tmp[0],dev);
			FD655_Disp(DIG2,tmp[1],dev);
			FD655_Disp(DIG3,tmp[2],dev);
			FD655_Disp(DIG4,tmp[3],dev);
			FD655_Disp(DIG5,data[4],dev);
			status = count;
		} 
		pr_dbg("fd655_dev_write count : %zu\n",count );
		return status;

}

//	

static struct file_operations fd655_fops = {
	.owner		=	THIS_MODULE,
	.open		=	fd655_dev_open,
	.release	=	fd655_dev_release,
	.write      =   fd655_dev_write,
	
};
static struct miscdevice fd655_device = {
	.minor	=	MISC_DYNAMIC_MINOR,
	.name	=	DEV_NAME,
	.fops	=	&fd655_fops,
};




static int register_fd655_driver(void)
{
    int ret = 0;
    ret = misc_register(&fd655_device);
	if(ret)
		pr_dbg("%s: failed to add fd655 module\n", __func__);
	else
		pr_dbg("%s: Successed to add fd655  module \n", __func__);
    return ret;
}

static void deregister_fd655_driver(void) 
{
	
	misc_deregister(&fd655_device);
	
}


static ssize_t fd655_vplay_flag_show(struct class *dev,
                 struct class_attribute *attr, char *buf){
	return sprintf(buf, "%d\n", pdata->vplay_flag);	
}

static ssize_t fd655_vplay_flag_store(struct class *dev,
                 struct class_attribute *attr,
                 const char *buf, size_t count){
    sscanf(buf, "%d", &pdata->vplay_flag);
	return count;
}

static ssize_t fd655_vpause_flag_show(struct class *dev,
                 struct class_attribute *attr, char *buf){
        return sprintf(buf, "%d\n", pdata->vpause_flag);
}

static ssize_t fd655_vpause_flag_store(struct class *dev,
                 struct class_attribute *attr,
                 const char *buf, size_t count){
    sscanf(buf, "%d", &pdata->vpause_flag);
        return count;
}



static ssize_t fd655_panel_show(struct class *dev,
                 struct class_attribute *attr, char *buf){
	return sprintf(buf, "%s\n", pdata->wbuf);
}

static ssize_t fd655_panel_store(struct class *dev,
                 struct class_attribute *attr,
                 const char *buf, size_t count){

        FD655_Command(FD655SYSON,pdata);
	sscanf(buf, "%s", pdata->wbuf);
	LedShow(pdata->wbuf,pdata);
	return count;
}

static ssize_t pvr_store(struct class *class,
			struct class_attribute *attr,	const char *buf, size_t count) {
	pvr = simple_strtoul(buf, NULL, 16);
	if(pvr == 1){
		FD655_Command(0x00,pdata);
	}else{
		FD655_Command(FD655SYSON,pdata);
	}
	printk("pvr set to %d\n", pvr);
	return count;
}

static struct class_attribute fd655_class_attrs[] = {
        __ATTR(vplay_flag, S_IRUGO|S_IWUSR|S_IWGRP, fd655_vplay_flag_show, fd655_vplay_flag_store),
        __ATTR(vpause_flag, S_IRUGO|S_IWUSR|S_IWGRP, fd655_vpause_flag_show, fd655_vpause_flag_store),
        __ATTR(panel, S_IRUGO|S_IWUSR|S_IWGRP, fd655_panel_show, fd655_panel_store),
		__ATTR(pvr, S_IRUGO|S_IWUSR|S_IWGRP, NULL, pvr_store),
        __ATTR_NULL
};

static int fd655_driver_remove(struct platform_device *pdev)
{   
    deregister_fd655_driver();
	class_unregister(&pdata->sysfs);
    fb_unregister_client(&pdata->notifier);
#ifdef CONFIG_OF
	gpio_free(pdata->clk_pin);
   	gpio_free(pdata->dat_pin);
    pr_error("======= gpio free in remove========");	
	kfree(pdata);
#endif
    return 0;
}

static int fd655_driver_suspend(struct platform_device *dev, pm_message_t state)
{
    /* disable gpio_free to fix kernel warning by terry
    gpio_free(pdata->clk_pin);
   	gpio_free(pdata->dat_pin);*/
    pr_error("==========gpio free in suspend might cause kernel warning, so disable it !!!");
	return 0;
}

static int fd655_driver_resume(struct platform_device *dev)
{
	/*int ret = -1;
    pr_error("fd655_driver_resume in resume");
	ret = gpio_request(pdata->clk_pin, DEV_NAME);
	if(ret){
		pr_error("---%s----can not request pin %d\n",__func__,pdata->clk_pin);
	}	
	ret = gpio_request(pdata->dat_pin, DEV_NAME);
	if(ret){
		pr_error("---%s----can not request pin %d\n",__func__,pdata->dat_pin);
	}	*/
    pr_error("fd655_driver_resume in resume");
    return 0;
}



static int fd655_driver_probe(struct platform_device *pdev)
{
//	pr_error( "fd655_driver_probe\n");
	int state=-EINVAL;
    
//	char buf[32];
	int ret;
	
//	unsigned int gpio;
//	unsigned long out_init;
	//enum of_gpio_flags gpio_flags;
	struct device *dev = &pdev->dev;
	//struct device_node *np = dev->of_node;
 // 	struct gpio_config config;
	
//	const char *str;
//	struct gpio_config  *gpio_p = NULL;
	//dts add for h6 by christan  end


	
	pdata = kzalloc(sizeof(*pdata), GFP_KERNEL);
    if (!pdata) {
        pr_error("platform data is required!\n");
        state = -EINVAL;
        goto get_fd655_mem_fail;
    }

	//pdata->clk_pin = of_get_named_gpio_flags(np, "leds_clk", 0, (enum of_gpio_flags *)&config);
	pdata->clk_pin = 148;
		pr_error("leds_clk = %d\n",pdata->clk_pin);
	if (!gpio_is_valid(pdata->clk_pin)) {
		dev_err(dev, "get gpio leds_clk failed\n");
	  }
	//pdata->dat_pin = of_get_named_gpio_flags(np, "leds_dat", 0, (enum of_gpio_flags *)&config);
	pdata->dat_pin = 147;
		pr_error("leds_dat = %d\n",pdata->dat_pin);
		if (!gpio_is_valid(pdata->clk_pin)) {
		dev_err(dev, "get gpio leds_dat failed\n");
	  }
	  
	pdata->sysfs.name = "fd655";
	pdata->vpause_flag = 0;
    pdata->vplay_flag = 0;

	fd655_sw_class=class_create(THIS_MODULE, "fd655");
		if (IS_ERR(fd655_sw_class)) {
		ret = PTR_ERR(fd655_sw_class);
		goto error_sysfs;
	}
	ret = class_create_file(fd655_sw_class, &fd655_class_attrs[0]);
       if (ret) {
               printk(KERN_ERR "%s:Fail to creat gyro class file\n", __func__);
               return ret;
       }
	 	ret = class_create_file(fd655_sw_class, &fd655_class_attrs[1]);
       if (ret) {
               printk(KERN_ERR "%s:Fail to creat gyro class file\n", __func__);
               return ret;
       }
	ret = class_create_file(fd655_sw_class, &fd655_class_attrs[2]);
       if (ret) {
               printk(KERN_ERR "%s:Fail to creat gyro class file\n", __func__);
               return ret;
       }
	ret = class_create_file(fd655_sw_class, &fd655_class_attrs[3]);
       if (ret) {
               printk(KERN_ERR "%s:Fail to creat gyro class file\n", __func__);
               return ret;
       }	   

	ret = gpio_request(pdata->clk_pin, DEV_NAME);
	if(ret){
		pr_error("---%s----can not request pin %d\n",__func__,pdata->clk_pin);
		goto get_fd655_mem_fail ;
	}	
	ret = gpio_request(pdata->dat_pin, DEV_NAME);
	if(ret){
		pr_error("---%s----can not request pin %d\n",__func__,pdata->dat_pin);
		goto get_fd655_mem_fail ;
	}	

	platform_set_drvdata(pdev, pdata);
    register_fd655_driver();
	
	
    //DTS 2018-04-13 Add for displaying boot  by terry
    FD655_Command(FD655SYSON,pdata);
    pr_error( "====================Command opne in probe=============\n");    
    /*FD655_Disp(DIG1,0x67,pdata);  //b
	FD655_Disp(DIG2,0x63,pdata);  //o
	FD655_Disp(DIG3,0x63,pdata);  //o
	FD655_Disp(DIG4,0x47,pdata);  //t
    FD655_Disp(DIG5,0x00,pdata);  //none    
    */
    FD655_Disp(DIG1,0x7C,pdata);  //b
	FD655_Disp(DIG2,0x5C,pdata);  //o
	FD655_Disp(DIG3,0x5C,pdata);  //o
	FD655_Disp(DIG4,0x78,pdata);  //t
    FD655_Disp(DIG5,0x00,pdata);  //none  
    pr_error( "====================Display boot in probe=============\n"); 
    //DTS
    return 0;
	
    get_fd655_mem_fail:
			kfree(pdata);
    error_sysfs:	
    return state;
}


#ifdef CONFIG_OF
static const struct of_device_id fd655_dt_match[]={
	{	.compatible = "rockchip,fd655_dev",
	},
	{},
};
//MODULE_DEVICE_TABLE(of,fd655_dt_match);
#else
#define fd655_dt_match NULL
#endif




static struct platform_driver fd655_driver = {
    .probe      = fd655_driver_probe,
    .remove     = fd655_driver_remove,
    .suspend    = fd655_driver_suspend,
    .resume     = fd655_driver_resume,
    .driver     = {
        .name   = DEV_NAME,
		.owner	= THIS_MODULE,
		.of_match_table = fd655_dt_match,
    },
};

static int __init fd655_driver_init(void)
{
    pr_dbg( "Fd655 Driver init.\n");
	if(platform_driver_register(&fd655_driver)) {
		pr_err("gpio user platform_driver_register  fail\n");
		return -1;
	}
	pr_err("gpio user platform_driver_register  sucess\n");
	return 0;
}

static void __exit fd655_driver_exit(void)
{
    pr_error("Fd655 Driver exit.\n");
    platform_driver_unregister(&fd655_driver);
}

module_init(fd655_driver_init);
module_exit(fd655_driver_exit);

MODULE_AUTHOR("kenneth");
MODULE_DESCRIPTION("fd655 Driver");
MODULE_LICENSE("GPL");










