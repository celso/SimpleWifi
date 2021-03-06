#ifndef MQTT_h
#define MQTT_h

#include <Arduino.h>
#include "Client.h"

// MQTT_KEEPALIVE : keepAlive interval in Seconds
#define MQTT_KEEPALIVE 15
#define MQTT_READBYTE_TIMEOUT 3000UL // miliseconds

#define MQTTPROTOCOLVERSION 3
#define MQTTCONNECT     1 << 4  // Client request to connect to Server
#define MQTTCONNACK     2 << 4  // Connect Acknowledgment
#define MQTTPUBLISH     3 << 4  // Publish message
#define MQTTPUBACK      4 << 4  // Publish Acknowledgment
#define MQTTPUBREC      5 << 4  // Publish Received (assured delivery part 1)
#define MQTTPUBREL      6 << 4  // Publish Release (assured delivery part 2)
#define MQTTPUBCOMP     7 << 4  // Publish Complete (assured delivery part 3)
#define MQTTSUBSCRIBE   8 << 4  // Client Subscribe request
#define MQTTSUBACK      9 << 4  // Subscribe Acknowledgment
#define MQTTUNSUBSCRIBE 10 << 4 // Client Unsubscribe request
#define MQTTUNSUBACK    11 << 4 // Unsubscribe Acknowledgment
#define MQTTPINGREQ     12 << 4 // PING Request
#define MQTTPINGRESP    13 << 4 // PING Response
#define MQTTDISCONNECT  14 << 4 // Client is Disconnecting
#define MQTTReserved    15 << 4 // Reserved

#define MQTTQOS0        (0 << 1)
#define MQTTQOS1        (1 << 1)
#define MQTTQOS2        (2 << 1)

class MQTTClient {
private:
   Client* _client;

   uint16_t nextMsgId;
   unsigned long lastOutActivity;
   unsigned long lastInActivity;
   bool pingOutstanding;
   void (*callback)(unsigned int, unsigned int);
   uint16_t readPacket(uint8_t*);
/*   uint16_t setPacketLength(unsigned int plength); */
   uint16_t getPacketLength(uint8_t *type, unsigned int *plength);
   uint16_t sendString(char* str);
   uint16_t getNewNextMsgID();
   uint8_t *ip;
   char* domain;
   uint16_t port;
   uint8_t lbuf[4];
   uint8_t lbuf_size;
public:
   MQTTClient();
   MQTTClient(uint8_t *, uint16_t, void(*)(unsigned int, unsigned int),Client& client);
   MQTTClient(char*, uint16_t, void(*)(unsigned int, unsigned int),Client& client);
   boolean connect(char *);
   boolean connect(char *, char *, char *);
   boolean connect(char *, char *, uint8_t, uint8_t, char *);
   boolean connect(char *, char *, char *, char *, uint8_t, uint8_t, char*);
   void disconnect();
   uint8_t readByte();
   boolean publish(char *, char *);
   boolean publish(char *, uint8_t *, unsigned int);
   boolean publish(char *, uint8_t *, unsigned int, boolean);
   boolean subscribe(char *);
   boolean unsubscribe(char *);
   boolean loop();
   boolean connected();
};


#endif
