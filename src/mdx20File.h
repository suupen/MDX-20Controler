#ifndef MDX20FILE_H
#define MDX20FILE_H

#include <Arduino.h>


#include "FS.h"     // C:\Users\suupen\.platformio\packages\framework-arduinoespressif32\libraries\FS\src
#include "SD.h"
#include "SPI.h"

#include "string.h"




int32_t fileDetect(uint32_t fileNumber, char *fileName, uint32_t *fileNumberMax);
int32_t fileSelect(char *fileName, uint32_t *numberLine);

int32_t fileOpen(char *fileName);
int32_t fileClose(void);
int32_t fileLineRead(char *data, uint32_t dataNumber, uint32_t *numberLine);

/**
 * sample program
 */
#if(0)
void setup() {
    Serial.begin(115200);

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
}

void loop() {
  // put your main code here, to run repeatedly:
}
#endif
 





#endif // MDX20FILE_H