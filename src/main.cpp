// 履歴
// 190512 : mdx20Uart.cpp initial()で、SPIFFSからのユーザ原点読み出し値の変数への代入で、読み出し文字列のNULLがなく
//          前の軸の文字列と重複して読み出して、原点データが異常になっていた。
//          ファイルからの１文字毎の読み出し時に仮のNULLを書き込むことで対応した。
 
// デバックについて
// MDX-20を接続しないで、動作確認するためuartのcts制御をキャンセルできるようにしてある
// mdx20Uart.cpp 先頭の
//      #define CTSCANCEL // _ctsを無視する場合有効にする
// を無効にすることで通常状態にできる

/*
free RTOSのサンプル
これを参考にSW認識をスレッド化した
https://www.mgo-tec.com/blog-entry-arduino-esp32-multi-task-dual-core-01.html
それ以外の参考
https : //qiita.com/hideakitai/items/bd95d0db63097b8808f9

このプログラムはvscode - platformIO で作成した
*/
#if(1)
#include <Arduino.h>
#include "controler.h"

//#define DEBUG
#ifdef DEBUG
#ifndef DEBUGTEMP_ARI
#define DEBUGTEMP_ARI // 他のファイルにdebugTemp[]が定義されていることを知らせる
        char debugTempMain[32] = {0};
#endif // DEBUGTEMP_ARI
#define DEBUG_PRINT(...) sprintf(debugTempMain, __VA_ARGS__), Serial.print(debugTempMain);
#undef DEBUG
#else
#define DEBUG_PRINT(...)
#endif // DEBUG


// sw認識をRTOSで10ms間隔で認識させる
// 参考HP
// https://www.mgo-tec.com/blog-entry-arduino-esp32-multi-task-dual-core-01.html
TaskHandle_t th[1];

void Task1(void *pvParameters) {
  while(1){
  swUpdate();
  DEBUG_PRINT("swninnsiki\r\n");
  vTaskDelay(10);
//  delay(100);
  }
}

void setup(){
  Serial.begin(115200);

  xTaskCreatePinnedToCore(Task1, "Task1", 4096, NULL, 3, &th[0], 0); //Task1実行
}

void loop(){
  swUpdate();
  Serial.println("start up");

  mainControler();
}
#endif


#if(0)
// これ以下は各部の動作確認した時のプログラム
#include <Arduino.h>
#include "mdx20File.h"
#include "mdx20Display.h"
#include "digitalSw.h"
//#include "analogSw.h"
#include "mdx20Uart.h"

//#define DEBUG
#ifdef DEBUG
#ifndef DEBUGTEMP_ARI
#define DEBUGTEMP_ARI   // 他のファイルにdebugTemp[]が定義されていることを知らせる
char debugTempMain[32] = {0};
#endif // DEBUGTEMP_ARI
#define DEBUG_PRINT(...) sprintf(debugTempMain, __VA_ARGS__), Serial.print(debugTempMain);
#undef DEBUG
#else
#define DEBUG_PRINT(...)
#endif // DEBUG

digitalSw sw1 = digitalSw(26);
digitalSw sw2 = digitalSw(27);

mdx20Uart com = mdx20Uart(2, 12, 4);



void setup() {
    Serial.begin(115200);

    com.initial();
    com.userXYOriginInitial();
    com.userZOriginInitial();

    float millimeter[3];
    com.answerPositonMillimeter(millimeter);
    DEBUG_PRINT("%f, %f, %f\r\n", millimeter[0], millimeter[1], millimeter[2]);

    //    com.xyOrigin();

#if(0)  // mdx20File.cpp sample
        uint32_t fileNumberMax;
    char fileName[32];
    (void)fileDetect(1, fileName, &fileNumberMax);
    Serial.println(fileName);

    uint32_t cnt;
    (void)fileSelect("/jishaku.prn", &cnt);


    (void)fileOpen("/jishaku.prn");
    
    char data[32]={0};
    while(fileLineRead(data, 32, &cnt)){
    //    Serial.print(data);
    }
    
    fileClose();

    (void)fileSelect("/jishaku.prn", &cnt);
#endif // mdx20File.cpp sample

#if(0)  // mdx20Display.cpp sample
  Wire.begin(21,22,400000); // sda, scl, clock
  // put your setup code here, to run once:
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setContrast(100);            // コントラスト設定
  lcd.setCursor(0,0);
  lcd.print("hello !");
#endif
      mdx20DisplaySetup();

/*
  mdx20DisplaySetup();
  lcdTest();
  delay(2000);
  mdx20DisplaySetup();
  delay(2000);
  lcdDisplayInitial();
  delay(2000);
  lcdMagnification(40);
  delay(2000);
  lcdPositionMillimeter();
  delay(2000);
  lcdFileName("test.prn");
  delay(2000);
  lcdFileColumnNum(1234);
  delay(2000);
  lcdFileNumber(123, 9999);
  delay(2000);
  lcdPositionOverError();
  delay(2000);
  lcdPositionKakunin();
*/

}




uint8_t cnt=0, cnt2=0;
void loop() {
#if (0)
  com.sendData("MC1;");
  Serial.println(cnt, DEC);
  delay(100);
  cnt++;
#endif

#if(0)
// ad精度の確認
dacWrite(25, da);
DEBUG_PRINT("da = %d  ad = %d\r\n",da ,analogRead(26)>>4);
da++;
delay(100);
#endif

  // put your main code here, to run repeatedly:


  sw1.refresh();
  sw2.refresh();

  if (true == sw2.onEdge())
  {
    com.offsetXAxisAdjustment(1*40);
   com.xyOrigin();
  }
  
  if (true == sw1.onEdge())
  {
    float millimeter[3];
    com.answerPositonMillimeter(millimeter);
    DEBUG_PRINT("%f, %f, %f\r\n",millimeter[0], millimeter[1], millimeter[2]);
//    lcdMagnification(millimeter[0] );
  }
#if(0)
  if (true == sw2.offEdge())
  {
    switch (cnt2)
    {
    case 1:
      lcdMagnification(1);

      break;
      case 2:
lcdMagnification(40);
      break;
      default:
lcdMagnification(200);
      cnt2 = 0;
      break;
    }
    cnt2++;
    }
#endif

/*
  if(true == sw1.level(1)){
    switch (cnt){
      case 1:
  lcdFileName("1");
      break;
      case 2:
  lcdFileName("2");
      break;
      default:
  lcdFileName("3");
      cnt = 0;
      break;
    }
    cnt++;
    }
*/
}

#endif
