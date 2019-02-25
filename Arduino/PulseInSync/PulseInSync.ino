const int controlLedPin = 13;
const int potPin = A0;

const int lowPulse = 20;
const int highPulse = 255;

const int intervalLength = 2000;  // ms

const int intervalLengthMidPoint = int(float(intervalLength) / 2.0);
const float slopePulse = float(highPulse - lowPulse) / float(intervalLengthMidPoint);


bool inSync = true; // True when in sync with another bicycle

void setup() {
  // put your setup code here, to run once:

  pinMode(controlLedPin, OUTPUT);

  Serial.begin(9600);
  while (!Serial)
  {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
}

void loop() {

  int currentTime = millis();
  // variable to track where we are in the interval
  int intervalTime = abs(currentTime % intervalLength);

  // TODO:
  // Read in radio packet to check if there is a bike to get in sync with
  // reset interval time if necessary
  // send out packet

  if (inSync) {
    pulseLight(intervalTime);
  } else {
    // set the light on HIGH
    light(highPulse);
  }
}

void pulseLight(int intervalTime) {
  // The light pulses on a linear path
  // It increases in amplitude until the interval length's midpoint
  // And then decreases in amplitude
  int pulseAmplitude;
  if (intervalTime < intervalLengthMidPoint) {
    pulseAmplitude = lowPulse + (slopePulse * intervalTime);
  } else {
    pulseAmplitude = highPulse - (slopePulse * (intervalTime - intervalLengthMidPoint));
  }
  light(pulseAmplitude);
}

void light(int pulseAmplitude) {
  analogWrite(controlLedPin, pulseAmplitude);
}
