#include <Wire.h>
// include the SD library:
#include <SPI.h>
#include <SD.h>

const int MPU = 0x68; // MPU6050 I2C address
float AccX, AccY, AccZ, GyroX, GyroY, GyroZ;
float accAngleX, accAngleY, gyroAngleX, gyroAngleY, gyroAngleZ;
float AccErrorX, AccErrorY, AccErrorZ, GyroErrorX, GyroErrorY, GyroErrorZ;
int c = 0;
File myFile;
const int chipSelect = 10;


void setup() 
{
  ////Serial.begin(9600);
  Wire.begin();                      // Initialize comunication
  Wire.beginTransmission(MPU);       // Start communication with MPU6050

  //The 0x6B register is the power management register, and writing a value of 0x00 to it sets the device to use its internal oscillator as the clock source. 
  Wire.write(0x6B);                  
  Wire.write(0x00);                
  Wire.endTransmission(true);        //end the transmission

  if (SD.begin()){
    //Serial.println("SD card is ready to use.");
  }
  else
  {
    //Serial.println("SD card initialization failed");
    while(1);
  } 
  
  setup_SDcard();
  calculate_error();
  delay(20);
}

// IMU error values
// An Inertial Measurement Unit (IMU) combines linear accelerations from an accelerometer and rotations from a gyroscope to deliver navigation parameters and position update information. 
void calculate_error()
{
  
  // Read accelerometer values 200 times
  while (c < 200) 
  {
    Wire.beginTransmission(MPU);
    Wire.write(0x3B);             //register we want to read from to the I2C bus
    Wire.endTransmission(false); //'false' indicates a "restart" signal instead of a "stop" signal
    Wire.requestFrom(MPU, 6, true);
    AccX = (Wire.read() << 8 | Wire.read()) / 16384.0 ;
    AccY = (Wire.read() << 8 | Wire.read()) / 16384.0 ;
    AccZ = (Wire.read() << 8 | Wire.read()) / 16384.0 ;

    // Sum all readings
    AccErrorX = AccErrorX + AccX;
    AccErrorY = AccErrorY + AccY;
    AccErrorZ = AccErrorZ + AccZ;

    c++;
    
  }
  
  //Divide the sum by 200 to get the error value
  AccErrorX = AccErrorX / 200;
  AccErrorY = AccErrorY / 200;
  AccErrorZ = AccErrorZ / 200;

  
  c = 0;
  
  // Read gyro values 200 times
  while (c < 200) 
  {
    Wire.beginTransmission(MPU);
    Wire.write(0x43);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU, 6, true);
    GyroX = Wire.read() << 8 | Wire.read();
    GyroY = Wire.read() << 8 | Wire.read();
    GyroZ = Wire.read() << 8 | Wire.read();
    
    // Sum all readings
    GyroErrorX = GyroErrorX + (GyroX / 131.0);
    GyroErrorY = GyroErrorY + (GyroY / 131.0);
    GyroErrorZ = GyroErrorZ + (GyroZ / 131.0);
    c++;
  }
  //Divide the sum by 200 to get the error value
  GyroErrorX = GyroErrorX / 200;
  GyroErrorY = GyroErrorY / 200;
  GyroErrorZ = GyroErrorZ / 200;
  
  // Print the error values on the //Serial Monitor
  //Serial.print("AccErrorX: ");
  //Serial.println(AccErrorX);
  //Serial.print("AccErrorY: ");
  //Serial.println(AccErrorY);
  //Serial.print("AccErrorZ: ");
  //Serial.println(AccErrorZ);
  //Serial.println();

  //Serial.print("GyroErrorX: ");
  //Serial.println(GyroErrorX);
  //Serial.print("GyroErrorY: ");
  //Serial.println(GyroErrorY);
  //Serial.print("GyroErrorZ: ");
  //Serial.println(GyroErrorZ);
  //Serial.println();
  
}


void loop() {

  // === Read acceleromter data === //
  Wire.beginTransmission(MPU);
  Wire.write(0x3B); // Start with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);

  Wire.requestFrom(MPU, 6, true);
  
  AccX = ((Wire.read() << 8 | Wire.read()) / 16384.0) - AccErrorX; // X-axis value
  AccY = ((Wire.read() << 8 | Wire.read()) / 16384.0) - AccErrorY; // Y-axis value
  AccZ = ((Wire.read() << 8 | Wire.read()) / 16384.0) - AccErrorZ; // Z-axis value

  
  Wire.beginTransmission(MPU);
  Wire.write(0x43); 
  Wire.endTransmission(false);
  
  Wire.requestFrom(MPU, 6, true); 
  // For a 250deg/s range we have to divide first the raw value by 131.0, according to the datasheet
  //for the ±250 DPS range, one LSB represents 1/131 degrees per second
  GyroX = ((Wire.read() << 8 | Wire.read()) / 131.0) - GyroErrorX; 
  GyroY = ((Wire.read() << 8 | Wire.read()) / 131.0) - GyroErrorY;
  GyroZ = ((Wire.read() << 8 | Wire.read()) / 131.0) - GyroErrorZ;
  

//   Print the values on the //Serial monitor
  //Serial.print("Acceleration X: ");
  //Serial.print(AccX);
  //Serial.print(", Y: ");
  //Serial.print(AccY);
  //Serial.print(", Z: ");
  //Serial.print(AccZ);
  //Serial.println(" m/s^2");

  //Serial.print("Rotation X: ");
  //Serial.print(GyroX);
  //Serial.print(", Y: ");
  //Serial.print(GyroY);
  //Serial.print(", Z: ");
  //Serial.print(GyroZ);
  //Serial.println(" deg/s");
  //Serial.println();

  // write the accelerometer values to SD card
    myFile = SD.open("two-wheeler.txt", FILE_WRITE);
    if (myFile) 
    {
      String(datastr) = (AccX + String(", ") + AccY + String(", ") + AccZ + String(", ") + GyroX + String(", ") + GyroY + String(", ") + GyroZ);
      myFile.println(datastr);      
      myFile.close();
      delay(100);
    }
    else
    {
      //Serial.println("error opening .txt file");
    }
}


void setup_SDcard()
{
  myFile = SD.open("two-wheeler.txt", FILE_WRITE);
  
  if (myFile) 
  {
    String(datastr) = (String("Linear Acceleration X, Linear Acceleration Y, Linear Acceleration Z, Angular Velocity X, Angular Velocity Y, Angular Velocity Z"));
    myFile.println(datastr);
    
    myFile.close();

    //Serial.println("written into SD card");
  }
  else 
  {
    //Serial.println("error opening .txt file");
  }
}
