#include <avr/io.h>
#include <avr/interrupt.h>
#include <Wire.h>

#define PIN_D2 2
#define PIN_D3 3
#define PIN_D4 4
#define PIN_D5 5
#define PIN_D6 6
#define PIN_D7 7

//#define SensorPins XXX

bool rotationCW = true;
const uint8_t states1[] = {0b100000, 0b010000, 0b001000, 0b000100, 0b000010, 0b000001};
const uint8_t states2[] = {0b110000, 0b011000, 0b001100, 0b000110, 0b000011, 0b100001};
const uint8_t states3[] = {0b100000, 0b110000, 0b010000, 0b011000, 0b001000, 0b001100};
const uint8_t states4[] = {0b111000, 0b011100, 0b001110, 0b000111, 0b100011, 0b110001};
const uint8_t states5[] = {0b111000, 0b011000, 0b011100, 0b001100, 0b001110, 0b000110};
const uint8_t states6[] = {0b100000, 0b110000, 0b111000, 0b011000, 0b001000, 0b001100};

const uint8_t *currentStates = states4;
uint8_t numStates = sizeof(states4) / sizeof(states4[0]);
volatile uint8_t currentStateIndex = 0;
volatile bool updateFlag = false;

uint32_t interval_ms = 100;
double freq;

//--------------------Functions--------------------

ISR(TIMER1_COMPA_vect) {
  //Read Sensor Value
  /* Placeholder */
  //rotate to next/previous sector of Motor according to Serial Input
  currentStateIndex = (updateFlag && rotationCW) ? (currentStateIndex == 0 ? numStates - 1 : currentStateIndex - 1) : (currentStateIndex + 1) % numStates;
  if (updateFlag) updateOutputs(currentStates[currentStateIndex]);
  updateFlag = false;
}

void setTimerInterval(uint32_t desiredInterval_ms) {
  uint32_t clockCycles = (desiredInterval_ms * 1000) / 64;
  if (clockCycles > 65535) clockCycles = 65535;
  TCCR1A = 0;
  TCCR1B = (1 << WGM12) | (1 << CS12) | (1 << CS10);
  OCR1A = clockCycles - 1;
  TIMSK1 = (1 << OCIE1A);
}

void updateOutputs(uint8_t state) {
  digitalWrite(PIN_D2, (state & 0b100000) ? HIGH : LOW);
  digitalWrite(PIN_D3, (state & 0b010000) ? HIGH : LOW);
  digitalWrite(PIN_D4, (state & 0b001000) ? HIGH : LOW);
  digitalWrite(PIN_D5, (state & 0b000100) ? HIGH : LOW);
  digitalWrite(PIN_D6, (state & 0b000010) ? HIGH : LOW);
  digitalWrite(PIN_D7, (state & 0b000001) ? HIGH : LOW);
}

void debugOutput(uint8_t state) {
  Serial.print("Current interval: ");
  Serial.print(interval_ms);
  Serial.print("ms | ");
  Serial.print(freq);
  Serial.print("Hz || ");
  Serial.print("Current State: ");
  Serial.println(state, BIN);
}

/* String stateReaderUpdater() {
  String contInput = Serial.readStringUntil('\n');
  int contDelimiterIndex = contInput.indexOf('|');
  char direction = contInput.substring(0, contDelimiterIndex).charAt(0);
  int
}
*/
//--------------------Main Arduino Functions--------------------

void setup() {
  // put your setup code here, to run once:
   pinMode(PIN_D2, OUTPUT);
  pinMode(PIN_D3, OUTPUT);
  pinMode(PIN_D4, OUTPUT);
  pinMode(PIN_D5, OUTPUT);
  pinMode(PIN_D6, OUTPUT);
  pinMode(PIN_D7, OUTPUT);

  Serial.begin(9600);
  
  Serial.println("Enter configuration (1-6) and timer interval (ms) in format: Config|Interval");
  while (!Serial.available());
  String input = Serial.readStringUntil('\n');
  
  int delimiterIndex = input.indexOf('|');
  if (delimiterIndex != -1) {
    int config = input.substring(0, delimiterIndex).toInt();
    interval_ms = input.substring(delimiterIndex + 1).toInt();
    
    switch (config) {
      case 1: currentStates = states1; numStates = sizeof(states1) / sizeof(states1[0]); break;
      case 2: currentStates = states2; numStates = sizeof(states2) / sizeof(states2[0]); break;
      case 3: currentStates = states3; numStates = sizeof(states3) / sizeof(states3[0]); break;
      case 4: currentStates = states4; numStates = sizeof(states4) / sizeof(states4[0]); break;
      case 5: currentStates = states5; numStates = sizeof(states5) / sizeof(states5[0]); break;
      case 6: currentStates = states6; numStates = sizeof(states6) / sizeof(states6[0]); break;
    }
  }
  setTimerInterval(interval_ms);
  updateOutputs(currentStates[currentStateIndex]);

}

void loop() {
  // put your main code here, to run repeatedly:
  if(Serial.available()>0) {
    String contInput = Serial.readStringUntil('\n');
    int contDelimiterIndex = contInput.indexOf('|');
    int contConfig = contInput.substring(0,contDelimiterIndex).toInt();
    switch(contInput.charAt(contDelimiterIndex + 1)) {
      case '+': rotationCW=true; updateFlag=true; break;
      case '-': rotationCW=false; updateFlag = true; break;
    }
  }
}
