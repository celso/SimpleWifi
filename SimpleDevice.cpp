#include "SimpleWifi.h"
#include "Debug.h"

boolean SimpleDevice::findInResponse(const char *toMatch,
                                    unsigned int timeOut = 3000) {

    unsigned long timeOutTarget;
    byte byteRead;

    #if DEBUG_SIMPLEWIFI == 1
    SERIAL_PRINTLN(F("Entered findInResponse"));
    SERIAL_PRINT(F("DEBUG: Want to match:"));
    SERIAL_PRINT(toMatch);
    SERIAL_PRINTLN(F(""));
    SERIAL_PRINT(F("Found:"));
    #endif

    timeOutTarget = millis() + timeOut;

    for (unsigned int offset = 0; offset < strlen(toMatch); offset++) {

        while (!uart->available()) {
          // Wait, with optional time out.
          if (timeOut > 0) {
            if (millis() > timeOutTarget) {
              return false;
            }
          }
          delay(1); // This seems to improve reliability slightly
        }

        // get byte from uart
        byteRead = uart->read();
        delay(1); // Removing logging may affect timing slightly

        #if DEBUG_SIMPLEWIFI == 1
        SERIAL_PRINT((char) byteRead);
        #endif

        if (byteRead != toMatch[offset]) {
          offset = 0;
          // Ignore character read if it's not a match for the start of the string
          if (byteRead != toMatch[offset]) {
            offset = -1;
          }
          continue;
        }

    } // for cycle

    // read the rest of the buffer
    ignoreRemainder();

    return(true);
}

void SimpleDevice::ignoreRemainder() {
    while (uart->available()) {
        byte byteRead=uart->read();
        #if DEBUG_SIMPlLEWIFI == 1
        SERIAL_PRINT((char)byteRead);
        #endif
        delay(1);
    }
}

boolean SimpleDevice::enterCommandMode(boolean isAfterBoot) {
  for (int retryCount = 0;
     retryCount < COMMAND_MODE_ENTER_RETRY_ATTEMPTS;
     retryCount++) {

     if (isAfterBoot) {
        delay(1000); // This delay is so characters aren't missed after a reboot.
     }

     delay(COMMAND_MODE_GUARD_TIME);
     uart->print(F("$$$"));
     delay(COMMAND_MODE_GUARD_TIME);
     uart->println();
     uart->println();
     // check if command mode worked
     uart->println(F("ver"));

     if (findInResponse(COMMAND_PROMPT_MESSAGE, 1000)) {
      // TODO: Flush or leave remainder of output?
      return true;
     }

   }
}

SimpleDevice::SimpleDevice(void) {

}

void  SimpleDevice::setUart(Stream* newUart)
{
    uart = newUart;
}

void SimpleDevice::begin() {
    enterCommandMode();
}

boolean SimpleDevice::sendCommand(const __FlashStringHelper *command,
                                 const char *expectedResponse = "AOK") {
    uart->println(command);

    if (!findInResponse(expectedResponse, 5000)) {
      return false;
    }

    return(true);
}


boolean SimpleDevice::sendCommand(const char *command) {
    #if DEBUG_SIMPLEWIFI == 1
    SERIAL_PRINTLN(command);
    #endif
    uart->println(command);
    delay(500);
    ignoreRemainder();
    return(true);
}


boolean SimpleDevice::sendCommand(const char *command,
                                 const char *expectedResponse = "AOK") {

    #if DEBUG_SIMPLEWIFI == 1
    SERIAL_PRINTLN(command);
    #endif
    uart->println(command);
    delay(500);

    if (!findInResponse(expectedResponse, 5000)) {
      return false;
    }

    return(true);
}


boolean SimpleDevice::join(const char *ssid) {
    uart->print(F("join "));
    uart->println(ssid);

    if (!findInResponse(COMMAND_JOINED, 5000)) {
      return false;
    }

    return true;
}


boolean SimpleDevice::join(const char *ssid, const char *passphrase,
                          boolean isWPA) {

    uart->print(F("set wlan "));

    if (isWPA) {
        uart->print(F("passphrase "));
    } else {
        uart->print(F("key "));
    }

    uart->println(passphrase);

    delay(3000); // this is needed
    return join(ssid);
}


// Preinstantiate required objects
SimpleDevice SimpleWifi;
