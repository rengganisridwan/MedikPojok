/*
  Setting Sampling Rate
  https://github.com/rengganisridwan/Arduino-DSP

  Author: Rengganis R.H. Santoso
*/
#include <MegunoLink.h>

const unsigned long SAMPLING_RATE_HZ = 300;
const unsigned long BAUD_RATE = 500000;
const double SIGNAL_FREQ_HZ = 1;
const double NOISE_FREQ_HZ = 50;

TimePlot MyPlot;

void setup() {
  Serial.begin(BAUD_RATE);
}

void loop() {
  // Calculate elapsed time
  static unsigned long tPast = 0;
  unsigned long tPresent = micros();
  unsigned long tInterval = tPresent - tPast;
  tPast = tPresent;

  // Subtract the timer by the elapsed time between subsequent loop. If the 
  // timer value reaches zero, we do the measurement. Then, the timer value 
  // will be added by the sampling time period. Thus, the measurement will be 
  // conducted in a periodic manner.
  static long timer = 0;
  timer -= tInterval;
  if (timer < 0) {
    timer += 1e6 / SAMPLING_RATE_HZ;
    double t = micros() / 1.0e6;

    double signal = sin(2 * PI * SIGNAL_FREQ_HZ * t);
    double noise = 0.5 * sin(2 * PI * NOISE_FREQ_HZ * t);
    double data = signal + noise;
    
    MyPlot.SendData("t",millis());
    MyPlot.SendData("CH-01",data);
  }
}