// Adafruit IO Publish & Subscribe Example
//
// Adafruit invests time and resources providing this open source code.
// Please support Adafruit and open source hardware by purchasing
// products from Adafruit!
//
// Written by Todd Treece for Adafruit Industries
// Copyright (c) 2016 Adafruit Industries
// Licensed under the MIT license.
//
// All text above must be included in any redistribution.

/************************** Configuration ***********************************/

// edit the config.h tab and enter your Adafruit IO credentials
// and any additional configuration needed for WiFi, cellular,
// or ethernet clients.
#include "config.h"

HardwareSerial NanoSerial(2);
static const int NANO_RX = 16;   // ESP32 RX2  <- TX Nano (con level shifting)
static const int NANO_TX = 17;   // ESP32 TX2  -> RX Nano (con level shifting)
static const uint32_t UART_BAUD = 9600;

String lastCL = "";
String lastPS = "";
String lastMD = "";
String lastSV = "";
String lastST = "";
/************************ Example Starts Here *******************************/

// this int will hold the current count for our sketch
int count = 0;


// Track time of last published messages and limit feed->save events to once
// every IO_LOOP_DELAY milliseconds.
//
// Because this sketch is publishing AND subscribing, we can't use a long
// delay() function call in the main loop since that would prevent io.run()
// from being called often enough to receive all incoming messages.
//
// Instead, we can use the millis() function to get the current time in
// milliseconds and avoid publishing until IO_LOOP_DELAY milliseconds have
// passed.
#define IO_LOOP_DELAY 5000
unsigned long lastUpdate = 0;
static unsigned long lastPublish = 0;
#define SEND_INTERVAL 15000   // 15 segundos
unsigned long lastSend = 0;

// set up the 'counter' feed
//AdafruitIO_Feed *counter = io.feed("counter");
// Feeds de monitoreo de datos
AdafruitIO_Feed *ColorFeed = io.feed("proyecto-1-digital-2.rx-color");
AdafruitIO_Feed *MotorDCFeed = io.feed("proyecto-1-digital-2.rx-motdc");
AdafruitIO_Feed *PesoFeed = io.feed("proyecto-1-digital-2.rx-peso");
AdafruitIO_Feed *ServoFeed = io.feed("proyecto-1-digital-2.rx-servo");
AdafruitIO_Feed *StepperFeed = io.feed("proyecto-1-digital-2.rx-stepper");

AdafruitIO_Feed *MotDCSend = io.feed("proyecto-1-digital-2.tx-motdc");
AdafruitIO_Feed *ServoSend = io.feed("proyecto-1-digital-2.tx-servo");
AdafruitIO_Feed *StepperSend = io.feed("proyecto-1-digital-2.tx-stepper");

// Función para enviar al nano variables de control
void sendToNano(const char* prefijo, const String& payload) {
  NanoSerial.print(prefijo);
  NanoSerial.print(payload);
  NanoSerial.print('\n');

  Serial.print("-> Nano: ");
  Serial.print(prefijo);
  Serial.println(payload);
}

// Envío de callbacks de Adafruit al Nano
void onMotDCTx(AdafruitIO_Data *data) { sendToNano("MD:", data->value()); }
void onServoTx(AdafruitIO_Data *data) { sendToNano("SV:", data->value()); }
void onStepperTx(AdafruitIO_Data *data){ sendToNano("ST:", data->value()); }

// Función para recibir datos desde el nano usando prefijos
void publishByPrefix(const String& line) {
  if (line.startsWith("CL:"))      lastCL = line.substring(3);
  else if (line.startsWith("PS:")) lastPS = line.substring(3);
  else if (line.startsWith("MD:")) lastMD = line.substring(3);
  else if (line.startsWith("SV:")) lastSV = line.substring(3);
  else if (line.startsWith("ST:")) lastST = line.substring(3);
  else {
    Serial.print("Prefijo desconocido: ");
    Serial.println(line);
  }
}

String uartBuf;

void setup() {

  // start the serial connection
  Serial.begin(115200);
  delay(200);

  NanoSerial.begin(UART_BAUD, SERIAL_8N1, NANO_RX, NANO_TX);

  // wait for serial monitor to open
  while(! Serial);

  Serial.print("Connecting to Adafruit IO");

  // connect to io.adafruit.com
  io.connect();

  // set up a message handler for the count feed.
  // the handleMessage function (defined below)
  // will be called whenever a message is
  // received from adafruit io.
  //counter->onMessage(handleMessage);
  MotDCSend->onMessage(onMotDCTx);
  ServoSend->onMessage(onServoTx);
  StepperSend->onMessage(onStepperTx);

  // wait for a connection
  while(io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  // we are connected
  Serial.println();
  Serial.println(io.statusText());
  //counter->get();

}

void loop() {

  // io.run(); is required for all sketches.
  // it should always be present at the top of your loop
  // function. it keeps the client connected to
  // io.adafruit.com, and processes any incoming data.
  io.run();

  while (NanoSerial.available()) {
    char c = (char)NanoSerial.read();

    if (c == '\n') {
      String line = uartBuf;
      uartBuf = "";
      line.trim();

      if (line.length() > 0) {
        Serial.print("<- Nano: ");
        Serial.println(line);
        publishByPrefix(line); // solo guarda el último valor
      }
    } else {
      if (uartBuf.length() < 120) uartBuf += c;
    }
  }

  // 2) Publicar a Adafruit SOLO cada 15s
  if (millis() - lastSend >= SEND_INTERVAL) {
    lastSend = millis();

    Serial.println("Publishing batch -> Adafruit");

    if (lastCL.length()) ColorFeed->save(lastCL);
    if (lastPS.length()) PesoFeed->save(lastPS);
    if (lastMD.length()) MotorDCFeed->save(lastMD);
    if (lastSV.length()) ServoFeed->save(lastSV);
    if (lastST.length()) StepperFeed->save(lastST);
  }
}

// this function is called whenever a 'counter' message
// is received from Adafruit IO. it was attached to
// the counter feed in the setup() function above.
void handleMessage(AdafruitIO_Data *data) {

  Serial.print("received <- ");
  Serial.println(data->value());

}
