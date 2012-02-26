/*
 *  softButtons.cpp
 *  Button Library for soft buttons on adafruit TFT LCD Touch Screen
 *  based on buttons library Created by Franky on 29/01/09.
 *  adated by Owen DeLong 13/02/12
 *  Licensed under LGPL (free to modify and use as you wish)
 */

/*
LL = 236,5
LR = 5,0
UR = 8, 317
UL = 240, 308
*/


#include "Arduino.h"
#include "softButtons.h"
#include <TFTLCD.h>
#include <TouchScreen.h>

#define MINPRESSURE 10
#define MAXPRESSURE 1000
#define extractrgb(color) r=(color & 0xf800) >> 8; g=(color & 0x7e0) >> 2; b=(color & 0x0018) << 3;

#define TS_MINX 150
#define TS_MINY 120
#define TS_MAXX 920
#define TS_MAXY 940

// DOC Instantiation: softButton(TFTLCD Display_v, TouchScreen ts_v, unsigned int x1_v, unsigned int y1_v, unsigned int width, unsigned int height, unsigned int bgcolor_v, [, byte mode])
// DOC Where:
// DOC 
// DOC   Display_v is a TFTLCD Screen object for drawing on the buttons
// DOC   ts_v      is a TouchScreen object for sensing button presses
// DOC   x1_v      is coordinate of left   edge of button
// DOC   y1_v      is coordinate of top    edge of button
// DOC   width     is coordinate of right  edge of button
// DOC   height    is coordinate of bottom edge of button
// DOC   mode      is button mode (default is OneShot)
// DOC   bgcolor   is used the background color around the edges used for beveling the button.
softButton::softButton(TFTLCD *Display_v, TouchScreen *ts_v, int ts_xp, int ts_xm, int ts_yp, int ts_ym, unsigned int x1_v,
			unsigned int y1_v, unsigned int width, unsigned int height, unsigned int bgcolor_v) { 
  softButton(Display_v, ts_v, ts_xp, ts_xm, ts_yp, ts_ym, x1_v, y1_v, width, height, bgcolor_v, OneShot);
}

softButton::softButton(TFTLCD *Display_v, TouchScreen *ts_v, int ts_xp, int ts_xm, int ts_yp, int ts_ym, unsigned int x1_v,
			unsigned int y1_v, unsigned int width, unsigned int height, unsigned int bgcolor_v, byte mode_v) { 
	Display = Display_v;
	ts = ts_v;
	previous = false;
	mode = mode_v;
	hold_timer = millis();
	refresh_timer = millis();
	hold_level = 1000;
	hold_refresh = 100;
	x1 = x1_v;
	y1 = y1_v;
	x2 = x1+width-1;
	y2 = y1+height-1;
	bgcolor = bgcolor_v;
	TS_XP = ts_xp;
	TS_XM = ts_xm;
	TS_YP = ts_yp;
	TS_YM = ts_ym;

}
	
void softButton::bevelButton(bool pressed)
{
  short i;
  float m;
  unsigned short int c;
  byte r, g, b;

  extractrgb(bgcolor);
  // Top and Left edges
  pinMode(TS_XM, OUTPUT);
  pinMode(TS_XP, OUTPUT);
  pinMode(TS_YM, OUTPUT);
  pinMode(TS_YP, OUTPUT);

  extractrgb(bgcolor);
  for(i=3; i>=1; i--)		// Top Left
  {
    if(pressed)	// Darken top left
    {
      r= constrain(r - ((max(max(r,g),b))>>1), 0, r);
      g= constrain(g - ((max(max(r,g),b))>>1), 0, g);
      b= constrain(b - ((max(max(r,g),b))>>1), 0, b);
    }
    else	// Lighten top left
    {
      r= constrain(r + ((max(max(r,g),b))>>1), r, 255);
      g= constrain(g + ((max(max(r,g),b))>>1), g, 255);
      b= constrain(b + ((max(max(r,g),b))>>1), b, 255);
    }
    c = Display->Color565(r,g,b);
    Display->drawHorizontalLine(x1+i, y1+i, (x2-x1)-i*2, c);	// Top
    Display->drawVerticalLine(x1+i, y1+i, (y2-y1)-i*2, c);	// Left
  }

  extractrgb(bgcolor);
  for(i=3; i>=1; i--)			 // Bottom Right
  {
    if(pressed)	// Lighten bottom right
    {
      r= constrain(r + ((max(max(r,g),b))>>1), r, 255);
      g= constrain(g + ((max(max(r,g),b))>>1), g, 255);
      b= constrain(b + ((max(max(r,g),b))>>1), b, 255);
    }
    else	// Darken bottom right
    {
      r= constrain(r - ((max(max(r,g),b))>>1), 0, r);
      g= constrain(g - ((max(max(r,g),b))>>1), 0, g);
      b= constrain(b - ((max(max(r,g),b))>>1), 0, b);
    }
    c = Display->Color565(r,g,b);
    Display->drawHorizontalLine(x1+i, y2-i, (x2-x1)-i*2, c);	// Bottom
    Display->drawVerticalLine(x2-i, y1+i, (y2-y1)-i*2, c);	// Right
  }
}

