#define CTSCANCEL   // _ctsを無視する場合有効にする

#ifdef CTSCANCEL
#define CTSOK (1)
#define CTSNG (0)
#else
#define CTSOK (0)
#define CTSNG (1)
#endif

#include <stdio.h>
#include <arduino.h>

// SPI Flash File System用(設定値の記憶)
// 参考:https://github.com/espressif/arduino-esp32/blob/master/libraries/SPIFFS/examples/SPIFFS_Test/SPIFFS_Test.ino
//https://qiita.com/T-YOSH/items/0485af213c31f7425151
#include "FS.h"
#include "SPIFFS.h"
const uint32_t BUF_SIZE = 255;
/* You only need to format SPIFFS the first time you run a
   test or else use the SPIFFS plugin to create a partition
   https://github.com/me-no-dev/arduino-esp32fs-plugin */
#define FORMAT_SPIFFS_IF_FAILED true

#include "mdx20Uart.h"

//#define DEBUG
#ifdef DEBUG
#ifndef DEBUGTEMP_ARI
#define DEBUGTEMP_ARI // 他のファイルにdebugTemp[]が定義されていることを知らせる
char debugTempf[32] = {0};
#endif // DEBUGTEMP_ARI

#define DEBUG_PRINT(...) sprintf(debugTempf, __VA_ARGS__), Serial.print(debugTempf);
#undef DEBUG
#else
#define DEBUG_PRINT(...)
#endif // DEBUG

    HardwareSerial _serial(1); // UART1 (RX=9, TX=10 初期時の設定)

mdx20Uart::mdx20Uart(uint16_t tx, uint16_t rx, uint16_t cts){


    _serial.begin(9600, SERIAL_8N1, rx, tx);

    _cts = cts;
    pinMode(cts, INPUT_PULLUP);


}



mdx20Uart::~mdx20Uart()
{
}


/** ユーザー原点情報を機械原点にクリアする
 *  
 */
void mdx20Uart::userOriginPositionInitial(void)
{
    D_userOriginPosition[Z_x] = 0;
    D_userOriginPosition[Z_y] = 0;
    D_userOriginPosition[Z_z] = 0;
}

/** シーケンサーの記憶位置を機械原点に初期化
 *  この処理を行ったときには、MDXの軸も機械原点に移動させること
 */
void mdx20Uart::clearPositon(void)
{
    D_position[Z_x] = 0;
    D_position[Z_y] = 0;
    D_position[Z_z] = 0;
}

void mdx20Uart::answerPositon(int16_t *position)
{
    *(position + Z_x) = D_position[Z_x];
    *(position + Z_y) = D_position[Z_y];
    *(position + Z_z) = D_position[Z_z];

}

void mdx20Uart::answerPositonMillimeter(float *position)
{
    *(position + Z_x) = (float)D_position[Z_x] * countToMillimeter;
    *(position + Z_y) = (float)D_position[Z_y] * countToMillimeter;
    *(position + Z_z) = (float)D_position[Z_z] * countToMillimeter;

}

/** 制御データに対してユーザー原点を加味したデータに変換する
 *
 */
