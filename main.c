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
#include <limits.h>

#define DISPLAY_DELAY 2     // delay is needed to display numbers higher than 9
#define BUFFER_SIZE 128     // buffer for timestamps
#define COUNT_TIME 10       // pariod of time

char last_ch;                 // naposledy precteny znak
int char_cnt = 0;             // all typed characters
int timer = 11;               // counting from time from start
int hits[BUFFER_SIZE] = {0};  // hits time stamps
int hit_index = 0;            // index determines where are timestamps written
short display_overall = 0;    // boolean switch between overall hits and hits/period of time
int nums[10]= {               // numbers on secondary 7 segment display
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

void print_user_help(void)
{
  term_send_str_crlf("Keyboard hits counter");
}

unsigned char decode_user_cmd(char *cmd_ucase, char *cmd)
{
  return CMD_UNKNOWN;
}

void fpga_initialized()
{
}

/**
* show number on external 7 segment display
*/
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
/**
* handle new hits
*/
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
        hits[hit_index] = timer;      // write timestamp
        hit_index++;
        if (hit_index == BUFFER_SIZE) // start over when needed
        {
          hit_index = 0;
        }
        char_cnt++;
      }
    }
  }
  return 0;
}

/**
* Timer A initiated, information in comments are from MCU datasheet
*/
void init_timer(){
  CCTL0 = CCIE;
  TACCR0 = 32768;   // restart/start timer
                    // timer is stopped
                    // wirting a non-zero value will restart the timer
  TACTL = (unsigned int)0x0100010110;
  /*
  01 - source - ACLK (32768 Hz)
  00 - devided by 1
  01 - mode up
  0 - unused
  1 - start counting from zero
  1 - enable interrupt
  0 - no interrupt pendding
  */
}

/**
* Timer A interrupt
*/
interrupt (TIMERA0_VECTOR) Timer_A (void){
  timer++;
  if (timer == INT_MAX) // just in case our application will be running for years
  {
    timer = 11;
  }
  return;
}

/**
* go throgh time stamp buffer and make statistic
*/
int count_hits(){
  int i;
  int n = 0;
  for (i = 0; i < BUFFER_SIZE; ++i)
  {
    if (hits[i] > (timer - COUNT_TIME))
    {
      n++;
    }
  }
  return n;
}

int main(void)
{
  char_cnt = 0;
  last_ch = 0;

  initialize_hardware();
  keyboard_init();
  LCD_init();
  LCD_clear();
  init_timer();

  P1DIR |= 0xFF;
  P6DIR |= 0xFF;

  while (1)
  {
    keyboard_idle();
    terminal_idle();  // obsluha terminalu
    if (display_overall)
      displayNumber(char_cnt);
    else
      displayNumber(count_hits());
  }
}
