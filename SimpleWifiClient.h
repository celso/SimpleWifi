#ifndef __SIMPLEWIFI_CLIENT_H__
#define __SIMPLEWIFI_CLIENT_H__

#include "Client.h"
#include "IPAddress.h"

#include "SimpleDevice.h"

class SimpleWifiClient : public Client {
 public:
  SimpleWifiClient(void);

  virtual int connect(IPAddress ip, uint16_t port);
  virtual int connect(const char *host, uint16_t port);
  virtual size_t write(uint8_t);
  virtual size_t write(const uint8_t *buf, size_t size);
  virtual int available();
  virtual int read();
  virtual int read(uint8_t *buf, size_t size);
  virtual int peek(); 
  virtual void flush();
  virtual void stop();
  virtual uint8_t connected();
  virtual operator bool();

  uint8_t *_ip;
  uint16_t _port;
  const char *_domain;

private:
  SimpleDevice& _SimpleWifi;
  boolean _connect();
  bool isOpen;
};

#endif
