// sonar

#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

byte mac[] = {};
IPAddress ip(192, 168, 1, 101);
IPAddress target(192, 168, 1, 100); //target minnie

unsigned int port = 1234;

EthernetUDP Udp;

void setup() {
	Ethernet.begin(mac, ip); //dns & gateway default to ip with final octet 1
	Udp.begin(port);
}

void loop() {

}