void mdx20Uart::integralPosition(char *str)
{
    char strData[FILEMEMORY];
    char *p;
    static uint8_t AorR = 'A';  // 'A'=absolute 'R'=relative

    strcpy(strData, str);

    if( 0 == strncmp("^PA", strData, 3)) {
        AorR = 'A';
        return;
    } else if( 0 == strncmp("^PR", strData, 3)) {
        AorR = 'R';
        *(str + 2) = 'A';  // MDX-20に送信するときは相対値から絶対値にする(この関数の中で相対値から絶対値にしているため）
        return;
    }

    //---------------------------------------
    // 読み出したコマンドから各軸の数値を抽出する
    //---------------------------------------
    // コマンド中の","を" "に置き換える(","があるとそれも区別のしないといけないため)
    while ((p = strchr(strData, ','))!=NULL) *p = ' ';
    int32_t a[3] = {0,0,0};

    // 各軸の値を取得
    if(strncmp(str, "!ZZ",3) == 0) {
        sscanf((strData + 3), "%d %d %d", &a[0], &a[1], &a[2]);
    } else if(strncmp(str, "Z",1) == 0) {
        sscanf((strData + 1), "%d %d %d", &a[0], &a[1], &a[2]);
    } else if(strncmp(str, "^PU",3) == 0) {
        sscanf((strData + 3), "%d %d", &a[0], &a[1]);
        a[2] = SHRT_MAX;
    } else if(strncmp(str, "^PD",3) == 0) {
        sscanf((strData + 3), "%d %d", &a[0], &a[1]);
        a[2] = SHRT_MAX;
    } else if(strncmp(str, "!ZM",3) == 0) {
        sscanf((strData + 3), "%d", &a[2]);
        a[0] = SHRT_MAX;
        a[1] = SHRT_MAX;

    } else {
        return;
    }

    if(AorR == 'A') {
        if(a[0] != SHRT_MAX) {
            D_position[Z_x] = a[0] + D_userOriginPosition[Z_x];
        }
        if(a[1] != SHRT_MAX) {
            D_position[Z_y] = a[1] + D_userOriginPosition[Z_y];
        }
        if(a[2] != SHRT_MAX) {
            D_position[Z_z] = a[2] + D_userOriginPosition[Z_z];
        }
    } else {
        // controler axis move data change to absolute from relative
        if(a[0] != SHRT_MAX) {
            D_position[Z_x] += a[0];
        }
        if(a[1] != SHRT_MAX) {
            D_position[Z_y] += a[1];
        }
        if(a[2] != SHRT_MAX) {
            D_position[Z_z] += a[2];
        }
    }
    translationToControlerAxisMoveDataFromRMLAxisMoveData(str);
}


/** ユーザー原点情報を加味した値を元に制御データに組みなおす
 *
 */
void mdx20Uart::translationToControlerAxisMoveDataFromRMLAxisMoveData(char *str)
{
    char buffer[FILEMEMORY];

    if(strncmp(str, "!ZZ",3) == 0) {
        sprintf(buffer,"!ZZ%d,%d,%d;",D_position[Z_x], D_position[Z_y], D_position[Z_z]);
        strcpy(str, buffer);
    } else if(strncmp(str, "Z",1) == 0) {
        sprintf(buffer,"Z%d,%d,%d;",D_position[Z_x], D_position[Z_y], D_position[Z_z]);
        strcpy(str, buffer);
    } else if(strncmp(str, "^PU",3) == 0) {
        sprintf(buffer,"^PU%d,%d;",D_position[Z_x], D_position[Z_y]);
        strcpy(str, buffer);
    } else if(strncmp(str, "^PD",3) == 0) {
        sprintf(buffer,"^PD%d,%d;",D_position[Z_x], D_position[Z_y]);
        strcpy(str, buffer);
    } else if(strncmp(str, "!ZM",3) == 0) {
        sprintf(buffer,"!ZM%d;",D_position[Z_z]);
        strcpy(str, buffer);
    } else {
        // nothing
    }
}


/** X,Y軸のユーザー原点を記憶
 * 今いる位置をX,Y軸のユーザー原点としてファイルに記憶する
 */
uint8_t mdx20Uart::xyOrigin(void)
{
    char buffer[FILEMEMORY]; /* file copy buffer */
    uint8_t ans = false;

    //----------------------------
    // 今いる位置をユーザー原点とする
    //----------------------------
    D_userOriginPosition[Z_x] = D_position[Z_x];
    D_userOriginPosition[Z_y] = D_position[Z_y];
//  D_userOriginPosition[Z_z] = 0;

    //----------------------------
    // ユーザー原点をファイルに記憶する
    //----------------------------
    if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED))
    {
        DEBUG_PRINT("SPIFFS Mount Failed\r\n");
        return ans;
    }

    File file = SPIFFS.open("/ORINGINX.INI",FILE_WRITE);    // 引数の"FILE_wRITEは先頭から上書きする
    if(!file){
        DEBUG_PRINT("- failed to open file for writing\r\n");
        return ans;
    }
    sprintf(buffer,"%d",D_userOriginPosition[Z_x]);
 
     if(file.print(buffer)){
        DEBUG_PRINT("- file written X = %s\r\n",buffer);
    } else {
        DEBUG_PRINT("- frite failed\r\n");
        return ans;
    }
    file.close();

    file = SPIFFS.open("/ORINGINY.INI",FILE_WRITE);    // 引数の"FILE_wRITEは先頭から上書きする
    if(!file){
        DEBUG_PRINT("- failed to open file for writing\r\n");
        return ans;
    }
    sprintf(buffer,"%d",D_userOriginPosition[Z_y]);
 
     if(file.print(buffer)){
        DEBUG_PRINT("- file written Y = %s\r\n",buffer);
    } else {
        DEBUG_PRINT("- frite failed\r\n");
        return ans;
    }
    file.close();
    ans = true;
    return ans;
}



