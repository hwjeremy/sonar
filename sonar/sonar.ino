// sonar

#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

byte mac[] = {};
IPAddress localIp(192, 168, 1, 101);
IPAddress targetIp(192, 168, 1, 100); //target minnie
short port = 1234;

short waitTime = 5000; //in milliseconds

EthernetUDP Udp;

void setup() {
	Ethernet.begin(mac, localIp); //dns & gateway default to ip with final octet 1
	Udp.begin(port);
}

void loop() {
	//get sensor data
	
	//send sensor data
	Udp.beginPacket(targetIp, port);
	Udp.write("data");
	Udp.endPacket;
	
	//wait before next transmission
	delay(waitTime);
}