// Setters
void softButton::setMode(byte mode_v) { mode = mode_v; }
void softButton::setTimer(unsigned int t) { if (mode > Memory) hold_level = (unsigned long)t; }
void softButton::setRefresh(unsigned int r) { hold_refresh = r; }

byte softButton::check(byte mode_v) {
  byte mem = mode;
  mode = mode_v;
  byte res = check();
  mode = mem;
  return res;
}
	

byte softButton::check() {
  Point p;
  int tx, ty, dx, dy;

  p = ts->getPoint();
  // TS_Native: x goes from header to top, y goes from header pin 20 to pin 1
  // Display native: X goes from header pin 1 to header pin 20, Y goes from 1x20 header to 2x10 header
  // Map point (TS) to point (display) allowing for rotation
/*
LL = 236,5
LR = 5,0
UR = 8, 317
UL = 240, 308

(Rotation 0)
Y goes from top (0) to bottom (320)	dy = constrain(Y, 0, Display->height())
X goes from left (0) to right (240)	dx = constrain(X, 0, Display->width())

(Rotation 1)


(Rotation 2)
Y goes from bottom 0 to top (320)	(dy=Display->height() - (constrain(Y, 0, Display->height())))
X goes from right 0 to left (240)	(dx=Display->width()  - (constrain(X, 0, Display->width ())))

*/
  // Display rotation 0
  tx = map(p.x, TS_MINX, TS_MAXX, Display->width(), 0);
  ty = map(p.y, TS_MINY, TS_MAXY, Display->height(), 0);
  if(Display->getRotation()==0)
  {
    dx = constrain(tx, 0, Display->width());
    dy = constrain(ty, 0, Display->height());
  }
  // Display rotation 1
  if(Display->getRotation() ==1)
  {
    dx=ty;
    dy=tx;
  }
  // Display rotation 2
  if(Display->getRotation() ==2)
  {
    dx = (Display->width()  - (constrain(tx, 0, Display-> width())));
    dy = (Display->height() - (constrain(ty, 0, Display->height())));
  }
  // Display rotation 3
  if(Display->getRotation() ==3)
  {
     dx=Display->width() - ty;
     dy=Display->height() - tx;
  }
  if (x1 < dx && dx < x2 &&
      y1 < dy && dy < y2 &&
      MINPRESSURE < p.z && p.z < MAXPRESSURE)  // Appropriate pressure within button coordinates
  {
    if(!previous) bevelButton(true);    // Draw button as pressed (if not already)
    switch (mode) {
      case OneShot:
        if (previous) {
          return OFF;
        }
        else {
          previous = true;
          return ON;
        }
        break;
      case Memory:
        if (previous) {
          return ON;
        }
        else {
          previous = true;
          return Pressed;
        }
        break;
      case Timer:
        if (previous) {
          // Is treshold reached?
          if (millis() > hold_timer+hold_level) {
            if (millis() > refresh_timer+hold_refresh) {
              refresh_timer = millis();
              return Hold;
            }
            else return OFF;
          }
          else {
            return ON;
          }
        }
        else {
          previous = true;
          hold_timer = millis();
          refresh_timer = millis();
          return ON;
        }
        break;
      case OneShotTimer:
        if (previous) {
          // Is treshold reached?
          if (millis() > hold_timer+hold_level) {
            if (millis() > refresh_timer+hold_refresh) {
              refresh_timer = millis();
              return Hold;
            }
            else return OFF;
          }
          else {
            return OFF;
          }
        }
        else {
          previous = true;
          hold_timer = millis();
          refresh_timer = millis();
          return ON;
        }
        break;
      case MemoryTimer:
        if (previous) {
          // Is treshold reached?
          if (millis() > hold_timer+hold_level) {
            if (millis() > refresh_timer+hold_refresh) {
              refresh_timer = millis();
              return Hold;
            }
            else return OFF;
          }
          else {
            return ON;
          }
        }
        else {
          previous = true;
          hold_timer = millis();
          refresh_timer = millis();
          return Pressed;
        }
        break;
      default:
        break;
    }
  }
  else if(p.z == 0) {	// Check for clean release
    p = ts->getPoint();
    if(p.z == 0 && ts->getPoint().z == 0 && ts->getPoint().z == 0) {	// Require consecutive release samples for debounce
      if(previous) bevelButton(false);  // If freshly released, draw as unpressed
      if (previous && (mode == 1 || mode == 4)) {
        previous = false;
        return Released;
      }
      previous = false;
      hold_timer = millis();
      refresh_timer = millis();
      return OFF;
    }
    return (previous ? ON : OFF); // Indeterminant state -- return ON/OFF based on previous
  }
  else if(p.z >= MINPRESSURE && p.z <= MAXPRESSURE) {	// Check for sliding to another button (if p.z within pressure range)
    if(previous) bevelButton(false);
    if (previous && (mode == 1 || mode == 4)) {
      previous=false;
      return Released;
    }
    previous = false;
    hold_timer = millis();
    refresh_timer = millis();
    return OFF;
  }
  else // Indeterminant state -- return ON/OFF based on previous
  return (previous ? ON : OFF);
}