/** ZY軸のユーザー原点を記憶
 * 今いる位置をZ軸のユーザー原点としてファイルに記憶する
 */
uint8_t mdx20Uart::zOrigin(void)
{
    char buffer[FILEMEMORY];   /* file copy buffer */

    uint8_t ans = false;

    //----------------------------
    // 今いる位置をユーザー原点とする
    //----------------------------
    D_userOriginPosition[Z_z] = D_position[Z_z];


    //----------------------------
    // ユーザー原点をファイルに記憶する
    //----------------------------
    if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED))
    {
        DEBUG_PRINT("SPIFFS Mount Failed\r\n");
        return ans;
    }

    File file = SPIFFS.open("/ORINGINZ.INI", FILE_WRITE); // 引数の"FILE_wRITEは先頭から上書きする
    if (!file)
    {
        DEBUG_PRINT("- failed to open file for writing\r\n");
        return ans;
    }
    sprintf(buffer, "%d", D_userOriginPosition[Z_z]);

    if (file.print(buffer))
    {
        DEBUG_PRINT("- file written Z = %s\r\n",buffer);
    }
    else
    {
        DEBUG_PRINT("- frite failed\r\n");
        return ans;
    }

    ans = true;
    return ans;
}



/** offset加算後の各軸の可動範囲チェック
 * @para data RMLデータ一命令文字列の先頭アドレス
 * @returns
 *  false :逸脱あり
 *  true  :正常
 */
int32_t mdx20Uart::axisMovingCheck(char* data)
{
    int32_t ans = true;
    integralPosition(data);
    if(
        ((D_position[Z_x] < Z_xAxisMin) || (Z_xAxisMax < D_position[Z_x])) ||
        ((D_position[Z_y] < Z_yAxisMin) || (Z_yAxisMax < D_position[Z_y])) ||
        ((D_position[Z_z] < Z_zAxisMin) || (Z_zAxisMax < D_position[Z_z]))
    ) {
        ans = false;
    }
    return ans;
}

/**
* MDX-15/20へのデータ送信
* @@para *data : データ一行の先頭アドレス
* @@para uint8_t : false(0):送信キャンセル true(1):送信完了 (無条件にtrueにしている)
*/
uint8_t mdx20Uart::sendData(char* data)
{
    uint8_t ans = false;    // 0:送信キャンセル 1:送信完了
    char buffer[FILEMEMORY];
    char dat;
    uint16_t p = 0;

    strcpy(buffer, data);

    integralPosition(buffer);

    strcat(buffer, "\r\n");
    DEBUG_PRINT("send buffer = %s", buffer);

    do
    {
        dat = buffer[p++];
//      DEBUG_PRINT("p = %d  %02x\r\n", p, dat);

        if (dat != 0)
        {
            delay(10);
            while (digitalRead(_cts) != CTSOK)
            {
                delay(10); // ctsチェックに10msの間を置く
            }
            _serial.write(dat);
//          DEBUG_PRINT("S = %d  %02x\r\n", p, dat);
        }
    } while (dat != 0);

    /*
    delay(200); // このwait timeがないとMDX-20からのwait指示を読み飛ばす(最初はctsチェック後に入れていたが、本来の意味はctsの変化を待つものなので前に持ってくる)
    while (digitalRead(_cts) != 0)
    {
        delay(10); // ctsの変化を待つ。送信可能になるのはms単位なので、10ms程度待ちを持つ
        DEBUG_PRINT("cts wait %d\r\n", digitalRead(_cts));
    }
    _serial.printf("%s\r\n", buffer);

    DEBUG_PRINT("sendData send  %s\r\n", buffer);
*/
    if(strncmp(buffer, "!MC0", 4) == 0) {
        motorState = false;
    } else if(strncmp(buffer, "!MC1", 4) == 0) {
        motorState = true;
    } else {
        // nothing
    }
    
//    DEBUG_PRINT("sendData motor check\r\n");

    


    // 可動範囲外に出たらmin,maxに差し替える
    if(D_position[Z_x] < Z_xAxisMin) {
        D_position[Z_x] = Z_xAxisMin;
    }
    if(Z_xAxisMax < D_position[Z_x]) {
        D_position[Z_x] = Z_xAxisMax;
    }
    if(D_position[Z_y] < Z_yAxisMin) {
        D_position[Z_y] = Z_yAxisMin;
    }
    if(Z_yAxisMax < D_position[Z_y]) {
        D_position[Z_y] = Z_yAxisMax;
    }
    if(D_position[Z_z] < Z_zAxisMin) {
        D_position[Z_z] = Z_zAxisMin;
    }
    if(Z_zAxisMax < D_position[Z_z]) {
        D_position[Z_z] = Z_zAxisMax;
    }

    ans = true;
    return (ans);
}



