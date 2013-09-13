#include "SimpleWifi.h"
#include "Debug.h"
#include "SimpleWifiClient.h"

SimpleWifiClient::SimpleWifiClient() :
  _SimpleWifi (SimpleWifi) {
      isOpen = false;
}


size_t  SimpleWifiClient::write(uint8_t value) {
  _SimpleWifi.uart->write(value);
  return 1;
}


size_t  SimpleWifiClient::write(const uint8_t *buf, size_t size) {
  while(size--)
          _SimpleWifi.uart->write(*buf++);
  return size;
}

int SimpleWifiClient::connect(IPAddress ip, uint16_t port){
    _ip = rawIPAddress(ip);
    _port = port;
    _domain = NULL;

    return _connect();
}

int SimpleWifiClient::connect(const char *host, uint16_t port){
    _ip = NULL;
    _port = port;
    _domain = host;

    return _connect();
}

boolean SimpleWifiClient::_connect() {

    _SimpleWifi.enterCommandMode();

    _SimpleWifi.uart->print(F("open "));
    if (_ip != NULL) {
      for (int index = 0; /* break inside loop*/ ; index++) {
        _SimpleWifi.uart->print(_ip[index], DEC);
        if (index == 3) {
          break;
        }
        _SimpleWifi.uart->print('.');
      }
    } else if (_domain != NULL) {
      _SimpleWifi.uart->print(_domain);
    } else {
      isOpen = false;
      return false;
    }

    _SimpleWifi.uart->print(" ");
    _SimpleWifi.uart->println(_port, DEC);

    if (_SimpleWifi.findInResponse(COMMAND_OPEN_MESSAGE, 10000)) {
        #if DEBUG_SIMPlLEWIFI == 1
        SERIAL_PRINTLN(F("SimpleWifiClient CONNECTED"));
        #endif
        isOpen = true;
    }

     return(isOpen);
}


int SimpleWifiClient::available() {
  if (!isOpen) { // no connection
    return 0;
  }
  return(_SimpleWifi.uart->available());
}


int SimpleWifiClient::read() {

  if (!isOpen) {
    return -1;
  }

  if (!_SimpleWifi.uart->available()) {
    return -1;
  }

  return(_SimpleWifi.uart->read());
}

int SimpleWifiClient::read(uint8_t *buf, size_t size){
  return(read()); // TODO
}

int SimpleWifiClient::peek() {
  if (!isOpen) {
    return -1;
  }

  if (!_SimpleWifi.uart->available()) {
    return -1;
  }

  return(_SimpleWifi.uart->peek());
}

void SimpleWifiClient::flush(void) {
  if (!isOpen) {
    return;
  }

  _SimpleWifi.ignoreRemainder();
}

uint8_t SimpleWifiClient::connected() {
  return isOpen;
}

void SimpleWifiClient::stop() {
  _SimpleWifi.uart->flush();
  isOpen = false;
}

SimpleWifiClient::operator bool() {
  return !((_ip == NULL) && (_domain == NULL) && (_port == 0));
}
