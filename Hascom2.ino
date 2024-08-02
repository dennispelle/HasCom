/***************************************************
  This is a library for the ST7789 IPS SPI display.

  Originally written by Limor Fried/Ladyada for
  Adafruit Industries.

  Modified by Ananev Ilia
  ü is 0x81
  ä is 0x84
  ö is 0x94
  Ä is 0x8E
  Ö is 0x99
  Ü is 0x9A
 *************************************************** */
 /* Temperatursensoren  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 4
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
unsigned long timesincerequest=0;
float T1,T2;
#include <SimpleDHT.h>
byte temperature = 0;
byte humidity = 0; 
int err;  
SimpleDHT11 dht11(5);
/* RTC MODUL XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXx*/
#include <Wire.h>
#include <TimeLib.h>
#include <DS1307RTC.h>
tmElements_t tm;
/* Display XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
#include <Adafruit_GFX.h>    // Core graphics library by Adafruit
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <SPI.h>
  #define TFT_CS        10
  #define TFT_RST        9 // Or set to -1 and connect to Arduino RESET pin
  #define TFT_DC         3


Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
/* GPS XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
#include <SoftwareSerial.h>
#include <TinyGPS++.h>
// The TinyGPS++ object
TinyGPSPlus gps;
static const int RXPin = 6, TXPin = 7;
static const uint32_t GPSBaud = 9600;
SoftwareSerial ss(RXPin, TXPin);
byte gmonat, gtag, gstunde, gminute, gsekunde, zeitzone=1 ,gpsdn,gpstn; ; //Zeitzone für sommer/winterzeit usw
int gjahr;
bool gpst=0,gpsd=0,savetime=1;

// variablendefinition
float gx, gy; // fließkommzahl für die G-Kräfte
// ende Variablendefintition
void setup(void) {
  pinMode(3,OUTPUT);
  analogWrite(3,255);
//  tft.init(240, 240);   // initialize a ST7789 chip, 240x240 pixels
  tft.setRotation(3);
  tft.fillScreen(ST77XX_WHITE);
  ss.begin(GPSBaud);
  sensors.begin();
  
  drawFrame();

}

void getTemp(){ // Hole die Tempereatur und die Luftfeuchtigkeit und speichere sie in Var: temperature und humidity
  if ((timesincerequest+2000)<millis()){// schaue ob die letzte Messung der der Temperatursenosren wenigstens eine sekunde her ist, ansonsten überspringen
   T1=sensors.getTempCByIndex(0);
   T2=sensors.getTempCByIndex(1);
   sensors.requestTemperatures();
   timesincerequest=millis();
   err = dht11.read(&temperature, &humidity, NULL);}
  }

void loop() {
  
  showInnen();
  showgf();
  showTime();
  showGPS();
  showTemp();
}
/*
 * hier die Befehle, die noch eingepflegt werden müssen
 *   printFloat(gps.altitude.meters(), gps.altitude.isValid(), 7, 2);
     printFloat(gps.course.deg(), gps.course.isValid(), 7, 2);
     printFloat(gps.speed.kmph(), gps.speed.isValid(), 6, 2);
 * 
 * 
 * 
 */
void showTemp(){//Zeige die Temperaturen der Aussensensoren an
  tft.setCursor(2, 62);   tft.setTextColor(ST77XX_BLACK, ST77XX_WHITE);   tft.setTextSize(2);
  tft.print("T1=");   tft.print(T1,1);    tft.print(char(0xF7));    tft.print("C");
  
  tft.setCursor(2, 82);   tft.setTextColor(ST77XX_BLACK, ST77XX_WHITE);   tft.setTextSize(2);
  tft.print("T2=");   tft.print(T2,1);    tft.print(char(0xF7));    tft.print("C");
  }
void getGpsClock() { //Hole die Zeit über das GPSmodul und korrigiere Sie 
  while (ss.available() > 0)
    if (gps.encode(ss.read()));
    
      gjahr=gps.date.year();
      gjahr-=2000;

      gmonat = gps.date.month();
      gtag = gps.date.day();
      if ((gmonat+gtag)>0){gpsd=1;gpsdn++;}else gpsd=0;


      
  gstunde = gps.time.hour();
  gminute = gps.time.minute();
  gsekunde = gps.time.second();
  if (gstunde+gsekunde+gminute){gpst=1;gpstn++;}else gpst=0;
  gstunde = gstunde + zeitzone;
  gstunde = gstunde % 24;
  if (gstunde < zeitzone){ //Zeitkorrektur
    gtag++;
    if (gtag == 32) {
      gtag = 1;   //wenn der monat 32tage hat
      gmonat++;
    }
    if (gtag == 31) //möglicher überlauftag
    {
      if (gmonat == 4) {
        gtag = 1;
        gmonat++;
      }
      if (gmonat == 6) {
        gtag = 1;
        gmonat++;
      }
      if (gmonat == 9) {
        gtag = 1;
        gmonat++;
      }
      if (gmonat == 11) {
        gtag = 1;
        gmonat++;
      }
    }
    if (gmonat == 2) {
      if (((gjahr%4)==0)&&(gtag==30)) {
        gtag = 1;
        gmonat++;
      }
      if (gtag == 29) {
        if ((gjahr % 4) != 0) {
          gtag = 1;
          gmonat++;
        }
      }
    }   //wenn februar und kein schaltjahr
      //wenn februar und schaltjahr

    if (gmonat == 13) {
      gmonat = 1;
      gjahr++;
    }

  }


  if (gpsd&gpst&savetime)savedate();
  if (gpsd&gpst&savetime)savedate();






}
void savedate(){//Speichere die aktuelle Zeit im Batteriemodul
  tm.Day=gtag;
  tm.Month=gmonat;
  tm.Year=CalendarYrToTm(gjahr);
  tm.Hour=gstunde;
  tm.Minute=gminute;
  tm.Second=gsekunde;
  RTC.write(tm);
  savetime=0;

  }
