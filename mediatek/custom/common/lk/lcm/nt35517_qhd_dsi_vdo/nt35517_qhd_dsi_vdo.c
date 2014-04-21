/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

#ifndef BUILD_LK
#include <linux/string.h>
#endif
#include "lcm_drv.h"

#ifdef BUILD_LK
	#include <platform/mt_gpio.h>
	#define LCM_PRINT printf
#elif defined(BUILD_UBOOT)
	#include <asm/arch/mt_gpio.h>
#else
	#include <linux/kernel.h>
	#include <mach/mt_gpio.h>
	#define LCM_PRINT printk
#endif
#define LCM_DBG
#if  defined(LCM_DBG)
#define LCM_DBG(fmt, arg...) \
	LCM_PRINT("[LCM-NT35517-DSI] %s (line:%d) :" fmt "\r\n", __func__, __LINE__, ## arg)
#else
#define LCM_DBG(fmt, arg...) do {} while (0)
#endif

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  (540)
#define FRAME_HEIGHT (960)
#define ACTIVE_WIDTH  										(66.7)
#define ACTIVE_HEIGHT 										(120.4)
//#define LCM_DSI_CMD_MODE

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))

#define REGFLAG_DELAY             							0XFE
#define REGFLAG_END_OF_TABLE      							0xFFF   // END OF REGISTERS MARKER
#define LCM_ID_NT35517    0X5517  //0x008000

// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg											lcm_util.dsi_read_reg()
#define read_reg_v2(cmd, buffer, buffer_size)   			lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)    
       

static struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[64];
};



static struct LCM_setting_table lcm_initialization_setting[] = {
	
	/*
	Note :

	Data ID will depends on the following rule.
	
		count of parameters > 1	=> Data ID = 0x39
		count of parameters = 1	=> Data ID = 0x15
		count of parameters = 0	=> Data ID = 0x05

	Structure Format :

	{DCS command, count of parameters, {parameter list}}
	{REGFLAG_DELAY, milliseconds of time, {}},

	...

	Setting ending by predefined flag
	
	{REGFLAG_END_OF_TABLE, 0x00, {}}
	*/
#if 1
	{0xFF,4,{0xAA, 0x55, 0x25, 0x01}},
//	{0xFF,4,{0xAA, 0x55, 0x25, 0x01}},
	{0x6F,1,{0x0A}},
	{0xFA,1,{0x03}}, 
 
// PAGE 1
	{0xF0,5,{0x55,0xAA,0x52,0x08,0x00}},
//	{0xF0,5,{0x55,0xAA,0x52,0x08,0x00}},
	{0xB0,5,{0x00, 0x0C, 0x40, 0x3C, 0x3C}},
	{0xB1,2,{0xFC, 0x00}},  
	{0xB6,1,{0x08}},
	{0xB7,2,{0x00,0x00}},
	{0xBA,1,{0x01}},
	{0xBC,1,{0x00}},
	{0xBD,5,{0x01, 0x41, 0x10, 0x37, 0x01}}, 
	{0xCC,3,{0x03, 0x00, 0x00}},  


//PAGE 2	
	{0xF0,5,{0x55, 0xAA, 0x52, 0x08, 0x01}},
//	{0xF0,5,{0x55, 0xAA, 0x52, 0x08, 0x01}},
	
	{0xB0,3,{0x0A, 0x0A, 0x0A}},
	{0xB6,3,{0x44, 0x44, 0x44}},
	{0xB1,3,{0x0A, 0x0A, 0x0A}},
	{0xB7,3,{0x24, 0x24, 0x24}},
	{0xB2,3,{0x03, 0x03, 0x03}},
	{0xB8,3,{0x33, 0x33, 0x33}},
	{0xB3,3,{0x0D, 0x0D, 0x0D}},
	{0xB9,3,{0x24, 0x24, 0x24}},
	{0xB4,3,{0x0A, 0x0A, 0x0A}},
	{0xBA,3,{0x24, 0x24, 0x24}},
	{0xB5,3,{0x07, 0x07, 0x07}},
	{0xBC,3,{0x00, 0x78, 0x00}},
	{0xBD,3,{0x00, 0x78, 0x00}},
	{0xBE,1,{0x34}},   //VCOM 4f

	//GAMMA CODE
	{0xD1,16,{0x00,0x43,0x00,0x5A,0x00,0x7E,0x00,0x96,
			  0x00,0xA1,0x00,0xC3,0x00,0xE9,0x01,0x14}},
			  
