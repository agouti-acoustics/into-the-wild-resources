#include <SoftwareSerial.h>
#include <Arduino.h>
#include <SensirionI2CSht4x.h>
#include "TFT_eSPI.h"
#include "disk91_LoRaE5.h"
#include <Wire.h>

TFT_eSPI tft;
SensirionI2CSht4x sht4x;
Disk91_LoRaE5 lorae5(&Serial); // Where the AT command and debut traces are printed

#define BACKCOL TFT_BLACK  // Homescreen background colour
#define Frequency DSKLORAE5_ZONE_EU868 // LORA frequency settings

uint8_t deveui[] = "6081F913845B8E31";
uint8_t appeui[] = "6081F9077760534C";
uint8_t appkey[] = "46F05B094E9A4DF05F36C7913A90CE58";

const unsigned long FRAMES = 16000 * 5; // Number of frames of audio to return (= sampling_rate * num_seconds)

void data_decord(int val_1, int val_2, uint8_t data[4])
{
  int val[] = {val_1, val_2};

  for(int i = 0, j = 0; i < 2; i++, j += 2)
  {
    if(val[i] < 0)
    {
      val[i] = ~val[i] + 1;
      data[j] = val[i] >> 8 | 0x80;
      data[j+1] = val[i] & 0xFF;
    }
    else
    {
      data[j] = val[i] >> 8 & 0xFF;
      data[j+1] = val[i] & 0xFF;
    }
  }

}

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

  uint32_t start = millis();
  while ( !Serial && (millis() - start) < 1500 );  // Open the Serial Monitor to get started or wait for 1.5"

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

  // init the library, search the LORAE5 over the different WIO port available
  if ( ! lorae5.begin(DSKLORAE5_SWSERIAL_WIO_P2) ) {
    Serial.println("LoRa E5 Init Failed");
    while(1); 
  }

  // ----- LORA setup code ----- //
  // Setup the LoRaWan Credentials
  if ( ! lorae5.setup(
        Frequency,
        deveui,
        appeui,
        appkey
     ) ){
    Serial.println("LoRa E5 Setup Failed");
    while(1);         
  }
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

    // Send sensor data over LORA
    int int_temp, int_humi;
    int_temp = temperature*100;
    int_humi = humidity*100;
    static uint8_t data[4] = { 0x00 };  //Use the data[] to store the values of the sensors

    data_decord(int_temp, int_humi, data);
    
    if ( lorae5.send_sync(              //Sending the sensor values out
          8,                            // LoRaWan Port
          data,                         // data array
          sizeof(data),                 // size of the data
          false,                        // we are not expecting a ack
          7,                            // Spread Factor
          14                            // Tx Power in dBm
         ) 
    ) {
        Serial.println("Uplink done");
        if ( lorae5.isDownlinkReceived() ) {
          Serial.println("A downlink has been received");
          if ( lorae5.isDownlinkPending() ) {
            Serial.println("More downlink are pending");
          }
        }
    }
  } else {
    delay(100);
  }
}
