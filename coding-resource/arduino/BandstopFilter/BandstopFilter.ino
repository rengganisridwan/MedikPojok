/*
  Bandstop Filter
  https://github.com/rengganisridwan/Arduino-DSP

  Author: Rengganis R.H. Santoso
*/

#include <MegunoLink.h>

const unsigned long SAMPLING_RATE = 500;
const unsigned long BAUD_RATE = 500000;
const double SIGNAL_FREQ = 5;
const double NOISE_FREQ = 50;

TimePlot MyPlot;

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

    double inputSignal = sin(2 * PI * SIGNAL_FREQ * t) + 0.5 * sin(2 * PI * NOISE_FREQ * t);
    // double inputSignal = analogRead(A0);
    // inputSignal = BandpassFilter(inputSignal);
    Serial.print(inputSignal);
    Serial.print(",");
    double filteredSignal = BandstopFilter(inputSignal);
    // Serial.print(filteredSignal1);
    // Serial.print(",");
    // double filteredSignal = BandstopFilter50Hz_1(inputSignal);
    // filteredSignal = BandstopFilter50Hz_2(inputSignal);
    // filteredSignal = BandstopFilter50Hz_3(inputSignal);
    // filteredSignal = BandstopFilter50Hz_4(inputSignal);
    // filteredSignal = BandstopFilter50Hz_5(inputSignal);
    // Serial.print(filteredSignal);
    // Serial.print(",");
    // filteredSignal = BandstopFilter100Hz(filteredSignal);
    // Serial.print(filteredSignal);
    // Serial.print(",");
    // filteredSignal = BandstopFilter150Hz(filteredSignal);
    // filteredSignal = BandstopFilter60Hz(filteredSignal);
    // filteredSignal = BandstopFilter65Hz(filteredSignal);
    Serial.println(filteredSignal);
    // MyPlot.SendData("CH-01",filteredSignal);

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
  double inputs = b[0] * x_n + b[1] * x_n1 + b[2] * x_n2 + b[3] * x_n3 + b[4] * x_n4;
  double y_n = inputs - (a[1] * y_n1 + a[2] * y_n2 + a[3] * y_n3 + a[4] * y_n4);

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

double BandstopFilter(double x_n) {
  /*
    50 Hz Bandstop (Notch) Filter, Butterworth IIR, 4th Order
    Direct-Form II, Single Section
    Fs = 500 Hz, Fc1 = 49 Hz, Fc2 = 51 Hz
    The filter coefficients are obtained from MATLAB filterDesigner
  */

  const int FILTER_ORDER = 4;
  static double x[FILTER_ORDER + 1] = {};
  static double y[FILTER_ORDER + 1] = {};
  double b[FILTER_ORDER + 1] = { 0.9824, -3.1793, 4.5371, -3.1793, 0.9824 };
  double a[FILTER_ORDER + 1] = { 1.0000, -3.2076, 4.5368, -3.1511, 0.9651 };

  x[0] = x_n;
  y[0] = b[0] * x[0];

  for (int i = 1; i < FILTER_ORDER + 1; i++) {
    y[0] += b[i] * x[i] - a[i] * y[i];
  }

  for (int i = FILTER_ORDER; i > 0; i--) {
    x[i] = x[i - 1];
    y[i] = y[i - 1];
  }

  return y[0];
}

