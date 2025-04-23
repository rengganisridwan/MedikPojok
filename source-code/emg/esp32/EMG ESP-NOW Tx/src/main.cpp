#include <Arduino.h>
/*
  EMG ESP-NOW Tx
  LstUpd: 2024-09-24
*/

// Include Libraries
#include <esp_now.h>
#include <WiFi.h>
// Libraries for SD card
#include "FS.h"
#include "SD.h"
#include <SPI.h>

const uint8_t PATIENT_ID = 1;

// MAC Address of responder - edit as required
// uint8_t broadcastAddress[] = {0xD4, 0xD4, 0xDA, 0x5D, 0x2E, 0xC0};
uint8_t broadcastAddress[] = {0xE8, 0x6B, 0xEA, 0xD4, 0xD4, 0xF4};
// uint8_t broadcastAddress[] = {0x9C, 0x9C, 0x1F, 0xE3, 0x7F, 0x10};

// Data sampling and transmission parameters
const uint16_t SAMPLING_FREQ = 1000;
const uint8_t TX_BUFFER_SIZE = 10;   // Send data to the receiver as a packet of 10
uint8_t bufferIdx = 0;
typedef struct struct_message {  // Define a data structure
  int16_t a[TX_BUFFER_SIZE];
  int16_t b[TX_BUFFER_SIZE];
} struct_message;
struct_message myData;          // Create a structured object
esp_now_peer_info_t peerInfo;   // Peer info

// LED for transmission
const uint8_t onBoardLEDPin = 2;    // Indicate the transmitted data
const uint8_t sdWriteLEDPin = 13;   // Indicate the writing process

// Buffer array for saving to SD card
const uint8_t SD_BUFFER_DUR_SECOND = 10;  // Define the length of the saved data in last n second 
const int SD_BUFFER_SIZE = SAMPLING_FREQ * SD_BUFFER_DUR_SECOND;
int16_t bufferArrayCh1[SD_BUFFER_SIZE];
int16_t bufferArrayCh2[SD_BUFFER_SIZE];
// int16_t bufferArrayCh3[SD_BUFFER_SIZE];
// int16_t bufferArrayCh4[SD_BUFFER_SIZE];
int readIdx = 1;  // readIdx is always ahead of writeIdx in the circular buffer
int writeIdx = 0;

// Preparation for push button
const uint8_t buttonPin = 27;   // the number of the pushbutton pin
const uint8_t buttonLEDPin = 4; // the number of the LED pin
int ledState = LOW;         // the current state of the output pin
int buttonState;            // the current reading from the input pin
int lastButtonState = LOW;  // the previous reading from the input pin
unsigned long lastDebounceTime = 0;           // the last time the output pin was toggled
unsigned long debounceDelayMillisecond = 50;  // the debounce time; increase if the output flickers


// ----- Function declaration -----

void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
void initSDCard();
void writeFile(fs::FS &fs, const char * path, const char * message);
void saveToSDCard(uint8_t patientID);
int bandpass1(int input);
int bandpass2(int input);
int notch50(int input);
int notch100(int input);
int notch150(int input);
int notch200(int input);
int notch250(int input);
int notch300(int input);
int notch350(int input);
int notch400(int input);
int notch450(int input);
int notch50_2(int input);
int notch100_2(int input);
int notch150_2(int input);
int notch200_2(int input);
int notch250_2(int input);
int notch300_2(int input);
int notch350_2(int input);
int notch400_2(int input);
int notch450_2(int input);

// ----- End of function declaration ----- 

void setup() {
  // Set up Serial Monitor
  Serial.begin(115200);
  pinMode(onBoardLEDPin, OUTPUT);
  pinMode(sdWriteLEDPin, OUTPUT);
  Serial.print("SD_BUFFER_SIZE = ");
  Serial.println(SD_BUFFER_SIZE);

  initSDCard();
  // If the data.txt file doesn't exist
  // Create a file on the SD card and write the data labels
  File file = SD.open("/data0.txt");
  if(!file) {
    Serial.println("File doesn't exist");
    Serial.println("Creating file...");
    writeFile(SD, "/data0.txt", "ch01\r\n");
  }
  else
    Serial.println("File already exists");
  file.close();
 
  // Set ESP32 as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Initilize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register the send callback
  esp_now_register_send_cb(onDataSent);
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }

  // Initialize button LED
  pinMode(buttonPin, INPUT);
  pinMode(buttonLEDPin, OUTPUT);

  // set initial LED state
  digitalWrite(buttonLEDPin, ledState);
}

