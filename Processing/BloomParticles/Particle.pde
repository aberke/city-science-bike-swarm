class Particle {
  float posX;
  float posY;

  float growInc = 0.0;

  int lowPulse = 2;
  
  //size of the dot and pulse
  int highPulse = 45;
  // The length of the full breathing period
  
  //period speed
  int period = 7000;  // ms

  //phase
  int defaultPhase = 0;
  int phase = defaultPhase; 

  int lastTimeCheck = 0; 
  
  float  amplitude = 0;

  Particle(float posX, float posY) {
    this.posX = posX;
    this.posY = posY;
  }

  //animation
  void update(PGraphics2D pg) {
    posX += 0.5;
    if (posX > width) {
      posX = 0;
    }

    float ph =updatePhase();
    pulseLightCurve(ph);
    drawAmp(pg, amplitude);
  }


  void pulseLightCurve(float ph) {
    // The light pulses on a sinusoidal corve
    // It starts HI and decreases in amplitude until the period midpoint: LO
    // And then increases in amplitude
    //0:HI mid:LO
    // A = [cos(phase*2*pi/period) + 1]((HI - LO)/2) + LO
    float theta = ph*(2*PI/float(period));
    amplitude = (cos(theta) + 1)*((highPulse - lowPulse)/2) + lowPulse;
  }

  float updatePhase() {
    int currentTime = millis();
    int  timeDelta = currentTime - lastTimeCheck;
    return (phase + timeDelta) % period;
  }

  void drawAmp(PGraphics2D pg, float amp) {
    println(amp);
    pg.ellipse(posX, posY, amp, amp);
  }

  void draw(PGraphics2D pg) {
    pg.ellipse(posX, posY, 10, 10); 
  }
  
}
