#include <Arduino.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <Wire.h>

#define PIN_D2 2
#define PIN_D3 3
#define PIN_D4 4
#define PIN_D5 5
#define PIN_D6 6
#define PIN_D7 7

bool rotationCW = true; //rotational direction (if true => clockwise)
//Sector state approaches (1 Sector = 2|3 HASELs)
//1 Sector active
const uint8_t states1[] = {0b100000, 0b010000, 0b001000, 0b000100, 0b000010, 0b000001, 0b100000, 0b010000, 0b001000, 0b000100, 0b000010, 0b000001}; //1-1-1 approach
//2 Sectors active
const uint8_t states2[] = {0b110000, 0b011000, 0b001100, 0b000110, 0b000011, 0b100001, 0b110000, 0b011000, 0b001100, 0b000110, 0b000011, 0b100001}; //2-2-2 approach
const uint8_t states3[] = {0b100000, 0b110000, 0b010000, 0b011000, 0b001000, 0b001100, 0b000100, 0b000110, 0b000010, 0b000011, 0b000001, 0b100001}; //2-1-2 approach
//3 Sectors active
const uint8_t states4[] = {0b111000, 0b011100, 0b001110, 0b000111, 0b100011, 0b110001, 0b111000, 0b011100, 0b001110, 0b000111, 0b100011, 0b110001}; // 3-3-3 approach
const uint8_t states5[] = {0b111000, 0b011000, 0b011100, 0b001100, 0b001110, 0b000110, 0b000111, 0b000011, 0b100011, 0b100001, 0b110001, 0b110000}; //3-2-3 approach
const uint8_t states6[] = {0b100000, 0b110000, 0b111000, 0b011000, 0b001000, 0b001100, 0b001110, 0b000110, 0b000010, 0b000011, 0b100011, 0b100001}; //caterpillar approach

const uint8_t *currentStates = states4; //which state vector (=which approach) to use
uint8_t numStates = sizeof(states3) / sizeof(states3[0]); //number of states
volatile uint8_t currentStateIndex = 0; //index of current state
volatile bool updateFlag = false;

uint32_t interval_ms = 100; //timeinterval for interrupt (=T_T)
double freq_ms = 1/float(interval_ms); //interrupt frequency in 1/ms (=f_T)
double freq = 1000/(float(interval_ms)*6); //overall frequency of motor (=f_M)

unsigned int valueEncoder; //Raw value of encoder sensor
double angleEncoder; //angle converted from raw sensor data

//void setTimerInterval(uint32_t desiredInterval_ms);
void updateOutputs(uint8_t state);
//void debugOutput(uint8_t state);

ISR(TIMER1_COMPA_vect) {
  //Read Sensor output
  /*
  ---   Insert code here...    ---
  valueEncoder = someArduinoCommand;  //Read output value of sensor
  angleEncoder = (valEnc/255)*360; (scale from 0-255 to 0-360 degrees)
  */
  //adjust which sector to actuate according to previous state, rotational direction and actuation approach 
  if (rotationCW) {
    if (currentStateIndex == 0) currentStateIndex = numStates - 1;
    else currentStateIndex--;
  } else {
    currentStateIndex = (currentStateIndex + 1) % numStates;
  }
  updateOutputs(currentStates[currentStateIndex]);
  updateFlag = true; //update
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
  // Debugging output (commented out)
  //Print out the states:
  Serial.print("Current interval: ");
  Serial.print(interval_ms);
  Serial.print("ms | ");
  Serial.print(freq);
  Serial.print("Hz || ");

  //Print out the states:
  Serial.print("Current State: ");
  Serial.print(state, BIN);  // Print state as binary
  Serial.print(" | D3: ");
  Serial.print((state & 0b100000) ? "HIGH" : "LOW");
  Serial.print(" | D4: ");
  Serial.print((state & 0b010000) ? "HIGH" : "LOW");
  Serial.print(" | D5: ");
  Serial.print((state & 0b001000) ? "HIGH" : "LOW");
  Serial.print(" | D6: ");
  Serial.print((state & 0b000100) ? "HIGH" : "LOW");
  Serial.print(" | D7: ");
  Serial.print((state & 0b000010) ? "HIGH" : "LOW");
  Serial.print(" | D8: ");
  Serial.println((state & 0b000001) ? "HIGH" : "LOW");
}

void setup() {
  pinMode(PIN_D2, OUTPUT);
  pinMode(PIN_D3, OUTPUT);
  pinMode(PIN_D4, OUTPUT);
  pinMode(PIN_D5, OUTPUT);
  pinMode(PIN_D6, OUTPUT);
  pinMode(PIN_D7, OUTPUT);

  Serial.begin(9600);
  updateOutputs(currentStates[currentStateIndex]);
  setTimerInterval(interval_ms);
}

void loop() {
  if (updateFlag) {
    debugOutput(currentStates[currentStateIndex]);
    updateFlag = false;
  }

  if (Serial.available() > 0) {
    char receivedChar = Serial.read();
    switch (receivedChar) {
      case 'R':
        rotationCW = true;
        break;
      case 'L':
        rotationCW = false;
        break;
      case '1':
        currentStates = states1; numStates = sizeof(states1) / sizeof(states1[0]);
        break;
      case '2':
        currentStates = states2; numStates = sizeof(states2) / sizeof(states2[0]);
        break;
      case '3':
        currentStates = states3; numStates = sizeof(states3) / sizeof(states3[0]);
        break;
      case '4':
        currentStates = states4; numStates = sizeof(states4) / sizeof(states4[0]);
        break;
      case '5':
        currentStates = states5; numStates = sizeof(states5) / sizeof(states5[0]);
        break;
      case '6':
        currentStates = states6; numStates = sizeof(states6) / sizeof(states6[0]);
        break;
      case '-': //Decreases the frequency of the motor
        if (interval_ms < 200) interval_ms += 10;
        setTimerInterval(interval_ms);
        Serial.print("Interval: "); Serial.print(interval_ms); Serial.println(" ms");
        break;
      case '+': //Increases frequency of the motor
        if (interval_ms > 15) interval_ms -= 10;
        setTimerInterval(interval_ms);
        Serial.print("Interval: "); Serial.print(interval_ms); Serial.println(" ms");
        break;
    }
  }
  freq = 1000/(float(interval_ms)*6);
}
