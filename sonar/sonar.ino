// sonar

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
		//v = 331.4 + 0.6T m/s, T = temp in degrees celsius
		//v = 331.4 + 0.6T m/s + 0.0124*H, H = relative humidity %
	
	unsigned int pingTime = pinger.ping();
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
	
	//send sensor data
	Udp.beginPacket(targetIp, port);
	Udp.write("data");
	Udp.endPacket;
	
	//wait before next transmission
	delay(waitTime);
}
