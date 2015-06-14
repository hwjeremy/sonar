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
		//consider sending several pings a-la NewPing.h ping_median
	
	float h = dht.readHumidity();
	float t = dht.readTemperature();
	
	if (isnan(h) || isnan(t))
	{
		dhtValid = false;
	}
	else
	{
		dhtValid = true;
	}
	
	//prepare packet data
	//uno stores 16-bit ints & 32 bit floats
	byte packetData[10];
	
	packetData[0] = (pingTime >> 8) & 0xFF;
	packetData[1] = pingTime & 0xFF;
	
	packetData[2] = (h >> 24) & 0xFF;
	packetData[3] = (h >> 16) & 0xFF;
	packetData[4] = (h >> 8) & 0xFF;
	packetData[5] = h & 0xFF;
	
	packetData[6] = (t >> 24) & 0xFF;
	packetData[7] = (t >> 16) & 0xFF;
	packetData[8] = (t >> 8) & 0xFF;
	packetData[9] = t & 0xFF;
	
	//send sensor data
	Udp.beginPacket(targetIp, port);
	Udp.write(packetData, 10);
	Udp.endPacket;
	
	//wait before next transmission
	delay(waitTime);
}
