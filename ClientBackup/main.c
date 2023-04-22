#include <Arduino.h>
#include <WiFiNINA.h>
#include <WiFiUdp.h>
#include <wifistuff.h>
#include <Stepper.h>

/** Audio Sampling **/
#define SOUND_BUFFER_SIZE 400
#define NUM_BUFFERS 2
volatile int buffer_counter;
int16_t sound_buffer[NUM_BUFFERS][SOUND_BUFFER_SIZE];
int active_buffer;
/** **/

/** UDP Client **/
int status = WL_IDLE_STATUS;
char ssid[] = MYSSID;    // network SSID (name)
char pass[] = MYPASS;    // network password (use for WPA, or use as key for WEP)
unsigned int localPort = remotePORT - 1;
uint16_t remotePort = remotePORT;
const size_t readBufferSize = 8;
char readBuffer[readBufferSize];
WiFiUDP Udp;
/** **/

/** Motor Driver **/
Stepper spiceMotor(100, 2, 3, 4, 5); // red, blue, green, black
volatile bool motor_moving;
int steps_left;

#define FULL_ROTATION 1510
const int SPICE_MOVEMENT = (FULL_ROTATION / 8);

enum Spices {
  BASIL,
  CUMIN,
  THYME,
  MINT,
  SALT,
  CHILI,
  DILL,
  MACE
};

int currentPosition = BASIL;
/** **/

/** Temp/Random **/
unsigned long timet;
/** **/

void moveTo(int newPosition) {
  noInterrupts();
  motor_moving = true;
  interrupts();

  int toMove = 0;
  if (newPosition > currentPosition) {
    toMove = newPosition - currentPosition;
  } else {
    toMove = newPosition + 8 - currentPosition;
  }
  toMove = toMove  * SPICE_MOVEMENT;

  if (toMove == SPICE_MOVEMENT) {
    toMove += 20;
  }
  

  spiceMotor.step(toMove);
  currentPosition = newPosition;

  noInterrupts();
  motor_moving = false;
  interrupts();
}

ISR(TCB0_INT_vect) {
  if (!motor_moving) {
    if (buffer_counter < SOUND_BUFFER_SIZE && !(ADC0.COMMAND & ADC_STCONV_bm)) {
      sound_buffer[active_buffer][buffer_counter] = ADC0.RES;
      ADC0.COMMAND = ADC_STCONV_bm;
      buffer_counter++;
    }
  }
  TCB0.INTFLAGS = TCB_CAPT_bm;
}

void WifiSetup() {
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    // wait 10 seconds for connection:
    delay(5000);
  }
  Serial.println("Connected to WiFi");
  Serial.println("\nStarting connection to server...");
  Udp.begin(localPort);
  Serial.println("Connected!");
}

void setup() {
  Serial.begin(9600);
  buffer_counter = 0;
  active_buffer = 0;
  motor_moving = false;

  /** Interrupt Handler setup **/
  cli();
  CLKCTRL.OSC20MCTRLA = CLKCTRL_RUNSTDBY_bm;
  CLKCTRL.MCLKCTRLA = CLKCTRL_CLKOUT_bm | CLKCTRL_CLKSEL_OSC20M_gc;
  CLKCTRL.MCLKCTRLB = 0;

  TCB0.CTRLB = TCB_CNTMODE_INT_gc; // Use timer compare mode
  TCB0.CCMP = 2000; // Value to compare with - 16 mhz divided by this value is 8 khz
  TCB0.INTCTRL = TCB_CAPT_bm; // Enable the interrupt
  TCB0.CTRLA = TCB_CLKSEL_CLKDIV1_gc | TCB_ENABLE_bm; // Use Timer A as clock, enable timer
  sei();
  /** **/

  /** Async analog read setup **/
  int pin = digitalPinToAnalogInput(A0);
  ADC0.MUXPOS = (pin << ADC_MUXPOS_gp);
  ADC0.COMMAND = ADC_STCONV_bm;
  /** **/

  /** Wifi Setup **/
  WifiSetup();
  /** **/
  
  /** Motor Setup **/
  spiceMotor.setSpeed(150);
  /** **/

  /** Temp/Random **/
  timet = millis();
  /** **/
}

void loop() {
  if (buffer_counter == SOUND_BUFFER_SIZE) {
    /** Reset relevant variables without interruptions */
    noInterrupts();
    buffer_counter = 0;

    void* tempBuffer = (void*)(sound_buffer[active_buffer]);
    char* packetBuffer = (char*)tempBuffer;

    active_buffer = (active_buffer == 0) ? 1 : 0;

    interrupts();

    /** Begin data transfer on now inactive buffer **/
    Udp.beginPacket(remoteIP, remotePort);
    Udp.write(packetBuffer, SOUND_BUFFER_SIZE * sizeof(int16_t));
    Udp.endPacket();
    /** **/
  }

  if (Udp.parsePacket() != 0) { // if we get a message from the server
    Udp.read(readBuffer, readBufferSize);
    int spiceValue = atoi(readBuffer);

    Serial.println(spiceValue);

    moveTo(spiceValue);
  }
}