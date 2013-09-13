// This class implements the MQTT protocol, lightweight broker-based publish/subscribe messaging framework.
// MQTT specs at http://public.dhe.ibm.com/software/dw/webservices/ws-mqtt/mqtt-v3r1.html

#include "MQTT.h"
#include "Debug.h"
#include <string.h>

MQTTClient::MQTTClient() {
   this->_client = NULL;
}

MQTTClient::MQTTClient(uint8_t *ip, uint16_t port, void (*callback)(unsigned int, unsigned int), Client& client) {
   this->_client = &client;
   this->callback = callback;
   this->ip = ip;
   this->port = port;
   this->domain = NULL;
}

MQTTClient::MQTTClient(char* domain, uint16_t port, void (*callback)(unsigned int, unsigned int), Client& client) {
   this->_client = &client;
   this->callback = callback;
   this->domain = domain;
   this->port = port;
}

boolean MQTTClient::connect(char *id) {
   return connect(id,NULL,NULL,0,0,0,0);
}

boolean MQTTClient::connect(char *id, char *user, char *pass) {
   return connect(id,user,pass,0,0,0,0);
}

boolean MQTTClient::connect(char *id, char* willTopic, uint8_t willQos, uint8_t willRetain, char* willMessage)
{
   return connect(id,NULL,NULL,willTopic,willQos,willRetain,willMessage);
}

boolean MQTTClient::connect(char *id, char *user, char *pass, char* willTopic, uint8_t willQos, uint8_t willRetain, char* willMessage) {
   if (!connected()) {
        int result = 0;

        // ask the underlying Client framework to connect to the MQTT server first      
        if (domain != NULL) {
            result = _client->connect(this->domain, this->port);
        } else {
            result = _client->connect(this->ip, this->port);
        }
      
        if (result) {
            // read http://public.dhe.ibm.com/software/dw/webservices/ws-mqtt/mqtt-v3r1.html#connect

            int plength = 12;

            uint8_t var_header[14] = {
                            0x10, // fixed header byte 1 CONNECT
                            0x00, // fixed header byte 2 (remaining lenght)
                            0x00, // Contains the Message Type and Flags (DUP, QoS level, and RETAIN) fields.
                            0x06, // (At least one byte) contains the Remaining Length field.
                            'M',
                            'Q',
                            'I',
                            's',
                            'd',
                            'p',
                            MQTTPROTOCOLVERSION,
                            0, // for connect flags
                            0, // keep alive timer MSB
                            0  // keep alive timer LSB
                        };

             uint8_t v;

             // Connect flags
             if (willTopic) {
                v = 0x06|(willQos<<3)|(willRetain<<5);
                plength += strlen(willTopic) + strlen(willMessage) + 4;
             } else {
                v = 0x02;
             }

             if(user != NULL) {
                v = v|0x80;
                plength += strlen(user) + 2;
                if(pass != NULL) {
                   v = v|(0x80>>1);
                    plength += strlen(pass) + 2;
                }
             }

             var_header[11] = v;

             // keep alive
             var_header[12] = ((MQTT_KEEPALIVE) >> 8);
             var_header[13] = ((MQTT_KEEPALIVE) & 0xFF);

             // Set the remaining lenght, the length of the variable header (12 bytes) plus the length of the Payload.
             plength += strlen(id) + 2;
             var_header[1] = plength;

            _client->write(var_header,14);

            #if DEBUG_SIMPLEWIFI == 1
            SERIAL_PRINT(F("MQQT Connect plength: "));
            SERIAL_PRINTLN(plength);
            #endif

            // Now send the PAYLOAD...

            // The first UTF-encoded string. The Client Identifier (Client ID) is between 1 and 23 characters long, and uniquely identifies the client to the server. It must be unique across all clients connecting to a single server, and is the key in handling Message IDs messages with QoS levels 1 and 2. If the Client ID contains more than 23 characters, the server responds to the CONNECT message with a CONNACK return code 2: Identifier Rejected.
            sendString(id);

            if (willTopic) {
                // If the Will Flag is set, this is the next UTF-8 encoded string. The Will Message is published to the Will Topic. The QoS level is defined by the Will QoS field, and the RETAIN status is defined by the Will RETAIN flag in the variable header.
                sendString(willTopic);
                // If the Will Flag is set, this is the next UTF-8 encoded string. The Will Message defines the content of the message that is published to the Will Topic if the client is unexpectedly disconnected. This may be a zero-length message.
                sendString(willMessage);
            }

            if(user != NULL) {
               sendString(user);
               if(pass != NULL) {
                sendString(pass);
                }
            }

            lastInActivity = lastOutActivity = millis();

            // If we don't get an answer from the server before the KEEPALIVE period, then something went wrong
            while (!_client->available()) {
                unsigned long t = millis();
                if (t-lastInActivity > MQTT_KEEPALIVE*1000UL) {
                    _client->stop();
                    return false;
                }
            }

            // Get the response and check it
            for(int i=0;i<4;i++) {
                uint8_t c = readByte();
                if(i==3 && c == 0) {
                    lastInActivity = millis();
                    pingOutstanding = false;
                    return(true); // SUCCESS
                }
            }

        }

      _client->stop();
   }
   return false;
}

uint8_t MQTTClient::readByte() {
   unsigned long t = millis()+MQTT_READBYTE_TIMEOUT;

   while(!_client->available()) {
      if (millis() > t) return 0;
   }
   return _client->read();
}

