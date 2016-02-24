#include <RF24_config.h>
#include <nRF24L01.h>
#include <SPI.h>
#include "DHT.h"
#include "RF24.h"
#include "printf.h"

RF24 radio(5, 8);
DHT dht(3, DHT11);

const uint64_t pipes[2] = { 0xABCDABCD71LL, 0x544d52687CLL };
bool TX = 1, RX = 0, role = 0;
typedef enum { packetType_unknown = 0, packetType_temperature = 1, packetType_humidity = 2 } packetType_e;
typedef union
{
	float number;
	uint8_t bytes[4];
} FLOATUNION_t;
packetType_e packetType = packetType_unknown;

void setup(void)
{
	Serial.begin(9600);
	printf_begin();
	dht.begin();
	radio.begin();
	radio.setChannel(1);
	radio.setPALevel(RF24_PA_MAX);
	radio.setDataRate(RF24_1MBPS);
	radio.setAutoAck(1);
	radio.setRetries(2, 15);
	radio.setCRCLength(RF24_CRC_8);
	radio.openWritingPipe(pipes[0]);
	radio.openReadingPipe(1, pipes[1]);
	radio.enableDynamicPayloads();
	radio.startListening();
	radio.printDetails();
	printf("*** PRESS 'T' to begin transmitting to the other node\n\r");
	radio.powerUp();
}

void loop(void)
{
	//printSensorData();
	if (false)
	{
		readRequest();

		if (packetType == packetType_humidity)
		{
			float h = dht.readHumidity();
			if (!isnan(h))
			{
				sendHumidity(h);
			}
		}

		if (packetType == packetType_temperature)
		{
			float t = dht.readTemperature(true);
			if (!isnan(t))
			{
				sendTemperature(t);
			}
		}
	}

	if (role == TX)
	{
		delay(2000);
		sendString("Hello world");
	}

	if (role == RX)
	{
		readLine();
	}

	if (Serial.available())
	{
		char c = toupper(Serial.read());
		if (c == 'T' && role == RX)
		{
			printf("*** CHANGING TO TRANSMIT ROLE -- PRESS 'R' TO SWITCH BACK\n\r");
			radio.openWritingPipe(pipes[0]);
			radio.openReadingPipe(1, pipes[1]);
			radio.stopListening();
			role = TX;                  // Become the primary transmitter (ping out)
		}
		else if (c == 'R' && role == TX)
		{
			radio.openWritingPipe(pipes[0]);
			radio.openReadingPipe(1, pipes[1]);
			radio.startListening();
			printf("*** CHANGING TO RECEIVE ROLE -- PRESS 'T' TO SWITCH BACK\n\r");
			role = RX;                // Become the primary receiver (pong back)
		}
	}
}

void sendString(String s)
{
	unsigned int length = s.length();
	char buffer[length];
	s.toCharArray(buffer, length);
	radio.write(&buffer, length);
	radio.txStandBy();
	char endSignal[8] = "1234567";
	radio.writeFast(&endSignal, 8);
	radio.txStandBy();
	Serial.println("String Sent: " + s);
}

void sendTemperature(float temperature)
{
	FLOATUNION_t floatToBytes;
	floatToBytes.number = temperature;
	byte buffer[5];
	buffer[0] = packetType_temperature;
	memcpy(buffer + 1, floatToBytes.bytes, 4);
	radio.writeFast(&buffer, 5);
	radio.txStandBy();
	printf("Sent temperature: %5.2f\r\n", (double)temperature);
}

void sendHumidity(float humidity)
{
	FLOATUNION_t floatToBytes;
	floatToBytes.number = humidity;
	byte buffer[5];
	buffer[0] = packetType_humidity;
	memcpy(buffer + 1, floatToBytes.bytes, 4);
	radio.writeFast(&buffer, 5);
	radio.txStandBy();
	printf("Sent humidity: %5.2f\r\n", (double)humidity);
}

void readRequest()
{
	while (radio.available())
	{
		byte payloadSize = radio.getDynamicPayloadSize();
		byte buffer[payloadSize + 1];
		buffer[payloadSize] = 0;
		radio.read(&buffer, payloadSize);
		packetType = (packetType_e)buffer[0];
		break;
	}
}

void readLine()
{
	while (radio.available())
	{
		byte payloadSize = radio.getDynamicPayloadSize();
		char buffer[payloadSize + 1];
		buffer[payloadSize] = 0;
		radio.read(&buffer, payloadSize);
		Serial.println(buffer);
	}
}


void printSensorData() {
	// Wait a few seconds between measurements.
	delay(2000);

	// Reading temperature or humidity takes about 250 milliseconds!
	// Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
	float h = dht.readHumidity();
	// Read temperature as Celsius (the default)
	float t = dht.readTemperature();
	// Read temperature as Fahrenheit (isFahrenheit = true)
	float f = dht.readTemperature(true);

	// Check if any reads failed and exit early (to try again).
	if (isnan(h) || isnan(t) || isnan(f)) {
		Serial.println("Failed to read from DHT sensor!");
		return;
	}

	// Compute heat index in Fahrenheit (the default)
	float hif = dht.computeHeatIndex(f, h);
	// Compute heat index in Celsius (isFahreheit = false)
	float hic = dht.computeHeatIndex(t, h, false);

	Serial.print("Humidity: ");
	Serial.print(h);
	Serial.print(" %\t");
	Serial.print("Temperature: ");
	Serial.print(t);
	Serial.print(" *C ");
	Serial.print(f);
	Serial.print(" *F\t");
	Serial.print("Heat index: ");
	Serial.print(hic);
	Serial.print(" *C ");
	Serial.print(hif);
	Serial.println(" *F");
}
