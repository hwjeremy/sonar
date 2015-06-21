// sonar - local testing version
// to control an Arduino Uno based ultrasonic distance sensor

#include <SPI.h>
#include <Ethernet.h> //still include for memory footprint simulation
#include <EthernetUdp.h>
#include <NewPing.h>
#include <DHT.h>
#include <ArduinoJson.h>

//network - not functional in test, set for memory footprint
byte mac[] = {0x32, 0x81, 0x7F, 0x25, 0x6a, 0x7d}; //set mac!
const byte sensorId = 1;
const byte sensorType = 1;
IPAddress localIpFallback(192, 168, 1, 101);
IPAddress targetIp(192, 168, 1, 100); //target minnie
const short port = 1234;
EthernetUDP Udp;

//pinger
const byte pin_trigger = 7;
const byte pin_echo = 6;
const int maxDistance = 200; //in centimetres - check tank height!
unsigned int pingTime = 0;
NewPing pinger(pin_trigger, pin_echo, maxDistance);

//temp & humidity
const byte pin_dht = 2;
const byte dhtType = 22;
float t = 0;
float h = 0;
DHT dht(pin_dht, dhtType);

//watchdog
const byte pin_watch = 4;

//indicator led (blink after each transmission)
const byte pin_led = 13;
const short ledInterval = 1000; //in milliseconds. Must be lower than transmissionInterval
bool ledState = LOW;

//transmission interval
const short transmissionInterval = 5000; //in milliseconds. Must be higher than ledInterval
unsigned long lastTransmissionTime = 0;

//web server
EthernetServer server(80);

void setup() {
	//attempt to get an ip address via DHCP
	if (Ethernet.begin(mac) == 0)
	{
		//fall back to default ip
		Ethernet.begin(mac, localIpFallback); //dns & gateway default to ip with final octet 1
	}
	Udp.begin(port);
	Serial.begin(115200);
	pinMode(pin_led, OUTPUT);
	server.begin();
}

void loop() {
	if (millis() - lastTransmissionTime >= transmissionInterval)
	{
		//reset watchdog timer
		digitalWrite(pin_watch, HIGH);
		delay(20);
		digitalWrite(pin_watch, LOW);
	
		//get sensor data
		pingTime = pinger.ping(); //in microseconds
		h = dht.readHumidity();
		t = dht.readTemperature();
	
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
		//Udp.beginPacket(targetIp, port);
		//Udp.write(data);
		//Udp.endPacket();
		
		//send to serial instead for local testing
		Serial.println("\n-- Begin cycle --");
		Serial.print("Ping time: ");
		Serial.print(pingTime);
		Serial.println(" microseconds");
		Serial.print("Humidity:");
		Serial.println(h);
		Serial.print("Temperature:");
		Serial.print(t);
		
		//cache transmission time and turn on indicator LED
		lastTransmissionTime = millis();
		ledState = HIGH;
		digitalWrite(pin_led, ledState);
	}
	
	if (ledState == HIGH && millis() - lastTransmissionTime >= ledInterval)
	{
		ledState = LOW;
		digitalWrite(pin_led, ledState);
	}
	
	EthernetClient client = server.available();
	if (client){
		bool lineBlank = true;
		while(client.connected())
		{
			if (client.available())
			{
				char c = client.read();
				if (c == '\n' && lineBlank)
				{
					client.println("HTTP/1.1 200 OK");
					client.println("Content-Type: text/html");
					client.println("Connection: close");
					client.println("Refresh: 5");
					client.println("<!doctype html");
					client.println("<html>");
					client.println("<strong>Thornleigh Sensor Unit</strong></br>");
					client.print("Sensor type: ");
					client.print(sensorType);
					client.println("</br");
					client.print("Sensor id: ");
					client.print(sensorId);
					client.println("</br>");
					client.print("Time since last UDP transmission: ");
					client.print(millis() - lastTransmissionTime);
					client.println(" milliseconds</br>");
					client.print("Ultrasonic ping time: ");
					client.print(pingTime);
					client.println(" microseconds</br>");
					client.print("Temperature: ");
					client.print(t);
					client.println("</br>");
					client.print("Humidity: ");
					client.print(h);
					client.println("</br></html>");
					break;
				}
				if (c == '\n') {
					lineBlank = true;
				}			
				else if (c != '\r'){
					lineBlank = false;
				}
			}
		}
		delay(1);
		client.stop();
	}
}
