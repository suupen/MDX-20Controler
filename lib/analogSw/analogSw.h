#ifndef ANALOGSW_H
#define ANALOGSW_H

// 181216 ESP32ではAD変換精度が悪く使用できない
// 参考HP
// https://ht-deko.com/arduino/esp-wroom-32.html#13_01

/** アナログポートで３つのSW認識する library
 * 
 * swはIO端子とGND間に接続する
 *　main関数でrefresh()関数を呼ぶ。この関数で、SWレベルの認識・更新を行い,edge認識を行う. edgeデータはこの関数でクリアされる
 * onEdge(), offEdge()の戻り値が認識値になる
 */

/** *******************************************************************
*  SwAnalogInput Library example program
*   mbed no analog port de 3hon no sw wo ninsiki suru.
*   6hon no analog port (p15 - p20) de 6 * 3 = 18 ko no sw ninsiki ga dekiru.
* 
*  <schematic>
*    -.- mbed VOUT(+3.3[V])
*     |                                               |--------------------> mbed p20(ADinput)
*     |   ---------       ---------       ---------   |   ---------
*     .---| Rsw2  |---.---| Rsw1  |---.---| Rsw0  |---.---| Rout  |----|
*     |   ---------   |   ---------   |   ---------   |   ---------    |
*     |     ----      |     -----     |     -----     |                |
*     |-----o  o------.-----o  o------.-----o  o------|              -----
*            SW2            SW1              SW0                      mbed GND(0[V])
*  
*   Rsw2 : 8.2[kohm], Rsw1 = 3.9[kohm], Rsw0 = 2.0[kohm], Rout = 1.0[kohm] (R no seido ha +-1[%])
* 
*  *********************************************************************
*/
#include <Arduino.h>


class analogSw {
    public:
    analogSw(uint16_t pin);

    void refresh();
    /** Check Off to On edge
    *
    * @param uint8_t swNo     : 0:sw0, 1:sw1, ... ,17:sw17     
    * @param return uint8_t  On edge check  0: edge Nasi  1: edge Ari
    *
    */
    uint8_t onEdge(uint8_t swNo);

    /** Check On to Off edge
    *
    * @param uint8_t swNo     : 0:sw0, 1:sw1, ... ,17:sw17   
    * @param return uint8_t Off edge check   0 : Nasi   1 : Ari
    *
    */
    uint8_t offEdge(uint8_t swNo);
    
    /** Check sw Level
    *
    * @param uint8_t swNo     : 0:sw0, 1:sw1, ... ,17:sw17   
    * @param return uint8_t sw level check   0 : Off   1 : On
    *
    */    
    uint8_t level(uint8_t swNo);
    

    

private:
    uint16_t _pin;

   #define Z_swInNoMax    (3)  // 1pin atari no sw setuzoku suu (1pin ni 3ko no sw setuzoku)

uint8_t B_kariLevel[Z_swInNoMax]; // kakutei mae no ninsiki Level 0bit:saisin(t) 1bit:t-1, ... ,7bit:t-7  0:Off  1:On
    // match number define
    //#define Z_itchiPattern     (0x03)   // 2kai itch
    #define Z_itchiPattern    (0x07)  // 3kai itchi
    //#define Z_itchiPattern    (0x0f)  // 4kai itchi
    //#define Z_itchiPattern    (0x1f)  // 5kai itchi
    //#define Z_itchiPattern    (0x3f)  // 6kai itchi
    //#define Z_itchiPattern    (0x7f)  // 7kai itchi
    //#define Z_itchiPattern    (0xff)  // 8kai itchi
   
   // sw level data
   uint8_t D_nowLevel[Z_swInNoMax];  // saisin no kakutei Level 0:Off  1:On
   uint8_t D_oldLevel[Z_swInNoMax];  // zenkai no kakutei Level 0:Off  1:On
    #define Z_levelOff (0)
    #define Z_levelOn  (1)
   
   // sw edge data
   // swDigital.c naibu hensu 
   uint8_t B_edgeOn[Z_swInNoMax];    // off kara on  no ninsiki(on edge)  0:Nasi  1:Ari
   uint8_t B_edgeOff[Z_swInNoMax];   // on  kara off no ninsiki(off edge) 0:Nasi  1:Ari
    #define Z_edgeNasi (0)
    #define Z_edgeAri  (1)

    //------------------
    // Resistor network
    //------------------
    
    //   -.- mbed VOUT(+3.3[V])
    //    |                                               |--------------------> mbed p15 - p20(analog port)
    //    |   ---------       ---------       ---------   |   ---------
    //    .---| Rsw2  |---.---| Rsw1  |---.---| Rsw0  |---.---| Rout  |----|
    //    |   ---------   |   ---------   |   ---------   |   ---------    |
    //    |     ----      |     -----     |     -----     |                |
    //    |-----o  o------.-----o  o------.-----o  o------|              -----
    //           SW2            SW1              SW0                      mbed GND(0[V])
    // 
    //    |                                                                |
    //    |<----------------------- Rall --------------------------------->|    
    //    |                          |                                     |
    //                               ----> Z_R0 to Z_R7
    
    
    #define Z_Rsw2 (8200.0F)   // SW2 no R (1/1 [ohm]/count)
    #define Z_Rsw1 (3900.0F)   // SW1 no R (1/1 [ohm]/count)
    #define Z_Rsw0 (2000.0F)   // adinput1 no R (1/1 [ohm]/count)
    #define Z_Rout (1000.0F)   // Vout no R (1/1 [ohm]/count)
    //Z_Rsw2,Z_Rsw1,Z_Rsw0,Z_Rout niwa +-1[%]no seido no teiko wo tukau koto
    
