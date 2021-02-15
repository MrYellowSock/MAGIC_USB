/* Copyright (c) 2021, Suppanut Kulkamthorn
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

#define led_blue A1
#define led_red A2
#define led_green A3
#define pinButton A0
#define state A10
#include <Arduino.h>
#include <PinChangeInterrupt.h>

bool red =false,green = false,blue = false;

void redON(){
    digitalWrite(led_red,HIGH);
}
void redOFF(){
    digitalWrite(led_red,LOW);
}
void blueON(){
    digitalWrite(led_blue,HIGH);
}
void blueOFF(){
    digitalWrite(led_blue,LOW);
}
void greenON(){
    digitalWrite(led_green,HIGH);
}
void greenOFF(){
    digitalWrite(led_green,LOW);
}
bool isPress(){
    return digitalRead(pinButton) == HIGH;
}
void waitCon(){
    if(digitalRead(state)){
        blueON();
        blue = true;
    }
    else
    {
        blueOFF();
        blue = false;
    }
}
void pinSet(){
    pinMode(pinButton, INPUT);
    pinMode(led_blue, OUTPUT);
    pinMode(led_red, OUTPUT);
    pinMode(led_green, OUTPUT);   
    pinMode(state,INPUT);
    waitCon();
    attachPCINT(digitalPinToPCINT(state),waitCon,CHANGE);
}