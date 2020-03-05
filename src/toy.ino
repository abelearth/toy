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

}

char greeting[]="hanjunqi";

int i = 0;
void loop() {
  
  #if 0
  for (i = 0; i<8; i++){  
    pan_ch455_display(greeting+i, 4);
    delay(300);
  }

  for (i=0; i<5000;i++){
    pan_ch455_display_number(i);
    delay(20);
  }
  #endif

  panel_ch455_task();
}
