int controlLedPin = 13;
int potPin = A0;

int lowPulse = 0;
int highPulse = 255;

//ms
int intervalLength = 2000;

int intervalLengthMidPoint = int(float(intervalLength) / 2.0);
float slopePulse = float(highPulse) / float(intervalLengthMidPoint);

void setup() {
  // put your setup code here, to run once:

  pinMode(controlLedPin, OUTPUT);

  Serial.begin(9600);
  while (!Serial)
  {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  Serial.println("slopePulse");
  Serial.println(slopePulse);
  delay(5000);

}

void loop() {

  int currentTime = millis();
  // variable to track where we are in the interval
  int intervalTime = abs(currentTime % intervalLength);

  //Serial.print("loop currentTime: ");
  //Serial.print(currentTime);
  //Serial.print(" intervalTime: ");
  //Serial.println(intervalTime);

  // determine if going up or down
  bool directionUp = true;

  if (intervalTime > intervalLengthMidPoint) {
    directionUp = false;
  }

  int pulseAmplitude;
  if (directionUp) {
    pulseAmplitude = slopePulse * intervalTime;
  } else {
    pulseAmplitude = highPulse - (slopePulse * intervalTime);
  }

  analogWrite(controlLedPin, pulseAmplitude);
  Serial.print("pulseAmplitude :");
  Serial.println(pulseAmplitude);
  
  //Serial.print(" dir: ");
  //Serial.print(directionUp);
  //Serial.print(" Pulse: ");
  //Serial.println(slopePulse);


}
