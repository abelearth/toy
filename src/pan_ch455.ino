/*SMT DVBI18110L-PS1 front panel dirver. 
* Features:
* 1 - 8*4 led display
* 3 buttones
* IR sensor
*/

#include <Wire.h>
#include "types.h"
#include "pan_ch455.h"


#define PAN_CH455_PRINTF(...) Serial.println("hello")

struct led_bitmap bitmap_table[] =
{
	{'.', 0x80}, /* Let's put the dot bitmap into the table */
	{'0', 0x3f}, {'1', 0x06}, {'2', 0x5b}, {'3', 0x4f}, 
	{'4', 0x66}, {'5', 0x6d}, {'6', 0x7d}, {'7', 0x07}, 
	{'8', 0x7f}, {'9', 0x6f}, {'a', 0x77}, {'A', 0x77}, 
	{'b', 0x7c}, {'B', 0x7c}, {'c', 0x39}, {'C', 0x39}, 
	{'d', 0x5e}, {'D', 0x5e}, {'e', 0x79}, {'E', 0x79}, 
	{'f', 0x71}, {'F', 0x71}, {'g', 0x6f}, {'G', 0x3d}, 
	{'h', 0x76}, {'H', 0x76}, {'i', 0x04}, {'I', 0x30}, 
	{'j', 0x0e}, {'J', 0x0e}, {'l', 0x38}, {'L', 0x38}, 
	{'n', 0x54}, {'N', 0x37}, {'o', 0x5c}, {'O', 0x3f}, 
	{'p', 0x73}, {'P', 0x73}, {'q', 0x67}, {'Q', 0x67}, 
	{'r', 0x50}, {'R', 0x77}, {'s', 0x6d}, {'S', 0x6d}, 
	{'t', 0x78}, {'T', 0x31}, {'u', 0x3e}, {'U', 0x3e}, 
	{'y', 0x6e}, {'Y', 0x6e}, {'z', 0x5b}, {'Z', 0x5b}, 
	{':', 0x80}, {'-', 0x40}, {'_', 0x08}, {' ', 0x00},
};

#define PAN_CH455_CHAR_LIST_NUM sizeof(bitmap_table)/sizeof(struct led_bitmap)

static UINT8 display_val[4] = {0,0,0,0};
static int light_status = 0;
static int colon_status = 0;


static int transfer_key(UINT8 data, struct PAN_KEY* key)
{
    static int key_status = CH455_STATUS_UP;
    static int key_cnt = 0;
    static int key_press_cnt = 0;
    static int key_press_intv = 3;
    UINT32 keypress = PAN_KEY_INVALID;
    UINT8 row, column, status, check;  

    column = data & CH455_KEY_COLUMN_MASK;  //(bit 0~1)
    row = (data & CH455_KEY_ROW_MASK) >> 3; //(bit 3~5)
    status = (data & CH455_KEY_STATUS_MASK) >> 6; // (bit 6)
    check = (data & CH455_KEY_CHECK_MASK);

    if(check != 0x04){
        return -1;
    }

    keypress = 0xffff0000 | data;

    if (key_status == CH455_STATUS_UP)
    {
        if (status == CH455_STATUS_UP)
           return -2;

        //status == CH455_STATUS_DOWN
        key->type = PAN_KEY_TYPE_PANEL;
        key->state = PAN_KEY_PRESSED;
        key->count = key_cnt;
        key->code = keypress;

        key_cnt ++;
        key_status = status;
        key_press_cnt = 0;
    }
    else
    {
        if (status == CH455_STATUS_UP)
        {
            key_cnt = 1;
            key->type = PAN_KEY_TYPE_PANEL;
            key->state = PAN_KEY_RELEASE;
            key->count = 1;
            key->code = keypress;
            key_status = status;
        }
        else
        {
            //status == CH455_STATUS_DOWN
            key_press_cnt ++;
            if (key_press_cnt >= key_press_intv)
            {
                key->type = PAN_KEY_TYPE_PANEL;
                key->state = PAN_KEY_PRESSED;
                key->count = key_cnt;
                key->code = keypress;

                key_cnt++;
                key_press_cnt = 0;
            }
        }

    }

    return 0;
}

