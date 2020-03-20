/** fileを開いて1行づつデータを読み出す
 *  
 */
// Git 参考 https : //github.com/espressif/arduino-esp32/blob/master/libraries/SD/src/SD.cpp

#include "mdx20File.h"

        File commonfile; // 関数をまたいで利用するfile

/*
**************************************************************************************************************
*                                        PRINT CONFIGURATION
**************************************************************************************************************
*/

//#define DEBUG
#ifdef DEBUG

#ifndef DEBUGTEMP_ARI
#define DEBUGTEMP_ARI   // 他のファイルにdebugTemp[]が定義されていることを知らせる
char debugTempf[32] = {0};
#endif // DEBUGTEMP_ARI

#define DEBUG_PRINT(...) sprintf(debugTempf, __VA_ARGS__), Serial.print(debugTempf);
#undef DEBUG
#else
#define DEBUG_PRINT(...)
#endif // DEBUG



/** File check 第一引数の番号のファイル名を第二引数に返す.　第三引数には対象フォルダにあるファイル総数を返す
 * @para fileNumber : 取得するファイル名の番号  This number get filename 0,1:first 2: second ...
 * @para *fileName  : get filename
 * @para *fileNumberMax : 対象フォルダにあるファイル総数 .prn file number
 * @return
 *  false: non file true: file exists
 */
int32_t fileDetect(uint32_t fileNumber, char *fileName, uint32_t *fileNumberMax)
{

    File root;
    File file;

    uint32_t numMax = 0;
    int32_t ans = false;

    *fileNumberMax = numMax;    // 異常終了用の戻り値
    SD.end();                   // 以前のSD認識をクリアする。本当は関数を抜けるときに行ったほうが良いがreturnが複数あるのでここで処理する。これがないと、SD動作後一度抜き差しすると再認識できない
    if(!SD.begin()){
        DEBUG_PRINT("don't mount SD\r\n");
        return (ans);
    }

    root = SD.open("/");
    if(!root){
        DEBUG_PRINT("no directory or file\r\n");
        return(ans);
    }

    
    while(1) {
        file = root.openNextFile();

        if(!file){
            DEBUG_PRINT("end of check\r\n");
            break;
        }

        if(file.isDirectory()){
            DEBUG_PRINT("directory name = %s\r\n",file.name());
//            continue;
        } else {
            if((NULL != strstr(file.name(), ".prn")) || (NULL != strstr(file.name(), ".PRN"))) {
                // *.prn" fileあり
                numMax++;
                if(numMax == fileNumber) {
                    strcpy(fileName, file.name());
                }
                DEBUG_PRINT("%d  detect = %s\r\n",numMax, file.name());
                ans = true;
            } else {
                DEBUG_PRINT("not *.prn file = %s\r\n",file.name());
            }

        }
    }
    *fileNumberMax = numMax;
    return ans;
}

/** 指示したfileの総行数を取得
 * @param *fileName : 指示するfile name
 * @param *numberLine : 取得したbyte数
 * @return false:行数取得できず true:処理成功
 */
int32_t fileSelect(char *fileName, uint32_t *numberLine)
{
    int32_t ans = false;
    File file;

    if(!SD.begin()){
        DEBUG_PRINT("don't mount SD\r\n");
        return (ans);
    }

    file = SD.open(fileName);
    if(!file){
        *numberLine = 0;
        return (ans);
    }

    *numberLine = file.size();
    DEBUG_PRINT("file byte = %d\r\n",*numberLine);
    ans = true;

    return ans;
}


/** file open
 * @param *fileName : 指示するfile name
 * @return false:openできず true:処理成功
 */
int32_t fileOpen(char *fileName)
{
    int32_t ans = false;

    if(!SD.begin()){
        DEBUG_PRINT("don't mount SD\r\n");
        return (ans);
    }

    commonfile = SD.open(fileName);
    if(!commonfile){
        DEBUG_PRINT("file open file open error\r\n");
        return (ans);
    }
    ans = true;
    DEBUG_PRINT("success file open\r\n ");
    return ans;
}

/** file close
 * @param void
 * @return (false:ありえない) true:処理成功
 */
int32_t fileClose(void)
{
    int32_t ans = true;

    DEBUG_PRINT("file close\r\n");

    commonfile.close();

    return ans;
}

/** 順番に１行づつ読み出す
 * @param *data : 取得した行データを格納するバッファ
 * @param dataNumber : *dataのbyte数
 * @param numberLine : 読み出したファイル位置)
 * @return false:失敗 true:成功
 */
int32_t fileLineRead(char *data, uint32_t dataNumber, uint32_t *numberLine)
{
    int32_t ans = false;
    char readData;              // 1文字取り出し用
    int32_t cnt = 0;            // 一行分のbufferがいっぱいになったらそこで一度抜ける

    while(commonfile.available()){
        ans = true;
        readData = commonfile.read();
        data[cnt] = readData;
        data[cnt + 1] = 0x00;
        if((readData == '\n') || (cnt >= (dataNumber - 2))){
            DEBUG_PRINT("file Line read = %s \r\n",data);
            break;
        }
        cnt++;
    }
    *numberLine = (int32_t)commonfile.position();
    DEBUG_PRINT("position = %d \r\n", *numberLine);
    return (ans);
}

