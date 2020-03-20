// libraryの作り方
// http://stupiddog.jp/note/archives/266

#include <arduino.h>
#include "digitalSw.h"

//#define DEBUG
#ifdef DEBUG
#ifndef DEBUGTEMP_ARI
#define DEBUGTEMP_ARI   // 他のファイルにdebugTemp[]が定義されていることを知らせる
char debugTemp[32] = {0};
#endif // DEBUGTEMP_ARI
#define DEBUG_PRINT(...) sprintf(debugTemp, __VA_ARGS__), Serial.print(debugTemp);
#undef DEBUG
#else
#define DEBUG_PRINT(...)
#endif // DEBUG

    uint8_t i;


digitalSw::digitalSw(uint16_t pin){
    _pin = pin;
    pinMode(pin, INPUT_PULLUP);
}

// RTOSでSW認識するために、edgeクリアを別関数にした
void digitalSw::refresh(){
    _onEdge = _onEdgeTemp;
    _offEdge = _offEdgeTemp;

    _onEdgeTemp = 0;
    _offEdgeTemp = 0;
}

void digitalSw::update(){

    // edgeデータのクリア (edgeClear()関数に分離)
//    _onEdge = 0;
//    _offEdge = 0;

    // SW認識更新
    _inputLevel = (_inputLevel << 1) | !digitalRead(_pin);

    if((_inputLevel & CHECKMASKBIT) == ONLEVEL){
        _kakuteiLevel = (_kakuteiLevel << 1) | 0x01;

        if((_kakuteiLevel & 0x03) == 0x01){
            _onEdgeTemp = 0x01; 
        }
    }
    else if((_inputLevel & CHECKMASKBIT) == OFFLEVEL){
        _kakuteiLevel = (_kakuteiLevel << 1) | 0x00;
        
        if((_kakuteiLevel & 0x03) == 0x02){
            _offEdgeTemp = 0x01; 
        }
    }
    else{
        // nothing
    }

    DEBUG_PRINT("%2x %d %2x, %2x\r\n",i++, _pin, _inputLevel, !digitalRead(_pin));
}

boolean digitalSw::onEdge(){
    boolean ans = false;
    if(_onEdge == 0x01)ans= true;

    return ans;
}

boolean digitalSw::offEdge(){
    boolean ans = false;
    if(_offEdge == 0x01)ans= true;

    return ans;
}

boolean digitalSw::level(){
    boolean ans = false;
    if((_kakuteiLevel  & 0x01) == 0x01)ans= true;

    return ans;
}

