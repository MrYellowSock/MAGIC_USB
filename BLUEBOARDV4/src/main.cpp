/* Copyright (c) 2021, Suppanut Kulkamthorn
** Bluetooth HIDs emulator
**  
** Permission to use, copy, modify, and/or distribute this software for  
** any purpose with or without fee is hereby granted, provided that the  
** above copyright notice and this permission notice appear in all copies.  
** 
** THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL  
** WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED  
** WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR  
** BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES  
** OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,  
** WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,  
** ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS  
** SOFTWARE.  
*/

#include <_config.h>
#include "HID-Project.h"
#include <hardware.h>
#include "HardwareSerial.h"
#include <EEPROM.h>
#include "bufferTool.h"
#include <USBCore.h>

bool ProcessByte(uint8_t c);
bool isConnected(){
    if(Serial1.available()){
        uint8_t c = Serial1.read();
        if(c != 0){
            clrbuff();
            ProcessByte(c);
            return true;
        }
    }
    return false;
}

void waitUSB(){     
    u8 f = UDFNUML;         //USB FRAME COUNT IF THEY RE NOT MOVING THEY NOT CONNECTED
	delay(3);
    while (f == UDFNUML)
    {
        delay(3);
    }
}
bool stopable_delay(uint16_t desiredDelay_ms)
{
    unsigned long t_start = millis(); //ms
    while (millis()-t_start<desiredDelay_ms)
        if(isConnected())return true;
    return false;
}
bool waitButton(){
    redON();
    while (!isPress())
    {
        if(isConnected()){redOFF(); return true;}
    }
    redOFF();
    delay(150);//debounce   
    return false;
}

uint16_t EE_R_Indx = 0; //reading index
void setup() {
    pinSet();
    Serial1.begin(BAUD);
    #ifdef debugmode
        Serial.begin(BAUD);
        while (!Serial);
        Serial.println("WELCOME TO BLUEBOARD CONFIGURATION MODE");

    #else
        greenON();
        waitUSB(); 
        greenOFF();
        BootKeyboard.begin();
        BootMouse.begin();
        System.begin();
        Consumer.begin();
        EEPROM.begin();
        delay(1200);//WAIT keyboard initialized.

        if(EEPROM[0] > 0 && EEPROM[1] != 0xFF){//avoid default empty EEPROM space(0xFF)
            if(EEPROM[0] != 0xFF)EEPROM[0] = EEPROM[0]-1;    //age decreased.
            for(EE_R_Indx=1;EE_R_Indx<promsize;EE_R_Indx++){
                if(ProcessByte(EEPROM[EE_R_Indx])){
                    return;
                }
            }
            clrbuff();
        }
    #endif
}
//TEST
#ifdef debugmode
bool NL = false;
void loop() {
  // Light led if keyboard uses the boot protocol (normally while in bios)
  // Keep in mind that on a 16u2 and Arduino Micro HIGH and LOW for TX/RX Leds are inverted.
  /*

    */
    Serial.println(BootKeyboard.getLeds());
    delay(1000);
    // Read from the Bluetooth module and send to the Arduino Serial Monitor
    uint8_t c;
    if (Serial1.available())
    {
        c = Serial1.read();
        Serial.write(c);
        blueOFF();
    }
    else{
        if(blue)blueON();
    }
 
     if (Serial.available())
    {
        c = Serial.read();
        Serial1.write(c);   
        //Serial.println((uint8_t)c);
        // Echo the user input to the main window. The ">" character indicates the user entered text.
        if (NL) { Serial.print(">");  NL = false; }
        Serial.write(c);
        if (c==10) { NL = true; }
    }

}
#else

bool execLoop(uint16_t cnt){
    uint16_t loop_layer = 1;
    uint16_t start = EE_R_Indx;
    uint16_t end=0;
    //find end
    for(uint16_t i=start;i<promsize;i++){
        uint8_t c = EEPROM[i];
        //if(c == EE_End)break; skip because some other command argument use 0
        if(c == loop_start){loop_layer++;}
        else if(c == loop_end){
            loop_layer--;
            if(loop_layer == 0){
                end = i;
                break;
            }
        }
    }
    if(end == 0)return;//abort
    else{
        for(uint16_t i=0;i<cnt;i++){
            for(EE_R_Indx = start;EE_R_Indx < end;EE_R_Indx++){
                if(ProcessByte(EEPROM[EE_R_Indx]))return true;
            }
        }
    }
    return false;
}

//read command byte by byte
//return true when EE_End is encountered.
bool ProcessByte(uint8_t c){
    if(is0Arg(buff[0]) || buff[0] == 0){
        if(isLiveCommand(c) || isEECommand(c)){//any command
            clrbuff();
            if(c == EE_End)return true;
            push(c);
        }
        else{
            switch (buff[0])
            {
                case Key_down:BootKeyboard.press((KeyboardKeycode)c);   //not ASCII
                break;
                case Key_up:
                    if(c == 0xA5)BootKeyboard.releaseAll(); //could just be any key not used by regular Keyboard
                    else BootKeyboard.release((KeyboardKeycode)c);
                break;
                case Sys_down:
                    System.press((SystemKeycode)c);
                break;
                case Sys_up:
                    System.release();
                break;
                case type:
                    BootKeyboard.write(c);  //ASCII
                break;
            }
        }
    }
    else if(is1Arg(buff[0]) && fullfill(c,2)){
        switch (buff[0])
        {
            case mouse_down:
                BootMouse.press(buff[1]);
            break;
            case mouse_up:
                if(buff[1] == 0xFF)BootMouse.releaseAll();
                else BootMouse.release(buff[1]);
            break;
            case mouse_wheel:
                BootMouse.move(0,0,(int8_t)buff[1]);
            break;
        }
        clrbuff();
    }
    else if(is2Arg(buff[0]) && fullfill(c,3)){
        uint16_t code;
        switch (buff[0])
        {
            case mouse_move:
                BootMouse.move((int8_t)buff[1],(int8_t)buff[2]);
            break;
            case Con_down:
                code = mergebyte(buff[1],buff[2]);
                Consumer.press((ConsumerKeycode)code);
            break;
            case Con_up:
                code = mergebyte(buff[1],buff[2]);
                if(code == 0xFFFF)Consumer.releaseAll();
                else Consumer.release((ConsumerKeycode)code);
            break;
            case wait:
                code = mergebyte(buff[1],buff[2]);
                if(code == 0){//pause mode if 0 0
                    if(waitButton())return true;
                } 
                else if(stopable_delay(code))return true;
            break;
            case loop_start:
                EE_R_Indx++;   //move inside loop
                code = mergebyte(buff[1],buff[2]);
                clrbuff();
                if(execLoop(code))return true;
            break;
        }
        clrbuff();
    }
    else if(is3Arg(buff[0]) && fullfill(c,4)){
        switch (buff[0])
        {
            case EE_Start:  //age is the first 'c'
                if(EE_W_Indx < promsize){
                    EEPROM[EE_W_Indx++] = c;                
                } 
                if(poplen()){
                    if(EE_W_Indx < promsize)EEPROM[EE_W_Indx] = EE_End;
                    clrbuff();
                }
            break;
        }
    }
    return false;
}

void loop(){
    if(BootKeyboard.getProtocol() == HID_BOOT_PROTOCOL)greenON();
    else greenOFF();
    if (Serial1.available()){
        blueOFF();
        ProcessByte(Serial1.read());
    }
    else{
        if(blue)blueON();
    }
}
#endif