	{0xD2,16,{0x01,0x35,0x01,0x6D,0x01,0x9A,0x01,0xE1,
	 		  0x02,0x19,0x02,0x1B,0x02,0x4D,0x02,0x85}},

	{0xD3,16,{0x02,0xA6,0x02,0xD4,0x02,0xF2,0x03,0x23,
	 		  0x03,0x41,0x03,0x6A,0x03,0x7F,0x03,0x9E}},
	 		 
	{0xD4,4,{0x03,0xB4,0x03,0xFF}},

	{0xD5,16,{0x00,0x43,0x00,0x5A,0x00,0x7E,0x00,0x96,
	 		  0x00,0xA1,0x00,0xC3,0x00,0xE9,0x01,0x14}},

	{0xD6,16,{0x01,0x35,0x01,0x6D,0x01,0x9A,0x01,0xE1,
	 		  0x02,0x19,0x02,0x1B,0x02,0x4D,0x02,0x85}},

	{0xD7,16,{0x02,0xA6,0x02,0xD4,0x02,0xF2,0x03,0x23,
	 		  0x03,0x41,0x03,0x6A,0x03,0x7F,0x03,0x9E}},
	 		 
	{0xD8,4,{0x03,0xB4,0x03,0xFF}},

	{0xD9,16,{0x00,0x43,0x00,0x5A,0x00,0x7E,0x00,0x96,
	 		  0x00,0xA1,0x00,0xC3,0x00,0xE9,0x01,0x14}},

	{0xDD,16,{0x01,0x35,0x01,0x6D,0x01,0x9A,0x01,0xE1,
	 		  0x02,0x19,0x02,0x1B,0x02,0x4D,0x02,0x85}},

	{0xDE,16,{0x02,0xA6,0x02,0xD4,0x02,0xF2,0x03,0x23,
	 		  0x03,0x41,0x03,0x6A,0x03,0x7F,0x03,0x9E}},

	{0xDF,4,{0x03,0xB4,0x03,0xFF}},
	 		  
	
	{0xE0,16,{0x00,0x43,0x00,0x5A,0x00,0x7E,0x00,0x96, 
		  	  0x00,0xA1,0x00,0xC3,0x00,0xE9,0x01,0x14}},

	{0xE1,16,{0x01,0x35,0x01,0x6D,0x01,0x9A,0x01,0xE1,
		  	  0x02,0x19,0x02,0x1B,0x02,0x4D,0x02,0x85,}},

	{0xE2,16,{0x02,0xA6,0x02,0xD4,0x02,0xF2,0x03,0x23,
	 		  0x03,0x41,0x03,0x6A,0x03,0x7F,0x03,0x9E}},

	{0xE3,4,{0x03,0xB4,0x03,0xFF}},

	{0xE4,16,{0x00,0x43,0x00,0x5A,0x00,0x7E,0x00,0x96,
		  	  0x00,0xA1,0x00,0xC3,0x00,0xE9,0x01,0x14}},

	{0xE5,16,{0x01,0x35,0x01,0x6D,0x01,0x9A,0x01,0xE1,
		  	  0x02,0x19,0x02,0x1B,0x02,0x4D,0x02,0x85}},

	{0xE6,16,{0x02,0xA6,0x02,0xD4,0x02,0xF2,0x03,0x23,
		  	  0x03,0x41,0x03,0x6A,0x03,0x7F,0x03,0x9E}},

	{0xE7,4,{0x03,0xB4,0x03,0xFF}},

	{0xE8,16,{0x00,0x43,0x00,0x5A,0x00,0x7E,0x00,0x96,
		  	  0x00,0xA1,0x00,0xC3,0x00,0xE9,0x01,0x14}},

	{0xE9,16,{0x01,0x35,0x01,0x6D,0x01,0x9A,0x01,0xE1,
		  	  0x02,0x19,0x02,0x1B,0x02,0x4D,0x02,0x85}},

	{0xEA,16,{0x02,0xA6,0x02,0xD4,0x02,0xF2,0x03,0x23,
		  	  0x03,0x41,0x03,0x6A,0x03,0x7F,0x03,0x9E}},

	{0xEB,4,{0x03,0xB4,0x03,0xFF}},     

	//TE
	//{0x35,	1,     {0x00}},   
	//{0x36,	1,     {0x82}},   
#else
	{0XBA, 7, {0X41,0X93,0X00,0X16,0XA4,0X10,0X18}}, 
	
