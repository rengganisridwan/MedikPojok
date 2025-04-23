#include <Arduino.h>
/*
  ESP-NOW Demo - Receive
  esp-now-demo-rcv.ino
  Reads data from Initiator
  
  DroneBot Workshop 2022
  https://dronebotworkshop.com
*/

// Include Libraries
#include <esp_now.h>
#include <WiFi.h>

// Define a data structure
// typedef struct struct_message {
//   char a[32];
//   int b;
//   float c;
//   bool d;
//   int e[5];
// } struct_message;

const int BUFFER_SIZE = 10; // this should match the buffer size on the tx side
// int myData[BUFFER_SIZE];

typedef struct struct_message {
  int16_t a[BUFFER_SIZE];
  int16_t b[BUFFER_SIZE];
} struct_message;

// Create a structured object
struct_message myData;



// Callback function executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  digitalWrite(2, HIGH);
  for (int i = 0; i < BUFFER_SIZE; i++) {
    Serial.print(myData.a[i]);
    Serial.print(" ");
    Serial.println(myData.b[i]);
  }
  digitalWrite(2, LOW);
}

void setup() {
  // Set up Serial Monitor
  Serial.begin(115200);
  pinMode(2,OUTPUT);
  
  // Set ESP32 as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Initilize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Register callback function
  esp_now_register_recv_cb(OnDataRecv);
}
 
void loop() {
}
