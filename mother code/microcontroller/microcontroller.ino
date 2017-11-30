
#include <Wire.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>

static const int RXPin = 3, TXPin = 4;
TinyGPSPlus gps;
SoftwareSerial ss(RXPin, TXPin);

void setup() {
  Wire.begin(8);                // address
  Serial.begin(9600);           // start serial for output
  ss.begin(9600);
  Wire.onReceive(receiveEvent); // register event
  pinMode(2, OUTPUT);
  pinMode(4, OUTPUT);
}

void loop()
{
  smartDelay(1000);
}

void requestEventGPSNUM()
{
    if(gps.satellites.isValid())
    {
      Serial.print("Number of available satellites: ");
      Serial.println(gps.satellites.value());
      Wire.write(gps.satellites.value());
    }
    else
      Wire.write(NULL);
}

void requestEventLAT()
{
  char t[9];
  if(gps.location.isValid())
  {
    Serial.print("Location: ");
    printFloat(gps.location.lat());
    Serial.print(" / ");
    dtostrf(gps.location.lat(), 9, 6, t); //convers the float or integer to a string. (floatVar, minStringWidthIncDecimalPoint, numVarsAfterDecimal, empty array);
    Wire.write(t);
    }
  else
    Wire.write(NULL);
}

void requestEventLNG()
{
  char t[9];
  if(gps.location.isValid())
  {
    printFloat(gps.location.lng());
    Serial.println();
    dtostrf(gps.location.lng(), 9, 6, t);
    Wire.write(t);
    }
  else
    Wire.write(NULL);
}

void requestEventALT()
{
  char t[7];
  if(gps.altitude.isValid())
    {
      Serial.print("Altitude: ");
      Serial.println(gps.altitude.meters());
      dtostrf(gps.altitude.meters(), 7, 2, t);
      Wire.write(t);
    }
  else  
    Wire.write(NULL);
}

void requestEventSPD()
{
  char t[6];
    if(gps.speed.isValid())
    {
      Serial.print("Speed: ");
      Serial.println(gps.speed.kmph());
      dtostrf(gps.speed.kmph(), 6, 2, t);
      Wire.write(t);
    }
  else
    Wire.write(NULL);
}

void requestEventUV()
{
    char t[4];
    Serial.print("Speed: ");
    Serial.println(analogRead(A0));
    dtostrf(analogRead(A0), 4, 0, t);
    Wire.write(t);
}

void receiveEvent()
{
  while (1 < Wire.available())
  { // loop through all but the last
    char c = Wire.read(); // receive byte as a character
    Serial.println(c);         // print the character
    digitalWrite(2,HIGH); 
    delay(10000);
    digitalWrite(2,LOW);
  }
  int reg = Wire.read();    // receive byte as an integer
  Serial.print("registery = ");
  Serial.print(reg);         // print the integer
  Wire.onRequest(requestEventUV);
  
  if(reg==1)
    Wire.onRequest(requestEventGPSNUM);
  if(reg==2)
    Wire.onRequest(requestEventLAT);
  if(reg==3)
    Wire.onRequest(requestEventLNG);
  if(reg==4)
    Wire.onRequest(requestEventALT);
  if(reg==5)
    Wire.onRequest(requestEventSPD);
  if(reg==6)
    Wire.onRequest(requestEventUV);
    
  digitalWrite(4,HIGH);
  delay(5000);
  digitalWrite(4,LOW);
  
}

static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}

static void printFloat(float val)
{
  
    Serial.print(val, 6);
    int vi = abs((int)val);
    int flen = 6 + (val < 0.0 ? 2 : 1); // . and -
    flen += vi >= 1000 ? 4 : vi >= 100 ? 3 : vi >= 10 ? 2 : 1;
    for (int i=flen; i<11; ++i)
      Serial.print(' ');
  
  smartDelay(0);
}
