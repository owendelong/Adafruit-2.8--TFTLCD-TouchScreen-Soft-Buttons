/*
 *  softButtons.h
 *  One-shot and hold functions for TFTLCD TouchScreen soft buttons.
 *  based on buttons library Created by Franky on 29/01/09.
 *  adapted by Owen DeLong 13/02/12
 *  Licensed under LGPL (free to modify and use as you wish)
 */

#include <inttypes.h>
#ifndef TFTLCD_Library_H
#include <TFTLCD.h>
#endif

#ifndef TouchScreen_H
#include <TouchScreen.h>
#endif

#define OneShot 0
#define Memory 1
#define Timer 2
#define OneShotTimer 3
#define MemoryTimer 4

#define ON 1
#define OFF 0
#define Pressed 2
#define Released 3
#define Hold 4

typedef uint8_t byte;


class softButton {
public:
	softButton(TFTLCD *Display_v, TouchScreen *ts_v, int ts_xp, int ts_xm, int ts_yp, int ts_ym, unsigned int x1_v, unsigned int y1_v, unsigned int width, unsigned int height, unsigned int bgcolor);
	softButton(TFTLCD *Display_v, TouchScreen *ts_v, int ts_xp, int ts_xm, int ts_yp, int ts_ym, unsigned int x1_v, unsigned int y1_v, unsigned int width, unsigned int height, unsigned int bgcolor, byte mode_v);
	byte check();
	byte check(byte mode_v);
	// Setters
	void setMode(byte type_v);
	void setTimer(unsigned int t);
	void setRefresh(unsigned int r);
	void bevelButton(bool pressed);
	
        TFTLCD *Display;
	TouchScreen *ts;
        unsigned int bgcolor;
        unsigned int x1, y1, x2, y2;

private:	
	byte mode;
	unsigned long hold_timer;
	unsigned long refresh_timer;
	unsigned int hold_level;
	unsigned int hold_refresh;
	bool previous;
	int TS_XP;
	int TS_XM;
	int TS_YP;
	int TS_YM;
};



/*
 
 Button modes:
 
 - OneShot: OneShot only, returns 2 values (ON/OFF)
 - Memory: Returns (Pressed/ON/Released/OFF)
 - Timer: Hold System (OFF/ON/Hold)
 - OneShotTimer: Combi OneShot & Timer (ON/Hold/OFF)
 - MemoryTimer: Combi Memory & Timer 
 (Pressed/ON/Hold/Released/OFF)
 
 */
