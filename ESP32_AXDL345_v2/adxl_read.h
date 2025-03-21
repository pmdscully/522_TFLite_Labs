
#ifndef WIRE_H
#define WIRE_H
#include <Wire.h>
#endif // WIRE_H


// ADXL345 I2C address
#define ADXL345_ADDRESS 0x53

// ADXL345 registers
#define ADXL345_REG_DEVID      0x00  // Device ID
#define ADXL345_REG_POWER_CTL  0x2D  // Power control register
#define ADXL345_REG_DATA_FORMAT 0x31  // Data format register
#define ADXL345_REG_DATAX0     0x32  // X-axis data (low byte)
#define ADXL345_REG_DATAX1     0x33  // X-axis data (high byte)
#define ADXL345_REG_DATAY0     0x34  // Y-axis data (low byte)
#define ADXL345_REG_DATAY1     0x35  // Y-axis data (high byte)
#define ADXL345_REG_DATAZ0     0x36  // Z-axis data (low byte)
#define ADXL345_REG_DATAZ1     0x37  // Z-axis data (high byte)



void writeToRegister(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(ADXL345_ADDRESS);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

void initADXL345() {
  // Wake up the ADXL345 and set to measurement mode
  writeToRegister(ADXL345_REG_POWER_CTL, 0x08);  // 0x08 = Measure mode
  // Set data format (±2g, 10-bit resolution)
  writeToRegister(ADXL345_REG_DATA_FORMAT, 0x10);  
    //±2g:  Set the SENSITIVITY bits to 0x00 // 0x00 = ±2g, 10-bit resolution
    //±4g:  Set the SENSITIVITY bits to 0x01
    //±8g:  Set the SENSITIVITY bits to 0x10
    //±16g: Set the SENSITIVITY bits to 0x11
}


void readAccelData_int(int16_t &x, int16_t &y, int16_t &z) {
  uint8_t rawData[6];  // Buffer to hold the 6 bytes of accelerometer data

  // Request 6 bytes of data from the ADXL345
  Wire.beginTransmission(ADXL345_ADDRESS);
  Wire.write(ADXL345_REG_DATAX0);
  Wire.endTransmission(false);
  Wire.requestFrom(ADXL345_ADDRESS, 6);

  // Read the 6 bytes into the rawData buffer
  for (int i = 0; i < 6; i++) {
    rawData[i] = Wire.read();
  }

  // Combine the 2 bytes for each axis
  x = (int16_t)((rawData[1] << 8) | rawData[0]);
  y = (int16_t)((rawData[3] << 8) | rawData[2]);
  z = (int16_t)((rawData[5] << 8) | rawData[4]); //int16_t
}

void readAccelData_float(float &x, float &y, float &z) {
  uint8_t rawData[6];  // Buffer to hold the 6 bytes of accelerometer data

  // Request 6 bytes of data from the ADXL345
  Wire.beginTransmission(ADXL345_ADDRESS);
  Wire.write(ADXL345_REG_DATAX0);
  Wire.endTransmission(false);
  Wire.requestFrom(ADXL345_ADDRESS, 6);

  // Read the 6 bytes into the rawData buffer
  for (int i = 0; i < 6; i++) {
    rawData[i] = Wire.read();
  }

  // Combine the 2 bytes for each axis
  int16_t rawX = (int16_t)((rawData[1] << 8) | rawData[0]);
  int16_t rawY = (int16_t)((rawData[3] << 8) | rawData[2]);
  int16_t rawZ = (int16_t)((rawData[5] << 8) | rawData[4]);
  
  // Convert the raw data to float and scale by sensitivity: 
  x = (float)rawX / 128;
  y = (float)rawY / 128;
  z = (float)rawZ / 128;
  // 2g  mode sensitivity is 512 
  // 4g  mode sensitivity is 256 LSB/g - each increment of the raw data corresponds to 1/256th of a g (gravitational acceleration, 9.81 m/s²)
  // 8g  mode sensitivity is 128 
  // 16g mode sensitivity is  64
  // ~~  mode sensitivity is  42.6667 - is approximate range used by JoaoYukio's Example.
}
