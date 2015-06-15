// sonar
// to control an Arduino Uno based ultrasonic distance sensor

#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <NewPing.h>
#include <DHT.h>
#include <ArduinoJson.h>

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
	
	//prepare packet data using json
	StaticJsonBuffer<100> jsonBuffer;
	JsonObject& data = jsonBuffer.createObject();
	data["1"] = sensorType;
	data["2"] = sensorId;
	data["3"] = pingTime;
	data["4"].set(h, 4);
	data["5"].set(t, 4);

	//send the packet
	Udp.beginPacket(targetIp, port);
	data.printTo(Udp);
	Udp.println();
	Udp.endPacket();
	
	//wait before next transmission
	delay(waitTime);
}
