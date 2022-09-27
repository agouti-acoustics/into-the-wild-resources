#include <Arduino.h>
#include <SensirionI2CSht4x.h>
#include "TFT_eSPI.h"
#include <Wire.h>

TFT_eSPI tft;
SensirionI2CSht4x sht4x;

#define BACKCOL TFT_BLACK  // Homescreen background colour

const unsigned long FRAMES = 16000 * 5; // Number of frames of audio to return (= sampling_rate * num_seconds)

// Create project homescreen on Wio
void project(){
  tft.fillScreen(BACKCOL);
  tft.setTextColor(TFT_GREEN);
  tft.setTextSize(3);
  tft.drawString("Project AGOUTI", 35, 20);
  tft.setTextSize(2);
  tft.drawString("by TEAM ENIGMA", 100, 200);
}

void setup() {
  pinMode(WIO_MIC, INPUT);
  pinMode(WIO_LIGHT, INPUT);
  Serial.begin(115200);

  tft.begin();
  tft.setRotation(3);
  
  project();

  // ----- SHT 40 setup code ----- //
  Wire.begin();
 
  uint16_t error;
  char errorMessage[256];

  sht4x.begin(Wire);

  uint32_t serialNumber;
  error = sht4x.serialNumber(serialNumber);
} 

void loop() {
  if (Serial.available() > 0) {
    // Read string from serial
    String msg = Serial.readStringUntil('\n');

    // Get calibrated delay value between each audio frame from serial
    int delay_ms = Serial.parseInt();

    // Write message received from serial
    tft.fillRect(30, 100, 285, 60, BACKCOL);  // Erase previous outputs
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2.5);
    tft.drawString(msg, 35, 100);
    
    delay(100);
    
    // Read and return light sensor value
    int light = analogRead(WIO_LIGHT);
    Serial.print(light);
    Serial.print(',');

    // Read and return temperature and humidity
    uint16_t error;
    float temperature;
    float humidity;
    error = sht4x.measureHighPrecision(temperature, humidity);
    if (!error) {
        Serial.print(temperature);
        Serial.print(',');
        Serial.println(humidity);
    }

    // Return delay value to make sure it has been received correctly
    Serial.write(delay_ms);
    
    // Return required number of audio frames
    for (unsigned long i=0; i<FRAMES; i++) {
      int val = analogRead(WIO_MIC);
      Serial.write(val);
      delayMicroseconds(delay_ms);  
    }

    // Final digit to signal completion
    Serial.write((byte) 0);
    
    Serial.flush();
  } else {
    delay(100);
  }
}