void loop() {
  // ----- Button press to save to SD card -----
  int reading = digitalRead(buttonPin);
  if (reading != lastButtonState)
    lastDebounceTime = millis();
  if ((millis() - lastDebounceTime) > debounceDelayMillisecond) {
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == HIGH) { 
        digitalWrite(buttonLEDPin, HIGH);
        saveToSDCard(PATIENT_ID);
        digitalWrite(buttonLEDPin, LOW);
      }
    }
  }
  lastButtonState = reading;

  // ----- Periodic sampling -----
  static unsigned long tPast = 0;
  unsigned long tPresent = micros();
  unsigned long tInterval = tPresent - tPast;
  tPast = tPresent;

  static double timer = 0;
  timer -= tInterval;
  if (timer < 0) {  
    timer += 1e6 / SAMPLING_FREQ;

    // Dummy sinusoid signal
    double t = micros() / 1.0e6;
    int16_t raw1 = analogRead(34); //100*sin(2*PI*50*t) + 100*sin(2*PI*1*t);  
    raw1 = bandpass1(raw1);
    raw1 = notch50(raw1);
    raw1 = notch100(raw1);
    raw1 = notch150(raw1);
    raw1 = notch200(raw1);
    raw1 = notch250(raw1);
    raw1 = notch300(raw1);
    raw1 = notch350(raw1);
    raw1 = notch400(raw1);
    raw1 = notch450(raw1);

    int16_t raw2 = analogRead(35); //100*sin(2*PI*50*t) + 100*sin(2*PI*1*t);  
    raw2 = bandpass2(raw2);
    raw2 = notch50_2(raw2);
    raw2 = notch100_2(raw2);
    raw2 = notch150_2(raw2);
    raw2 = notch200_2(raw2);
    raw2 = notch250_2(raw2);
    raw2 = notch300_2(raw2);
    raw2 = notch350_2(raw2);
    raw2 = notch400_2(raw2);
    raw2 = notch450_2(raw2);
    // int16_t raw2 = 100*sin(2*PI*1*t);

    // Save measured data to Tx Buffer
    myData.a[bufferIdx] = raw1;
    myData.b[bufferIdx] = raw2;
    
    // Transmit the measured signal
    bufferIdx = (bufferIdx + 1) % TX_BUFFER_SIZE;
    if (bufferIdx == 0) // Transmit the signal after Tx buffer is full
      esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));

    // Save measured data to SD card buffer
    bufferArrayCh1[writeIdx] = raw1;
    bufferArrayCh2[writeIdx] = raw2;
    // bufferArrayCh3[writeIdx] = raw1;
    // bufferArrayCh4[writeIdx] = raw2;
    writeIdx = (writeIdx + 1) % (SD_BUFFER_SIZE);
    readIdx = (readIdx + 1) % (SD_BUFFER_SIZE);
  }
  // ----- End of periodic sampling -----
}

// ----- Function definition -----

// Callback function called when data is sent
void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  if (status == ESP_NOW_SEND_SUCCESS) // Uses ESP32 internal LED
    digitalWrite(onBoardLEDPin, HIGH);
  else
    digitalWrite(onBoardLEDPin, LOW);
}

/** @brief
 *  Initialize SD card.
 * @return Void.
*/
void initSDCard(){
 
  if (!SD.begin()) {
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
    Serial.println("No SD card attached");
    return;
  }
  Serial.print("SD Card Type: ");
  if(cardType == CARD_MMC){
    Serial.println("MMC");
  } else if(cardType == CARD_SD){
    Serial.println("SDSC");
  } else if(cardType == CARD_SDHC){
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);
}

// Write to the SD card
void writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if(file.print(message))
    Serial.println("File written");
  else
    Serial.println("Write failed");
  file.close();
}

void saveToSDCard(uint8_t patientID) {
  static uint8_t fileIdx = 1;
  char filename[30];

  File dataFile;
  sprintf(filename, "/p%02u-%02u.txt", patientID, fileIdx);

  Serial.println(readIdx);
  Serial.println(filename);
  Serial.println("");

  // Save to SD
  for (int n = 0; n < SD_BUFFER_SIZE; n++) {
    int i;
    if (n == 0) {
      dataFile = SD.open(filename, FILE_WRITE);
      i = readIdx;
    }
    if (i == SD_BUFFER_SIZE)
      i = 0;
    String dataMsg = String(bufferArrayCh1[i]) + "," + String(bufferArrayCh2[i]);
    dataFile.println(dataMsg);
    // Serial.println(dataMsg);
    i = i+1;
  }
  dataFile.close();
  fileIdx = fileIdx + 1;
  
  // Reinitialize the buffer
  for(int n = 0; n < SD_BUFFER_SIZE; n++) {
    bufferArrayCh1[n] = 0;
    bufferArrayCh2[n] = 0;
    // bufferArrayCh3[n] = 0;
    // bufferArrayCh4[n] = 0;
  }
}

/** @brief
 *  IIR Butterworth Bandpass Filter, 4th order.
 *  Cutoff frequency: 20-480 Hz.
 *  Written in second order sections.
 * 
 *  @param input Input signal.
 *  @return Output signal.
 */
int bandpass1(int input) {
  const double sos[2][6] = {
    1,            0,           -1,            1,      -1.8226,       0.8372,
    1,            0,           -1,            1,       1.8226,       0.8372
    // 1, -2, 1, 1, -1.9492, 0.95307,
    // 1, -2, 1, 1, -1.8866, 0.89034
  };
  // Gain matrix, obtained from MATLAB
  const double gain[3] = {0.91493, 0.91493, 1};
  
  // Filter coefficients for the first section
  const double b1[3] = {sos[0][0], sos[0][1], sos[0][2]};
  const double a1[3] = {sos[0][3], sos[0][4], sos[0][5]};
  
  // Filter coefficients for the second section
  const double b2[3] = {sos[1][0], sos[1][1], sos[1][2]};
  const double a2[3] = {sos[1][3], sos[1][4], sos[1][5]};
  int x_n = input;
  
  { // First section
    static double x[3] = {};
    static double y[3] = {};

    x[0] = x_n * gain[0];
    y[0] = b1[0] * x[0];

    for (int i = 1; i < 3; i++)
      y[0] += b1[i] * x[i] - a1[i] * y[i];
    for (int i = 2; i > 0; i--) {
      x[i] = x[i - 1];
      y[i] = y[i - 1];
    }
    x_n = y[0];
  }
  { // Second section
    static double x[3] = {};
    static double y[3] = {};

    x[0] = x_n * gain[1];
    y[0] = b2[0] * x[0];

    for (int i = 1; i < 3; i++)
      y[0] += b2[i] * x[i] - a2[i] * y[i];
    for (int i = 2; i > 0; i--) {
      x[i] = x[i - 1];
      y[i] = y[i - 1];
    }
    x_n = y[0];
  }
  return x_n * gain[2];
}

