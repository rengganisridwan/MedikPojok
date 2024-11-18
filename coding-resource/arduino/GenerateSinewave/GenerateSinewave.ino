/*
  Generate Sinewave
  https://github.com/rengganisridwan/Arduino-DSP

  Author: Rengganis R.H. Santoso
*/

#include <MegunoLink.h>
TimePlot MyPlot;

const unsigned long SAMPLING_RATE = 300;
const unsigned long BAUD_RATE = 500000;
const double SIGNAL_FREQ_1 = 1;
const double SIGNAL_FREQ_2 = 10;

void setup() {
  Serial.begin(BAUD_RATE);

  MyPlot.SetSeriesProperties("signal_1", Plot::Blue, Plot::Solid, 1, Plot::NoMarker);
  MyPlot.SetSeriesProperties("signal_2", Plot::Red, Plot::Solid, 1, Plot::NoMarker);
}

void loop() {
  static unsigned long tPast = 0;
  unsigned long tPresent = micros();
  unsigned long tInterval = tPresent - tPast;
  tPast = tPresent;

  static double timer = 0;
  timer -= tInterval;
  if (timer < 0) {
    timer += 1e6 / SAMPLING_RATE;

    double t = micros() / 1.0e6;
    int t_ms = millis();
    double inputSig1 = sin(2 * PI * SIGNAL_FREQ_1 * t);
    double inputSig2 = sin(2 * PI * SIGNAL_FREQ_2 * t);
    
    // String to_print = String(t_ms) + "," + String(inputSig1) + "," + String(inputSig2);
    // Serial.println(to_print);

    MyPlot.SendData("time (ms)",t_ms);
    MyPlot.SendData("signal 1",inputSig1);
    MyPlot.SendData("signal 2",inputSig2);
  }
}
