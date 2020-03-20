#ifndef CONTROLER_H
#define CONTROLER_H

#include <Arduino.h>

#include "string.h"


//=========================================
// 1ms base timer interrupt
//=========================================
//#define PRIODTIMER
#ifdef PRIODTIMER
// 1ms interrupt
Ticker kijunTimer;

void timer1ms(void);
#endif


//==============================================
// motor on/off switch
//==============================================
//digitalSw motorOnOffSwitchSw = digitalSw(35);  // digitalSWで定義 


void motorOnOffSwitch(void);

//==============================================
// axis origin setting
//==============================================
//DigitalSw axisZOriginSettingSw (p17, 1);  // digitalSWで定義

void axisZOriginSetting(void);

//==============================================
// axis origin setting
//==============================================
//DigitalSw axisXYOriginSettingSw (p16, 1); // digitalSWで定義

void axisXYOriginSetting(void);

//==============================================
// machine axis initialize
//==============================================
//DigitalSw axisInitializeSw (p23, 1); // digitalSWで定義

void axisInitialize(void);

//==============================================
// machine axis user origin initialize
//==============================================
//DigitalSw axisUserZOriginInitializeSw (p21, 1); // digitalSWで定義

void axisUserZOriginInitialize(void);

//DigitalSw axisUserXYOriginInitializeSw (p22, 1); // digitalSWで定義

void axisUserXYOriginInitialize(void);

//==============================================
// axis control
//==============================================

// axis control
//DigitalSw axisMagnificationSw(p15, 0);    // digitalSWで定義


void axisMagnificationChange(void);
void axisAjustment(void);
void axisLED(int32_t mode);



//=============================================
// main program
//=============================================
    void swUpdate(void);
    int mainControler();


#endif // CONTROLER_H