// include library for reading values from MPU6050 
#include <Wire.h>

// include the SD library:
#include <SPI.h>
#include <SD.h>


const int MPU = 0x68; // MPU6050 I2C address
float AccX, AccY, AccZ, GyroX, GyroY, GyroZ;
float AccErrorX, AccErrorY, AccErrorZ, AngleErrorX, AngleErrorY, GyroErrorX, GyroErrorY, GyroErrorZ;
int c = 0;

File myFile;
const int chipSelect = 10;

//include the libraries for gps module
#include <TinyGPS++.h>
#include <SoftwareSerial.h>

static const int RXPin = 4, TXPin = 3;// Here we make pin 4 as RX of arduino & pin 3 as TX of arduino 
//static const uint32_t GPSBaud = 9600;

volatile int degree, secs, mins;
volatile float minutes, seconds;
volatile double lat_val, lng_val;
String date_, time_, latitude, longitude;   
String filename = "meghana_.txt";

TinyGPSPlus gps;
SoftwareSerial ss(RXPin, TXPin);

void setup() 
{
  Serial.begin(9600);
  ss.begin(9600); 

  Wire.begin();                      // Initialize comunication
  Wire.beginTransmission(MPU);       // Start communication with MPU6050
  //The 0x6B register is the power management register, and writing a value of 0x00 to it sets the device to use its internal oscillator as the clock source. 
  Wire.write(0x6B);                  
  Wire.write(0x00);                
  Wire.endTransmission(true);        //end the transmission

  if (SD.begin(chipSelect))
  {
    Serial.println("SD card is ready to use.");
  }
  else
  {
    Serial.println("SD card initialization failed");
    return;
  }
  
  setup_SDcard();  
  calculate_error();
  delay(1000);
}


