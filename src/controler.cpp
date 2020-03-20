#define MDX20USE    // MDX-20を接続しているときに有効(コメントアウトしているときにはMDX-20への出力を行わない)

#define NOFILECHECK // 動作前のデータチェックを行わないときに有効にする
//==================================================================================
// MDX-20 controlor
//
// MDX-20 COM： dp16(tx) dp15(rx) 受信は行わないがBufferedSerialの定義で必要なのでいれる
//
// PCとのシリアル通信 :USB-Serial
//      テスト用のコマンド受信、モニタ用として使用する
//      9600[bps], startBit = 1bit, stopBit = 1bit, parity なし, LSBファースト
//      送受信するデータは、バイナリーデータをAsciiデータに変換したもの（ターミナルソフトでのデータ確認が可能)
//
//==================================================================================
//#define DEBUG
#ifdef DEBUG
#ifndef DEBUGTEMP_ARI
#define DEBUGTEMP_ARI // 他のファイルにdebugTemp[]が定義されていることを知らせる
char debugTemp[32] = {0};
#endif // DEBUGTEMP_ARI
#define DEBUG_PRINT(...) sprintf(debugTemp, __VA_ARGS__), Serial.print(debugTemp);
#undef DEBUG
#else
#define DEBUG_PRINT(...)
#endif // DEBUG

#include "controler.h"


//==============================================
// digital sw
//==============================================

#include "digitalSw.h"
#define PCB

#ifdef PCB
// 移動X+, X-
digitalSw axisXpulse = digitalSw(34);
digitalSw axisXminus = digitalSw(33);

// 移動Y+, Y-
digitalSw axisYpulse = digitalSw(32);
digitalSw axisYminus = digitalSw(35);

// 移動Z+, Z-
digitalSw axisZpulse = digitalSw(39);
digitalSw axisZminus = digitalSw(36);

// モータON/OFF
digitalSw motorOnOffSwitchSw = digitalSw(27);

// file select/start/stop
digitalSw fileControlSw = digitalSw(13);

// 0:機械原点移動
digitalSw axisInitializeSw = digitalSw(14);
//1:相対XY移動
digitalSw axisUserXYOriginInitializeSw = digitalSw(25);
//2:相対Z移動
digitalSw axisUserZOriginInitializeSw = digitalSw(26);
#define fileDown (axisUserXYOriginInitializeSw)
#define fileUp (axisUserZOriginInitializeSw)
// 3:倍率変更
digitalSw axisMagnificationSw = digitalSw(17);
//4:設定XY確定
digitalSw axisXYOriginSettingSw = digitalSw(16);

//5:設定Z確定
digitalSw axisZOriginSettingSw = digitalSw(15);

//#define ledcontrastmodeSwLevel (swanalog.checkLevel(3)) // axisMagnificationSw lcd contrast mode SW

#else // ブレットボード
// 移動X+, X-
digitalSw axisXpulse = digitalSw(25);
digitalSw axisXminus = digitalSw(27);

// 移動Y+, Y-
digitalSw axisYpulse = digitalSw(14);
digitalSw axisYminus = digitalSw(26);

// 移動Z+, Z-
digitalSw axisZpulse = digitalSw(32);
digitalSw axisZminus = digitalSw(33);

// モータON/OFF
digitalSw motorOnOffSwitchSw = digitalSw(35);

// file select/start/stop
digitalSw fileControlSw = digitalSw(16);

// 0:機械原点移動
digitalSw axisInitializeSw = digitalSw(15);
//1:相対XY移動
digitalSw axisUserXYOriginInitializeSw = digitalSw(13);
//2:相対Z移動
digitalSw axisUserZOriginInitializeSw = digitalSw(17);

// 3:倍率変更
digitalSw axisMagnificationSw = digitalSw(36);
//4:設定XY確定
digitalSw axisXYOriginSettingSw = digitalSw(39);

//5:設定Z確定
digitalSw axisZOriginSettingSw = digitalSw(34);

//#define ledcontrastmodeSwLevel (swanalog.checkLevel(3)) // axisMagnificationSw lcd contrast mode SW
#endif
//==============================================
// file detect & read & stop
//==============================================
//digitalSw fileControlSw = digitalSw(16);    // digitalSWで定義