int bandpass2(int input) {
  const double sos[2][6] = {
    1,            0,           -1,            1,      -1.8226,       0.8372,
    1,            0,           -1,            1,       1.8226,       0.8372
    // 1, -2, 1, 1, -1.9492, 0.95307,
    // 1, -2, 1, 1, -1.8866, 0.89034
  };
  // Gain matrix, obtained from MATLAB
  const double gain[3] = {0.91493, 0.91493, 1};
  
  // Filter coefficients for the first section
  const double b1[3] = {sos[0][0], sos[0][1], sos[0][2]};
  const double a1[3] = {sos[0][3], sos[0][4], sos[0][5]};
  
  // Filter coefficients for the second section
  const double b2[3] = {sos[1][0], sos[1][1], sos[1][2]};
  const double a2[3] = {sos[1][3], sos[1][4], sos[1][5]};
  int x_n = input;
  
  { // First section
    static double x[3] = {};
    static double y[3] = {};

    x[0] = x_n * gain[0];
    y[0] = b1[0] * x[0];

    for (int i = 1; i < 3; i++)
      y[0] += b1[i] * x[i] - a1[i] * y[i];
    for (int i = 2; i > 0; i--) {
      x[i] = x[i - 1];
      y[i] = y[i - 1];
    }
    x_n = y[0];
  }
  { // Second section
    static double x[3] = {};
    static double y[3] = {};

    x[0] = x_n * gain[1];
    y[0] = b2[0] * x[0];

    for (int i = 1; i < 3; i++)
      y[0] += b2[i] * x[i] - a2[i] * y[i];
    for (int i = 2; i > 0; i--) {
      x[i] = x[i - 1];
      y[i] = y[i - 1];
    }
    x_n = y[0];
  }
  return x_n * gain[2];
}

/** @brief
 *  IIR Butterworth Notch Filter, 4th order.
 *  Bandstop frequency: 48-52 Hz.
 *  Written in second order sections.
 * 
 *  @param input Input signal.
 *  @return Output signal.
 */
int notch50( int input) {
 // SOS matrix, obtained from MATLAB
  const double sos[2][6] = {
    1, -1.9023, 1, 1, -1.8795, 0.98191,
    1, -1.9023, 1, 1, -1.8913, 0.98286
  };
  // Gain matrix, obtained from MATLAB
  const double gain[3] = {0.99115, 0.99115, 1};
  
  // Filter coefficients for the first section
  const double b1[3] = {sos[0][0], sos[0][1], sos[0][2]};
  const double a1[3] = {sos[0][3], sos[0][4], sos[0][5]};
  
  // Filter coefficients for the second section
  const double b2[3] = {sos[1][0], sos[1][1], sos[1][2]};
  const double a2[3] = {sos[1][3], sos[1][4], sos[1][5]};
  int x_n = input;
  
  { // First section
    static double x[3] = {};
    static double y[3] = {};

    x[0] = x_n * gain[0];
    y[0] = b1[0] * x[0];

    for (int i = 1; i < 3; i++) {
      y[0] += b1[i] * x[i] - a1[i] * y[i];
    }
    for (int i = 2; i > 0; i--) {
      x[i] = x[i - 1];
      y[i] = y[i - 1];
    }
    x_n = y[0];
  }
  { // Second section
    static double x[3] = {};
    static double y[3] = {};

    x[0] = x_n * gain[1];
    y[0] = b2[0] * x[0];

    for (int i = 1; i < 3; i++) {
      y[0] += b2[i] * x[i] - a2[i] * y[i];
    }
    for (int i = 2; i > 0; i--) {
      x[i] = x[i - 1];
      y[i] = y[i - 1];
    }
    x_n = y[0];
  }
  return x_n * gain[2];
}

int notch100( int input) {
 // SOS matrix, obtained from MATLAB
  const double sos[2][6] = {
    1, -1.6182, 1, 1, -1.5933, 0.98217,
    1, -1.6182, 1, 1, -1.6143, 0.9826
  };
  // Gain matrix, obtained from MATLAB
  const double gain[3] = {0.99115, 0.99115, 1};
  
  // Filter coefficients for the first section
  const double b1[3] = {sos[0][0], sos[0][1], sos[0][2]};
  const double a1[3] = {sos[0][3], sos[0][4], sos[0][5]};
  
  // Filter coefficients for the second section
  const double b2[3] = {sos[1][0], sos[1][1], sos[1][2]};
  const double a2[3] = {sos[1][3], sos[1][4], sos[1][5]};
  int x_n = input;
  
  { // First section
    static double x[3] = {};
    static double y[3] = {};

    x[0] = x_n * gain[0];
    y[0] = b1[0] * x[0];

    for (int i = 1; i < 3; i++) {
      y[0] += b1[i] * x[i] - a1[i] * y[i];
    }
    for (int i = 2; i > 0; i--) {
      x[i] = x[i - 1];
      y[i] = y[i - 1];
    }
    x_n = y[0];
  }
  { // Second section
    static double x[3] = {};
    static double y[3] = {};

    x[0] = x_n * gain[1];
    y[0] = b2[0] * x[0];

    for (int i = 1; i < 3; i++) {
      y[0] += b2[i] * x[i] - a2[i] * y[i];
    }
    for (int i = 2; i > 0; i--) {
      x[i] = x[i - 1];
      y[i] = y[i - 1];
    }
    x_n = y[0];
  }
  return x_n * gain[2];
}

