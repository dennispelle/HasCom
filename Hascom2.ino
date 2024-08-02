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



// EEPROM zur Speicherung der Sommer/Winterzeit und der Helligkeitseinstellungen
#include <EEPROM.h>
boolean Sommerzeit; 
byte Nightlight,Daylight;
 /* Temperatursensoren  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 4
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
unsigned long timesincerequest=0;
float T1,T2,Ta1,Ta2,T1m,T2m;
byte temperature = 0;
byte humidity = 0; 
int err;  
#include <dhtnew.h>
DHTNEW mySensor(5);
/* RTC MODUL XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXx*/
#include <Wire.h>
#include <TimeLib.h>
#include <DS1307RTC.h>
tmElements_t tm;
/* Display XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
#include <Adafruit_GFX.h>    // Core graphics library by Adafruit
#include <Arduino_ST7789.h> // Hardware-specific library for ST7789 (with or without CS pin)
#include <SPI.h>

#define TFT_DC    8
#define TFT_RST   9
#define TFT_MOSI  11   // for hardware SPI data pin (all of available pins)
#define TFT_SCLK  13   // for hardware SPI sclk pin (all of available pins)
Arduino_ST7789 tft = Arduino_ST7789(TFT_DC, TFT_RST); //for display without CS pin
/* GPS XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX*/
#include <SoftwareSerial.h>
#include <TinyGPS++.h>
// The TinyGPS++ object
TinyGPSPlus gps;
static const int RXPin = 6, TXPin = 7;
static const uint32_t GPSBaud = 9600;
SoftwareSerial ss(RXPin, TXPin);
byte wtag, gmonat, gtag, gstunde, gminute,ominute=61, gsekunde, zeitzone=1 ,gpsdn,gpstn; ; //Zeitzone für sommer/winterzeit usw
int gjahr;
bool gpst=0,gpsd=0,savetime=1;

// variablendefinition
float gx,gy,gz,gkx,gky,gkz; // fließkommzahl für die G-Kräfte
byte menu=1; // Menu Variable.

  boolean first=1; //Hilfsvariable 
  byte x1,x2,px=1,xmax,xmin,xac;
  byte xav[10];
  unsigned long timer=millis();


// ende Variablendefintition

byte getlength(int N){// überprüfe die anzahl der Ziffern und wandle sie in Pixelweite
  if(N<-99)return 72;
  else if(N<-9) return 54;
  else if(N<0)return 36;
  else if(N<10)return 18;
  else if(N<100)return 36;
  else return 54;
  }
byte tempsensorstage=1;
unsigned long TempUhr;  
void getTemp(){ // Hole die Tempereatur und die Luftfeuchtigkeit und speichere sie in Var: temperature und humidity
  // schaue ob die letzte Messung der der Temperatursenosren wenigstens eine sekunde her ist, ansonsten überspringen
  if ((tempsensorstage==1)&((TempUhr+750)<millis())) {T2=sensors.getTempCByIndex(0);           tempsensorstage=2;}
  if ((tempsensorstage==2)&((TempUhr+1500)<millis())){T1=sensors.getTempCByIndex(1);           tempsensorstage=3;  sensors.requestTemperatures();}
  if ((tempsensorstage==3)&((TempUhr+2250)<millis())){temperature = mySensor.getTemperature(); tempsensorstage=4;}
  if ((tempsensorstage==4)&((TempUhr+3000)<millis())){humidity = mySensor.getHumidity();       tempsensorstage=1;  TempUhr=millis();mySensor.read();}

 
  }
void space(byte n){
  for(byte k=0;k<n;k++)tft.print(" ");
  }
