#define MDX20USE
/**
 * 20 * 2 lcd display
 */

#include "controler.h"
#include "mdx20Display.h"
ST7032 lcd;

/*
**************************************************************************************************************
*                                        PRINT CONFIGURATION
**************************************************************************************************************
*/
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


//---------------------------------------------
// lcdへのprint文で書式設定を使えるようにする
//---------------------------------------------
char lcdPrintTemp[17] = {0};
#define LCD_PRINT(...) sprintf(lcdPrintTemp, __VA_ARGS__), lcd.print(lcdPrintTemp);


#if(0)  // 後で対応
/** LCDコントラストの変更
 * この関数で設定しているbufferの値をLCDに転送する
 * 元はLCDモジュールにDAコンバータの出力を与える形だった。i2c版は通信で値を送る
 */
void lcdContrast(void)
{
    FATFS fs;         /* 論理ドライブのワーク・エリア(ファイル・システム・オブジェクト) */
    CHAR fileBuffer[FILEMEMORY];   /* file copy buffer */
    WORD br;
    WORD bw;
    FRESULT res;         /* FatFs function common result code */



    static int32_t initialCheck = 0;
    static float buffer = 0.1;
    int32_t henkou = 0;

    // 起動時に記憶値を読み込む
    if(0 == initialCheck) {
        initialCheck = 1;

        pf_mount(&fs);
        res = pf_open("Contrast.ini");
        if ( FR_OK == res ) {
            if(NULL != pf_gets(fileBuffer, sizeof(fileBuffer))) {
                buffer = (float)atof(fileBuffer);
            } else {
                // nothing
            }
            // file finalaize
            res == pf_read(0, 512, &br);
        }

        lcd.setContrast((char)(0x3f * buffer), true);
    }

    // コントラスト値の変更
    // 倍率変更SWを押したままの時にコントラスト変更モードになる。このときにZ軸変更SWを押すとコントラスト値を変更する
    if(ledcontrastmodeSwLevel == 1) {
        if((axisZminus.getOnEdge() == 1) && (buffer >= 0.01 )) {
            buffer -= 0.01;
            henkou = 1;
        } else if((axisZpulse.getOnEdge() == 1) && (buffer <= 0.99 )) {

            buffer += 0.01;
            henkou = 1;
        }
    }

    // コントラストモードを抜けて変化があればfileに記憶
    if((1 == henkou) && (0 == ledcontrastmodeSwLevel)) {
        pf_mount(&fs);
        res = pf_open("Contrast.ini");
        sprintf(fileBuffer,"%4.2f",(double)buffer);
        pf_printf(fileBuffer);
        pf_write(0, 0, &bw);    // file finalize

        lcd.setContrast((char)(0x3f * buffer), true);
    }

}
#endif // 後で対応

/**
 * setup()でコールする
*/
void mdx20DisplaySetup(void){
  Wire.begin(21,22,400000); // sda, scl, clock
  // put your setup code here, to run once:
  
  lcd.begin(16, 2);
  
  lcd.setContrast(100);            // コントラスト設定
  lcd.clear();

}

void lcdTest(void){
    lcd.clear();
  lcd.setCursor(0,0);
  LCD_PRINT("abcdefghijklnmop");
  lcd.setCursor(0,1);
  LCD_PRINT("1234567890123456");

}


/**
 *
 */
void lcdDisplayInitial(void)
{
    lcd.clear();
    lcd.setCursor(0,0);
    // 行数　    0123456789012345
    LCD_PRINT("MDX-15/20       ");
    lcd.setCursor(0,1);
    LCD_PRINT("     controller ");

    delay(1000);

}

/**　lcd display process
 * 表示レイアウト
 *　
 * <軸移動モード>
 *       01234567 89012345
 * 1行目  移動倍率| X軸位置
 * 2行目  Z軸位置 | Y軸位置
 *
 * <ファイル選択中>
 *       01234567 89012345
 * 1行目  ファイル番号
 * 2行目  ファイル名
 *
 * <ファイルデータ確認 or MDX-20へ出力中>
 *       01234567  89012345
 * 1行目  *1      | X軸位置
 * 2行目  Z軸位置  | Y軸位置
 * *1
 *　　file読み出し中は "残り行数" を表示
 *　　チェック異常時は　”error"を表示
 *
 */


/**
 * @param magnification 1:0.025mm, 40:1mm, 200:5mm
 */
void lcdMagnification(uint16_t  axisMagnification)
{
    lcd.setCursor(0,0);
    switch(axisMagnification) {
        case 1:
            // 表示位置  01234567
            LCD_PRINT("0.025mm ");
            break;
        case 40:
            // 表示位置  01234567
            LCD_PRINT("1mm     ");
            break;
        case 200:
            // 表示位置  01234567
            LCD_PRINT("5mm     ");
            break;
        default:
            LCD_PRINT("error   ");
            break;
    }
}

/**
 *
 */
void lcdPositionMillimeter(float *millimeter)
{

    lcd.setCursor(8,0);
    // 表示位置  89012345
    //          Xxxx.xxx
    LCD_PRINT("X%7.3f",*(millimeter + 0));
/*
    lcd.setCursor(8,1);
    // 表示位置  89012345
    //          Xxxx.xxx
    LCD_PRINT("Y%7.3f",millimeter[1]);
*/
    lcd.setCursor(0,1);
    // 表示位置  01234567
    //          Xxxx.xxx
    LCD_PRINT("Z%7.3fY%7.3f",*(millimeter + 2),*(millimeter + 1));
}

// MDXにデータ転送時のLCD表示
void lcdSendDataPositionMillimeter(float *millimeter)
{
    lcd.setCursor(8,0);
    // 表示位置  89012345
    //          Xxxx.xxx
    LCD_PRINT("X%7.3f",*(millimeter + 0));
/*
    lcd.setCursor(8,1);
    // 表示位置  89012345
    //          Xxxx.xxx
    LCD_PRINT("Y%7.3f",millimeter[1]);
*/
    lcd.setCursor(0,1);
    // 表示位置  01234567
    //          Xxxx.xxx
    LCD_PRINT("Z%7.3fY%7.3f", *(millimeter + 2), *(millimeter + 1));
}

/**
 * 処理しているファイル名を表示
 */
void lcdFileName(char *fileName)
{
    lcd.setCursor(0,1);
    // 表示位置  012345678901234567
    //          filename.prn
    LCD_PRINT("%12s    ",fileName);
}

/**
 * 処理しているファイルの残りbyte数を表示
 * @para char d : 'F':送信前 'C':範囲チェック 'O':MDX-20へ送信
 */
void lcdFileColumnNum(char d, uint32_t number)
{
    lcd.setCursor(0,0);

    // 表示位置  0123456789012345
    //          F=12345
    LCD_PRINT("%c%7d",d, number);
}

/**
 * 選択しているファイル番号 / 聡ファイル番号　の表示
 */
void lcdFileNumber(uint32_t fileNumberNow, uint32_t fileNumberMax)
{
    lcd.setCursor(0,0);
    // 表示位置  01234  56789012345
    LCD_PRINT("%2d/%2d           ",fileNumberNow, fileNumberMax);
}

/**
 *
 */
void lcdPositionOverError(void)
{
    lcd.setCursor(0,0);
    // 表示位置  01234567
    LCD_PRINT("error   ");
}

/**
 *
 */
void lcdPositionKakunin(void)
{
    lcd.setCursor(0,0);
    // 表示位置  01234567
    LCD_PRINT("Kakunin ");
}