int notch150( int input) {
 // SOS matrix, obtained from MATLAB
  const double sos[2][6] = {
    1, -1.1757, 1, 1, -1.1509, 0.98227,
    1, -1.1757, 1, 1, -1.1795, 0.9825
  };
  // Gain matrix, obtained from MATLAB
  const double gain[3] = {0.99115, 0.99115, 1};
  
  // Filter coefficients for the first section
  const double b1[3] = {sos[0][0], sos[0][1], sos[0][2]};
  const double a1[3] = {sos[0][3], sos[0][4], sos[0][5]};
  
  // Filter coefficients for the second section
  const double b2[3] = {sos[1][0], sos[1][1], sos[1][2]};
  const double a2[3] = {sos[1][3], sos[1][4], sos[1][5]};
  int x_n = input;
  
  { // First section
    static double x[3] = {};
    static double y[3] = {};

    x[0] = x_n * gain[0];
    y[0] = b1[0] * x[0];

    for (int i = 1; i < 3; i++) {
      y[0] += b1[i] * x[i] - a1[i] * y[i];
    }
    for (int i = 2; i > 0; i--) {
      x[i] = x[i - 1];
      y[i] = y[i - 1];
    }
    x_n = y[0];
  }
  { // Second section
    static double x[3] = {};
    static double y[3] = {};

    x[0] = x_n * gain[1];
    y[0] = b2[0] * x[0];

    for (int i = 1; i < 3; i++) {
      y[0] += b2[i] * x[i] - a2[i] * y[i];
    }
    for (int i = 2; i > 0; i--) {
      x[i] = x[i - 1];
      y[i] = y[i - 1];
    }
    x_n = y[0];
  }
  return x_n * gain[2];
}

int notch200( int input) {
 // SOS matrix, obtained from MATLAB
  const double sos[2][6] = {
    1, -0.61808, 1, 1, -0.59582, 0.98234,
    1, -0.61808, 1, 1, -0.62936, 0.98244
  };
  // Gain matrix, obtained from MATLAB
  const double gain[3] = {0.99115, 0.99115, 1};
  
  // Filter coefficients for the first section
  const double b1[3] = {sos[0][0], sos[0][1], sos[0][2]};
  const double a1[3] = {sos[0][3], sos[0][4], sos[0][5]};
  
  // Filter coefficients for the second section
  const double b2[3] = {sos[1][0], sos[1][1], sos[1][2]};
  const double a2[3] = {sos[1][3], sos[1][4], sos[1][5]};
  int x_n = input;
  
  { // First section
    static double x[3] = {};
    static double y[3] = {};

    x[0] = x_n * gain[0];
    y[0] = b1[0] * x[0];

    for (int i = 1; i < 3; i++) {
      y[0] += b1[i] * x[i] - a1[i] * y[i];
    }
    for (int i = 2; i > 0; i--) {
      x[i] = x[i - 1];
      y[i] = y[i - 1];
    }
    x_n = y[0];
  }
  { // Second section
    static double x[3] = {};
    static double y[3] = {};

    x[0] = x_n * gain[1];
    y[0] = b2[0] * x[0];

    for (int i = 1; i < 3; i++) {
      y[0] += b2[i] * x[i] - a2[i] * y[i];
    }
    for (int i = 2; i > 0; i--) {
      x[i] = x[i - 1];
      y[i] = y[i - 1];
    }
    x_n = y[0];
  }
  return x_n * gain[2];
}

int notch250( int input) {
 // SOS matrix, obtained from MATLAB
  const double sos[2][6] = {
    1,  -1.2247e-16, 1, 1,  0.017616, 0.98239,
    1,  -1.2247e-16, 1, 1, -0.017616, 0.98239
  };
  // Gain matrix, obtained from MATLAB
  const double gain[3] = {0.99115, 0.99115, 1};
  
  // Filter coefficients for the first section
  const double b1[3] = {sos[0][0], sos[0][1], sos[0][2]};
  const double a1[3] = {sos[0][3], sos[0][4], sos[0][5]};
  
  // Filter coefficients for the second section
  const double b2[3] = {sos[1][0], sos[1][1], sos[1][2]};
  const double a2[3] = {sos[1][3], sos[1][4], sos[1][5]};
  int x_n = input;
  
  { // First section
    static double x[3] = {};
    static double y[3] = {};

    x[0] = x_n * gain[0];
    y[0] = b1[0] * x[0];

    for (int i = 1; i < 3; i++) {
      y[0] += b1[i] * x[i] - a1[i] * y[i];
    }
    for (int i = 2; i > 0; i--) {
      x[i] = x[i - 1];
      y[i] = y[i - 1];
    }
    x_n = y[0];
  }
  { // Second section
    static double x[3] = {};
    static double y[3] = {};

    x[0] = x_n * gain[1];
    y[0] = b2[0] * x[0];

    for (int i = 1; i < 3; i++) {
      y[0] += b2[i] * x[i] - a2[i] * y[i];
    }
    for (int i = 2; i > 0; i--) {
      x[i] = x[i - 1];
      y[i] = y[i - 1];
    }
    x_n = y[0];
  }
  return x_n * gain[2];
}

