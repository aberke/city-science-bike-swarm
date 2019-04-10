// Radio libraries
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

//create an RF24 object (RADIO)
// Arduino Nano
RF24 radio(7, 8);  // CE=7, CSN=8

//Arduino Uno
//RF24 radio(9, 8);  // CE, CSN


// N total address for nodes to read/write on.
// TODO: configure per radio using!
int nodeNumber = 2; // TODO: switch between 0 and N depending on radio
const int N = 6;  // Reading only supported on pipes 1-5
// Addresses through which N modules communicate.
// Note: Using other types for addressing did not work for reading from multiple pipes.  Why?  IDK but this works ;-)
long long unsigned addresses[6] = {0xF0F0F0F0F0, 0xF0F0F0F0AA, 0xF0F0F0F0BB, 0xF0F0F0F0CC, 0xF0F0F0F0DD, 0xF0F0F0F0EE};

// Note: The LED_BUILTIN is connected to tx/rx so it requires
// serial communication (monitor open) in order to work.
// Using other LED instead
const int LED_PIN = 6;

const int lowPulse = 10;
const int highPulse = 255;

// The length of the full breathing period
const int period = 2200;  // ms

const float periodMidpoint = float(period)/2.0;
const float amplitudeSlope = float(highPulse - lowPulse) / periodMidpoint;

// Keep track of when last message was received from another bike
// This tracks whether this bike is currently ‘in sync’ (in phase) with another bike
// Note: variables storing millis() must be unsigned longs
unsigned long lastReceiveTime;
unsigned long lastTransmitTime;
unsigned long currentTime;
unsigned long loopTime; // using for profiling/debugging
unsigned long loopLength;  // used for calculated expected latency

const int sendFrequency = period/2;

bool inSync = false; // True when in sync with another bicycle

// Keep track of where this bike is in its pulsating interval time
// This variable is updated with time when the bike is in sync with other bikes
// Otherwise it stays constant at the default
// The current value of the interval time determines the brightness of the light
const int defaultPhase = 0;
int phase = defaultPhase;  // initialize to default

int expectedLatency;  // ms; This number is updated based on length of loop.
const int maxAllowedPhaseShift = 100;  // ms

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
  
  pinMode(LED_PIN, OUTPUT);
}


void  setupRadio() {
  radio.begin();
  // The current configuration is for N radios with pre-assigned addresses.
  // This is a temporary implementation to get the light system working as desired.
  Serial.print("setupRadio for node:");
  Serial.println(nodeNumber);
  // Radio writes to one address and reads on all other configured addreses,
  // The writing and reading addresses are determined by the nodeNumber
  // Note: Pipe 0 is used by the writing pipe.
  // So if you open pipe 0 for reading, and then startListening(), it will overwrite the writing pipe.
  radio.openWritingPipe(addresses[(nodeNumber + 0) % N]);
  for (int i=1; i < N; i++) {
    radio.openReadingPipe(i, addresses[(nodeNumber + i) % N]);
  }
  // Configure for radio range.
  radio.setPALevel(-6);
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
  // without hearing from other bikes
  inSync = false;
  if ((currentTime - lastReceiveTime) < 2*period)  {
    inSync =  true;
  }
  updatePhase();
  // set the light brightness based on where we are in the interval time
  // fake pulsing when using just LEDs on arduino.
//  testLight(phase);
//  testLightAnalog(phase);
//  pulseLightLinear(phase);
  pulseLightCurve(phase);
  updatePhase();
  // listen for messages from other bikes
  bool changedPhase = false;
  int otherPhase = radioReceive();
  if (otherPhase >= 0) {  // -1 indicates no message was received
    // another bike sent their current interval time
    lastReceiveTime = millis();  // update for later checking whether in sync
    // change to other bike’s interval iff it is GREATER than our current interval
    // allow a phase shift to determine whose interval is sooner to account for latency
    updatePhase();
    int allowedPhaseShift = min(maxAllowedPhaseShift, loopLength);
    if ((otherPhase > phase) && computePhaseShift(phase, otherPhase) > allowedPhaseShift)  {
      // change to other phase
      expectedLatency = loopLength/2;
      phase = otherPhase + expectedLatency;
      lastTimeCheck = lastReceiveTime;
      changedPhase = true;
    }
  }
  // send to other bikes at limited frequency or if phase recently changed
  if (changedPhase || (millis() - lastTransmitTime) > period) {
    radioBroadcast(phase);
    lastTransmitTime = millis();
  }
}

int computePhaseShift(int phase1, int phase2) {
  // Use phase1 as lower value, phase2 as higher value
  if (phase1 > phase2) {
    int temp = phase1;
    phase1 = phase2;
    phase2 = temp;
  }
  int a = phase2 - phase1;
  int b = period - phase2 + phase1;
  int phaseShift = min(a, b);
  return phaseShift;
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


int radioReceive() {
  // Read all the available messages in the queue,
  // return the message with the highest phase message if messages available,
  // otherwise return -1 to indicate there was no message.
  int messageInt = -1;
  while (radio.available()) {
    char messageBuffer[32];
    radio.read(&messageBuffer, sizeof(messageBuffer));
    int newMessageInt = atoi(messageBuffer);
    if ((newMessageInt > messageInt) && (newMessageInt <= period)) {
      messageInt = newMessageInt;
    }
  }
  return messageInt;
}

void radioBroadcast(int phase) {
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
  if (phase > periodMidpoint) {
    digitalWrite(LED_PIN, LOW);
  } else  {
    digitalWrite(LED_PIN, HIGH);
  }
}

void testLightAnalog(int phase) {
  // This is a fake pulse light method used to keep track of the interval
  // when using a simple LED that cannot pulse with a varying amplitude.
  // at time between [0, period midpoint): OFF
  // at time between [period midpoint, period): ON
  // Check if interval time at HI: within lightTime of 0 or period
  if (phase > periodMidpoint) {
    analogWrite(LED_PIN, lowPulse);
  } else  {
    analogWrite(LED_PIN, highPulse);
  }
}

void pulseLightCurve(int phase) {
  // The light pulses on a sinusoidal corve
  // It starts HI and decreases in amplitude until the period midpoint: LO
  // And then increases in amplitude
  //0:HI mid:LO
  // A = [cos(2*pi/period) + 1]((HI - LO)/2) + LO
  float theta = phase*(2*PI/float(period));
  int amplitude = (cos(theta) + 1)*((highPulse - lowPulse)/2) + lowPulse;
  light(amplitude);
}

void pulseLightLinear(int phase) {
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
  analogWrite(LED_PIN, amplitude);
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