static char fileName[20];
static uint32_t fileNumberMax = 0;
static uint32_t fileNumberNow = 0;
static uint32_t fileLineMax = 0;
static uint32_t fileLineNow = 0;
static uint32_t filePresence = false; // 0:false 1:true

int32_t fileModeCheck(void);
void fileControl(void);
int32_t fileRequestDisplay(char *displayFileName, int32_t *displayFileNumberNow, int32_t *displayFileNumberMax, int32_t *displayFileLineNow, int32_t *displayFileLineMax, int32_t *status);

//=============================================
// MDX-20 control
//=============================================
#include "mdx20Uart.h"

mdx20Uart mdx20 = mdx20Uart(2, 12, 4); //tr, rx(これは未使用), cts

//=============================================
// PCからのコマンド・データ送受信 (デバックモニタ用)
//=============================================
/*
arudino libraryに定義されている基本機能
setup()で Serial.bign(baud)を実行するればよい

rx  IO3 (Pin34 U0RXD)
tx  IO1 (Pin35 U0TXD)
*/

//==============================================
// LCD monitor
//==============================================
// charactor LCD display 16*2 AE-AQM1602A(KIT)
// http://akizukidenshi.com/catalog/g/gK-08896/

#include "mdx20Display.h"

/*
sda IO21 (Pin33 VSPIHD)
scl IO22 (Pin36 VSPIWP)
*/

//=============================================
// SD card fat/fs
//=============================================
#include "mdx20File.h"

/* ESP32での端子割り当て

SDPin   name        ESP32
9       NC          NC
1       ~CS         IO5(Pin29 VSPICS0)  pull up(*1 = 50k(50k～100k))
2       DataIn      IO23(Pin37 VSPID)   pull up(*1)
3       Vss1        GND
4       Vdd         3.3V
5       CLK         IO18(Pin30 VSPICLK) pullup(*1)
6       Vss2        GND
7       DataOut     IO19(Pin31 VSPIQ)   pullup(*1)
8       NC          NC
*/

//=============================================
// main program
//=============================================
//#define LPC1114 // LPC1114で動作を変える機能に対する条件コンパイル

#define FILEMEMORY (32) // file関係で使用するbufferのbyte数  (一行毎に処理するので、一行の最大文字数を設定しておく)


#if(1)


//=========================================
// 1ms base timer interrupt
// 未使用
//=========================================
#ifdef PRIODTIMER

#define TIMER_DEC(t)     {if ((t) != 0){(t)--;}}

uint16_t T_flame;       // マイコン間通信フレーム時間 (1/1 [ms]/count)


/**
 * 1ms精度タイマのカウントダウン
 */
void timer1ms(void)
{
    TIMER_DEC(T_flame);
}
#endif

//==============================================
// file detect & read & stop
//==============================================
typedef enum {
    fileModeWait,
    fileDiscover,
    fileSelectWait,
    fileSelectExe,
    fileChoice,
    initial,
    waitRead,       // ここまでは自動で来る 次へはSW操作で
    checkAxis,      // 指示範囲確認
    readAndSend,    // sw操作するとstopへ
    stop,
    errorStop
} fileControl_t;

static fileControl_t fileControlState = fileModeWait;

/** file mode check
 * @return true: fileMode false:no fileMode
 */
int32_t fileModeCheck(void)
{
    int32_t ans = false;
    if((fileSelectWait < fileControlState) && (fileControlState < stop)) {
        ans = true;
    }
    return ans;
}




/**
 *
 */
