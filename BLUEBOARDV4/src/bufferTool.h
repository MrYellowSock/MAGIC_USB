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
#include <Arduino.h>
#define Key_down 0xEB
#define Key_up 0xEC
#define Sys_down 0xED
#define Sys_up 0xEE
#define type 0xEF

#define mouse_down 0xF6
#define mouse_up 0xF7
#define mouse_wheel 0xF8

#define mouse_move 0xF9
#define Con_down 0xFA       
#define Con_up 0xFB
//ADDED "EEPROM age" which decreased when EEPROM code executes for more stealthness.
#define EE_Start 0xFC //start writing to eeprom

//appear in eeprom only [Sent after EE_START]
#define loop_start 0xFD
#define wait 0xFE
#define loop_end 0xFF
#define EE_End 0


//0-31 is non printable safe to use as control commands (except 8-13 which is \r \n \t etc..)

//command parameter is not buffered. printable ascii only
bool is0Arg(uint8_t c){
    return c >= Key_down && c <= type;
}
bool is1Arg(uint8_t c){
    return c >= mouse_down && c <= mouse_wheel;
}
bool is2Arg(uint8_t c){
    return (c >= mouse_move && c <= Con_up) || c == wait || c == loop_start;
}
bool is3Arg(uint8_t c){
    return c==EE_Start;
}

//Command that can execute now 
bool isLiveCommand(uint8_t c){
    return c >= Key_down && c <= EE_Start;
}
bool isEECommand(uint8_t c){
    return c == EE_End || (c >= loop_start && c <= loop_end);
}

#define promsize 1024
uint16_t EE_W_Indx = 0;
//0 is command code the rest is parameters.
uint8_t buff[6] = {0,0,0,0,0,0};
uint8_t buff_len = 0;

void clrbuff(){
    buff[0] = 0;
    buff_len = 0;
    EE_W_Indx = 0;
}
uint16_t mergebyte (uint8_t a,uint8_t b){
    uint16_t ms = a;
    ms = ms << 8;
    ms = ms | b; 
    return ms;
}
bool poplen (){
    //write_EE need this.
    if(buff[2] > 0){
        buff[2]--;
    }
    else if(buff[1] > 0){
        buff[2] = 0xFF;
        buff[1]--;
    }
    return buff[1] == 0 && buff[2] == 0;
}
void push(uint8_t c){
    buff[buff_len++] = c;
}
bool fullfill (uint8_t c,uint8_t expected_buff_len){
    if(buff_len < expected_buff_len){
        push(c);
        return buff_len >= expected_buff_len;
    }
    else{
        return true;
    }
}

