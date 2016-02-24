#include <RF24_config.h>
#include <nRF24L01.h>
#include <SPI.h>
#include "RF24.h"
#include "printf.h"

RF24 radio(5, 8);

const uint64_t pipes[2] = { 0xABCDABCD71LL, 0x544d52687CLL };
bool TX = 1, RX = 0, role = 0;

void setup(void) 
{
	Serial.begin(9600);
	printf_begin();
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
	radio.writeFast(&buffer, length);
	radio.txStandBy();
	char endSignal[8] = "1234567";
	radio.writeFast(&endSignal, 8);
	radio.txStandBy();
	Serial.println("String Sent: " + s);
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

