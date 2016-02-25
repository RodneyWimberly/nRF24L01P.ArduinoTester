/* Arduino Interface to the PSC05 X10 Receiver.                       BroHogan 3/24/09
* SETUP: X10 PSC05/TW523 RJ11 to Arduino (timing for 60Hz)
* - RJ11 pin 1 (BLK) -> Pin 2 (Interrupt 0) = Zero Crossing
* - RJ11 pin 2 (RED) -> GND
* - RJ11 pin 3 (GRN) -> Pin 4 = Arduino receive
* - RJ11 pin 4 (YEL) -> Pin 5 = Arduino transmit (via X10 Lib)
* NOTES:
* - Must detach interrup when transmitting with X10 Lib
*/
#include <RF24_config.h>
#include <nRF24L01.h>
#include <SPI.h>
#include "RF24.h"
#include "printf.h"
#include "Arduino.h"                  // this is needed to compile with Rel. 0013
#include "x10.h"                       // X10 lib is used for transmitting X10
#include "x10constants.h"              // X10 Lib constants
#define RPT_SEND 2 

#define ZCROSS_PIN     2               // BLK pin 1 of PSC05
#define RCVE_PIN       4               // GRN pin 3 of PSC05
#define TRANS_PIN      5               // YEL pin 4 of PSC05
#define LED_PIN        9              // for testing 

RF24 radio(6, 8);
x10 SX10 = x10(ZCROSS_PIN, TRANS_PIN, RCVE_PIN, LED_PIN);// set up a x10 library instance:

#define MAX_RETRIES		20
const uint64_t pipes[2] = { 0xABCDABCD71LL, 0x544d52687CLL };
bool TX = 1, RX = 0, role = 0;
int transmitCount = 0;

void setup(void)
{
	Serial.begin(57600);
	printf_begin();
	radio.begin();
	radio.setChannel(1);
	radio.setPALevel(RF24_PA_MAX);
	radio.setDataRate(RF24_1MBPS);
	radio.setAutoAck(1);
	radio.setRetries(15, 15);
	radio.setCRCLength(RF24_CRC_8);
	radio.openWritingPipe(pipes[0]);
	radio.openReadingPipe(0, pipes[0]);
	radio.openReadingPipe(1, pipes[1]);
	radio.enableDynamicPayloads();
	radio.startListening();
	radio.printDetails();
	printf("*** PRESS 'T' to begin transmitting to the other node\n\r");
	radio.powerUp();
}

void loop(void)
{
	//if (Serial.available())
	//{
	//	char c = toupper(Serial.read());
	//	if (c == 'T' && role == RX)
	//	{
	//		printf("*** CHANGING TO TRANSMIT ROLE -- PRESS 'R' TO SWITCH BACK\n\r");
	//		radio.openWritingPipe(pipes[0]);
	//		radio.openReadingPipe(1, pipes[1]);
	//		radio.stopListening();
	//		role = TX;                  // Become the primary transmitter (ping out)
	//	}
	//	else if (c == 'R' && role == TX)
	//	{
	//		radio.openWritingPipe(pipes[0]);
	//		radio.openReadingPipe(1, pipes[1]);
	//		radio.startListening();
	//		printf("*** CHANGING TO RECEIVE ROLE -- PRESS 'T' TO SWITCH BACK\n\r");
	//		role = RX;                // Become the primary receiver (pong back)
	//	}
	//}

	/*if (role == TX)
	{*/
	radio.stopListening();
		delay(2000);
		transmitCount++;
		sendString("Payload, Count=" + (String)transmitCount);
	/*}

	if (role == RX)
	{*/
		radio.startListening();
		readLine();
	/*}*/

	/*if (SX10.received())
	{                      
		SX10.debug();       
		SX10.reset();
	}*/
}

void sendString(String s)
{

	unsigned int length = s.length() + 1;
	char buffer[length];
	buffer[length] = 0;
	s.toCharArray(buffer, length);
	int counter = 0;
	//radio.write(&buffer, length);
	while (!radio.write(&buffer, length))
	{
		counter++;
		if (counter > MAX_RETRIES)
		{
			break;
		}
		delayMicroseconds(500);
	}
	radio.txStandBy();
	Serial.println(counter < MAX_RETRIES ? "String Sent Successful: " + s : "Failed to send!");
}

void readLine()
{
	while (radio.available())
	{
		/*byte payloadSize = radio.getDynamicPayloadSize();
		byte buffer[payloadSize + 1];
		buffer[payloadSize] = 0;
		radio.read(&buffer, payloadSize);
		Serial.print("0=");
		Serial.println(buffer[0]);
		Serial.print("1=");
		Serial.println(buffer[1]);
		Serial.print("2=");
		Serial.println(buffer[2]);
		SX10.write(buffer[0], buffer[1], RPT_SEND);
		SX10.write(buffer[0], buffer[2], RPT_SEND);*/

		byte payloadSize = radio.getDynamicPayloadSize();
		char buffer[payloadSize + 1];
		buffer[payloadSize] = 0;
		radio.read(&buffer, payloadSize);
		Serial.println(buffer);
	}
}

