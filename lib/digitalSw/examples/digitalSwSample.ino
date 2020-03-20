/** sample program
 * このサンプルはESP32で動作確認した
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