void startbildschirm(){// Startbildschirm, Zeigt die Uhrzeit und das Datum Groß an, dazu Clima und Temperaturen an.
    
    
// Block für die Uhrzeit und das Datum
  getGpsClock();
  if (!gpst) getbatclock(); //schau ob die GPSzeit stimmt, wenn nicht hol die Zeit aus der Uhr
  if (!gpsd) getbatday(); // nochmal das gleiche mit dem Tag
  if ((first)||(gminute!=ominute)){// schau, ob das Bild aktualisiert werden muss
      ominute=gminute;
  // Uhrzeit, Stunde:Minute
        printStartTimeHour();
        
        // Wochentag Mittig
        tft.setTextSize(3 );
        tft.setCursor(-10,60);
        tft.print(" ");
        wochentag();
  
        tft.setCursor(-10, 90);
        tft.print(" "); 
        if (gtag < 10) tft.print("0"); tft.print(gtag); 
        tft.print("."); 
        monat(gmonat);
        tft.setCursor(-10, 120);
        tft.print(" "); 
        if (gjahr < 10) tft.print("0"); tft.print(gjahr);
        }
  //tft.setCursor(150,60);tft.print(millis()%10);
// Zeige das Raumklima an
  getTemp(); //Hole die Temperatur und Luftfeucht vom DHT
  if ((first)||(gminute!=ominute)){
        tft.setCursor(-10,150);
        tft.setTextColor(WHITE, BLACK);
        tft.print(" Innen:");
        }
        
        tft.setCursor(116,150);
        tft.setTextColor(RED, BLACK);
        tft.print(temperature);
        tft.fillRect((116+getlength(temperature)),165,30,14,BLACK);
        tft.setTextColor(WHITE, BLACK);
        tft.setTextSize(2);
        tft.print(char(0xF7));
        tft.print("C ");
        tft.setTextSize(3);
        tft.setTextColor(BLUE, BLACK);
        tft.print(humidity);
        tft.fillRect((150+getlength(temperature)+getlength(humidity)),166,25,13,BLACK);
        tft.setTextColor(WHITE, BLACK);
        tft.setTextSize(2);
        tft.print("%");

// Zeige Temperatursensoren an
  if ((first)||(gminute!=ominute)){
    tft.setCursor(-10,180);
    tft.setTextSize(3);
    tft.print(" Aussen: ");
  }
  
  tft.setTextSize(3);
  tft.setCursor(152,180);
  tft.print(T1,0);    tft.setTextSize(2);       tft.print(char(0xF7));    tft.print("C   ");
  tft.fillRect((152+getlength(T1)),194,40,15,BLACK);
  
  if ((first)||(gminute!=ominute)){
    tft.setTextSize(3);
    tft.setCursor(-10,210);
    tft.print(" Motor:  ");
  }
  
  tft.setTextSize(3);
  tft.setCursor(152,210);
  tft.print(T2,0);    tft.setTextSize(2);    tft.print(char(0xF7));    tft.print("C   ");
  tft.fillRect((152+getlength(T2)),224,40,15,BLACK);

  tft.setCursor(140,120);
  tft.setTextSize(3);
  tft.print(analogRead(A3)/1023.0*20.0,1); tft.setTextColor(RED, BLACK);tft.print("V  ");
  first=0;
}
void printStartTimeHour(){
    // Uhrzeit, Stunde:Minute
        tft.setCursor(-30, 5);
        tft.setTextColor(WHITE, BLACK);
        tft.setTextSize(7);
        tft.print(" ");
        if (gstunde < 10)tft.print("0"); tft.print(gstunde); 
        tft.print(":"); 
        if (gminute < 10) tft.print("0"); tft.print(gminute);
  
  }
