#ifndef __SIMPLE_DEVICE_H__
#define __SIMPLE_DEVICE_H__

#include <SPI.h>
#include <pins_arduino.h>

#define COMMAND_MODE_ENTER_RETRY_ATTEMPTS 5
#define COMMAND_MODE_GUARD_TIME 250
#define COMMAND_PROMPT_MESSAGE "\r\nwifly-EZX Ver 4.00"
#define COMMAND_OPEN_MESSAGE "*OPEN*"
#define COMMAND_JOINED "Associated!"

class SimpleDevice {
  public:
    SimpleDevice();

    void setUart(Stream* newUart);
    void begin();

    boolean join(const char *ssid);
    boolean join(const char *ssid, const char *passphrase, 
                 boolean isWPA = true);

    boolean sendCommand(const char *command,
                        const char *expectedResponse); // Has default value
    boolean sendCommand(const __FlashStringHelper *command,
                        const char *expectedResponse); // Has default value
    boolean sendCommand(const char *command);

    long getTime();

    const char * ip();
    
  private:
    Stream* uart;
    boolean serverConnectionActive;
    
    void ignoreRemainder();
    void switchToCommandMode();
    void reboot();

    boolean findInResponse(const char *toMatch, unsigned int timeOut);
    boolean enterCommandMode(boolean isAfterBoot = false);

    friend class SimpleWifiClient;
};

#endif
