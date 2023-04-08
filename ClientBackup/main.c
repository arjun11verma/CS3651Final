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
bool startMovement = false;
int stepsLeft;
/** **/

ISR(TCB0_INT_vect) {
  if (buffer_counter < SOUND_BUFFER_SIZE && !(ADC0.COMMAND & ADC_STCONV_bm)) {
    sound_buffer[active_buffer][buffer_counter] = ADC0.RES;
    ADC0.COMMAND = ADC_STCONV_bm;
    buffer_counter++;
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
  stepsLeft = 0;
  currentSteps = 0;

  /** Interrupt Handler setup **/
  cli();
  TCB0.CTRLB = TCB_CNTMODE_INT_gc; // Use timer compare mode
  TCB0.CCMP = 31; // Value to compare with - 250 khz divided by this value, 31 for ~8000 Hz
  TCB0.INTCTRL = TCB_CAPT_bm; // Enable the interrupt
  TCB0.CTRLA = TCB_CLKSEL_CLKTCA_gc | TCB_ENABLE_bm; // Use Timer A as clock, enable timer
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
  spiceMotor.setSpeed(500);
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
    /** **/

    /** Begin data transfer on now inactive buffer **/
    Udp.beginPacket(remoteIP, remotePort);
    Udp.write(packetBuffer, SOUND_BUFFER_SIZE * sizeof(int16_t));
    Udp.endPacket();
    /** **/
  }

  if (Udp.read(readBuffer, readBufferSize) > 0) { // if we get a message from the server
    noInterrupts();
    startMovement = true;
    stepsLeft = 100;
    interrupts();
  }

  if (startMovement) {
    noInterrupts();
    spiceMotor.step(1);
    stepsLeft--;
    interrupts();
  }
}