	{0xC6,1,{0x08}},
	
	{0xB1,19,{0x00,0x00,0x06,0xE8,0x59,0x10,0x11,0xD1,
	  		0xF1,0x3D,0x45,0x2E,0x2E,0x42,0x01,0x5A,
	  		0xF7,0x00,0xE6}},
	  		
	{0XB2, 7,   {0X00,0X00,0X78,0X0C,0X07,0x3F,0x80}},
	
	{0XB4, 23, 	{0X90,0X08,0X00,0X32,0X10,0X04,0X32,0X10,
				0X02,0X32,0X10,0X00,0X37,0X0A,0X40,0X08,
				0X37,0X0A,0X40,0X14,0X46,0X50,0X0A}}, 
				
	{0XD5, 56,             {0X00,0X00,0X00,0X00,0X01,0X00,0X00,0X00,
					0X60,0X00,0X99,0X88,0XAA,0XBB,0X88,0X23,
					0X88,0X01,0X88,0X67,0X88,0X45,0X01,0X23,
					0X88,0X88,0X88,0X88,0X88,0X88,0X99,0XBB,
					0XAA,0X88,0X54,0X88,0X76,0X88,0X10,0X88,
			                0x32,0x32,0x10,0x88,0X88,0X88,0X88,0X88,
					0X3C,0X04,0X00,0X00,0X00,0X00,0X00,0X00}}, 
					
	{0xB6,	4,	{0x00,0x9A,0x00,0x9A}},

	{0XE0, 34,   {0X00,0x09,0x0D,0x22,0x2A,0x3F,0x1B,0x3E,
		             0x07,0x0C,0x0F,0x12,0x14,0x11,0x13,0x15,
		             0x17,0x00,0x09,0x0D,0x22,0x2A,0x3F,0xEB,
		             0x3E,0x07,0x0C,0x0F,0x12,0x14,0x11,0x13,
		             0x15,0x17}},

	{0XCC, 1,   {0X02}},
	#endif

	
	{0x11,	1,     {0x00}}, 
	{REGFLAG_DELAY, 150, {}},
	{0x29,	1,     {0x00}}, 
	{REGFLAG_DELAY, 10, {}},
	// Note
	// Strongly recommend not to set Sleep out / Display On here. That will cause messed frame to be shown as later the backlight is on.

	// Setting ending by predefined flag
	{REGFLAG_END_OF_TABLE, 0x00, {}},
};

static void init_lcm_registers(void)
{
	unsigned int data_array[16];

	//data_array[0] = 0x00110500; // Sleep Out
//	dsi_set_cmdq(data_array, 1, 1);
	//MDELAY(200);

	data_array[0] = 0x00043902;                          
	data_array[1] = 0x8983ffb9;                 
	dsi_set_cmdq(data_array, 2, 1);

	data_array[0] = 0x00083902;                          
	data_array[1] = 0x009341ba;      
	data_array[2] = 0x1810a416;   
	dsi_set_cmdq(data_array, 3, 1);
	  

	data_array[0] = 0x00023902;                          
	data_array[1] = 0x000008c6;        
	dsi_set_cmdq(data_array, 2, 1);
	  

	data_array[0] = 0x00143902;                     
	data_array[1] = 0x060000b1;  
	data_array[2] = 0x111059e8;  
	data_array[3] = 0x453df1d1;  
	data_array[4] = 0x01422e2e;     
	data_array[5] = 0xe600f75a;           	  
	dsi_set_cmdq(data_array, 6, 1);   

	data_array[0] = 0x00083902;                          
	data_array[1] = 0x780000b2;      
	data_array[2] = 0x803f070c;     	  
	dsi_set_cmdq(data_array, 3, 1);
	  
// important
	data_array[0] = 0x00183902;                          
	data_array[1] = 0x000890b4; 
	data_array[2] = 0x32041032;
	data_array[3] = 0x10320210;  
	data_array[4] = 0x400a3700; 
	data_array[5] = 0x400a3708; 
	data_array[6] = 0x0a504614;      	  
	dsi_set_cmdq(data_array, 7, 1);  


	data_array[0] = 0x00393902;                          
	data_array[1] = 0x000000d5;      
	data_array[2] = 0x00000100;   
	data_array[3] = 0x99006000;   
	data_array[4] = 0x88bbaa88;   
	data_array[5] = 0x88018823;   
	data_array[6] = 0x01458867;   	
	data_array[7] = 0x88888823; 
	data_array[8] = 0x99888888; 
	data_array[9] = 0x5488aabb; 
	data_array[10] = 0x10887688; 
	data_array[11] = 0x10323288; 
	data_array[12] = 0x88888888; 
	data_array[13] = 0x00043c88; 
	data_array[14] = 0x00000000;   	
	data_array[15] = 0x00000000;  	  
	dsi_set_cmdq(data_array, 16, 1);
//important

	  
	data_array[0] = 0x00053902;                          
	data_array[1] = 0x009a00b6;      
	data_array[2] = 0x0000009a;   
	dsi_set_cmdq(data_array, 3, 1);

	data_array[0] = 0x00233902;                          
	data_array[1] = 0x0d0900e0;      
	data_array[2] = 0xeb3f2a22;   
	data_array[3] = 0x0f0c073e;   
	data_array[4] = 0x13111412;   
	data_array[5] = 0x09001715;   
	data_array[6] = 0x3f2a220d;   	
	data_array[7] = 0x0c073eeb;   	
	data_array[8] = 0x1114120f; 	
	data_array[9] = 0x00171513; 	  
	dsi_set_cmdq(data_array, 10, 1);
	  

	data_array[0] = 0x00023902;                          
	data_array[1] = 0x000002cc;         	  
	dsi_set_cmdq(data_array, 2, 1);  


	data_array[0] = 0x00110500; // Sleep Out
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(150);

	data_array[0] = 0x00290500; // Display On
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(120);


}