// IMU error values 
void calculate_error()
{  
  // Read accelerometer values 200 times
  while (c < 200) 
  {
    Wire.beginTransmission(MPU);
    Wire.write(0x3B);             
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
  
  GyroErrorX = GyroErrorX / 200;
  GyroErrorY = GyroErrorY / 200;
  GyroErrorZ = GyroErrorZ / 200;
  
  // Print the error values on the Serial Monitor
  Serial.print("AccErrorX: ");
  Serial.println(AccErrorX);
  Serial.print("AccErrorY: ");
  Serial.println(AccErrorY);
  Serial.print("AccErrorZ: ");
  Serial.println(AccErrorZ);
  Serial.println();

  Serial.print("GyroErrorX: ");
  Serial.println(GyroErrorX);
  Serial.print("GyroErrorY: ");
  Serial.println(GyroErrorY);
  Serial.print("GyroErrorZ: ");
  Serial.println(GyroErrorZ);
  Serial.println();
  
}

void dispInfo()
{

  Serial.print(F("Date : "));
  if (gps.date.isValid())
  {
    date_ = (String(gps.date.day()) + "/" + String(gps.date.month()) + "/" + String(gps.date.year()));
    Serial.print(date_);
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.print(F("\tTime: "));
  if (gps.time.isValid())
  {
    time_ = (String(gps.time.hour()) + ":" + String(gps.time.minute()) + ":" + String(gps.time.second()));
    Serial.println(time_);
  }
  else
  {
    Serial.println(F("INVALID"));
  }

  Serial.print(F("Location:\t")); 
  if (gps.location.isValid())
  {
    lat_val = gps.location.lat();
    Serial.print(F("Latitude in Decimal Degrees: "));
    Serial.print(lat_val, 6);
    Serial.print(F("\t"));
    DegMinSec(lat_val);
    Serial.print(F("Latitude in Degrees Minutes Seconds: "));
    latitude = (String(degree) + "°" + String(mins) + "'" + String(secs) + "N");
    Serial.println(latitude);
    
    lng_val = gps.location.lng();
    Serial.print(F("\tLongitude in Decimal Degrees: "));
    Serial.print(lng_val, 6);
    Serial.print(F("\t"));
    DegMinSec(lng_val);
    Serial.print(F("Longitude in Degrees Minutes Seconds: "));
    longitude = (String(degree) + "°" + String(mins) + "'" + String(secs) + "E");
    Serial.println(longitude);
  }
  else
  {
    Serial.println(F("INVALID"));
  }    
  delay(100);        
}

void loop() 
{
  
  // === Read gps data === //
  //  for (unsigned long start = millis(); millis() - start < 1000; start = millis())
  //  {
      while (ss.available() > 0)
      {
        if (gps.encode(ss.read()))
        {
          dispInfo();

          // === Read acceleromter data === //
          Wire.beginTransmission(MPU);
          Wire.write(0x3B); 
          Wire.endTransmission(false);
          Wire.requestFrom(MPU, 6, true);  
          AccX = ((Wire.read() << 8 | Wire.read()) / 16384.0) - AccErrorX; // X-axis value
          AccY = ((Wire.read() << 8 | Wire.read()) / 16384.0) - AccErrorY; // Y-axis value
          AccZ = ((Wire.read() << 8 | Wire.read()) / 16384.0) - AccErrorZ; // Z-axis value
          
          // === Read gyroscope data === //
          Wire.beginTransmission(MPU);
          Wire.write(0x43); 
          Wire.endTransmission(false);  
          Wire.requestFrom(MPU, 6, true); 
          GyroX = ((Wire.read() << 8 | Wire.read()) / 131.0) - GyroErrorX; 
          GyroY = ((Wire.read() << 8 | Wire.read()) / 131.0) - GyroErrorY;
          GyroZ = ((Wire.read() << 8 | Wire.read()) / 131.0) - GyroErrorZ;

          //Print the values on the serial monitor
          Serial.print("Acceleration X: ");
          Serial.print(AccX);
          Serial.print(", Y: ");
          Serial.print(AccY);
          Serial.print(", Z: ");
          Serial.print(AccZ);
          Serial.println(" m/s^2");

          Serial.print("Rotation X: ");
          Serial.print(GyroX);
          Serial.print(", Y: ");
          Serial.print(GyroY);
          Serial.print(", Z: ");
          Serial.print(GyroZ);
          Serial.println(" deg/s");
          Serial.println();

          // write the accelerometer values to SD card
          myFile = SD.open(filename, FILE_WRITE);
          if (myFile) 
          {
            String(datastr) = (date_ + time_ + lat_val + latitude + lng_val + longitude + AccX + String(", ") + AccY + String(", ") + AccZ + String(", ") + GyroX + String(", ") + GyroY + String(", ") + GyroZ);
            myFile.println(datastr);      
            myFile.close();

            Serial.println("written into SD card");
            
          }
          else
          {
            Serial.println("error opening .txt file");
          }
        }
      }
//  }      

  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println(F("No GPS detected: check wiring."));
    while(true);
  }

  // delay(300); 
  
}


void setup_SDcard()
{
  myFile = SD.open(filename, FILE_WRITE);
  
  if (myFile) 
  {
    String(datastr) = (String("Date, Time, Latitude(decimal), Latitude(degree min sec), Longitude(decimal), Longitude(degree min sec),Linear Acceleration X, Linear Acceleration Y, Linear Acceleration Z, Angular Velocity X, Angular Velocity Y, Angular Velocity Z"));
    myFile.println(datastr);
    
    myFile.close();

    Serial.println("written into SD card");
  }
  else 
  {
    Serial.println("error opening .txt file");
  }
}

void DegMinSec( double tot_val)   /* Convert data in decimal degrees into degrees minutes seconds form */
{  
  degree = (int)tot_val;
  minutes = tot_val - degree;
  seconds = 60 * minutes;
  minutes = (int)seconds;
  mins = (int)minutes;
  seconds = seconds - minutes;
  seconds = 60 * seconds;
  secs = (int)seconds;
}
