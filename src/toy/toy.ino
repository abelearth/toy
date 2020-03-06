/*
 * Archie's toy is in playing.
*/

#include "types.h"
#include "pan_ch455.h"

void setup() {  
  // Setup serial monitor
  Serial.begin(9600);
  
  Serial.println("Open ch455.");  
  pan_ch455_open();
  ir_open();

}

char greeting[]="hanjunqi";

void loop() 
{
    int i = 0;

#if 1
    for (i = 0; i<8; i++){  
        pan_ch455_display(greeting+i, 4);
        delay(300);
    }

    for (i=0; i<5000;i++){
        pan_ch455_display_number(i);
        if(ir_get()){
            if(i==10)
                pan_ch455_light_on();
            pan_ch455_display_number(i);
            delay(3000);
            pan_ch455_light_off();
            i=0;
            ir_reset();
        }
        delay(200);
    }
#endif
#if 0
    panel_ch455_task();
#endif
}
