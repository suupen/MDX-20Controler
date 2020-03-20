#ifndef DIGITALSW_H
#define DIGITALSW_H

/** タクトSWのノイズ除去＆on,offエッジ認識　library
 * 
 * swはIO端子とGND間に接続する
 * マイコンの設定でpull up設定するので、外部のpullup抵抗は不要(マイコンで設定できない場合は必要. R=10k程度)
 *　main関数でrefresh()関数を呼ぶ。この関数で、SWレベルの認識・更新を行い,edge認識を行う. edgeデータはこの関数でクリアされる
 * onEdge(), offEdge()の戻り値が認識値になる
 */


#include <Arduino.h>

class digitalSw {
    public:
    digitalSw(uint16_t pin);
    void edgeClear();    
    void refresh();
    void update();
    boolean onEdge();
    boolean offEdge();
    boolean level();

  private:
    uint16_t _pin;
    uint8_t _inputLevel;
    uint8_t _kakuteiLevel;

    // tempエッジデータ(SW認識で記憶する変数)
    uint8_t _onEdgeTemp;
    uint8_t _offEdgeTemp;

    // 確定エッジデータ(シーケンスでSW認識を使う変数)
    uint8_t _onEdge;
    uint8_t _offEdge;

#define CHECKMASKBIT (0x0f)
#define ONLEVEL (0x0f)
#define OFFLEVEL (0x00)
};


#endif // DIGITALSW_H






#if(0)
/** sample program
 *　sw:IO27 マイコン内部でpull up
 *  led:IO32   +3.3v →　LED　→ IO32
 * 
 * 動作内容
 * swのONエッジを認識するごとにcntを+1して、その結果が偶数ならLED=消灯 奇数ならLED=点灯させる
 */
 
#include <Arduino.h>
#include "digitalSw.h"

digitalSw sw1 = digitalSw(27);


int ledPin = 32;

void setup(){
pinMode(ledPin,OUTPUT);
}

uint8_t cnt = 0;

void loop(){
  sw1.refresh();    // sw認識の更新. edgeデータはここで設定＆クリアされる()

#if(1)
    // on edgeによる操作
  // sw =off→on でカウントする
    if( true == sw1.onEdge()){
        cnt++;
    }
#endif

#if(0)
  // off edgeによる操作
  // sw =on→off でカウントする
  if(true == sw1.offEdge()){
    cnt++;
  }
#endif

#if(0)
  // on levelによる操作
  // sw=on し続ける間カウントし続ける
  if(true == sw.level()){
    cnt++;
  }
#endif


#if(0)
  // off levelによる操作
  // sw=off し続ける間カウントし続ける
  if(false == sw.level()){
    cnt++;
  }
#endif

    // cntのカウントアップでLEDを点滅させる
    // cntの偶数の時に消灯(HIGH)
    // cntの奇数の時に点灯(LOW)
    if(0 == cnt %2){
        digitalWrite(ledPin, HIGH);
    }
    else{
        digitalWrite(ledPin, LOW);
    }

}



#endif 