#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include "SdFat.h"

// Barometer Pins
#define BMP_SCK 10
#define BMP_MISO 7 //SDO
#define BMP_MOSI 9 //SDI 
#define BMP_CS 8 

// SD Pins
#define SD_SCK 4
#define SD_MISO 2 
#define SD_MOSI 3
#define SD_CS 5

// Interval between data records in milliseconds.
// The interval must be greater than the maximum SD write latency plus the
// time to acquire and write data to the SD to avoid overrun errors.
// Run the bench example to check the quality of your SD card.
const uint32_t SAMPLE_INTERVAL_MS = 1000;

// Log file base name.  Must be six characters or Fewer.
#define FILE_BASE_NAME "Data"

Adafruit_BMP280 bme(BMP_CS, BMP_MOSI, BMP_MISO,  BMP_SCK);

SdFatSoftSpi<SOFT_MISO_PIN, SOFT_MOSI_PIN, SOFT_SCK_PIN> sd;

// Data file.
SdFile file;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println(F("BMP280 test"));

  // Initialise the altimeter
  if (!bme.begin()) {  
    Serial.println("Could not find a valid BMP280 sensor, check wiring!");
    while (1);
  }

  // Initialise the SD card
  if (!sd.begin(SD_CHIP_SELECT_PIN)) {
    sd.initErrorHalt();
  }
  
}

void loop() {
  // put your main code here, to run repeatedly:
    Serial.print("Temperature = ");
    Serial.print(bme.readTemperature());
    Serial.println(" *C");

//    Serial.print("Sea level pressure = ");
//    Serial.print(bme.sea_level_pressure);
//    Serial.println(" Pa");
    
    Serial.print("Pressure = ");
    Serial.print(bme.readPressure());
    Serial.println(" Pa");

    Serial.print("Approx altitude = ");
    Serial.print(bme.readAltitude(1013.25)); // this should be adjusted to your local forcase
    Serial.println(" m");
    
    Serial.println();
    delay(2000);
}
