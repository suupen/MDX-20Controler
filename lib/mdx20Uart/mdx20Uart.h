#ifndef MDX20UART_H
#define MDX20UART_H

#include <Arduino.h>
//#include "limits.h"

#define FILEMEMORY (32)   // file関係で使用するbufferのbyte数  (一行毎に処理するので、一行の最大文字数を設定しておく) 

class mdx20Uart {

    public:
    mdx20Uart(uint16_t tx, uint16_t rx, uint16_t cts);
//  mdx20Uart(PinName tx, PinName rx, PinName cts);

    ~mdx20Uart();

    void clearPositon(void);
    void answerPositon(int16_t *position);
    void answerPositonMillimeter(float *position);
    void integralPosition(char *str);
    void userOriginPositionInitial(void);

    uint8_t xyOrigin(void);
    uint8_t zOrigin(void);


    uint8_t sendData(char* data);
    int32_t axisMovingCheck(char* data);

    uint8_t initial(void);
    uint8_t motorOff(void);
    uint8_t motorOn(void);


//    uint8_t userOriginInitial(void);
    uint8_t userXYOriginInitial(void);
    uint8_t userZOriginInitial(void);


    uint8_t final(void);
    uint8_t XYZMove(int16_t x, int16_t y, int16_t z);
    void offsetXAxisAdjustment(int16_t axisData);
    void offsetYAxisAdjustment(int16_t axisData);
    void offsetZAxisAdjustment(int16_t axisData);

    int32_t motorStateCheck(void);
    int32_t connectCheck(void);


private:

    uint16_t _cts;


    void translationToControlerAxisMoveDataFromRMLAxisMoveData(char *str);

 //   char B_masterTx[0xff];
    int32_t D_position[3]; //[0]:x, [1]:y, [2]:z
    int32_t D_userOriginPosition[3]; //[0]:x, [1]:y, [2]:z
    int32_t motorState; // true:ON false:OFF
    


#define countToMillimeter (0.025)
#define Z_x (0)
#define Z_y (1)
#define Z_z (2)

#define Z_xAxisMin (0)
#define Z_xAxisMax (8128)

#define Z_yAxisMin (0)
#define Z_yAxisMax (6096)

#define Z_zAxisMin (-2420)
#define Z_zAxisMax (0)

};

#endif  //  MDX20UART_H