int notch300( int input) {
 // SOS matrix, obtained from MATLAB
  const double sos[2][6] = {
    1, 0.61808, 1, 1, 0.59582, 0.98234,
    1, 0.61808, 1, 1, 0.62936, 0.98244
  };
  // Gain matrix, obtained from MATLAB
  const double gain[3] = {0.99115, 0.99115, 1};
  // Filter coefficients for the first section
  const double b1[3] = {sos[0][0], sos[0][1], sos[0][2]};
  const double a1[3] = {sos[0][3], sos[0][4], sos[0][5]};
  // Filter coefficients for the second section
  const double b2[3] = {sos[1][0], sos[1][1], sos[1][2]};
  const double a2[3] = {sos[1][3], sos[1][4], sos[1][5]};
  int x_n = input;
  
  { // First section
    static double x[3] = {};
    static double y[3] = {};
    x[0] = x_n * gain[0];
    y[0] = b1[0] * x[0];
    for (int i = 1; i < 3; i++) {
      y[0] += b1[i] * x[i] - a1[i] * y[i];
    }
    for (int i = 2; i > 0; i--) {
      x[i] = x[i - 1];
      y[i] = y[i - 1];
    }
    x_n = y[0];
  }
  { // Second section
    static double x[3] = {};
    static double y[3] = {};
    x[0] = x_n * gain[1];
    y[0] = b2[0] * x[0];
    for (int i = 1; i < 3; i++) {
      y[0] += b2[i] * x[i] - a2[i] * y[i];
    }
    for (int i = 2; i > 0; i--) {
      x[i] = x[i - 1];
      y[i] = y[i - 1];
    }
    x_n = y[0];
  }
  return x_n * gain[2];
}

int notch350( int input) {
 // SOS matrix, obtained from MATLAB
  const double sos[2][6] = {
    1, 1.1757, 1, 1, 1.1509, 0.98227,
    1, 1.1757, 1, 1, 1.1795, 0.9825
  };
  // Gain matrix, obtained from MATLAB
  const double gain[3] = {0.99115, 0.99115, 1};
  
  // Filter coefficients for the first section
  const double b1[3] = {sos[0][0], sos[0][1], sos[0][2]};
  const double a1[3] = {sos[0][3], sos[0][4], sos[0][5]};
  
  // Filter coefficients for the second section
  const double b2[3] = {sos[1][0], sos[1][1], sos[1][2]};
  const double a2[3] = {sos[1][3], sos[1][4], sos[1][5]};
  int x_n = input;
  
  { // First section
    static double x[3] = {};
    static double y[3] = {};

    x[0] = x_n * gain[0];
    y[0] = b1[0] * x[0];

    for (int i = 1; i < 3; i++) {
      y[0] += b1[i] * x[i] - a1[i] * y[i];
    }
    for (int i = 2; i > 0; i--) {
      x[i] = x[i - 1];
      y[i] = y[i - 1];
    }
    x_n = y[0];
  }
  { // Second section
    static double x[3] = {};
    static double y[3] = {};

    x[0] = x_n * gain[1];
    y[0] = b2[0] * x[0];

    for (int i = 1; i < 3; i++) {
      y[0] += b2[i] * x[i] - a2[i] * y[i];
    }
    for (int i = 2; i > 0; i--) {
      x[i] = x[i - 1];
      y[i] = y[i - 1];
    }
    x_n = y[0];
  }
  return x_n * gain[2];
}

int notch400( int input) {
 // SOS matrix, obtained from MATLAB
  const double sos[2][6] = {
    1,1.6182, 1, 1, 1.5933, 0.98217,
    1,1.6182, 1, 1, 1.6143, 0.9826
  };
  // Gain matrix, obtained from MATLAB
  const double gain[3] = {0.99115, 0.99115, 1};
  
  // Filter coefficients for the first section
  const double b1[3] = {sos[0][0], sos[0][1], sos[0][2]};
  const double a1[3] = {sos[0][3], sos[0][4], sos[0][5]};
  
  // Filter coefficients for the second section
  const double b2[3] = {sos[1][0], sos[1][1], sos[1][2]};
  const double a2[3] = {sos[1][3], sos[1][4], sos[1][5]};
  int x_n = input;
  
  { // First section
    static double x[3] = {};
    static double y[3] = {};

    x[0] = x_n * gain[0];
    y[0] = b1[0] * x[0];

    for (int i = 1; i < 3; i++) {
      y[0] += b1[i] * x[i] - a1[i] * y[i];
    }
    for (int i = 2; i > 0; i--) {
      x[i] = x[i - 1];
      y[i] = y[i - 1];
    }
    x_n = y[0];
  }
  { // Second section
    static double x[3] = {};
    static double y[3] = {};

    x[0] = x_n * gain[1];
    y[0] = b2[0] * x[0];

    for (int i = 1; i < 3; i++) {
      y[0] += b2[i] * x[i] - a2[i] * y[i];
    }
    for (int i = 2; i > 0; i--) {
      x[i] = x[i - 1];
      y[i] = y[i - 1];
    }
    x_n = y[0];
  }
  return x_n * gain[2];
}