double BandstopFilter50Hz_1(double x_n) {
  /*
    50 Hz Bandstop (Notch) Filter, Butterworth IIR, 4th Order
    Direct-Form II, Single Section
    Fs = 500 Hz, Fc1 = 49 Hz, Fc2 = 51 Hz
    The filter coefficients are obtained from MATLAB filterDesigner
  */

  const int FILTER_ORDER = 2;
  static double x[FILTER_ORDER + 1] = {};
  static double y[FILTER_ORDER + 1] = {};
  double b[FILTER_ORDER + 1] = { 1.0000,   -1.6182,    1.0000 };
  double a[FILTER_ORDER + 1] = { 1.0000,   -1.5976,    0.9921 };
  double GAIN = 0.9961;

  x[0] = x_n;
  y[0] = GAIN * b[0] * x[0];

  for (int i = 1; i < FILTER_ORDER + 1; i++) {
    y[0] += GAIN * b[i] * x[i] - a[i] * y[i];
  }

  for (int i = FILTER_ORDER; i > 0; i--) {
    x[i] = x[i - 1];
    y[i] = y[i - 1];
  }

  return y[0];
}
double BandstopFilter50Hz_2(double x_n) {
  const int FILTER_ORDER = 2;
  static double x[FILTER_ORDER + 1] = {};
  static double y[FILTER_ORDER + 1] = {};
  double b[FILTER_ORDER + 1] = { 1.0000,   -1.6182,    1.0000 };
  double a[FILTER_ORDER + 1] = { 1.0000,   -1.6258,    0.9924 };
  double GAIN = 0.9961;

  x[0] = x_n;
  y[0] = GAIN * b[0] * x[0];

  for (int i = 1; i < FILTER_ORDER + 1; i++) {
    y[0] += GAIN * b[i] * x[i] - a[i] * y[i];
  }

  for (int i = FILTER_ORDER; i > 0; i--) {
    x[i] = x[i - 1];
    y[i] = y[i - 1];
  }

  return y[0];
}
double BandstopFilter50Hz_3(double x_n) {
  /*
    50 Hz Bandstop (Notch) Filter, Butterworth IIR, 4th Order
    Direct-Form II, Single Section
    Fs = 500 Hz, Fc1 = 49 Hz, Fc2 = 51 Hz
    The filter coefficients are obtained from MATLAB filterDesigner
  */

  const int FILTER_ORDER = 2;
  static double x[FILTER_ORDER + 1] = {};
  static double y[FILTER_ORDER + 1] = {};
  double b[FILTER_ORDER + 1] = { 1.0000,   -1.6182,    1.0000 };
  double a[FILTER_ORDER + 1] = { 1.0000,   -1.5930,    0.9797 };
  double GAIN = 0.9899;

  x[0] = x_n;
  y[0] = GAIN * b[0] * x[0];

  for (int i = 1; i < FILTER_ORDER + 1; i++) {
    y[0] += GAIN * b[i] * x[i] - a[i] * y[i];
  }

  for (int i = FILTER_ORDER; i > 0; i--) {
    x[i] = x[i - 1];
    y[i] = y[i - 1];
  }

  return y[0];
}
double BandstopFilter50Hz_4(double x_n) {
  const int FILTER_ORDER = 2;
  static double x[FILTER_ORDER + 1] = {};
  static double y[FILTER_ORDER + 1] = {};
  double b[FILTER_ORDER + 1] = { 1.0000,   -1.6182,    1.0000 };
  double a[FILTER_ORDER + 1] = { 1.0000,   -1.6106,    0.9801 };
  double GAIN = 0.9899;

  x[0] = x_n;
  y[0] = GAIN * b[0] * x[0];

  for (int i = 1; i < FILTER_ORDER + 1; i++) {
    y[0] += GAIN * b[i] * x[i] - a[i] * y[i];
  }

  for (int i = FILTER_ORDER; i > 0; i--) {
    x[i] = x[i - 1];
    y[i] = y[i - 1];
  }

  return y[0];
}
double BandstopFilter50Hz_5(double x_n) {
  const int FILTER_ORDER = 2;
  static double x[FILTER_ORDER + 1] = {};
  static double y[FILTER_ORDER + 1] = {};
  double b[FILTER_ORDER + 1] = { 1.0000,   -1.6182,    1.0000 };
  double a[FILTER_ORDER + 1] = { 1.0000,   -1.5981,    0.9752 };
  double GAIN = 0.9876;

  x[0] = x_n;
  y[0] = GAIN * b[0] * x[0];

  for (int i = 1; i < FILTER_ORDER + 1; i++) {
    y[0] += GAIN * b[i] * x[i] - a[i] * y[i];
  }

  for (int i = FILTER_ORDER; i > 0; i--) {
    x[i] = x[i - 1];
    y[i] = y[i - 1];
  }

  return y[0];
}

