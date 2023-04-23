#include <TinyGPS++.h>
#include <SoftwareSerial.h>

static const int RXPin = 4, TXPin = 3;// Here we make pin 4 as RX of arduino & pin 3 as TX of arduino 
static const uint32_t GPSBaud = 9600;

volatile int degree, secs, mins;
volatile float minutes, seconds;

TinyGPSPlus gps;

SoftwareSerial ss(RXPin, TXPin);

void setup()
{
  Serial.begin(9600);
  ss.begin(GPSBaud); 
}

void loop()
{
  while (ss.available() > 0)
    if (gps.encode(ss.read()))
      displayInfo();

  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println(F("No GPS detected: check wiring."));
    while(true);
  }
}

void displayInfo()
{
  double lat_val, lng_val;

  Serial.print(F("Date: "));
  if (gps.date.isValid())
  {
    String(date) = (String(gps.date.day()) + "/" + String(gps.date.month()) + "/" + String(gps.date.year()));
    Serial.print(date);
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.print(F("\tTime: "));
  if (gps.time.isValid())
  {
    String(time) = (String(gps.time.hour()) + ":" + String(gps.time.minute()) + ":" + String(gps.time.second()));
    Serial.println(time);
  }
  else
  {
    Serial.print(F("INVALID"));
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
    String(latitude) = (String(degree) + "°" + String(mins) + "'" + String(secs) + "N");
    Serial.println(latitude);
    
    lng_val = gps.location.lng();
    Serial.print(F("         \tLongitude in Decimal Degrees: "));
    Serial.print(lng_val, 6);
    Serial.print(F("\t"));
    DegMinSec(lng_val);
    Serial.print(F("Longitude in Degrees Minutes Seconds: "));
    String(longitude) = (String(degree) + "°" + String(mins) + "'" + String(secs) + "E");
    Serial.println(longitude);
  }
  else
  {
    Serial.print(F("INVALID"));
  }
    
  Serial.println();
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