void fileControl(void)
{
    char data[FILEMEMORY];

    static fileControl_t tempFileControlState = fileModeWait;

    if(tempFileControlState != fileControlState) {
        DEBUG_PRINT("file mode = %d\r\n",fileControlState);
        tempFileControlState = fileControlState;
    }

    switch(fileControlState) {
        case fileModeWait:  // 0
        {
            // file選択のための初期化をして、SW操作があればfile表示&選択の処理に移行させる
            *fileName = 0x00;
            fileNumberMax =0;
            fileNumberNow = 0;
            //fileLineMax = 0;
            //fileLineNow = 0;

            if(fileControlSw.onEdge() == 1) {
                fileControlState = fileDiscover;
            }
            break;
        }

        case fileDiscover:  // 1
        {
            // fileがあれば１個目のfile名を表示して、ファイル選択へ移行する
            // fileが無ければfile選択待ちに戻す
            if(fileDetect(1, fileName, &fileNumberMax) == true) {

                //*fileName = 0x00;
                //fileNumberMax =0;
                //fileNumberNow = 0;
                //fileLineMax = 0;
                //fileLineNow = 0;

                fileControlState = fileSelectWait;
            } else {
                fileControlState = fileModeWait;
            }
            break;
        }
        case fileSelectWait:    // 2
         {
            fileNumberNow = 1;
            lcdFileName(fileName);
            lcdFileNumber(fileNumberNow, fileNumberMax);

            fileControlState = fileSelectExe;
            break;
         }
        case fileSelectExe: // 3
          {
            int32_t numberTemp = fileNumberNow;
            // 複数ファイルが有る場合にfile名を切り替えて表示させる
            // file選択SWを押すとそのfileをMDX-20に送る処理に移行させる
            if(1 == fileUp.onEdge()) {
                if(++fileNumberNow > fileNumberMax) {
                    fileNumberNow = 1;
                }
                DEBUG_PRINT("file number = %d\r\n",fileNumberNow);
            }
            if(1 == fileDown.onEdge()) {
                if(--fileNumberNow < 1) {
                    fileNumberNow = fileNumberMax;
                }
                DEBUG_PRINT("file number = %d\r\n",fileNumberNow);
            }


            if(numberTemp != fileNumberNow) {
                if(true == fileDetect(fileNumberNow, fileName, &fileNumberMax)) {
                    //*fileName = 0x00;
                    //fileNumberMax =0;
                    //fileNumberNow = 0;
                    //fileLineMax = 0;
                    //fileLineNow = 0;

                    lcdFileName(fileName);
                    lcdFileNumber(fileNumberNow, fileNumberMax);
                } else {
                    fileControlState = fileModeWait;
                }
            }

            // fileSWを押すと、その時のfileを実行する
            if(1 == fileControlSw.onEdge()) {
                fileControlState = fileChoice;
            }

            // 機械原点SWを押すとfile選択モードを抜ける
            if (1 == axisInitializeSw.onEdge())
            {
                fileControlState = fileModeWait;
            }

            break;
          }
        case fileChoice:    // 4
        {
            // 選択したfileの行数を確認して表示してデータ読み出し状態に移行
            filePresence = false;
            lcdPositionKakunin();
#ifndef LPC1114
            filePresence = fileSelect(fileName, &fileLineMax);    // ここで総行数を確認
#else
            filePresence = true;
#endif // LPC1114

            //*fileName = 0x00;
            fileNumberMax =0;
            fileNumberNow = 0;
            //fileLineMax = 0;
            //fileLineNow = 0;

            lcdFileName(fileName);
            lcdFileColumnNum('F', fileLineMax);
            if(true == filePresence) {
                fileControlState = initial;
            } else {
                fileControlState = errorStop;
            }

            break;
        }
        
        case initial:   // 5
         {
            fileControlState = waitRead;
            break;
         }
        
        case waitRead:  // 6
        {
#ifdef MDX20USE
            mdx20.initial();
#endif
            //fileLineNow = 0;

            //*fileName = 0x00;
            fileNumberMax =0;
            fileNumberNow = 0;
            //fileLineMax = 0;
            fileLineNow = 0;

#ifdef NOFILECHECK  // don't file data check
            // ファイルを開いて通常読み出しへ移行
            if(true == fileOpen(fileName)) {
                fileControlState = readAndSend;
            } else {
                fileControlState = errorStop;
            }
#else // ~NOFILECHECK            
            // ファイルを開いてチェック状態へ移行
            if(true == fileOpen(fileName)) {
                fileControlState = checkAxis;
            } else {
                fileControlState = errorStop;
            }
#endif // NOFILECHECK
            break;
        }

        case checkAxis:   // 7
        case readAndSend: // 8
        {
            // filesw か 原点復帰SWを操作するとstopへ
            // MDXへのデータ送信でのCTS待ちのためにon Edgeを取りこぼすのでlevelで判定させる。
            // levelではfile swを操作するとキャンセル認識するのでfile swは対象から外す
            // この後でSW認識をRTOSで独立して行うようにしたので、エッジ認識にもどした
            //if ((1 == axisInitializeSw.level()))
           if ((1 == fileControlSw.onEdge()) || (1 == axisInitializeSw.onEdge()))
                {
                    fileControlState = stop;
                    break;
            }
#ifndef LPC1114
            if (fileLineNow < fileLineMax)
            {
#endif // LPC1114
    // fileで読み出す行があるときの処理
                if (true == fileLineRead(data, sizeof(data), &fileLineNow))
                {
                    //                    DEBUG_PRINT("%d = %s\r\n",fileLineMax - fileLineNow, data);

                    //*fileName = 0x00;
                    fileNumberMax = 0;
                    fileNumberNow = 0;
                    //fileLineMax = 0;
                    //fileLineNow = 0;
#ifndef LPC1114
                    if (fileControlState == checkAxis){
                        lcdFileColumnNum('C', fileLineMax - fileLineNow);
                    }
                    else{
                    lcdFileColumnNum('O', fileLineMax - fileLineNow);
                    }

                    // DEBUG_PRINT("now/max = %d / %d  %d\r\n", fileLineNow, fileLineMax, fileLineMax - fileLineNow);
#else
                // 総行数の算出をしていないので処理行数を表示
                    //lcdFileColumnNum(fileLineNow);
#endif
                    char *p;
                    char *pbfore;
                    char buffer[FILEMEMORY];
                    p = 0;
                    pbfore = data;

                    // 1行に複数命令がある場合の命令分離
                    do
                    {
                        p = strchr(pbfore, ';');
                        if (p != NULL)
                        {
                            strncpy(buffer, pbfore, (p - pbfore + 1));
                            *(buffer + (p - pbfore + 1)) = 0;//NULL;
                            switch (fileControlState)
                            {
                            case checkAxis:
                                // 範囲チェック
#ifdef MDX20USE
                                if (0 == mdx20.axisMovingCheck(buffer))
                                {
#else
                            if (0)
                            {
#endif
                                    // error data going to stop
                                    DEBUG_PRINT("NG　%d/%d  %s\r\n", fileLineNow, fileLineMax, buffer);
                                    lcdPositionOverError();
                                    fileControlState = stop;
                                }
                                else
                                {
                                    DEBUG_PRINT("OK  %d/%d %s\r\n", fileLineNow, fileLineMax, buffer);
                                }
                                break;
                            case readAndSend:
#ifdef MDX20USE
                                mdx20.sendData(buffer);
                                DEBUG_PRINT("send %d/%d %s\r\n", fileLineNow, fileLineMax, buffer);
#endif
                                break;
                            default:
                                break;
                            }

                            //DEBUG_PRINT("data = %s\r\n",buffer);
                            float millimeter[3];
                            mdx20.answerPositonMillimeter(millimeter);

                            lcdSendDataPositionMillimeter(millimeter);
                            //lcdPositionMillimeter();
                            p++;
                            pbfore = p;
                        }
                    } while (p != NULL);

                    //   fileLineNow++;

#ifndef LPC1114
                }
                else
                {
                    // 1行データが取得できない場合(異常)
                    fileControlState = errorStop;
                }
#endif // LPC1114
            }
            else
            {
                // fileの全行を読み出した後の処理
                lcdFileColumnNum('F', 0);

                switch (fileControlState)
                {
                case checkAxis:
                    // fileデータの範囲チェックが終わったら通常読み出しのための準備を行う
                    fileClose();

                    if (true == fileOpen(fileName))
                    {
#ifdef MDX20USE
                        mdx20.initial();
#endif
                        //*fileName = 0x00;
                        fileNumberMax = 0;
                        fileNumberNow = 0;
                        //fileLineMax = 0;
                        fileLineNow = 0;
                        fileControlState = readAndSend;
                    }
                    else
                    {
                        fileControlState = errorStop;
                    }
                    break;
                case readAndSend:
                    fileControlState = stop;
                    break;
                default:
                    fileControlState = errorStop;
                    break;
                }
            }
            break;
            }
        case stop: // 9
        {
            fileClose();
#ifdef MDX20USE
            mdx20.final();
#endif
            //*fileName = 0x00;
            fileNumberMax =0;
            fileNumberNow = 0;
            //fileLineMax = 0;
            //fileLineNow = 0;

            fileControlState = fileModeWait;
            break;
         }
        
        case errorStop: // 10        // sw操作するとinitalへ
        {
            fileClose();
#ifdef MDX20USE
            mdx20.final();
#endif

            //*fileName = 0x00;
            fileNumberMax =0;
            fileNumberNow = 0;
            //fileLineMax = 0;
            //fileLineNow = 0;

            fileControlState = fileModeWait;
            break;
         }
        
        default:
        {
            fileControlState = fileModeWait;
            break;
        }
    }
}


/** file関係の表示データ (prnファイルがない時の表示のことを考えていない）
 * @param *displayFileName 8.3charactor (ここがNULLなら表示は禁止。戻り値falseにする）
 * @param *displayFileNumberNow :*displayFileNameのprnファイルの通し番号
 * @param *displayFileNumberMax :prnファイルの総数　（ここが０なら通し番号表示は禁止）
 * @param *displayFileLineNow :対象ファイルの残りの行数
 * @param *displayFileLineMax :対象ファイルの総行数　（ここが０なら対象行数番号の表示は禁止）
 * @param *status : flase:異常(error表示など) true:正常(表示不要)
 * @return true:表示データあり false:なし
 */
int32_t fileRequestDisplay(char *displayFileName, int32_t *displayFileNumberNow, int32_t *displayFileNumberMax, int32_t *displayFileLineNow, int32_t *displayFileLineMax, int32_t *status)
{
    int32_t ans = false;

    if(fileName != 0x00) {
        ans = true;

        strcpy(displayFileName, fileName);
        *displayFileNumberMax = fileNumberMax;
        *displayFileNumberNow = fileNumberNow;
        *displayFileLineMax = fileLineMax;
        *displayFileLineNow = fileLineNow;

        if(fileControlState ==  errorStop) {
            *status = false;
        } else {
            *status = true;
        }
    } else {
        // nothing
    }
    return ans;
}

//==============================================
// motor on/off switch
//==============================================

/**
 *
 */
#ifdef MDX20USE
void motorOnOffSwitch(void)
{
    if(1 == motorOnOffSwitchSw.onEdge()) {
        if(true == mdx20.motorStateCheck()) {
            DEBUG_PRINT("motor off\r\n");
            mdx20.motorOff();
        } else {
            DEBUG_PRINT("motor on\r\n");
            mdx20.motorOn();
        }
    }
}
#else
void motorOnOffSwitch(void) {}
#endif

//==============================================
// Ｚ軸ユーザー原点の確定
//==============================================

/**
 *
 */
#ifdef MDX20USE
void axisZOriginSetting(void)
{
    if(1 == axisZOriginSettingSw.onEdge()) {
        mdx20.zOrigin();
    }
}
#else
void axisZOriginSetting(void) {}
#endif

//==============================================
// Ｘ，Ｙ軸ユーザー原点の確定
//==============================================

/**
 *
 */
#ifdef MDX20USE
void axisXYOriginSetting(void)
{
    if (1 == axisXYOriginSettingSw.onEdge())
    {
        mdx20.xyOrigin();
    }
}
#else
void axisXYOriginSetting(void) {}
#endif


//==============================================
// machine axis initialize
// 機械原点への移動(MDXの初期化も行う)
//==============================================

/**
 *
*/
#ifdef MDX20USE
void axisInitialize(void)
{
    if(1 == axisInitializeSw.onEdge()) {
        mdx20.initial();
    }
}
#else
void axisInitialize(void)
{}
#endif

//==============================================
// machine axis user origin initialize
// ユーザー原点への移動(X,Y軸 と Z軸で別で操作)
//==============================================

/**
 *
 */
#ifdef MDX20USE
void axisUserXYOriginInitialize(void)
{
    if (1 == axisUserXYOriginInitializeSw.onEdge())
    {
        mdx20.userXYOriginInitial();
    }
}
#else
void axisUserXYOriginInitialize(void)
{}
#endif

#ifdef MDX20USE
void axisUserZOriginInitialize(void)
{
    if (axisUserZOriginInitializeSw.onEdge() == 1)
    {
        mdx20.userZOriginInitial();
    }
}
#else
void axisUserZOriginInitialize(void)
{}
#endif

//==============================================
// axis control
//==============================================
typedef enum {
    x1 = 1,     // 0.025mm
    x1mm = 40,  // 1.000mm
    x5mm = 200  // 5.000mm
} axisMagnificationSw_t;
axisMagnificationSw_t axisMagnification = x1;

typedef enum {
    xAxis = 0,
    yAxis,
    zAxis
} axis_t;
axis_t axis = xAxis;

/** 軸倍率変更
 *
 */
void axisMagnificationChange(void)
{
    if (1 == axisMagnificationSw.onEdge())
    {
        switch(axisMagnification) {
            case x1:
                axisMagnification = x1mm;
                break;
            case x1mm:
                axisMagnification = x5mm;
                break;
            case x5mm:
                axisMagnification = x1;
                break;
            default:
                axisMagnification = x1;
                break;
        }
    }
}


/**
 *
 */
#ifdef MDX20USE
void axisAjustment(void)
{
    // X軸の移動
    if(1 == axisXpulse.onEdge()) {
        mdx20.offsetXAxisAdjustment(1 * axisMagnification);
    }
    if(1 == axisXminus.onEdge()) {
        mdx20.offsetXAxisAdjustment(-1 * axisMagnification);
    }

    // Y軸の移動
    if(1 == axisYpulse.onEdge()) {
        mdx20.offsetYAxisAdjustment(1 * axisMagnification);
    }
    if(1 == axisYminus.onEdge()) {
        mdx20.offsetYAxisAdjustment(-1 * axisMagnification);
    }

    // Z軸の移動
    // Z軸の移動範囲はマイナス側になる。X,Y軸と操作と移動方向を合わせるために、SW+でマイナス,Sw-でプラスに移動させる
    if(1 == axisZpulse.onEdge()) {
        mdx20.offsetZAxisAdjustment(-1 * axisMagnification);
    }
    if(1 == axisZminus.onEdge()) {
        mdx20.offsetZAxisAdjustment(1 * axisMagnification);
    }

}
#else
void axisAjustment(void)
{}
#endif

/** sw認識更新
 *  freeRTOSで実行する.
 *  この関数とは別でedgeデータは *.refresh()で更新する
 */
void swUpdate(void)
{
//-------------------------------
// sw level and edge data refresh
//-------------------------------
// 移動X+, X-
axisXpulse.update();
axisXminus.update();

// 移動Y+, Y-
axisYpulse.update();
axisYminus.update();

// 移動Z+, Z-
axisZpulse.update();
axisZminus.update();


// モータON/OFF
motorOnOffSwitchSw.update();

// file select/start/stop
fileControlSw.update();

// 0:機械原点移動
axisInitializeSw.update();

//1:相対XY移動
axisUserXYOriginInitializeSw.update();
//2:相対Z移動
axisUserZOriginInitializeSw.update();

// 3:倍率変更
axisMagnificationSw.update();

//4:設定XY確定
axisXYOriginSettingSw.update();

//5:設定Z確定
axisZOriginSettingSw.update();
}


//=============================================
// main program
//=============================================
int mainControler()
{

    //-----------------------------
    // 初期化
    //-----------------------------
    mdx20DisplaySetup();
#ifdef PRIODTIMER
        // 1ms基準タイマー割込み設定
        kijunTimer.attach_us(&timer1ms, 1000);
#endif // PRIODTIMER

    lcdDisplayInitial();
    mdx20.initial();

    //-----------------------------
    // 無限ループ
    //-----------------------------
    for(;;) {
//        swUpdate(); // これは main.cppでRTOSスレッド化する前の動作確認用
        //-------------------------------
        // sw level and edge data refresh
        //-------------------------------
        // 移動X+, X-
        axisXpulse.refresh();
        axisXminus.refresh();

        // 移動Y+, Y-
         axisYpulse.refresh();
         axisYminus.refresh();

        // 移動Z+, Z-
         axisZpulse.refresh();
         axisZminus.refresh();

/*         
         DEBUG_PRINT("x=%d,%d ", axisXpulse.level(), axisXminus.level());
         DEBUG_PRINT("y=%d,%d ", axisYpulse.level(), axisYminus.level());
         DEBUG_PRINT("z=%d,%d ", axisZpulse.level(), axisZminus.level());
  */      

         // モータON/OFF
         motorOnOffSwitchSw.refresh();
    //     DEBUG_PRINT("motor=%d ", motorOnOffSwitchSw.level());

         // file select/start/stop
         fileControlSw.refresh();
      //   DEBUG_PRINT("file=%d ", fileControlSw.level());

         // 0:機械原点移動
         axisInitializeSw.refresh();
        // DEBUG_PRINT("m_ido=%d ", axisInitializeSw.level());
         //DEBUG_PRINT("     ");

         //1:相対XY移動
         axisUserXYOriginInitializeSw.refresh();
        //2:相対Z移動
         axisUserZOriginInitializeSw.refresh();
         //DEBUG_PRINT("x_ido=%d z_ido=%d ", axisUserXYOriginInitializeSw.level(), axisUserZOriginInitializeSw.level());
         //DEBUG_PRINT("     ");

         // 3:倍率変更
         axisMagnificationSw.refresh();
         //DEBUG_PRINT("bai=%d ", axisMagnificationSw.level());
//         DEBUG_PRINT("     ");

         //4:設定XY確定
         axisXYOriginSettingSw.refresh();
  //       DEBUG_PRINT("setXY=%d ", axisXYOriginSettingSw.level());

         //5:設定Z確定
         axisZOriginSettingSw.refresh();
    //     DEBUG_PRINT("setZ=%d ", axisZOriginSettingSw.level());

      //  DEBUG_PRINT("\r\n");
#ifdef MDX20USE

         //-----------------------------
         // MDXがあれば1回だけ初期化する
         //  このプログラムの動作後にMDXを認識した場合を考えて
         //  無限ループ内で処理する
         //-----------------------------
         static int32_t initialzieMDX = false;
         if ((true == mdx20.connectCheck()) && (false == initialzieMDX))
         {
             initialzieMDX = true;

             // ユーザー原点を一度機械原点にした後、ファイルからの値に更新する
             mdx20.initial();
         }
#endif

        fileControl();
//確認まち        lcdContrast();

        // manual motor control
        motorOnOffSwitch();

        //---------------------------------------------------------
        // MDX-20にfileデータを送っていないときに実行してよい処理を呼び出す
        //      手動による軸移動とユーザー原点の確定処理
        //---------------------------------------------------------
        if(false == fileModeCheck()) {

            //---------------
            // 確定SW操作によるユーザー原点の確定
            //---------------
            axisZOriginSetting();
            axisXYOriginSetting();

            //---------------
            // SW操作による各種原点への移動
            //---------------
            // 機械原点への移動(MDXの初期化も行う)
            axisInitialize();

            // ユーザー原点への移動
            axisUserZOriginInitialize();
            axisUserXYOriginInitialize();

            //---------------
            // 軸倍率変更
            //---------------
            axisMagnificationChange();

            //---------------
            // 手動操作(SW操作)による軸移動
            //---------------
            axisAjustment();

            //---------------
            // monitor
            //---------------
            lcdMagnification((uint16_t)axisMagnification);
            float millimeter[3];
            mdx20.answerPositonMillimeter(millimeter);
            lcdPositionMillimeter(millimeter);
        }

    }
}




#endif

