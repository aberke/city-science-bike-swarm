// Radio libraries
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

//create an RF24 object (RADIO)
RF24 radio(9, 8);  // CE, CSN

// TODO: configure per radio using!
bool nodeNumber = 0; // TODO: switch between 0 and 1 depending on radio
// Note: The LED_BUILTIN is connected to tx/rx so it requires
// serial communication (monitor open) in order to work.
// Using other LED instead
const int TEST_LED = 3;
const int controlLedPin = 13;

const int lowPulse = 20;
const int highPulse = 255;

// The length of the full breathing period
const int period = 2200;  // ms

const int periodMidpoint = int(float(period) / 2.0);
const float amplitudeSlope = float(highPulse - lowPulse) / float(periodMidpoint);

// Addresses through which modules communicate.
const byte addresses[][6] = {"00000", "00001", "00002", "00003", "00004", "00005"};

// Keep track of when last message was received from another bike
// This tracks whether this bike is currently ‘in sync’ (in phase) with another bike
// Note: variables storing millis() must be unsigned longs
unsigned long lastReceiveTime;
unsigned long lastTransmitTime;
unsigned long currentTime;
unsigned long loopTime; // using for profiling/debugging
unsigned long loopLength;  // used for calculated expected latency

bool inSync = false; // True when in sync with another bicycle

// Keep track of where this bike is in its pulsating interval time
// This variable is updated with time when the bike is in sync with other bikes
// Otherwise it stays constant at the default
// The current value of the interval time determines the brightness of the light
const int defaultPhase = period;
int phase = defaultPhase;  // initialize to default

int expectedLatency;  // ms; This number is updated based on length of loop.
// TODO: determine best value for allowed phase shift
const int allowedPhaseShift = 300;  // ms

// A variable keeps track of when time was last checked in order to properly
// update the interval time with time
unsigned long lastTimeCheck = 0; // set to now() in setup


void setup() {
  // Starts serial communication, so that the Arduino can send out commands through the USB connection.
  // 9600 is the 'baud rate' 
  Serial.begin(9600);
  
  lastTimeCheck = millis(); // NOW
  lastReceiveTime = (-10)*period; // initialize at a long time ago
  lastTransmitTime = 0;

  setupRadio();
  
  // TODO: use controlledPin instead of built in LED
  pinMode(TEST_LED, OUTPUT);
//  pinMode(controlLedPin, OUTPUT); // TODO: uncomment when
}


void  setupRadio() {
  radio.begin();
  // TODO: figure out way to not have pre-numbered radios
  // The current configuration is for 2 radios with pre-assigned addresses.
  // This is a temporary implementation to get the light system working as desired.
  Serial.print("setupRadio for node:");
  Serial.println(nodeNumber);
  if (nodeNumber) {
    // Pipe 0 is also used by the writing pipe. So if you open pipe 0 for reading, and then startListening(), it will overwrite the writing pipe
    radio.openWritingPipe(addresses[0]);
    radio.openReadingPipe(1, addresses[1]);
    radio.openReadingPipe(2, addresses[2]);
    radio.openReadingPipe(3, addresses[3]);
    radio.openReadingPipe(4, addresses[4]);
    radio.openReadingPipe(5, addresses[5]);
  } else {
    radio.openWritingPipe(addresses[1]);
    radio.openReadingPipe(1, addresses[0]);
    radio.openReadingPipe(2, addresses[2]);
    radio.openReadingPipe(3, addresses[3]);
    radio.openReadingPipe(4, addresses[4]);
    radio.openReadingPipe(5, addresses[5]);
  }
//  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();
}