    #define Z_gosaMax (1.020F) // Z_Rout(max) / Z_Rx(min) = (Z_Rout * 1.01) / (Z_Rx * 0.99) = (Z_Rout / Z_Rx) * 1.020
    #define Z_gosaMin (0.990F) // Z_Rout(min) / Z_Rx(max) = (Z_Rout * 0.99) / (Z_Rx * 1.01) = (Z_Rout / Z_Rx) * 0.980
    
    // Rall keisanchi
                                                           // SW2  SW1  SW0
    #define Z_R0 ((Z_Rsw2 + Z_Rsw1 + Z_Rsw0 + Z_Rout))     // OFF  OFF  OFF
    #define Z_R1 ((Z_Rsw2 + Z_Rsw1 + 0      + Z_Rout))     // OFF  OFF   ON
    #define Z_R2 ((Z_Rsw2 + 0      + Z_Rsw0 + Z_Rout))     // OFF   ON  OFF 
    #define Z_R3 ((Z_Rsw2 + 0      + 0      + Z_Rout))     // OFF   ON   ON
    #define Z_R4 ((0      + Z_Rsw1 + Z_Rsw0 + Z_Rout))     //  ON  OFF  OFF
    #define Z_R5 ((0      + Z_Rsw1 + 0      + Z_Rout))     //  ON  OFF   ON
    #define Z_R6 ((0      + 0      + Z_Rsw0 + Z_Rout))     //  ON   ON  OFF
    #define Z_R7 ((0      + 0      + 0      + Z_Rout))     //  ON   ON   ON    
    
    // Rout : Rall (max , min)
    #define Z_R0max  (((Z_Rout * Z_gosaMax) / Z_R0))
    #define Z_R0min  (((Z_Rout * Z_gosaMin) / Z_R0))
    #define Z_R1max  (((Z_Rout * Z_gosaMax) / Z_R1))
    #define Z_R1min  (((Z_Rout * Z_gosaMin) / Z_R1))
    #define Z_R2max  (((Z_Rout * Z_gosaMax) / Z_R2))
    #define Z_R2min  (((Z_Rout * Z_gosaMin) / Z_R2))    
    #define Z_R3max  (((Z_Rout * Z_gosaMax) / Z_R3))
    #define Z_R3min  (((Z_Rout * Z_gosaMin) / Z_R3))    
    #define Z_R4max  (((Z_Rout * Z_gosaMax) / Z_R4))
    #define Z_R4min  (((Z_Rout * Z_gosaMin) / Z_R4))
    #define Z_R5max  (((Z_Rout * Z_gosaMax) / Z_R5))
    #define Z_R5min  (((Z_Rout * Z_gosaMin) / Z_R5))    
    #define Z_R6max  (((Z_Rout * Z_gosaMax) / Z_R6))
    #define Z_R6min  (((Z_Rout * Z_gosaMin) / Z_R6))   
    #define Z_R7max  (((Z_Rout * Z_gosaMax) / Z_R7))
    #define Z_R7min  (((Z_Rout * Z_gosaMin) / Z_R7))    
    
    // threshold lvevel
    //  GND(0[V]) -> 0_1 -> 1_2 -> 2_3 -> 3_4 -> 4_5 -> 5_6 -> 6_7 -> Vcc(3.3[V]) 
    //  --------------+------+------+------+------+------+------+---------------
    //   SW0 |    OFF     ON    OFF     ON    OFF    ON     OFF     ON     
    //   SW1 |    OFF    OFF     ON     ON    OFF   OFF      ON     ON
    //   SW2 |    OFF    OFF    OFF    OFF     ON    ON      ON     ON 
    
                                                        // 値           Vcc=3.3vの時の電圧値
    #define Z_threshold0_1  (((Z_R0max + Z_R1min) / 2)) // 0.071561
    #define Z_threshold1_2  (((Z_R1max + Z_R2min) / 2)) // 0.083128
    #define Z_threshold2_3  (((Z_R2max + Z_R3min) / 2)) // 0.099340   
    #define Z_threshold3_4  (((Z_R3max + Z_R4min) / 2)) // 0.127174      
    #define Z_threshold4_5  (((Z_R4max + Z_R5min) / 2)) // 0.174933 
    #define Z_threshold5_6  (((Z_R5max + Z_R6min) / 2)) // 0.269082  
    #define Z_threshold6_7  (((Z_R6max + Z_R7min) / 2)) // 0.665000          

};



#endif // ANALOGSW_H