double BandstopFilter100Hz(double x_n) {
  /*
    50 Hz Bandstop (Notch) Filter, Butterworth IIR, 4th Order
    Direct-Form II, Single Section
    Fs = 500 Hz, Fc1 = 49 Hz, Fc2 = 51 Hz
    The filter coefficients are obtained from MATLAB filterDesigner
  */

  const int FILTER_ORDER = 4;
  static double x[FILTER_ORDER + 1] = {};
  static double y[FILTER_ORDER + 1] = {};
  double b[FILTER_ORDER + 1] = { 0.9824,   -1.2144,    2.3401,   -1.2144,    0.9824 };
  double a[FILTER_ORDER + 1] = { 1.0000,   -1.2252,    2.3398,   -1.2036,    0.9651 };

  x[0] = x_n;
  y[0] = b[0] * x[0];

  for (int i = 1; i < FILTER_ORDER + 1; i++) {
    y[0] += b[i] * x[i] - a[i] * y[i];
  }

  for (int i = FILTER_ORDER; i > 0; i--) {
    x[i] = x[i - 1];
    y[i] = y[i - 1];
  }

  return y[0];
}

double BandstopFilter150Hz(double x_n) {
  /*
    50 Hz Bandstop (Notch) Filter, Butterworth IIR, 4th Order
    Direct-Form II, Single Section
    Fs = 500 Hz, Fc1 = 49 Hz, Fc2 = 51 Hz
    The filter coefficients are obtained from MATLAB filterDesigner
  */

  const int FILTER_ORDER = 4;
  static double x[FILTER_ORDER + 1] = {};
  static double y[FILTER_ORDER + 1] = {};
  double b[FILTER_ORDER + 1] = { 0.9824,    1.2144,    2.3401,    1.2144,    0.9824 };
  double a[FILTER_ORDER + 1] = { 1.0000,    1.2252,    2.3398,    1.2036,    0.9651 };

  x[0] = x_n;
  y[0] = b[0] * x[0];

  for (int i = 1; i < FILTER_ORDER + 1; i++) {
    y[0] += b[i] * x[i] - a[i] * y[i];
  }

  for (int i = FILTER_ORDER; i > 0; i--) {
    x[i] = x[i - 1];
    y[i] = y[i - 1];
  }

  return y[0];
}

double BandstopFilter60Hz(double x_n) {
  /*
    50 Hz Bandstop (Notch) Filter, Butterworth IIR, 4th Order
    Direct-Form II, Single Section
    Fs = 500 Hz, Fc1 = 49 Hz, Fc2 = 51 Hz
    The filter coefficients are obtained from MATLAB filterDesigner
  */

  const int FILTER_ORDER = 4;
  static double x[FILTER_ORDER + 1] = {};
  static double y[FILTER_ORDER + 1] = {};
  double b[FILTER_ORDER + 1] = { 0.9651,   -2.8149,    3.9828,   -2.8149,    0.9651 };
  double a[FILTER_ORDER + 1] = { 1.0000,   -2.8650,    3.9816,   -2.7649,    0.9314 };

  x[0] = x_n;
  y[0] = b[0] * x[0];

  for (int i = 1; i < FILTER_ORDER + 1; i++) {
    y[0] += b[i] * x[i] - a[i] * y[i];
  }

  for (int i = FILTER_ORDER; i > 0; i--) {
    x[i] = x[i - 1];
    y[i] = y[i - 1];
  }

  return y[0];
}

double BandstopFilter65Hz(double x_n) {
  /*
    50 Hz Bandstop (Notch) Filter, Butterworth IIR, 4th Order
    Direct-Form II, Single Section
    Fs = 500 Hz, Fc1 = 49 Hz, Fc2 = 51 Hz
    The filter coefficients are obtained from MATLAB filterDesigner
  */

  const int FILTER_ORDER = 4;
  static double x[FILTER_ORDER + 1] = {};
  static double y[FILTER_ORDER + 1] = {};
  double b[FILTER_ORDER + 1] = { 0.9824,   -2.7260,    3.8558,   -2.7260,    0.9824 };
  double a[FILTER_ORDER + 1] = { 1.0000,   -2.7502,    3.8555,   -2.7017,    0.9651 };

  x[0] = x_n;
  y[0] = b[0] * x[0];

  for (int i = 1; i < FILTER_ORDER + 1; i++) {
    y[0] += b[i] * x[i] - a[i] * y[i];
  }

  for (int i = FILTER_ORDER; i > 0; i--) {
    x[i] = x[i - 1];
    y[i] = y[i - 1];
  }

  return y[0];
}