int notch450( int input) {
 // SOS matrix, obtained from MATLAB
  const double sos[2][6] = {
    1, 1.9023, 1, 1, 1.8795, 0.98191,
    1, 1.9023, 1, 1, 1.8913, 0.98286
  };
  // Gain matrix, obtained from MATLAB
  const double gain[3] = {0.99115, 0.99115, 1};
  
  // Filter coefficients for the first section
  const double b1[3] = {sos[0][0], sos[0][1], sos[0][2]};
  const double a1[3] = {sos[0][3], sos[0][4], sos[0][5]};
  
  // Filter coefficients for the second section
  const double b2[3] = {sos[1][0], sos[1][1], sos[1][2]};
  const double a2[3] = {sos[1][3], sos[1][4], sos[1][5]};
  int x_n = input;
  
  { // First section
    static double x[3] = {};
    static double y[3] = {};

    x[0] = x_n * gain[0];
    y[0] = b1[0] * x[0];

    for (int i = 1; i < 3; i++) {
      y[0] += b1[i] * x[i] - a1[i] * y[i];
    }
    for (int i = 2; i > 0; i--) {
      x[i] = x[i - 1];
      y[i] = y[i - 1];
    }
    x_n = y[0];
  }
  { // Second section
    static double x[3] = {};
    static double y[3] = {};

    x[0] = x_n * gain[1];
    y[0] = b2[0] * x[0];

    for (int i = 1; i < 3; i++) {
      y[0] += b2[i] * x[i] - a2[i] * y[i];
    }
    for (int i = 2; i > 0; i--) {
      x[i] = x[i - 1];
      y[i] = y[i - 1];
    }
    x_n = y[0];
  }
  return x_n * gain[2];
}




int notch50_2( int input) {
 // SOS matrix, obtained from MATLAB
  const double sos[2][6] = {
    1, -1.9023, 1, 1, -1.8795, 0.98191,
    1, -1.9023, 1, 1, -1.8913, 0.98286
  };
  // Gain matrix, obtained from MATLAB
  const double gain[3] = {0.99115, 0.99115, 1};
  
  // Filter coefficients for the first section
  const double b1[3] = {sos[0][0], sos[0][1], sos[0][2]};
  const double a1[3] = {sos[0][3], sos[0][4], sos[0][5]};
  
  // Filter coefficients for the second section
  const double b2[3] = {sos[1][0], sos[1][1], sos[1][2]};
  const double a2[3] = {sos[1][3], sos[1][4], sos[1][5]};
  int x_n = input;
  
  { // First section
    static double x[3] = {};
    static double y[3] = {};

    x[0] = x_n * gain[0];
    y[0] = b1[0] * x[0];

    for (int i = 1; i < 3; i++) {
      y[0] += b1[i] * x[i] - a1[i] * y[i];
    }
    for (int i = 2; i > 0; i--) {
      x[i] = x[i - 1];
      y[i] = y[i - 1];
    }
    x_n = y[0];
  }
  { // Second section
    static double x[3] = {};
    static double y[3] = {};

    x[0] = x_n * gain[1];
    y[0] = b2[0] * x[0];

    for (int i = 1; i < 3; i++) {
      y[0] += b2[i] * x[i] - a2[i] * y[i];
    }
    for (int i = 2; i > 0; i--) {
      x[i] = x[i - 1];
      y[i] = y[i - 1];
    }
    x_n = y[0];
  }
  return x_n * gain[2];
}

int notch100_2( int input) {
 // SOS matrix, obtained from MATLAB
  const double sos[2][6] = {
    1, -1.6182, 1, 1, -1.5933, 0.98217,
    1, -1.6182, 1, 1, -1.6143, 0.9826
  };
  // Gain matrix, obtained from MATLAB
  const double gain[3] = {0.99115, 0.99115, 1};
  
  // Filter coefficients for the first section
  const double b1[3] = {sos[0][0], sos[0][1], sos[0][2]};
  const double a1[3] = {sos[0][3], sos[0][4], sos[0][5]};
  
  // Filter coefficients for the second section
  const double b2[3] = {sos[1][0], sos[1][1], sos[1][2]};
  const double a2[3] = {sos[1][3], sos[1][4], sos[1][5]};
  int x_n = input;
  
  { // First section
    static double x[3] = {};
    static double y[3] = {};

    x[0] = x_n * gain[0];
    y[0] = b1[0] * x[0];

    for (int i = 1; i < 3; i++) {
      y[0] += b1[i] * x[i] - a1[i] * y[i];
    }
    for (int i = 2; i > 0; i--) {
      x[i] = x[i - 1];
      y[i] = y[i - 1];
    }
    x_n = y[0];
  }
  { // Second section
    static double x[3] = {};
    static double y[3] = {};

    x[0] = x_n * gain[1];
    y[0] = b2[0] * x[0];

    for (int i = 1; i < 3; i++) {
      y[0] += b2[i] * x[i] - a2[i] * y[i];
    }
    for (int i = 2; i > 0; i--) {
      x[i] = x[i - 1];
      y[i] = y[i - 1];
    }
    x_n = y[0];
  }
  return x_n * gain[2];
}

int notch150_2( int input) {
 // SOS matrix, obtained from MATLAB
  const double sos[2][6] = {
    1, -1.1757, 1, 1, -1.1509, 0.98227,
    1, -1.1757, 1, 1, -1.1795, 0.9825
  };
  // Gain matrix, obtained from MATLAB
  const double gain[3] = {0.99115, 0.99115, 1};
  
  // Filter coefficients for the first section
  const double b1[3] = {sos[0][0], sos[0][1], sos[0][2]};
  const double a1[3] = {sos[0][3], sos[0][4], sos[0][5]};
  
  // Filter coefficients for the second section
  const double b2[3] = {sos[1][0], sos[1][1], sos[1][2]};
  const double a2[3] = {sos[1][3], sos[1][4], sos[1][5]};
  int x_n = input;
  
  { // First section
    static double x[3] = {};
    static double y[3] = {};

    x[0] = x_n * gain[0];
    y[0] = b1[0] * x[0];

    for (int i = 1; i < 3; i++) {
      y[0] += b1[i] * x[i] - a1[i] * y[i];
    }
    for (int i = 2; i > 0; i--) {
      x[i] = x[i - 1];
      y[i] = y[i - 1];
    }
    x_n = y[0];
  }
  { // Second section
    static double x[3] = {};
    static double y[3] = {};

    x[0] = x_n * gain[1];
    y[0] = b2[0] * x[0];

    for (int i = 1; i < 3; i++) {
      y[0] += b2[i] * x[i] - a2[i] * y[i];
    }
    for (int i = 2; i > 0; i--) {
      x[i] = x[i - 1];
      y[i] = y[i - 1];
    }
    x_n = y[0];
  }
  return x_n * gain[2];
}