/** MDXの初期化処理
 *  ファイルからユーザー原点を読み出す
 *  MDXの動作指示を初期化する(モータOFF, 全軸機械原点に移動させる)
 *  機械原点に移動した後、ユーザー原点に移動させる
 */
uint8_t mdx20Uart::initial(void)
{
    uint8_t ans = false;
    char buffer[FILEMEMORY] = {0};  /* file copy buffer */
    uint8_t cnt = 0; //buffer[cnt]

    char sbuffer[FILEMEMORY];
    
    
    
    //-----------------------------
    // シーケンサーのユーザー原点位置と、現在位置記憶を機械原点に初期化
    // ユーザー原点情報はこの後ファイルから読み出した値に更新する
    // (こうすることによって、ファイルから読み出せないときにユーザー原点が不定になることを
    //  防ぐ)
    //-----------------------------
    userOriginPositionInitial();
    clearPositon();

    //-----------------------------
    // ユーザー原点をファイルから読み出し
    //-----------------------------
    if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED))
    {
        DEBUG_PRINT("SPIFFS Mount Failed\r\n");
        return ans;
    }

    // X軸のユーザー原点読み出し
    File file = SPIFFS.open("/ORINGINX.INI", FILE_READ);
    if (!file)
    {
        DEBUG_PRINT("- failed to open file for reading\r\n");
        return ans;
    }
    DEBUG_PRINT("- read from file:\r\n");
    while (file.available())
    {
        buffer[cnt] = file.read();
        if(buffer[cnt] == '\n'){
            break;
        }
        cnt++;
        buffer[cnt] = 0; // 文字列終端のNULL(仮)
    }

    D_userOriginPosition[Z_x] = atoi(buffer);
    DEBUG_PRINT("X ini %d\r\n", D_userOriginPosition[Z_x]);

    // Y軸のユーザー原点読み出し
    file = SPIFFS.open("/ORINGINY.INI", FILE_READ);
    if (!file)
    {
        DEBUG_PRINT("- failed to open file for reading\r\n");
        return ans;
    }
    DEBUG_PRINT("- read from file:\r\n");
    cnt= 0;
    while (file.available())
    {
        buffer[cnt] = file.read();
        if (buffer[cnt] == '\n')
        {
            break;
        }
        cnt++;
        buffer[cnt] = 0;    // 文字列終端のNULL(仮)
    }
    D_userOriginPosition[Z_y] = atoi(buffer);
    DEBUG_PRINT("Y ini %d\r\n", D_userOriginPosition[Z_y] );

    // Z軸のユーザー原点読み出し
    file = SPIFFS.open("/ORINGINZ.INI", FILE_READ);
    if (!file)
    {
        DEBUG_PRINT("- failed to open file for reading\r\n");
        return ans;
    }
    DEBUG_PRINT("- read from file:\r\n");
    cnt = 0;
    while (file.available())
    {
        buffer[cnt] = file.read();
        if (buffer[cnt] == '\n')
        {
            break;
        }
        cnt++;
        buffer[cnt] = 0; // 文字列終端のNULL(仮)
    }
    D_userOriginPosition[Z_z] = atoi(buffer);
    DEBUG_PRINT("Z ini %d\r\n", D_userOriginPosition[Z_z]);

    //-----------------------------
    // 動作初期化(モータを止める.Ｚ軸を機械原点に移動.絶対位置設定.)
    //-----------------------------
    sendData("^IN;");
    sendData("!MC0;");
    sendData("!ZO0;");
    sendData("!ZO0;"); // Z axis origin initialaize
    sendData("^PA;");

    //-----------------------------
    // リミッタが働くまで動作させて、機械原点を確定させる
    // (移動量としてMDX-20の最大移動量を設定したほうがよくないか)
    //-----------------------------
    sprintf(sbuffer,"Z%d,%d,%d;",-D_userOriginPosition[Z_x], -D_userOriginPosition[Z_y], -D_userOriginPosition[Z_z]);

    sendData(sbuffer);

