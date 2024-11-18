/*
  Bandpass Filter
  https://github.com/rengganisridwan/Arduino-DSP

  Author: Rengganis R.H. Santoso
*/

#include <MegunoLink.h>
TimePlot MyPlot;

const unsigned long SAMPLING_RATE = 300;
const unsigned long BAUD_RATE = 500000;
// const double kSignalFreqHz = 5;
// const double kNoiseFreqHz = 50;

void setup() {
  Serial.begin(BAUD_RATE);
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

    // double main_signal = sin(2 * PI * kSignalFreqHz * t);
    // double noise_signal = 0.5 * sin(2 * PI * kNoiseFreqHz * t);
    // double input_signal = main_signal + noise_signal;
    double inputSig = analogRead(A0);

    double outputSig = BandpassFilter(input_signal);

    // Now we use the same specification of the previous bandpass filter but 
    // with different structure. The following bandpass filter is composed of 
    // second-order sections.
    // double outputSig_sos_1 = BandpassFilter_SOS1(input_sig);
    // double outputSig_sos_2 = BandpassFilter_SOS2(outputSig_sos_1);

    // Send the results via serial
    // String to_print = String(input_signal) + "," + String(outputSig);
    // Serial.println(to_print);
    MyPlot.SendData("CH-01",outputSig);
  }
}

double BandpassFilter(double x_n) {
  /*
    Bandpass, Butterworth IIR, 4th Order
    Direct-Form II, Single Section
    Fs = 500 Hz, Fc1 = 10 Hz, Fc2 = 200 Hz
    The filter coefficients are obtained from MATLAB filterDesigner
  */

  // We made these following variables static so that the stored values carry
  // into subsequent function calls.
  static double x_n1, x_n2, x_n3, x_n4, y_n1, y_n2, y_n3, y_n4;

  double b[5] = { 0.5825, 0, -1.1650, -0.0000, 0.5825 };
  double a[5] = { 1.0000, -0.6874, -0.8157, 0.1939, 0.3477 };
  double inputs = b[0]*x_n + b[1]*x_n1 + b[2]*x_n2 + b[3]*x_n3 + b[4]*x_n4;
  double y_n = inputs - (a[1]*y_n1 + a[2]*y_n2 + a[3]*y_n3 + a[4]*y_n4);

  x_n4 = x_n3;
  x_n3 = x_n2;
  x_n2 = x_n1;
  x_n1 = x_n;

  y_n4 = y_n3;
  y_n3 = y_n2;
  y_n2 = y_n1;
  y_n1 = y_n;

  return y_n;
}

double BandpassFilter_SOS1(double x_n) {
  /*
    Bandpass, Butterworth IIR, 4th Order
    Direct-Form II, Second-Order Sections (First Section)
    Fs = 500 Hz, Fc1 = 10 Hz, Fc2 = 200 Hz
    The filter coefficients are obtained from MATLAB filterDesigner
  */

  static double x_n1, x_n2, y_n1, y_n2;
  double b[3] = { 1, 0, -1 };
  double a[3] = { 1, -1.8224, 0.8375 };
  double GAIN = 0.7632;

  double inputs = b[0] * x_n + b[1] * x_n1 + b[2] * x_n2;
  double y_n = GAIN * inputs - (a[1] * y_n1 + a[2] * y_n2);

  x_n2 = x_n1;
  x_n1 = x_n;

  y_n2 = y_n1;
  y_n1 = y_n;

  return y_n;
}

double BandpassFilter_SOS2(double x_n) {
  /*
    Bandpass, Butterworth IIR, 4th Order
    Direct-Form II, Second-Order Sections (Second Section)
    Fs = 500 Hz, Fc1 = 10 Hz, Fc2 = 200 Hz
    The filter coefficients are obtained from MATLAB filterDesigner
  */
  
  static double x_n1, x_n2, y_n1, y_n2;
  double b[3] = { 1, 0, -1 };
  double a[3] = { 1, 1.1349, 0.4151 };
  double GAIN = 0.7632;

  double inputs = b[0] * x_n + b[1] * x_n1 + b[2] * x_n2;
  double y_n = GAIN * inputs - (a[1] * y_n1 + a[2] * y_n2);

  x_n2 = x_n1;
  x_n1 = x_n;

  y_n2 = y_n1;
  y_n1 = y_n;

  return y_n;
}