int notch200_2( int input) {
 // SOS matrix, obtained from MATLAB
  const double sos[2][6] = {
    1, -0.61808, 1, 1, -0.59582, 0.98234,
    1, -0.61808, 1, 1, -0.62936, 0.98244
  };
  // Gain matrix, obtained from MATLAB
  const double gain[3] = {0.99115, 0.99115, 1};
  
  // Filter coefficients for the first section
  const double b1[3] = {sos[0][0], sos[0][1], sos[0][2]};
  const double a1[3] = {sos[0][3], sos[0][4], sos[0][5]};
  
  // Filter coefficients for the second section
  const double b2[3] = {sos[1][0], sos[1][1], sos[1][2]};
  const double a2[3] = {sos[1][3], sos[1][4], sos[1][5]};
  int x_n = input;
  
  { // First section
    static double x[3] = {};
    static double y[3] = {};

    x[0] = x_n * gain[0];
    y[0] = b1[0] * x[0];

    for (int i = 1; i < 3; i++) {
      y[0] += b1[i] * x[i] - a1[i] * y[i];
    }
    for (int i = 2; i > 0; i--) {
      x[i] = x[i - 1];
      y[i] = y[i - 1];
    }
    x_n = y[0];
  }
  { // Second section
    static double x[3] = {};
    static double y[3] = {};

    x[0] = x_n * gain[1];
    y[0] = b2[0] * x[0];

    for (int i = 1; i < 3; i++) {
      y[0] += b2[i] * x[i] - a2[i] * y[i];
    }
    for (int i = 2; i > 0; i--) {
      x[i] = x[i - 1];
      y[i] = y[i - 1];
    }
    x_n = y[0];
  }
  return x_n * gain[2];
}

int notch250_2( int input) {
 // SOS matrix, obtained from MATLAB
  const double sos[2][6] = {
    1,  -1.2247e-16, 1, 1,  0.017616, 0.98239,
    1,  -1.2247e-16, 1, 1, -0.017616, 0.98239
  };
  // Gain matrix, obtained from MATLAB
  const double gain[3] = {0.99115, 0.99115, 1};
  
  // Filter coefficients for the first section
  const double b1[3] = {sos[0][0], sos[0][1], sos[0][2]};
  const double a1[3] = {sos[0][3], sos[0][4], sos[0][5]};
  
  // Filter coefficients for the second section
  const double b2[3] = {sos[1][0], sos[1][1], sos[1][2]};
  const double a2[3] = {sos[1][3], sos[1][4], sos[1][5]};
  int x_n = input;
  
  { // First section
    static double x[3] = {};
    static double y[3] = {};

    x[0] = x_n * gain[0];
    y[0] = b1[0] * x[0];

    for (int i = 1; i < 3; i++) {
      y[0] += b1[i] * x[i] - a1[i] * y[i];
    }
    for (int i = 2; i > 0; i--) {
      x[i] = x[i - 1];
      y[i] = y[i - 1];
    }
    x_n = y[0];
  }
  { // Second section
    static double x[3] = {};
    static double y[3] = {};

    x[0] = x_n * gain[1];
    y[0] = b2[0] * x[0];

    for (int i = 1; i < 3; i++) {
      y[0] += b2[i] * x[i] - a2[i] * y[i];
    }
    for (int i = 2; i > 0; i--) {
      x[i] = x[i - 1];
      y[i] = y[i - 1];
    }
    x_n = y[0];
  }
  return x_n * gain[2];
}

int notch300_2( int input) {
 // SOS matrix, obtained from MATLAB
  const double sos[2][6] = {
    1, 0.61808, 1, 1, 0.59582, 0.98234,
    1, 0.61808, 1, 1, 0.62936, 0.98244
  };
  // Gain matrix, obtained from MATLAB
  const double gain[3] = {0.99115, 0.99115, 1};
  // Filter coefficients for the first section
  const double b1[3] = {sos[0][0], sos[0][1], sos[0][2]};
  const double a1[3] = {sos[0][3], sos[0][4], sos[0][5]};
  // Filter coefficients for the second section
  const double b2[3] = {sos[1][0], sos[1][1], sos[1][2]};
  const double a2[3] = {sos[1][3], sos[1][4], sos[1][5]};
  int x_n = input;
  
  { // First section
    static double x[3] = {};
    static double y[3] = {};
    x[0] = x_n * gain[0];
    y[0] = b1[0] * x[0];
    for (int i = 1; i < 3; i++) {
      y[0] += b1[i] * x[i] - a1[i] * y[i];
    }
    for (int i = 2; i > 0; i--) {
      x[i] = x[i - 1];
      y[i] = y[i - 1];
    }
    x_n = y[0];
  }
  { // Second section
    static double x[3] = {};
    static double y[3] = {};
    x[0] = x_n * gain[1];
    y[0] = b2[0] * x[0];
    for (int i = 1; i < 3; i++) {
      y[0] += b2[i] * x[i] - a2[i] * y[i];
    }
    for (int i = 2; i > 0; i--) {
      x[i] = x[i - 1];
      y[i] = y[i - 1];
    }
    x_n = y[0];
  }
  return x_n * gain[2];
}

