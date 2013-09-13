SimpleWifi
==========

SimpleWifi uses the Arduino Ethernet Client api and it provides both Ethernet compatibility methods for the WiFly Shield and the RN-XV WiFly Module both available from SparkFun Electronics and a full MQTT v3 client.

SimpleWifi is an all in one, totally refactored, minified and simplified version of two libs for the Arduino:

1. Arduino Client for MQTT by Nick O’Leary http://knolleary.net/arduino-client-for-mqtt/
2. WiFly Sparkfun library https://github.com/dpslwk/WiFly

I had troubles with both libraries working with my setup. Namely: they're very unstable, had some bugs, memory footprint was big and flash size too. So I decided to take matters into my own hands and rewrote the whole thing into one single library.

For now the Client class expects version 4.00 of the wiFly firmware but you may adapt it to other versions. Look for the COMMAND_PROMPT_MESSAGE definition.

Communication with the WiFly module is serial only. Make sure to chose hardware RX/TX pins, I had troubles with SoftSerial.

I've tested this library with the Arduino Leonardo and the Arduino Mega (both versions).

Also, be warned: this is an unfinished project. Expect improvements and changes during the next weeks.

I don't have examples/ yet too. Soon.

More documentation to come.
