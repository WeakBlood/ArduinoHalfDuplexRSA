#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "Arduino_LED_Matrix.h"
RF24 radio(9, 10); // CE, CSN

ArduinoLEDMatrix matrix;
const byte address[6] = "00001";
uint8_t frame[8][12]={{0,0,0,0,0,0,0,0,0,0,0,0},
{0,0,1,0,0,1,1,1,1,0,0,0},
{0,0,1,0,0,1,0,0,0,0,0,0},
{0,0,1,1,1,1,1,1,1,0,0,0},
{0,0,0,0,0,1,0,0,1,0,0,0},
{0,0,1,1,1,1,0,0,1,0,0,0},
{0,0,0,0,0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0,0,0,0,0}};
void setup() {
    Serial.begin(9600);
    //matrix.begin();
    radio.begin();
    radio.openWritingPipe(address);  // Transmitter
    radio.setPALevel(RF24_PA_LOW);
    radio.stopListening();
}

void loop() {
    //matrix.renderBitmap(frame,8,12);
    const char text[] = "scrivo un messaggio piùlungofewoijfewfewio";
    radio.write(&text, sizeof(text));
    Serial.println(text);
    delay(1000);
}