void loop() {
  // Print how long a loop takes
  // Serial print statements add significant time
  // loop length 20-30ms without Serial print statements
  unsigned long newLoopTime = millis();
  loopLength = newLoopTime - loopTime;
  Serial.print("Total loop time length: ");
  Serial.println(loopLength);
  loopTime = newLoopTime;
  // update current interval
  currentTime = millis();
  // check/update whether in sync with another bike
  // being in sync with another bike times out after given amount of time
  // without hearing from other bikes: 2*period
  inSync = false;
  if ((currentTime - lastReceiveTime) < 3*period)  {
    inSync =  true;
  }
  updatePhase();
  // set the light brightness based on where we are in the interval time
  // for default period: set light on HI (TODO)
  // fake pulsing when using just LEDs on arduino.
  testLight(phase);
  // pulseLight(phase);
  updatePhase();
  // send to other bikes at limited frequency of once per period
  if ((millis() - lastTransmitTime) > period) {
    radioTransmit(phase);
    lastTransmitTime = millis();
  }
  updatePhase();
  // listen for messages from other bikes
  int otherPhase = radioReceive();
  if (otherPhase >= 0) {  // -1 indicates no message was received
    // another bike sent their current interval time
    lastReceiveTime = millis();  // update for later checking whether in sync
    // change to other bike’s interval iff it is SOONER than our current interval
    // allow a phase shift to determine whose interval is sooner to account for latency
    updatePhase();
    if (otherPhase < (phase - allowedPhaseShift))  {
      // change to other interval (put in helper function "setphase")
      expectedLatency = loopLength/2;
      phase = otherPhase + expectedLatency;
      lastTimeCheck = lastReceiveTime;
    }
  }
}


int updatePhase() {
  // Update phase when in sync with another bike
  // Otherwise set phase to default and stay there
  currentTime = millis();
  if (inSync) {
    int  timeDelta = currentTime - lastTimeCheck;
    phase = (phase + timeDelta) % period;
  } else {
    phase = defaultPhase;
  }
  lastTimeCheck = currentTime;
  return phase;
}


void printTime(int num) {
  unsigned long newTime = millis();
  Serial.print(num);
  Serial.print(": Time: ");
  Serial.print(newTime);
  Serial.print(";  Delta: ");
  Serial.println(newTime - currentTime);
  currentTime = newTime;
}


int radioReceive() {
  //  If there is a message AND message is an integer AND 0 <= int(message) <= period:
  //    Return int(message)
  //  Else:
  //    Return  -1
  if (radio.available()) {
    String messageString = "";
    char messageBuffer[32];
    radio.read(&messageBuffer, sizeof(messageBuffer));
    int messageInt = atoi(messageBuffer);
    if ((messageInt > 0) && (messageInt <= period)) {
      return  messageInt;
    }
  }
  return -1;
}

void radioTransmit(int phase) {
  radio.stopListening();
  char messageBuffer[32];
  itoa(phase, messageBuffer, 10);
  radio.write(&messageBuffer, sizeof(messageBuffer));
  radio.startListening();
}

void testLight(int phase) {
  // This is a fake pulse light method used to keep track of the interval
  // when using a simple LED that cannot pulse with a varying amplitude.
  // at time between [0, period midpoint): OFF
  // at time between [period midpoint, period): ON
  // Check if interval time at HI: within lightTime of 0 or period
  if (phase < periodMidpoint) {
    digitalWrite(TEST_LED, LOW);
  } else  {
    digitalWrite(TEST_LED, HIGH);
  }
}


void pulseLight(int phase) {
  // The light pulses on a linear path
  // It starts HI and decreases in amplitude until the period midpoint: LO
  // And then increases in amplitude
  // Phase picture:
  //
  // |\           /\
  // |  \       /    \
  // |   \    /        \
  // |     \/            \
  //0:HI mid:LO  
  int amplitude;
  if (phase < periodMidpoint) {
    amplitude = highPulse - (amplitudeSlope * phase);
  } else {
    amplitude = lowPulse + (amplitudeSlope * (phase - periodMidpoint));
  }
  light(amplitude);
}

void light(int amplitude) {
  analogWrite(controlLedPin, amplitude);
}