DEBUG_PRINT("initial after\r\n");

    return (true);
}

/**
 * 
 */
uint8_t mdx20Uart::motorOff(void)
{

    sendData("!MC0;");
    return (true);
}


/**
 * @note モータを動かすためには位置設定コマンドの発行が必要
 */
uint8_t mdx20Uart::motorOn(void)
{

    sendData("!MC1;");
    sendData("^PR;");
    sendData("Z0,0,0");


    return (true);
}

#if(0) //LPC1114では使っていない
/** 機械原点への移動
 *
 */
uint8_t mdx20Uart::userOriginInitial(void)
{
    char buffer[FILEMEMORY];

    sendData("^PA;");
    sendData("!MC0;");

    sprintf(buffer, "Z%d,%d,%d;",0, 0, 0);
    sendData(buffer);

    return (true);
}
#endif

/** X,Y軸のユーザー原点への移動(Z軸は現状位置)
 *
 */
uint8_t mdx20Uart::userXYOriginInitial(void)
{
    char buffer[FILEMEMORY];

    sendData("^PA;");
    sendData("!MC0;");

    sprintf(buffer, "Z%d,%d,%d;",0, 0, D_position[Z_z] - D_userOriginPosition[Z_z]);
    sendData(buffer);

    return true;
}


/** Z軸のユーザー原点への移動(X,Y軸は現状位置)
 *
 */
uint8_t mdx20Uart::userZOriginInitial(void)
{
    char buffer[FILEMEMORY];

    sendData("^PA;");
    sendData("!MC0;");

    sprintf(buffer, "!ZM%d;",0);
    sendData(buffer);

    return true;
}

/** 動作完了後の停止処理
 *  モータ停止,絶対位置設定,Z軸のみ機械原点へ移動(X,Y軸は現状位置停止)
 */
uint8_t mdx20Uart::final(void)
{

// モータを止めて、Z軸を機械原点に戻す。(X,Y軸はその場にとどめる)
    sendData("!MC0;");
    sendData("^PA;");
    sendData("!ZO0;");
    sendData("!ZO0;"); // Z axis origin initialaize

    /* このコードだとZ軸が原点に戻らないままXY実を原点に戻すためワークに引っ掛かり破損する
        ans &= sendData("!MC0;");
        ans &= sendData("^PA;");
        ans &= sendData("Z0,0,0;");
    */
    
    
    //clearPositon();
    D_position[Z_z] = 0;

    sendData("^IN;");
    return (true);
}

uint8_t mdx20Uart::XYZMove(int16_t x, int16_t y, int16_t z)
{
    char buffer[FILEMEMORY];

    sendData("!MC0;");
    strcpy(buffer, "^PR;");
    sendData(buffer);
    sprintf(buffer, "Z%05d,%05d,%05d;",x, y, z);
    sendData(buffer);

    return (true);
}

void mdx20Uart::offsetXAxisAdjustment(int16_t axisData)
{
    XYZMove(axisData, 0, 0);
//    D_userOriginPosition[Z_x] = 0;

}

void mdx20Uart::offsetYAxisAdjustment(int16_t axisData)
{
    XYZMove(0, axisData, 0);
//    D_userOriginPosition[Z_y] = 0;

}

void mdx20Uart::offsetZAxisAdjustment(int16_t axisData)
{
    XYZMove(0, 0, axisData);
//    D_userOriginPosition[Z_z] = 0;

}


int32_t mdx20Uart::motorStateCheck(void)
{
    return motorState;
}

/** MDX-20の接続確認
 * @return true:接続あり false:なし
 */
int32_t mdx20Uart::connectCheck(void)
{
    int32_t ans = false;

    if (digitalRead(_cts) == CTSOK)
    {
        ans = true;
    } else {
        ans = false;
    }
    return (ans);
}

