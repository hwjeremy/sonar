// sonar
// to control an Arduino Uno based ultrasonic distance sensor
// this code is not portable

#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <NewPing.h>
#include <DHT.h>

//network
byte mac[] = {}; //set mac!
byte sensorId = 1;
byte sensorType = 1;
IPAddress localIp(192, 168, 1, 101);
IPAddress targetIp(192, 168, 1, 100); //target minnie
const short port = 1234;
EthernetUDP Udp;

//pinger
const byte pin_trigger = 1;
const byte pin_echo = 2;
const short maxDistance = 400; //in centimetres - check tank height!
NewPing pinger(pin_trigger, pin_echo, maxDistance);

//temp & humidity
const byte pin_dht = 3;
const byte dhtType = 22;
bool dhtValid = false;
DHT dht(pin_dht, dhtType);

short waitTime = 5000; //in milliseconds

void setup() {
	Ethernet.begin(mac, localIp); //dns & gateway default to ip with final octet 1
	Udp.begin(port);
}

void loop() {
	//get sensor data
	unsigned int pingTime = pinger.ping(); //in microseconds

	float h = dht.readHumidity();
	float t = dht.readTemperature();
	
	//check for a bad sensor read, set to 0 and handle server-side
	//we know that relative humidity & temp are overwhelmingly unlikely to ever = 0
	if (isnan(h)) {h = 0;}
	if (isnan(t)) {t = 0;}
	
	//prepare packet data
	//uno uses 16-bit ints & 32 bit floats
	byte packetData[12];
	
	packetData[0] = sensorType;
	packetData[1] = sensorId;
	
	packetData[2] = (pingTime >> 8) & 0xFF;
	packetData[3] = pingTime & 0xFF;
	
	union {float humiFloat; byte humiBytes[4];} humiUnion;
	humiUnion.humiFloat = h;
	packetData[4] = humiUnion.humiBytes[0];
	packetData[5] = humiUnion.humiBytes[1];
	packetData[6] = humiUnion.humiBytes[2];
	packetData[7] = humiUnion.humiBytes[3];

	union {float tempFloat; byte tempBytes[4];} tempUnion;
	tempUnion.tempFloat = t;
	packetData[8] = tempUnion.tempBytes[0];
	packetData[9] = tempUnion.tempBytes[1];
	packetData[10] = tempUnion.tempBytes[2];
	packetData[11] = tempUnion.tempBytes[3];
	
	//send the packet
	Udp.beginPacket(targetIp, port);
	Udp.write(packetData, 10);
	Udp.endPacket();
	
	//wait before next transmission
	delay(waitTime);
}