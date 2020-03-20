#ifndef MDX20DISPLAY_H
#define MDX20DISPLAY_H

#include <Arduino.h>

/*
arduino IDEでのプログラム例（このページからST7032のlibraryを取得
https://qiita.com/T-YOSH/items/be0be9d66da85fb6a001

PIO Homeで使うlibrary dilactryは下記。このディレクトリにダウンロードしたファイルをフォルダ毎置く
PIO のMDX-20controlerプロジェクトの下にある"lib"フォルダに置く
*/
#include "Wire.h"
#include "ST7032.h"


void mdx20DisplaySetup(void);
void lcdDisplayInitial(void);
void lcdMagnification(uint16_t  axisMagnification);
void lcdPositionMillimeter(float *millimeter);
void lcdSendDataPositionMillimeter(float *millimeter);
void lcdFileName(char *fileName);
void lcdFileColumnNum(char d, uint32_t number);
void lcdFileNumber(uint32_t fileNumberNow, uint32_t fileNumberMax);
void lcdPositionOverError(void);
void lcdPositionKakunin(void);




void lcdTest(void);

#endif // MDX20DISPLAY_H