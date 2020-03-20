// libraryの作り方
// http://stupiddog.jp/note/archives/266

#include <arduino.h>
#include "analogSw.h"

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



analogSw::analogSw(uint16_t pin){
    _pin = pin;
} 


//=================================================
// analog input data wo sw data ni henkan
//=================================================
void analogSw::refresh(void){
    uint8_t ans = 0;    // SW ninsikichi   2bit = sw2, 1bit = sw1, 0bit = sw0  0:OFF 1:ON 
    float ad = (float)analogRead(_pin) / 4095;
    // edgeデータのクリア
        for(uint8_t swInNo = 0; swInNo < Z_swInNoMax; swInNo++){
            B_edgeOn [swInNo] = Z_edgeNasi;
            B_edgeOff[swInNo] = Z_edgeNasi;
        }

      
    // AD値から各SWの状態を推定する     
    if     (ad <= Z_threshold0_1){ans = 0;}
    else if(ad <= Z_threshold1_2){ans = 1;}
    else if(ad <= Z_threshold2_3){ans = 2;}
    else if(ad <= Z_threshold3_4){ans = 3;}
    else if(ad <= Z_threshold4_5){ans = 4;}
    else if(ad <= Z_threshold5_6){ans = 5;}
    else if(ad <= Z_threshold6_7){ans = 6;}
    else                         {ans = 7;}

   
    // 各SW毎に現在の認識値を格納
    B_kariLevel[0] = ((B_kariLevel[0] << 1) |  (ans & 0x01)      );
    B_kariLevel[1] = ((B_kariLevel[1] << 1) | ((ans & 0x02) >> 1));
    B_kariLevel[2] = ((B_kariLevel[2] << 1) | ((ans & 0x04) >> 2));

    // SWのレベルが連続していたら、SWレベルを確定値として更新
    uint8_t kakutei = 0;
        for(uint8_t swInNo = 0; swInNo < Z_swInNoMax; swInNo++){
            uint8_t work = B_kariLevel[swInNo] & Z_itchiPattern;
            if(work == 0x00){
                // SW=OFF確定
                D_oldLevel[swInNo] = D_nowLevel[swInNo];
                D_nowLevel[swInNo] = Z_levelOff;
                kakutei = 1;
            }
            else if(work == Z_itchiPattern){
                // SW=ON 確定
                D_oldLevel[swInNo] = D_nowLevel[swInNo];
                D_nowLevel[swInNo] = Z_levelOn;
                kakutei = 1;
            }
            else{
                // nothing
            }
        
            // swレベル確定値が更新されたらエッジ認識も更新する
            if(kakutei == 1){
                // edge kosin
                if((D_oldLevel[swInNo] == Z_levelOff) && (D_nowLevel[swInNo] == Z_levelOn)){
                    B_edgeOn[swInNo] = Z_edgeAri;
                }
                if((D_oldLevel[swInNo] == Z_levelOn) && (D_nowLevel[swInNo] == Z_levelOff)){
                    B_edgeOff[swInNo] = Z_edgeAri;
                }
            }
        }
        DEBUG_PRINT("%d %f %d %d %d\r\n",analogRead(_pin), ad,   D_nowLevel[0], D_nowLevel[1], D_nowLevel[2]);

} 

//===============================
// sw no Off to On edge hantei
//===============================
uint8_t analogSw::onEdge(uint8_t swNo){
    uint8_t ans = 0;
    
    if(B_edgeOn[swNo] == Z_edgeAri){
        ans = 1;
    } 

    return (ans);
}

//===============================
// sw no On to Off edge hantei
//===============================
uint8_t analogSw::offEdge(uint8_t swNo){
    uint8_t ans = 0;
    
    if(B_edgeOff[swNo] == Z_edgeAri){
        ans = 1;
    } 

    return (ans);
}

//==============================
// sw no ninsiki level hantei
//==============================
uint8_t analogSw::level(uint8_t swNo){
    uint8_t ans = 0;
    
    if(D_nowLevel[swNo] == Z_levelOn){
        ans = 1;
    } 

    return (ans);
}
