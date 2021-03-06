// Arduino NEC Protocol IR remote control decoder

static char text[16];
static boolean nec_ok = 0;
static byte  i, nec_state = 0, command, inv_command;
static unsigned int address;
static volatile unsigned long nec_code;
static const byte interruptPin = 2;
static volatile byte state = LOW;


void ir_open(void) {
    TCCR1A = 0;
    TCCR1B = 0;                                    // Disable Timer1 module
    TCNT1  = 0;                                    // Set Timer1 preload value to 0 (reset)
    TIMSK1 = 1;                                    // enable Timer1 overflow interrupt
    pinMode(interruptPin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(interruptPin), remote_read, CHANGE);       // Enable external interrupt (INT0)
}

void ir_reset(void){
    nec_ok = 0;                                  // Reset decoding process
    nec_state = 0;
    TCCR1B = 0;
    attachInterrupt(digitalPinToInterrupt(interruptPin), remote_read, CHANGE);     // Enable external interrupt (INT0)
} 

void remote_read() {
    unsigned int timer_value;

    if(nec_state != 0){
        timer_value = TCNT1;                         // Store Timer1 value
        TCNT1 = 0;                                   // Reset Timer1
    }
    switch(nec_state){
        case 0 :                                      // Start receiving IR data (we're at the beginning of 9ms pulse)
            TCNT1  = 0;                                  // Reset Timer1
            TCCR1B = 2;                                  // Enable Timer1 module with 1/8 prescaler ( 2 ticks every 1 us)
            nec_state = 1;                               // Next state: end of 9ms pulse (start of 4.5ms space)
            i = 0;
            return;
        case 1 :                                      // End of 9ms pulse
            if((timer_value > 19000) || (timer_value < 17000)){         // Invalid interval ==> stop decoding and reset
                nec_state = 0;                             // Reset decoding process
                TCCR1B = 0;                                // Disable Timer1 module
            }
            else
                nec_state = 2;                             // Next state: end of 4.5ms space (start of 562??s pulse)
            return;
        case 2 :                                      // End of 4.5ms space
            if((timer_value > 10000) || (timer_value < 8000)){
                nec_state = 0;                             // Reset decoding process
                TCCR1B = 0;                                // Disable Timer1 module
            }
            else
                nec_state = 3;                             // Next state: end of 562??s pulse (start of 562??s or 1687??s space)
            return;
        case 3 :                                      // End of 562??s pulse
            if((timer_value > 1400) || (timer_value < 800)){           // Invalid interval ==> stop decoding and reset
                TCCR1B = 0;                                // Disable Timer1 module
                nec_state = 0;                             // Reset decoding process
            }
            else
                nec_state = 4;                             // Next state: end of 562??s or 1687??s space
            return;
        case 4 :                                      // End of 562??s or 1687??s space
            if((timer_value > 3600) || (timer_value < 800)){           // Time interval invalid ==> stop decoding
                TCCR1B = 0;                                // Disable Timer1 module
                nec_state = 0;                             // Reset decoding process
                return;
            }
            if( timer_value > 2000)                      // If space width > 1ms (short space)
                bitSet(nec_code, (31 - i));                // Write 1 to bit (31 - i)
            else                                         // If space width < 1ms (long space)
                bitClear(nec_code, (31 - i));              // Write 0 to bit (31 - i)
            i++;
            if(i > 31){                                  // If all bits are received
                nec_ok = 1;                                // Decoding process OK
                detachInterrupt(0);                        // Disable external interrupt (INT0)
                return;
            }
            nec_state = 3;                               // Next state: end of 562??s pulse (start of 562??s or 1687??s space)
    }
}

ISR(TIMER1_OVF_vect) {                           // Timer1 interrupt service routine (ISR)
    nec_state = 0;                                 // Reset decoding process
    TCCR1B = 0;                                    // Disable Timer1 module
}

boolean ir_get(void){
    return nec_ok;
}

#if 0
void loop() {
    if(nec_ok){                                    // If the mcu receives NEC message with successful
        // Disable Timer1 module
        address = nec_code >> 16;
        command = nec_code >> 8;
        inv_command = nec_code;

        sprintf(text, "%04X-%02X-%02X", address, command, inv_command);
        Serial.println(text);    

    }
}
#endif