void  panel_ch455_task(void)
{
    struct PAN_KEY key;

    UINT8 data;

    while(1)
    {

        //Serial.println("request key.");
        Wire.requestFrom(KEY_ADDR,1);

        while (Wire.available()) {
            byte data = Wire.read();
            if(0 == transfer_key(data, &key)){
                Serial.print("Get new key.");
                Serial.println(key.code, HEX);
            }else{
                //Serial.print("Get wrong data from front panel i2c.");
                //Serial.println(data, HEX);
            }
        } 

        delay(CH455_KEY_INTERVAL);
    }

}


INT32 pan_ch455_open(void)
{
  
  Wire.begin();
  Wire.beginTransmission(SETING_ADDR);
  Wire.write(CH455_MODE);
  Wire.endTransmission();

		
#if 0
	irc_m6303irc_init(tp->hw_info);

	OSAL_T_CTSK task_param;
	//creat ch455 task for keyboard scan.
	task_param.task   =  (FP)panel_ch455_task;
	task_param.stksz  =  0x1000; 
	task_param.quantum = 10;
	task_param.itskpri = OSAL_PRI_NORMAL;
	task_param.para1= (UINT32)dev;
	task_param.para2 = 0;

	tp->task_id = osal_task_create(&task_param);
	if(OSAL_INVALID_ID == tp->task_id)
	{
		PAN_CH455_PRINTF("%s()=>Create panel_ch455_task FAILED!\n", __FUNCTION__);
		return ERR_FAILUE;
	}
#endif

	return SUCCESS;
}

#if 0
static INT32  pan_ch455_close(struct pan_device *dev)
{
	struct pan_ch455_private *tp = (struct pan_ch455_private *)dev->priv;
	int i;
	
#ifdef _DEBUG_VERSION_
	/* Un-register an 1mS cycle interrupt ISR */
	if (tp->task_id != OSAL_INVALID_ID)
	{
		osal_task_delete(tp->task_id);
		tp->task_id = OSAL_INVALID_ID;
	}
#endif
#if(SYS_IRP_MOUDLE != SYS_DEFINE_NULL)
    if(tp->is_local == 1)
     irc_m6303irc_close();
#endif
	return SUCCESS;
}
#endif

static UINT8 pan_ch455_bitmap(UINT8 c)
{

	int len = PAN_CH455_CHAR_LIST_NUM;
	struct led_bitmap node;
	UINT8 bitmap = 0;
	UINT16 i;

	for (i=0; i<len; i++)
	{
		node = bitmap_table[i];
		if (node.character == c)
		{
			bitmap = (UINT8)node.bitmap;
			break;
		}
	}

	if(i == len)
	{
		PAN_CH455_PRINTF("%s()=>Character not found.\n", __FUNCTION__);
		return 0;
	}
 
	return bitmap;
}

void pan_ch455_display(char *str, UINT32 len)
{
    UINT8 addr, val;
    UINT32 i;

    i = 0;
    
    for (i = 0; i < 4 && i < len ; i++)
    {
        if (i == 0){
            addr = DIG0_ADDR;
        }
        else if (i == 1){
            addr = DIG1_ADDR;
        }
        else if (i == 2){
            addr = DIG2_ADDR;
        }
        else {
          addr = DIG3_ADDR;
        }

        val = pan_ch455_bitmap(str[i]);
        /* HenryHan 2020-03-05
         * use DP of seg2 to control LED2.
         */
        if(i == 1)
        {
          if( colon_status == 0){
            val &= ~0x80;
          }
          else {
            val |= 0x80;
          }
        }
        if(i == 2)
        {
          if( light_status == 0){
            val &= ~0x80;
          }
          else {
            val |= 0x80;
          }
        }

        Wire.beginTransmission(addr);
        Wire.write(val);
        Wire.endTransmission();

    }
}

void pan_ch455_display_number(UINT16 num)
{
  UINT8 val0,val1, val2, val3;
  
  val0 = num%10;
  val1 = (num%100)/10;
  val2 = (num%1000)/100;
  val3 = (num%10000)/1000;

  char str[5];

  str[0] = '0'+ val3;
  str[1] = '0'+ val2;
  str[2] = '0'+ val1;
  str[3] = '0'+ val0;
  pan_ch455_display(str, 4); 
}