void getbatclock(){//Hole die Zeit aus dem Batteriemodul
    RTC.read(tm);
    gstunde=tm.Hour;
    gminute=tm.Minute;
    gsekunde=tm.Second;
  }
void getbatday(){// Hole das datum aus dem Batteriemodul
    RTC.read(tm);
    gtag=tm.Day;
    gmonat=tm.Month;
    gjahr=tmYearToCalendar(tm.Year)-2000;
  }
void drawFrame() {//male einen Rahmen aufs display
  tft.drawFastHLine(0, 190, 240, ST77XX_BLACK);
  tft.drawFastHLine(0, 20, 240, ST77XX_BLACK);
  tft.drawFastVLine(120, 190, 50, ST77XX_BLACK);
}
void showTime() {// stelle die Zeit dar
  getGpsClock();
  if (!gpst) getbatclock(); //schau ob die GPSzeit stimmt, wenn nicht hol die Zeit aus der Uhr
  if (!gpsd) getbatday(); // nochmal das gleiche mit dem Tag
  tft.setCursor(150, 193);
  tft.setTextColor(ST77XX_BLACK, ST77XX_WHITE);
  tft.setTextSize(2);
  if (gstunde < 10)tft.print("0"); tft.print(gstunde); 
  tft.print(":"); 
  if (gminute < 10) tft.print("0"); tft.print(gminute);

  tft.setCursor(130, 218);
  tft.setTextColor(ST77XX_BLACK, ST77XX_WHITE);
  tft.setTextSize(2 );

  if (gtag < 10) tft.print("0"); tft.print(gtag); 
  tft.print(":"); 
  if (gmonat < 10) tft.print("0"); tft.print(gmonat);
  tft.print(":"); 
  if (gjahr < 10) tft.print("0"); tft.print(gjahr);
}
void showInnen() {//stelle die Innentemperatur und die Luftfeuchte dar
  getTemp();
  tft.setCursor(5, 193);
  tft.setTextColor(ST77XX_BLACK, ST77XX_WHITE);
  tft.setTextSize(2);
  tft.print("T.i:");
  tft.print(temperature);
  tft.print(char(0xF7));
  tft.print("C");
  tft.setCursor(5, 218);
  tft.setTextColor(ST77XX_BLACK, ST77XX_WHITE);
  tft.setTextSize(2);
  tft.print("H.i: ");
  tft.print(humidity);
  tft.print("%");
}
void showGPS() { //Zeige die GPSdaten an, so
  tft.setCursor(2, 2);
  tft.setTextColor(ST77XX_BLACK, ST77XX_WHITE);
  tft.setTextSize(2);
  tft.print("S");
  tft.print(gps.satellites.value());
  tft.print(" ");
  if (gps.location.isValid())
  {
    tft.print(gps.location.lat(), 6);
    tft.print(",");
    tft.print(gps.location.lng(), 6);
  }
  else
  {
    tft.print("XXXXXXXXXXXXXXXX");
  }
}
void getGf() {//Miss die Gkräfte und korrigiere sie
  byte c;
  gx = 0;
  gy = 0;


  gx += analogRead(A0);
  gy += analogRead(A1);


  gx *= 5 / 1023.0;
  gy *= 5 / 1023.0;
  gx -= 1.8;
  gy -= 1.8;
  gx /= 0.9;
  gy /= 0.9;


}
void showgf(){//stelle die Gkräfte dar
  getGf();
  tft.setCursor(2, 22);
  tft.setTextColor(ST77XX_BLACK, ST77XX_WHITE);
  tft.setTextSize(2);
  tft.print("G=");
  tft.print(gx);
  tft.setCursor(2, 42);
  tft.setTextColor(ST77XX_BLACK, ST77XX_WHITE);
  tft.setTextSize(2);
  tft.print("S="); tft.print(gy);
  tft.setCursor(2, 62);
}
