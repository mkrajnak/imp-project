/**
 * Martin Krajnak, xkrajn02
 * edited for IMP project - keyboard typing counter
 * 10/12/16
 **/
/*******************************************************************************
   main.c: LCD + keyboard demo
   Copyright (C) 2012 Brno University of Technology,
                      Faculty of Information Technology
   Author(s): Michal Bidlo <bidlom AT fit.vutbr.cz>

   LICENSE TERMS

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in
      the documentation and/or other materials provided with the
      distribution.
   3. All advertising materials mentioning features or use of this software
      or firmware must display the following acknowledgement:

        This product includes software developed by the University of
        Technology, Faculty of Information Technology, Brno and its
        contributors.

   4. Neither the name of the Company nor the names of its contributors
      may be used to endorse or promote products derived from this
      software without specific prior written permission.

   This software or firmware is provided ``as is'', and any express or implied
   warranties, including, but not limited to, the implied warranties of
   merchantability and fitness for a particular purpose are disclaimed.
   In no event shall the company or contributors be liable for any
   direct, indirect, incidental, special, exemplary, or consequential
   damages (including, but not limited to, procurement of substitute
   goods or services; loss of use, data, or profits; or business
   interruption) however caused and on any theory of liability, whether
   in contract, strict liability, or tort (including negligence or
   otherwise) arising in any way out of the use of this software, even
   if advised of the possibility of such damage.

   $Id$


*******************************************************************************/

#include <fitkitlib.h>
#include <keyboard/keyboard.h>
#include <lcd/display.h>


#define DISPLAY_DELAY 5

char last_ch; //naposledy precteny znak
int char_cnt = 0;
short display_overall = 0;
int nums[10]= { //numbers on secondary 7 segment display
  0xFC, //0 0110 0000
  0x60, //1 0x60
  0xDA, //2 0xDA
  0xF2, //3 0xF2
  0x66, //4 0x66
  0xB6, //5 0xB6
  0x3E, //6
  0xE0, //7
  0xFE, //8
  0xE6, //9
};



void print_user_help(void) { }

unsigned char decode_user_cmd(char *cmd_ucase, char *cmd)
{
  return CMD_UNKNOWN;
}

void fpga_initialized()
{
}

// show number on external 7 segment display
void displayNumber(int display) {
  if (display < 0)      // shut display down
  {
    P6OUT = 0xFF;
  }
  else if ( display < 10) // only 1 of 4 displays is operating
  {
    P1OUT = 0x20;
    P6OUT = ~nums[display];
  }
  else if( display < 100) // 2 of 4 displays are operating
  {
    P1OUT = 0x10;
    P6OUT = ~nums[display/10];
    delay_ms(DISPLAY_DELAY);
    P1OUT = 0x20;
    P6OUT = ~nums[display%10];
    delay_ms(DISPLAY_DELAY);
  }
  else if( display < 1000) // 3 of 4 displays are operating
  {
    P1OUT = 0x08;
    P6OUT = ~nums[display/100];
    delay_ms(DISPLAY_DELAY);
    P1OUT = 0x10;
    P6OUT = ~nums[display/10%10];
    delay_ms(DISPLAY_DELAY);
    P1OUT = 0x20;
    P6OUT = ~nums[display%10];
    delay_ms(DISPLAY_DELAY);
  } 
  else if( display < 10000) // all of displays are operating
  {
    P1OUT = 0x04;
    P6OUT = ~nums[display/1000];
    delay_ms(DISPLAY_DELAY);
    P1OUT = 0x08;
    P6OUT = ~nums[display/100%10];
    delay_ms(DISPLAY_DELAY);
    P1OUT = 0x10;
    P6OUT = ~nums[display/10%10];
    delay_ms(DISPLAY_DELAY);
    P1OUT = 0x20;
    P6OUT = ~nums[display%10];
    delay_ms(DISPLAY_DELAY);
  } 
}

int keyboard_idle()
{
  char ch;
  ch = key_decode(read_word_keyboard_4x4());
  if (ch != last_ch) // osetreni drzeni klavesy
  {
    last_ch = ch;
    if (ch != 0) // pokud byla stisknuta klavesa
    {
      if (ch == '#')  // clear on hit #
      {
        LCD_clear();
        char_cnt = 0;
      }
      else if (ch == '*') // change between showing overall
      {
        display_overall = !display_overall;
      }
      else{
        LCD_append_char(ch);
        char_cnt++;
      }
    }
  }
  
  return 0;
}

int main(void)
{
  char_cnt = 0;
  last_ch = 0;

  initialize_hardware();
  keyboard_init();
  LCD_init();
  LCD_clear();

  P1DIR |= 0xFF;
  P6DIR |= 0xFF;

  while (1)
  {
    keyboard_idle();
    terminal_idle();  // obsluha terminalu
    if (display_overall)
      displayNumber(char_cnt);
    else
      displayNumber(-1);
  }         
}