void monat(byte t){//konvertiert eine zahl in einen String 
  switch (t) {
    case 1:
      tft.print("Januar");
    break;
    case 2:
      tft.print("Februar");
    break;
    case 3:
      tft.print("Ma");
      tft.print("er");
      tft.print("z");
    break;
    case 4:
      tft.print("April");
    break;
    case 5:
      tft.print("Mai");
    break;
    case 6:
      tft.print("Juni");
    break;
    case 7:
      tft.print("Juli");
    break;
    case 8:
      tft.print("August");
    break;
    case 9:
      tft.print("September");
    break;
    case 10:
      tft.print("Oktober");
    break;
    case 11:
      tft.print("November");
    break;
    case 12:
      tft.print("Dezember");
    break;
  }  
}
void wochentag(){//gibt einen String auf dem TFT aus
  getbatday();
  switch (wtag) {
    case 0:  
      tft.print("Mon");
    break;
    case 1:  
      tft.print("Diens");
    break;
    case 2:
      tft.print("Mittwoch");
    break;   
    case 3:
      tft.print("Donners");
    break;   
    case 4:
      tft.print("Frei");
    break;   
    case 5:
      tft.print("Sams");
    break;
    case 6:
      tft.print("Sonn");
    break;
  }

  if (wtag!=2) tft.print("tag");
  space(5);
}
void getGpsClock() { //Hole die Zeit über das GPSmodul und korrigiere Sie 
  while (ss.available() > 0)
    if (gps.encode(ss.read()));
    
      gjahr=gps.date.year();
      

      gmonat = gps.date.month();
      gtag = gps.date.day();
      if ((gmonat+gtag)>0){gpsd=1;gpsdn++;}else gpsd=0;


      
  gstunde = gps.time.hour();
  gminute = gps.time.minute();
  gsekunde = gps.time.second();
  if (gstunde+gsekunde+gminute){gpst=1;gpstn++;}else gpst=0;
  gstunde = gstunde + zeitzone+Sommerzeit;
  gstunde = gstunde % 24;
  if (gstunde < (zeitzone+Sommerzeit)){ //Zeitkorrektur
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
    wtag=tm.Wday;
    gtag=tm.Day;
    gmonat=tm.Month;
    gjahr=tmYearToCalendar(tm.Year);//-2000;
  }
void refresh(){// Bildschirm löschen
  tft.fillScreen(BLACK);
  first=1;
  px=1;
  }
// Globale Variablen fpr GPSstatus
byte satelitenanzahl;
float latitude, longitude;
int hoehenmeter,degree,gpsjahr;
byte gpstag,gpsmonat,gpsstunde,gpsminute,gpssekunde,gpsspeed;
// Globale Variablen fpr GPSstatus
void gpsstatus(){// Funktionsbildschirm für GPS, zeigt Satelitenanzahl, Koordinaten, Geschwindigkeit usw an
  
   while (ss.available() > 0)
    if (gps.encode(ss.read()));  // hole die daten vom GPS sensor
  
  tft.setTextColor(WHITE, BLACK);
  tft.setTextSize(3);
  
  if(first){
    printSatAnz();
    printLatitude();
    printLongitude();
    printHoehenmeter();
    printGpsDatum();
    printGpsTime();
    printGpsGeschwindigkeit();
    printGpsRichtung();
    first=0;   
  }
  if (satelitenanzahl!=gps.satellites.value())  printSatAnz();
  if (latitude!=gps.location.lat())             printLatitude();
  if (longitude!=gps.location.lng())            printLongitude();
  if (hoehenmeter!=gps.altitude.meters())       printHoehenmeter();

  if ((gpstag!=gps.date.day())||(gpsmonat!=gps.date.month())||(gpsjahr!=gps.date.year()))             printGpsDatum(); 
  if ((gpssekunde!=gps.time.second())||(gpsminute!=gps.time.minute())||(gpsstunde!=gps.time.hour()))  printGpsTime();
  
  if (gpsspeed!=gps.speed.kmph())               printGpsGeschwindigkeit();
  if (degree!=gps.course.deg())                 printGpsRichtung();
  }
void printSatAnz(){
    satelitenanzahl=gps.satellites.value();
    tft.setCursor(-10, 5);
    tft.print(" Sateliten:");
    if(satelitenanzahl<10) tft.print(" ");
    tft.print(satelitenanzahl);
    tft.print(" ");
  }
void printLatitude(){
    latitude=gps.location.lat(); 
    tft.setCursor(-10, 30);
    tft.print(" B:"); 
    if (latitude<100)tft.print(" ");
    if (latitude<10)tft.print(" "); 
    tft.print(latitude, 6);
  }
void printLongitude(){
    longitude=gps.location.lng();
    tft.setCursor(-10, 55);
    tft.print(" L:"); 
    if (longitude<100)tft.print(" ");
    if (longitude<10)tft.print(" "); 
    tft.print(longitude, 6);
  }
void printHoehenmeter(){
    hoehenmeter=gps.altitude.meters();
    tft.setCursor(-10, 80);
    tft.print(" H:");
    if (hoehenmeter<100)tft.print(" "); 
    if (hoehenmeter<100)tft.print(" ");
    if (hoehenmeter<10)tft.print(" "); 
    tft.print(hoehenmeter,0); 
    tft.print("m");space(1);
  }
void printGpsDatum(){
    gpstag=gps.date.day(); 
    gpsmonat=gps.date.month(); 
    gpsjahr=gps.date.year();
    
    tft.setCursor(-10, 130);
    tft.print(" D:"); 
    if (gpstag<10){tft.print("0");}
    tft.print(gpstag); 
    tft.print(".");
    if (gpsmonat<10){tft.print("0");}  
    tft.print(gpsmonat);tft.print("."); 
    tft.print(gpsjahr);
  }
void printGpsTime(){
    gpsstunde=gps.time.hour();  
    gpsminute=gps.time.minute();  
    gpssekunde=gps.time.second();
    tft.setCursor(-10, 105);
    
    tft.print(" C:");
    
    if (gpsstunde<10){tft.print("0");}  
    tft.print(gpsstunde); 
    
    tft.print(":");
    
    if (gpsminute<10){tft.print("0");} 
    tft.print(gpsminute);
    tft.print(":");
    
    if (gpssekunde<10){tft.print("0");} 
    tft.print(gpssekunde);
  }
void printGpsGeschwindigkeit(){
    gpsspeed=gps.speed.kmph();
    
    tft.setCursor(-10, 155);
    
    tft.print(" S:");
    if (gpsspeed<100){tft.print(" ");}
    if (gpsspeed<10){tft.print(" ");} 
    tft.print(gpsspeed,1);tft.print("Km/h");
  }
void printGpsRichtung(){
    degree=gps.course.deg();
    if (degree<0)degree+=360;
    tft.setCursor(-10, 180);
    
    tft.print(" R: ");
    if (degree<100){tft.print(" ");}
    if (degree<10){tft.print(" ");} 
    tft.print(degree);
    tft.print(char(0xF7));
  }  
void voltstatus(){// Oszifunktion :)
  

    if (first){
      tft.fillRect(0,1,239,127,WHITE);
      x1=analogRead(A1)/8;
      first=0;
      for (xac=0;xac<10;xac++){
        xav[xac]=analogRead(A1)/8;
      }
    }
  if (xac>9)xac=0;
  if (px==1)  tft.drawFastVLine(0,1,127,WHITE); 
  tft.drawFastVLine(px,1,127,WHITE);                  //Radieren für neue Messewerte
  
  x2=analogRead(A3)/8;                                //neue Messnung
  xav[xac]=x2;                                        //Messung ins Variablen Array packen, für Durchschnitsberechnung
  xac++;                                              //Arraycounter +1
  if (x2>xmax)xmax=x2;                                //Verlgeich ob höchster je gemessener Wert überschritten wurde
  if (x2<xmin)xmin=x2;                                //Vergleich ob geringster je gemessener Wert Unterschritten wurde
  tft.drawLine(px-1,127-x1,px,127-x2,RED);                    //Zeichne eine Line vom letzten zum aktuellen Messwert
  x1=x2;                                              //Neuer Messwert in die Ablage für den Alten
  px++;                                               //Positionszähler eins aufadieren.
  if (px>239)px=1;                                    //Wenn Positionszähler größer als Bildschirm, von vorn beginnen
  if (timer<(millis()-1000)){
    timer=millis();
    tft.setTextColor(WHITE, BLACK);
    tft.setTextSize(2);
    tft.setCursor(-10, 135);
    tft.print(" Boardspannung:");tft.print((xav[0]+xav[1]+xav[2]+xav[3]+xav[4]+xav[5]+xav[6]+xav[7]+xav[8]+xav[9])/64.0,1);tft.print("V  ");  // Gebe durchschnittsspannung an
    tft.setCursor(-10, 155);
    tft.print(" Spannungsmaxi:");tft.print(xmax/6.4,1);tft.print("V   ");
    tft.setCursor(-10, 175);
    tft.print(" Spannungsmini:");tft.print(xmin/6.4,1);tft.print("V   ");
    tft.setCursor(-10, 195);
    tft.print(" Spannungsdiff:");tft.print((xmax-xmin)/6.4,1);tft.print("V   ");
    xmax=0;xmin=255;
  }
  
  
  }
void Gstatus(){// Gmeter like Gran Tourismo!
  byte kali;
  if (!digitalRead(0)){ gkalibrate();tft.fillScreen(BLACK);}
  tft.fillCircle(-gy*100+100,-gx*100+100,2,0B0011100011100111);
  tft.fillCircle(220,-gz*100,2,0B0011100011100111);
  tft.drawFastVLine(100,0,200,WHITE);
  tft.drawFastVLine(220,0,200,WHITE);
  tft.drawFastHLine(0,100,240,WHITE);
  tft.drawFastHLine(210,150,20,WHITE);
  tft.drawFastHLine(210,50,240,WHITE);
  tft.drawCircle(100,100,100,WHITE);
  tft.drawCircle(100,100,50,WHITE);
  gx=0;gy=0;gz=0;
  //for (kali=0;kali<10;kali++){
  gx+=analogRead(A1);
  gy+=analogRead(A2);
  gz+=analogRead(A0);
  //}
  //gx/=10;gy/=10;gz/=10;
  gx=gx*0.0061-2.0625-gkx;
  gy=gy*0.0061-2.0625-gky;
  gz=gz*0.0061-2.0625-gkz;
  
  
  tft.fillCircle(-gy*100+100,-gx*100+100,2,RED);
  tft.fillCircle(220,-gz*100,2,RED);

    tft.setTextColor(WHITE, BLACK);
    tft.setTextSize(2);
    tft.setCursor(-10, 205);
    tft.print(" Gx:");if (gx>=0)tft.print(" "); tft.print(gx,2);tft.print(" ");
    tft.setCursor(-10, 225);
    tft.print(" Gy:");if (gy>=0)tft.print(" "); tft.print(gy,2);tft.print(" ");
    tft.setCursor(100, 205);
    tft.print(" Gz:");if (gz>=0)tft.print(" ");tft.print(gz,2);tft.print(" ");

  }
void gkalibrate(){// Gsensor Kalibrieren//
  float kali=0;
  for(kali=1;kali<100;kali++){
    gkx+=analogRead(A1);
    gky+=analogRead(A2);
    gkz+=analogRead(A0);
    }
  gkx/=100;
  gky/=100;
  gkz/=100;
  gkx=gkx*0.0061-2.0625;gky=gky*0.0061-2.0625;gkz=gkz*0.0061-3.0625;
  }
void tempstatus(){// Temperaturdiagramm
  byte counter;
    if (first){
      tft.setTextColor(WHITE);
    tft.setTextSize(1);
    
      tft.fillRect(0,1,239,161,WHITE);
      first=0;
      for (counter=1;counter<8;counter++){tft.drawFastHLine(0,20*counter+1,240,BLACK);
      if (counter==7){tft.drawFastHLine(0,20*counter+2,240,BLACK);tft.drawFastHLine(0,20*counter,240,BLACK);}}
      for (counter=1;counter<6;counter++){
        tft.drawFastVLine(counter*40,1,161,BLACK);
        
        
        tft.setCursor(40*counter-10, 165);
        tft.print(counter*2);
        tft.print("min");
      }
      T2=141-sensors.getTempCByIndex(0);
      T1=141-sensors.getTempCByIndex(1);
      timesincerequest=millis();  
      }
    
  if ((timesincerequest+2500)<millis()){
    Ta1=T1;
    Ta2=T2;
    T2=141-sensors.getTempCByIndex(0);
    T1=141-sensors.getTempCByIndex(1);
    sensors.requestTemperatures(); 
    timesincerequest=millis();   
    if (px>238)px=1; else px++;  


  if (px==1)  tft.drawFastVLine(0,1,161,WHITE);
  else if (!(px%40)) tft.drawFastVLine(px,1,161,BLACK);
  else  tft.drawFastVLine(px,1,161,WHITE);                  //Radieren für neue Messewerte
  
    for (counter=1;counter<8;counter++){
      tft.drawPixel(px,counter*20+1,BLACK);
      if (counter==7){tft.drawPixel(px,counter*20+2,BLACK);tft.drawPixel(px,counter*20,BLACK);}
    }          
  tft.drawFastVLine(px+1,1,161,GREEN);  
  tft.drawLine(px-1,Ta2,px,T2,RED);
  tft.drawLine(px-1,Ta1,px,T1,BLUE); //Zeichne eine Line vom letzten zum aktuellen Messwert
    
    tft.setTextColor(BLUE, BLACK);
    tft.setTextSize(2);
    tft.setCursor(-10, 185);
    tft.print(" AussenTemp:");tft.print((141-T1));tft.print(char(0xF7));tft.print("C  ");  // Gebe durchschnittsspannung an
    tft.setCursor(-10, 205);
    tft.setTextColor(RED, BLACK);
    tft.print(" MotorTemp :");tft.print((141-T2));tft.print(char(0xF7));tft.print("C  ");
    tft.setCursor(-10, 225);
    tft.setTextColor(WHITE, BLACK);
    tft.print(" Tempdiff. :");tft.print(abs(T1-T2));tft.print(char(0xF7));tft.print("C  ");
    
    tft.setTextColor(BLACK);
    tft.setTextSize(1);
    tft.setCursor(205, 13);
    tft.print("100");tft.print(char(0xF7));tft.print("C");
    
    tft.setCursor(211, 53);
    tft.print("80");tft.print(char(0xF7));tft.print("C");
   
    tft.setCursor(211, 93);
    tft.print("40");tft.print(char(0xF7));tft.print("C");

    tft.setCursor(211, 133);
    tft.print(" 0");tft.print(char(0xF7));tft.print("C");

  }                                         

  }
void option(){//Optionsmenu um Helligkeiten Sommer und Winterzeit einzustellen
   byte omenu=1;
   boolean out=0,change=0;
   if (first){
    tft.setTextColor(WHITE, BLACK);
    tft.setTextSize(2);
    tft.setCursor(-10, 10);
    if(Sommerzeit) tft.print(" Sommerzeit ");   else tft.print(" Winterzeit ");
    tft.setCursor(-10, 40);
    tft.print(" Helligk.-Tag  :");tft.print(Daylight);
    tft.setCursor(-10, 70);
    tft.print(" Helligk.-Nacht:");tft.print(Nightlight);
    tft.setCursor(-10, 100);
    tft.print(" Exit");
   }
    if (!digitalRead(0)){
      change=1;
      out=1;
      while (!digitalRead(0));
      while (out){
       
        if (!digitalRead(2)){omenu++;change=1;}
        if (!digitalRead(1)){omenu--;change=1;}
        if (!digitalRead(0)){change=1;}
        if(omenu>4)omenu=1;
        if(omenu<1)omenu=4;
          if (change){// Bildschirm nur aktualisieren, wenn sich was geändert hat.
            if (omenu==1)tft.setTextColor(BLACK, WHITE);else tft.setTextColor(WHITE, BLACK);
            tft.setCursor(-10, 10);
            if (Sommerzeit) tft.print(" Sommerzeit ");   else tft.print(" Winterzeit ");
            if (omenu==2)tft.setTextColor(BLACK, WHITE);else tft.setTextColor(WHITE, BLACK);
            tft.setCursor(-10, 40);
            tft.print(" Helligk.-Tag  :");tft.print(Daylight);tft.print("  ");
            if (omenu==3)tft.setTextColor(BLACK, WHITE);else tft.setTextColor(WHITE, BLACK); 
            tft.setCursor(-10, 70);
            tft.print(" Helligk.-Nacht:");tft.print(Nightlight);tft.print("  ");
            if (omenu==4)tft.setTextColor(BLACK, WHITE);else tft.setTextColor(WHITE, BLACK);
            tft.setCursor(-10, 100);
            tft.print(" Exit");
            change=0;
          }
          if (!digitalRead(0)) if (omenu==4){// user wählt EXIT
            out=0;
            tft.setTextColor(WHITE, BLACK);
            tft.setCursor(-10, 100);
            tft.print(" Exit");
            while(!digitalRead(0));
            }
          if (!digitalRead(0)) if (omenu==1){
                                              Sommerzeit=!Sommerzeit;
                                              tft.setTextColor(BLACK,YELLOW); 
                                              tft.setCursor(-10, 10);
                                              if (Sommerzeit) tft.print(" Sommerzeit ");   else tft.print(" Winterzeit "); 
                                              EEPROM.put(0,Sommerzeit); while(!digitalRead(0));
                                              }// user wechselt SommerWinterzeit
                                              
          if (!digitalRead(0)) if (omenu==2){while(!digitalRead(0));while(digitalRead(0)){  
                tft.setTextColor(BLACK,YELLOW); 
                tft.setCursor(-10, 40);
                tft.print(" Helligk.-Tag  :");
                tft.print(Daylight);
                tft.print("  ");
                if (!digitalRead(1))if (Daylight<255)Daylight++;
                if (!digitalRead(2))if (Daylight > 1)Daylight--;
                analogWrite(3,Daylight);
                }
                EEPROM.put(1,Daylight);
            }
           if (!digitalRead(0)) if (omenu==3){while(!digitalRead(0));while(digitalRead(0)){  
                tft.setTextColor(BLACK,YELLOW); 
                tft.setCursor(-10, 70);
                tft.print(" Helligk.-Nacht:");
                tft.print(Nightlight);
                tft.print("  ");
                if (!digitalRead(1))if (Nightlight<255)Nightlight++;
                if (!digitalRead(2))if (Nightlight> 1)Nightlight--;
                analogWrite(3,Nightlight);
                }
                EEPROM.put(2,Nightlight);
            }
          }
        }   
        first=0;
      } 
void setup(void){// Setupfunktion, Initialisieren von Sensoren, Auslesen des Speichers usw.
  if(RTC.read(tm)==0)RTC.write(tm);
  EEPROM.get(0, Sommerzeit);
  EEPROM.get(1, Daylight);
  EEPROM.get(2, Nightlight);
  if(Daylight==0)Daylight=1;
  if(Nightlight==0)Nightlight=1;
  pinMode(3,OUTPUT);
  digitalWrite(0,1);
  digitalWrite(1,1);
  digitalWrite(2,1);
  
  gkalibrate();
  tft.init(240, 240);   // initialize a ST7789 chip, 240x240 pixels
  tft.setTextWrap(false);
  tft.setRotation(3);
  tft.fillScreen(BLACK);
  ss.begin(GPSBaud);
  sensors.begin();
  sensors.requestTemperatures();
  

}
void loop() {
 // while(!wtag)getbatday();
// Menu1
  if (!digitalRead(2)){menu++;refresh();}
  if (!digitalRead(1)){menu--;refresh();}
  if (menu==0) menu=6; else if (menu==7) menu=1;
  if (menu==1) startbildschirm();
  if (menu==2) gpsstatus();
  if (menu==3) voltstatus();
  if (menu==4) Gstatus();
  if (menu==5) tempstatus();
  if (menu==6) option();  
  if (analogRead(A6)>300) analogWrite(3,Nightlight);else if(analogRead(A6)<100) analogWrite(3,Daylight);
 /* showInnen();
  showgf();
  showTime();
  showGPS();
  showTemp();*/
}