static struct LCM_setting_table lcm_set_window[] = {
	{0x2A,	4,	{0x00, 0x00, (FRAME_WIDTH>>8), (FRAME_WIDTH&0xFF)}},
	{0x2B,	4,	{0x00, 0x00, (FRAME_HEIGHT>>8), (FRAME_HEIGHT&0xFF)}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_compare_id_setting[] = {

	{0xF0,5,{0x55, 0xAA, 0x52, 0x08, 0x01}},
	{0xF0,5,{0x55, 0xAA, 0x52, 0x08, 0x01}},

	{REGFLAG_END_OF_TABLE, 0x00, {}}

};

static struct LCM_setting_table lcm_sleep_out_setting[] = {
    // Sleep Out
	{0x11, 0, {0x00}},
    {REGFLAG_DELAY, 120, {}},

    // Display ON
	{0x29, 0, {0x00}},
	{REGFLAG_DELAY, 10, {}},
	
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_sleep_in_setting[] = {
	// Display off sequence
	{0x28, 0, {0x00}},
	{REGFLAG_DELAY, 10, {}},
    // Sleep Mode On
	{0x10, 0, {0x00}},
	{REGFLAG_DELAY, 120, {}},

	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static void dsi_send_cmdq_tinno(unsigned cmd, unsigned char count, unsigned char *para_list, unsigned char force_update)
{
	unsigned int item[16];
	unsigned char dsi_cmd = (unsigned char)cmd;
	unsigned char dc;
	int index = 0, length = 0;
	memset(item,0,sizeof(item));
	if(count+1 > 60)
	{
		LCM_DBG("Exceed 16 entry\n");
		return;
	}
	if(count == 0)
	{
		item[0] = 0x0500 | (dsi_cmd<<16);
		length = 1;
	}
	else if(count == 1)
	{
		item[0] = 0x1500 | (dsi_cmd<<16) | (para_list[0]<<24);
		length = 1;
	}
	else
	{
		item[0] = 0x3902 | ((count+1)<<16);//Count include command.
		++length;
		while(1)
		{
			if (index == count+1)
				break;
			if ( 0 == index ){
				dc = cmd;
			}else{
				dc = para_list[index-1];
			}
			item[index/4+1] |= (dc<<(8*(index%4)));  
			if ( index%4 == 0 ) ++length;
			++index;
		}
	}
	dsi_set_cmdq(&item, length, force_update);
}

static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
	unsigned int i;

    for(i = 0; i < count; i++) {
		
        unsigned cmd;
        cmd = table[i].cmd;
		
        switch (cmd) {
			
            case REGFLAG_DELAY :
                MDELAY(table[i].count);
                break;
				
            case REGFLAG_END_OF_TABLE :
                break;
				
            default:
			//	dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
			dsi_send_cmdq_tinno(cmd, table[i].count, table[i].para_list, force_update);
       	}
    }
	
}


// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}


static void lcm_get_params(LCM_PARAMS *params)
{
		memset(params, 0, sizeof(LCM_PARAMS));
	
		params->type   = LCM_TYPE_DSI;

		params->width  = FRAME_WIDTH;
		params->height = FRAME_HEIGHT;
		params->active_width= ACTIVE_WIDTH;
		params->active_height= ACTIVE_HEIGHT;

		// enable tearing-free

		params->dbi.te_mode 				= LCM_DBI_TE_MODE_DISABLED;
		params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;

#if (LCM_DSI_CMD_MODE)
		params->dsi.mode   = CMD_MODE;
#else
		params->dsi.mode   = BURST_VDO_MODE;//SYNC_PULSE_VDO_MODE;
#endif
	
		// DSI
		/* Command mode setting */
		params->dsi.LANE_NUM				= LCM_TWO_LANE;
		//The following defined the fomat for data coming from LCD engine.
		params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
		params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
		params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
		params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;


		// Highly depends on LCD driver capability.
		// Not support in MT6573
		params->dsi.packet_size=256;

		// Video mode setting		
		params->dsi.intermediat_buffer_num = 0;

		params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;

		params->dsi.vertical_sync_active				= 2; //5
		params->dsi.vertical_backporch					= 13; //5
		params->dsi.vertical_frontporch					= 9;//5
		params->dsi.vertical_active_line				= FRAME_HEIGHT; 

		params->dsi.horizontal_sync_active				= 20; 
		params->dsi.horizontal_backporch				= 90;//46
		params->dsi.horizontal_frontporch				= 90;//21
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

		// Bit rate calculation
	//	params->dsi.pll_div1=38;		// fref=26MHz, fvco=fref*(div1+1)	(div1=0~63, fvco=500MHZ~1GHz)
	//	params->dsi.pll_div2=1; 		// div2=0~15: fout=fvo/(2*div2)
		params->dsi.PLL_CLOCK = LCM_DSI_6589_PLL_CLOCK_273;

}

static void lcm_init(void)
{

    SET_RESET_PIN(1);
    MDELAY(5);
    SET_RESET_PIN(0);
    MDELAY(10);
    SET_RESET_PIN(1);
    MDELAY(120);//MDELAY(50);//Must over 6 ms,SPEC request
    LCM_DBG("[WUHAI] --  lcm_init start ");
    push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
	LCM_DBG("[WUHAI] --  lcm_init end ");
}

static unsigned int test_compare_id(void)
{

	int   array[4];
	unsigned char  buffer[3];
	unsigned int   id0=0;
	unsigned int   id1=0;
	unsigned int   id2=0;
	unsigned int   id = 0;


	#if 1
	push_table(lcm_compare_id_setting, sizeof(lcm_compare_id_setting) / sizeof(struct LCM_setting_table), 1);
	array[0] = 0x00083700;// read id return two byte,version and id
	dsi_set_cmdq(array, 1, 1);

	read_reg_v2(0xC5,buffer, 3);

	id0 = buffer[0]; //should be 0x55
	id1 = buffer[1];//should be 0x17
	id2 = buffer[2];
	LCM_DBG("test read id buffer[0] == %x ,buffer[1] == %x, buffer[2] == %x  \n",buffer[0],buffer[1],buffer[2]);
	id0 = id0 <<8;
	LCM_DBG("test read id2222 ==================== %x \n",id0);
	id2 = buffer[0]<<8 | buffer[1];     // bu
	LCM_DBG("test read id ==================== %x \n",id2);
	if(id2 == LCM_ID_NT35517)
		return 1;
	return 1;
		
#else
//    read register 04H to get ID ,not advised by BOE MR Feng.
	array[0] = 0x00083700;// read id return two byte,version and id
	dsi_set_cmdq(array, 1, 1);

	read_reg_v2(0x04,buffer, 3);

	id0 = buffer[0]; //should be 0x00
	id1 = buffer[1];//should be 0x80
	id2 = buffer[2];//should be 0x00

	LCM_DBG("test read id buffer[0] == %x ,buffer[1] == %x ,buffer[2] == %x \n",buffer[0],buffer[1],buffer[2]);
	if(id0 == 0x0 && id1==0x80 && id2==0x0)
		return 1;
	return 0; 
#endif


}


static void lcm_suspend(void)
{
unsigned int data_array[16];
   //     data_array[0] = 0x00100500; // Sleep Out
//	dsi_set_cmdq(data_array, 1, 1);
//	MDELAY(150);
	//SET_RESET_PIN(0);	
	//MDELAY(1);	
	//SET_RESET_PIN(1);

	push_table(lcm_sleep_in_setting, sizeof(lcm_sleep_in_setting) / sizeof(struct LCM_setting_table), 1);

}


static void lcm_resume(void)
{
	lcm_init();
//	test_compare_id();
	
	//push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);
}


static void lcm_update(unsigned int x, unsigned int y,
                       unsigned int width, unsigned int height)
{
	unsigned int x0 = x;
	unsigned int y0 = y;
	unsigned int x1 = x0 + width - 1;
	unsigned int y1 = y0 + height - 1;

	unsigned char x0_MSB = ((x0>>8)&0xFF);
	unsigned char x0_LSB = (x0&0xFF);
	unsigned char x1_MSB = ((x1>>8)&0xFF);
	unsigned char x1_LSB = (x1&0xFF);
	unsigned char y0_MSB = ((y0>>8)&0xFF);
	unsigned char y0_LSB = (y0&0xFF);
	unsigned char y1_MSB = ((y1>>8)&0xFF);
	unsigned char y1_LSB = (y1&0xFF);

	unsigned int data_array[16];

	data_array[0]= 0x00053902;
	data_array[1]= (x1_MSB<<24)|(x0_LSB<<16)|(x0_MSB<<8)|0x2a;
	data_array[2]= (x1_LSB);
	data_array[3]= 0x00053902;
	data_array[4]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
	data_array[5]= (y1_LSB);
	data_array[6]= 0x002c3909;

	dsi_set_cmdq(&data_array, 7, 0);

}
static unsigned int lcm_compare_id(void)
{
	unsigned int array[16];  
	unsigned char  buffer[3];
	unsigned int   id0=0;
	unsigned int   id1=0;
	unsigned int   id2=0;
	unsigned int   id = 0;

	SET_RESET_PIN(1);
	MDELAY(5);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(120);//MDELAY(50);//Must over 6 ms,SPEC request

#if 1
	

  /*    array[0] = 0x00063902;                      //{0xF0,5,{0x55, 0xAA, 0x52, 0x08, 0x01}},   
      array[1] = 0x52AA55F0; 
      array[2] = 0x00000108;  
      dsi_set_cmdq(array, 3, 1);
	  
	MDELAY(10);  */
	push_table(lcm_compare_id_setting, sizeof(lcm_compare_id_setting) / sizeof(struct LCM_setting_table), 1);
	array[0] = 0x00083700;// read id return two byte,version and id
	dsi_set_cmdq(array, 1, 1);

	read_reg_v2(0xC5,buffer, 3);

	id0 = buffer[0]; //should be 0x55
	id1 = buffer[1];//should be 0x17
	LCM_DBG("test read id buffer[0] == %x ,buffer[1] == %x  \n",buffer[0],buffer[1]);
	id0 = id0 <<8;
	LCM_DBG("test read id2222 ==================== %x \n",id0);
	id2 = buffer[0]<<8 | buffer[1];     // bu
	LCM_DBG("test read id ==================== %x \n",id2);
	if(id2 == LCM_ID_NT35517)
		return 1;
	return 0;
		
#else
//    read register 04H to get ID ,not advised by BOE MR Feng.
	array[0] = 0x00083700;// read id return two byte,version and id
	dsi_set_cmdq(array, 1, 1);

	read_reg_v2(0x04,buffer, 3);

	id0 = buffer[0]; //should be 0x00
	id1 = buffer[1];//should be 0x80
	id2 = buffer[2];//should be 0x00
 #ifdef BUILD_LK
	printf("test read id buffer[0] == %x ,buffer[1] == %x ,buffer[2] == %x \n",buffer[0],buffer[1],buffer[2]);
	
   #else
	printk("test read id buffer[0] == %x ,buffer[1] == %x ,buffer[2] == %x \n",buffer[0],buffer[1],buffer[2]);
	
   #endif
//	LCM_DBG("test read id buffer[0] == %x ,buffer[1] == %x ,buffer[2] == %x \n",buffer[0],buffer[1],buffer[2]);
	if(id0 == 0x0 && id1==0x80 && id2==0x0)
		return 1;
	return 0; 
#endif
}


LCM_DRIVER nt35517_qhd_dsi_vdo_lcm_drv = 
{
	.name		= "nt35517_qhd_dsi_vdo_boe",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id     = lcm_compare_id,
#if (LCM_DSI_CMD_MODE)
	.set_backlight	= lcm_setbacklight,
        .update         = lcm_update,
#endif
    };
