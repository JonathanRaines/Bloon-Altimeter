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

// Create altimeter object
Adafruit_BMP280 bme(BMP_CS, BMP_MOSI, BMP_MISO,  BMP_SCK);

uint32_t temperature;
uint32_t pressure;
uint32_t altitude;

// Create software SPI SD card object
SdFatSoftSpi<SD_MISO, SD_MOSI, SD_SCK> sd;

// Data file.
SdFile file;
// Time in micros for next data record.
uint32_t logTime;

// Interval between data records in milliseconds.
// The interval must be greater than the maximum SD write latency plus the
// time to acquire and write data to the SD to avoid overrun errors.
// Run the bench example to check the quality of your SD card.
const uint32_t SAMPLE_INTERVAL_MS = 1000;

// Log file base name.  Must be six characters or Fewer.
#define FILE_BASE_NAME "Data"

// Write data header.
void writeHeader() {
  file.print(F("Time (ms), Temperature, Pressure, Altitude"));
  file.println();
}

// Log a data record.
void logData() {
  // Write data to file.  Start with log time in micros.
  file.print(logTime);

  // Write ADC data to CSV record.
  file.print(',');
  file.print(temperature);
  file.print(',');
  file.print(pressure);
  file.print(',');
  file.print(altitude);
  file.println();
}

// Error messages stored in flash.
#define error(msg) sd.errorHalt(F(msg))

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println(F("Data logger"));

  // Initialise the altimeter
  if (!bme.begin()) {  
    Serial.println("Could not find a valid BMP280 sensor, check wiring!");
    while (1);
  }

  const uint8_t BASE_NAME_SIZE = sizeof(FILE_BASE_NAME) - 1;
  char fileName[13] = FILE_BASE_NAME "00.csv"; 
  
  // Initialize at the highest speed supported by the board that is
  // not over 50 MHz. Try a lower speed if SPI errors occur.
  if (!sd.begin(SD_CS, SD_SCK_MHZ(50))) {
    sd.initErrorHalt();
  }

  // Find an unused file name.
  if (BASE_NAME_SIZE > 6) {
    error("FILE_BASE_NAME too long");
  }
  while (sd.exists(fileName)) {
    if (fileName[BASE_NAME_SIZE + 1] != '9') {
      fileName[BASE_NAME_SIZE + 1]++;
    } else if (fileName[BASE_NAME_SIZE] != '9') {
      fileName[BASE_NAME_SIZE + 1] = '0';
      fileName[BASE_NAME_SIZE]++;
    } else {
      error("Can't create file name");
    }
  }
  if (!file.open(fileName, O_CREAT | O_WRITE | O_EXCL)) {
    error("file.open");
  }

  // Write data header.
  writeHeader();
  Serial.println("Writing header...");

  // Start on a multiple of the sample interval.
  logTime = micros()/(1000UL*SAMPLE_INTERVAL_MS) + 1;
  logTime *= 1000UL*SAMPLE_INTERVAL_MS;
  
}

void loop() {

  temperature = bme.readTemperature();
  pressure = bme.readPressure();
  altitude = bme.readAltitude(1020.5);
  Serial.print("T: ");
  Serial.print(temperature);
  Serial.print(" *C, P: ");
  Serial.print(pressure);
  Serial.print(" Pa, A: ");
  Serial.print(altitude);
  Serial.println(" m.");

  // Wait for log time.
  int32_t diff;
  do {
    diff = micros() - logTime;
  } while (diff < 0);

  // Check for data rate too high.
  if (diff > 10) {
    error("Missed data record");
  }
  
  Serial.println("Logging data to SD");
  logData();

  // Force data to SD and update the directory entry to avoid data loss.
  if (!file.sync() || file.getWriteError()) {
    error("write error");
  }
  
}