int notch350_2( int input) {
 // SOS matrix, obtained from MATLAB
  const double sos[2][6] = {
    1, 1.1757, 1, 1, 1.1509, 0.98227,
    1, 1.1757, 1, 1, 1.1795, 0.9825
  };
  // Gain matrix, obtained from MATLAB
  const double gain[3] = {0.99115, 0.99115, 1};
  
  // Filter coefficients for the first section
  const double b1[3] = {sos[0][0], sos[0][1], sos[0][2]};
  const double a1[3] = {sos[0][3], sos[0][4], sos[0][5]};
  
  // Filter coefficients for the second section
  const double b2[3] = {sos[1][0], sos[1][1], sos[1][2]};
  const double a2[3] = {sos[1][3], sos[1][4], sos[1][5]};
  int x_n = input;
  
  { // First section
    static double x[3] = {};
    static double y[3] = {};

    x[0] = x_n * gain[0];
    y[0] = b1[0] * x[0];

    for (int i = 1; i < 3; i++) {
      y[0] += b1[i] * x[i] - a1[i] * y[i];
    }
    for (int i = 2; i > 0; i--) {
      x[i] = x[i - 1];
      y[i] = y[i - 1];
    }
    x_n = y[0];
  }
  { // Second section
    static double x[3] = {};
    static double y[3] = {};

    x[0] = x_n * gain[1];
    y[0] = b2[0] * x[0];

    for (int i = 1; i < 3; i++) {
      y[0] += b2[i] * x[i] - a2[i] * y[i];
    }
    for (int i = 2; i > 0; i--) {
      x[i] = x[i - 1];
      y[i] = y[i - 1];
    }
    x_n = y[0];
  }
  return x_n * gain[2];
}

int notch400_2( int input) {
 // SOS matrix, obtained from MATLAB
  const double sos[2][6] = {
    1,1.6182, 1, 1, 1.5933, 0.98217,
    1,1.6182, 1, 1, 1.6143, 0.9826
  };
  // Gain matrix, obtained from MATLAB
  const double gain[3] = {0.99115, 0.99115, 1};
  
  // Filter coefficients for the first section
  const double b1[3] = {sos[0][0], sos[0][1], sos[0][2]};
  const double a1[3] = {sos[0][3], sos[0][4], sos[0][5]};
  
  // Filter coefficients for the second section
  const double b2[3] = {sos[1][0], sos[1][1], sos[1][2]};
  const double a2[3] = {sos[1][3], sos[1][4], sos[1][5]};
  int x_n = input;
  
  { // First section
    static double x[3] = {};
    static double y[3] = {};

    x[0] = x_n * gain[0];
    y[0] = b1[0] * x[0];

    for (int i = 1; i < 3; i++) {
      y[0] += b1[i] * x[i] - a1[i] * y[i];
    }
    for (int i = 2; i > 0; i--) {
      x[i] = x[i - 1];
      y[i] = y[i - 1];
    }
    x_n = y[0];
  }
  { // Second section
    static double x[3] = {};
    static double y[3] = {};

    x[0] = x_n * gain[1];
    y[0] = b2[0] * x[0];

    for (int i = 1; i < 3; i++) {
      y[0] += b2[i] * x[i] - a2[i] * y[i];
    }
    for (int i = 2; i > 0; i--) {
      x[i] = x[i - 1];
      y[i] = y[i - 1];
    }
    x_n = y[0];
  }
  return x_n * gain[2];
}

int notch450_2( int input) {
 // SOS matrix, obtained from MATLAB
  const double sos[2][6] = {
    1, 1.9023, 1, 1, 1.8795, 0.98191,
    1, 1.9023, 1, 1, 1.8913, 0.98286
  };
  // Gain matrix, obtained from MATLAB
  const double gain[3] = {0.99115, 0.99115, 1};
  
  // Filter coefficients for the first section
  const double b1[3] = {sos[0][0], sos[0][1], sos[0][2]};
  const double a1[3] = {sos[0][3], sos[0][4], sos[0][5]};
  
  // Filter coefficients for the second section
  const double b2[3] = {sos[1][0], sos[1][1], sos[1][2]};
  const double a2[3] = {sos[1][3], sos[1][4], sos[1][5]};
  int x_n = input;
  
  { // First section
    static double x[3] = {};
    static double y[3] = {};

    x[0] = x_n * gain[0];
    y[0] = b1[0] * x[0];

    for (int i = 1; i < 3; i++) {
      y[0] += b1[i] * x[i] - a1[i] * y[i];
    }
    for (int i = 2; i > 0; i--) {
      x[i] = x[i - 1];
      y[i] = y[i - 1];
    }
    x_n = y[0];
  }
  { // Second section
    static double x[3] = {};
    static double y[3] = {};

    x[0] = x_n * gain[1];
    y[0] = b2[0] * x[0];

    for (int i = 1; i < 3; i++) {
      y[0] += b2[i] * x[i] - a2[i] * y[i];
    }
    for (int i = 2; i > 0; i--) {
      x[i] = x[i - 1];
      y[i] = y[i - 1];
    }
    x_n = y[0];
  }
  return x_n * gain[2];
}