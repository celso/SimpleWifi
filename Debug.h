#ifndef __DEBUG_H__
#define __DEBUG_H__

#define DEBUG_SIMPLEWIFI    0

#define SERIAL_PRINTLN(message) \
    Serial.println(message);\
    Serial.flush();  

#define SERIAL_PRINT(message) \
    Serial.print(message);\
    Serial.flush();  

#endif