/*
uint16_t MQTTClient::setPacketLength(unsigned int plength) {
    uint8_t digit;
    unsigned int  len;

    lbuf_size = 0;
    len = plength;
    do{
        digit = len % 128;
        len = len >> 7;
        // if there are more digits to encode, set the top bit of this digit
        if ( len > 0 ) {
            // more digits? set the MSB
            digit = digit | 0x80;
        }
        lbuf[lbuf_size]=digit;
        lbuf_size++;
    } while ( len > 0 );
    return len;
}
*/

uint16_t MQTTClient::getPacketLength(uint8_t *type, unsigned int *plength) {
    uint8_t multiplier = 1;
    uint16_t value = 0;
    uint8_t digit = 0;
    uint8_t nbytes = 1;

    digit = readByte();
    *type = digit & 0xF0;

    do {
        digit = readByte();
        nbytes++;
        value += (digit & 127) * multiplier; 
        multiplier *= 128;
    } while (
        (digit & 128) != 0
    );

    *plength = value;

    return nbytes;
}


uint16_t MQTTClient::readPacket(uint8_t* lengthLength) {
   uint16_t len = 0;
   return len;
}

boolean MQTTClient::loop() {
   if (connected()) {

        // Implements PINGREQ
        // http://public.dhe.ibm.com/software/dw/webservices/ws-mqtt/mqtt-v3r1.html#pingreq
        unsigned long t = millis();
        if ((t - lastInActivity > MQTT_KEEPALIVE*1000UL) || (t - lastOutActivity > MQTT_KEEPALIVE*1000UL)) {
            if (pingOutstanding) {
                _client->stop();
                return false;
            }
            else
            {
                lbuf[0] = MQTTPINGREQ;
                lbuf[1] = 0;
                _client->write(lbuf,2);
                lastOutActivity = t;
                lastInActivity = t;
                pingOutstanding = true;
            }
        }

        if (_client->available()) {
            // We have a new packet, let's see how big it is
            uint8_t type = 0;
            unsigned int plength = 0;
            uint16_t nbytes = getPacketLength(&type,&plength);
            if (nbytes > 0) {
                lastInActivity = t;

                #if DEBUG_SIMPLEWIFI == 1
                SERIAL_PRINT(F("MQTT bytes received: "));
                SERIAL_PRINTLN(nbytes);
                SERIAL_PRINT(F("MQTT packet length: "));
                SERIAL_PRINTLN(plength);
                SERIAL_PRINT(F("MQTT packet type: "));
                SERIAL_PRINTLN(type);
                #endif

                if (type == MQTTPINGREQ) { // servers wants a PING response
                    lbuf[0] = MQTTPINGRESP;
                    lbuf[1] = 0;
                    _client->write(lbuf,2);

                } else if (type == MQTTPINGRESP) { // servers sends a PING ack
                    pingOutstanding = false;

                } else if (type == MQTTPUBLISH) { // got a message from one topic, process
                    uint16_t c;

                    lbuf[0]=readByte();
                    lbuf[1]=readByte();
                    c=(lbuf[0] << 8) + (lbuf[1] % 256);

                   // callback should handle the rest of the payload
                   if (callback) {
                      callback(c, plength - c - 2);
                   }


                } else { // read the buffer
                    for(unsigned int i=0;i<plength;i++) {
                        readByte();
                    }
                }

            }

        }


   }
   return false;
}

boolean MQTTClient::publish(char* topic, char* payload) {
   return publish(topic,(uint8_t*)payload,strlen(payload),false);
}

boolean MQTTClient::publish(char* topic, uint8_t* payload, unsigned int plength) {
   return publish(topic, payload, plength, false);
}

boolean MQTTClient::publish(char* topic, uint8_t* payload, unsigned int plength, boolean retained) {
   if (connected()) {
        // fixed header
        lbuf[0]=MQTTPUBLISH;
        if ( retained ) {
            lbuf[0]|=1;
        }
        lbuf[1]=strlen(topic) + 2 + plength;
        _client->write(lbuf,2);

        sendString(topic);
        _client->write(payload,plength);

        lastOutActivity = millis();
        return true;
   }
   return false;
}

uint16_t MQTTClient::sendString(char* str) {
   uint16_t len = strlen(str);

   lbuf[0] = len >> 8; // msb
   lbuf[1] = len % 256; // lsb

   _client->write(lbuf,2);

   _client->write((uint8_t *)str,len);

   return (len + 2);
}

uint16_t MQTTClient::getNewNextMsgID() {
    nextMsgId++;
    if (nextMsgId == 0) {
        nextMsgId = 1;
    }
    return(nextMsgId);
}

boolean MQTTClient::subscribe(char* topic) {
    // http://public.dhe.ibm.com/software/dw/webservices/ws-mqtt/mqtt-v3r1.html#subscribe
   if (connected()) {
        // fixed header
        lbuf[0]=MQTTSUBSCRIBE|MQTTQOS1;
        lbuf[1]=strlen(topic) + 5;
        _client->write(lbuf,2);
        // var header
        getNewNextMsgID();
        lbuf[0] = (nextMsgId >> 8);
        lbuf[1] = (nextMsgId & 0xFF);
        _client->write(lbuf,2);
        // topic
        sendString(topic);
        // qos
        lbuf[0]=0; // QoS 0
        _client->write(lbuf,1);

        return(true);
   }
   return false;
}

boolean MQTTClient::unsubscribe(char* topic) {
   if (connected()) {

// CRY BABY

   }
   return false;
}

void MQTTClient::disconnect() {
//   _client->write(buffer,2);
   _client->stop();
   lastInActivity = lastOutActivity = millis();
}


boolean MQTTClient::connected() {
   boolean rc;
   if (_client == NULL ) {
      rc = false;
   } else {
      rc = (int)_client->connected();
      if (!rc) _client->stop();
   }
   return rc;
}
