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
byte wtag, gmonat, gtag, gstunde, gminute, gsekunde, zeitzone=1 ,gpsdn,gpstn; ; //Zeitzone für sommer/winterzeit usw
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


void getTemp(){ // Hole die Tempereatur und die Luftfeuchtigkeit und speichere sie in Var: temperature und humidity
  if ((timesincerequest+2000)<millis()){// schaue ob die letzte Messung der der Temperatursenosren wenigstens eine sekunde her ist, ansonsten überspringen
   T1=sensors.getTempCByIndex(0);
   T2=sensors.getTempCByIndex(1);
   sensors.requestTemperatures();
   timesincerequest=millis();
   err = dht11.read(&temperature, &humidity, NULL);}
  }
void startbildschirm(){// Startbildschirm, Zeigt die Uhrzeit und das Datum Groß an, dazu Clima und Temperaturen an.
    
    
// Block für die Uhrzeit und das Datum
  getGpsClock();
  if (!gpst) getbatclock(); //schau ob die GPSzeit stimmt, wenn nicht hol die Zeit aus der Uhr
  if (!gpsd) getbatday(); // nochmal das gleiche mit dem Tag

  // Uhrzeit, Stunde:Minute:Sekunde
  tft.setCursor(-30, 5);
  tft.setTextColor(WHITE, BLACK);
  tft.setTextSize(7);
  tft.print(" ");
  if (gstunde < 10)tft.print("0"); tft.print(gstunde); 
  tft.print(":"); 
  if (gminute < 10) tft.print("0"); tft.print(gminute);
  //tft.print(":"); 
  //if (gsekunde < 10) tft.print("0"); tft.print(gsekunde);
  // Wochentag Mittig
  
  
  tft.setTextSize(3 );
  tft.setCursor(-10,60);
  tft.print(" ");
  tft.print(wochentag(wtag));
  
  tft.setCursor(-10, 90);
  tft.print(" "); 
  if (gtag < 10) tft.print("0"); tft.print(gtag); 
  tft.print("."); 
  tft.print(monat(gmonat)); 
  //if (gmonat < 10) tft.print("0"); tft.print(gmonat);
  tft.setCursor(-10, 120);
  tft.print(" "); 
  if (gjahr < 10) tft.print("0"); tft.print(gjahr);

// Zeige das Raumklima an
  getTemp(); //Hole die Temperatur und Luftfeucht vom DHT
  tft.setCursor(-10,150);
  tft.print(" Innen:");
  tft.setTextColor(RED, BLACK);
  tft.print(temperature);
  tft.setTextColor(WHITE, BLACK);
  tft.setTextSize(2);
  tft.print(char(0xF7));
  tft.print("C ");
  tft.setTextSize(3);
  tft.setTextColor(BLUE, BLACK);
  tft.print(humidity);
  tft.setTextColor(WHITE, BLACK);
  tft.setTextSize(2);
  tft.print("%");

// Zeige Temperatursensoren an
  tft.setCursor(-10,180);
  tft.setTextSize(3);
  tft.print(" Aussen: ");tft.print(T1,0);    tft.setTextSize(2);       tft.print(char(0xF7));    tft.print("C");
  tft.setTextSize(3);
  tft.setCursor(-10,210);
  tft.print(" Motor:  ");tft.print(T2,0);    tft.setTextSize(2);    tft.print(char(0xF7));    tft.print("C");
 

  tft.setCursor(140,120);
  tft.setTextSize(3);
  tft.print(analogRead(A6)/1023.0*20.0,1); tft.setTextColor(RED, BLACK);tft.print("V  ");
 
}
String monat(byte t){//konvertiert eine zahl in einen String 
  switch (t) {
    case 1:
      return "Januar   ";
    break;
    case 2:
      return "Februar  ";
    break;
    case 3:
      return "März     ";
    break;
    case 4:
      return "April    ";
    break;
    case 5:
      return "Mai      ";
    break;
    case 6:
      return "Juni     ";
    break;
    case 7:
      return "Juli     ";
    break;
    case 8:
      return "August   ";
    break;
    case 9:
      return "September";
    break;
    case 10:
      return "Oktober  ";
    break;
    case 11:
      return "November ";
    break;
    case 12:
      return "Dezember ";
    break;
  }  
}
String wochentag(byte t){//konvertiert eine zahl in einen String 
  switch (t) {
    case 1:  
      return "Montag    ";
    break;
    case 3:  
      return "Dienstag  ";
    break;
    case 4:
      return "Mittwoch  ";
    break;   
    case 5:
      return "Donnerstag";
    break;   
    case 6:
      return "Freitag   ";
    break;   
    case 7:
      return "Samstag   ";
    break;
    case 2:
      return "Son°tag   ";
    break;
  }  
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
void setup(void){// Setupfunktion, Initialisieren von Sensoren, Auslesen des Speichers usw.
  EEPROM.get(0, Sommerzeit);
  EEPROM.get(1, Daylight);
  EEPROM.get(2, Nightlight);

  
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
  
  //drawFrame();

}
void refresh(){// Bildschirm löschen
  tft.fillScreen(BLACK);
  first=1;
  px=1;
  }
void gpsstatus(){// Funktionsbildschirm für GPS, zeigt Satelitenanzahl, Koordinaten, Geschwindigkeit usw an
   while (ss.available() > 0)
    if (gps.encode(ss.read())); 
  tft.setTextColor(WHITE, BLACK);
  tft.setTextSize(3);
  tft.setCursor(-10, 5);
  tft.print(" Sateliten:"); tft.print(gps.satellites.value());tft.print(" ");
  tft.setCursor(-10, 30);
  tft.print(" B:"); tft.print(gps.location.lat(), 6);tft.print("  ");
  tft.setCursor(-10, 55);
  tft.print(" L:"); tft.print(gps.location.lng(), 6);tft.print("  "); 
  tft.setCursor(-10, 80);
  tft.print(" H:"); tft.print(gps.altitude.meters(),0); tft.print("m    ");
  tft.setCursor(-10, 130);
  tft.print(" D:"); tft.print(gps.date.day()); tft.print(".");  tft.print(gps.date.month());tft.print("."); tft.print(gps.date.year());
  tft.setCursor(-10, 105);
  tft.print(" C:");  tft.print(gps.time.hour()); tft.print(":"); tft.print(gps.time.minute());tft.print(":"); tft.print(gps.time.second());tft.print(" ");
  tft.setCursor(-10, 155);
  tft.print(" S:"); tft.print(gps.speed.kmph(),1);tft.print("Km/h   ");
  tft.setCursor(-10, 180);
  tft.print(" R: "); tft.print(gps.course.deg(),0);tft.print(char(0xF7));tft.print("   ");
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
  
  x2=analogRead(A6)/8;                                //neue Messnung
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
  tft.fillCircle(-gy*50+100,-gx*50+100,2,0B0001100001100011);
  tft.fillCircle(220,gz*50+100,2,0B0000100000100001);
  tft.drawFastVLine(100,0,200,WHITE);
  tft.drawFastVLine(220,0,200,WHITE);
  tft.drawFastHLine(0,100,240,WHITE);
  tft.drawFastHLine(210,150,20,WHITE);
  tft.drawFastHLine(210,50,240,WHITE);
  tft.drawCircle(100,100,100,WHITE);
  tft.drawCircle(100,100,50,WHITE);
  gx=0;gy=0;gz=0;
  //for (kali=0;kali<10;kali++){
  gx+=analogRead(A0);
  gy+=analogRead(A1);
  gz+=analogRead(A2);
  //}
  //gx/=10;gy/=10;gz/=10;
  gx=gx*0.0061-2.0625-gkx;
  gy=gy*0.0061-2.0625-gky;
  gz=gz*0.0061-2.0625-gkz;
  
  
  tft.fillCircle(-gy*50+100,-gx*50+100,2,RED);
  tft.fillCircle(220,gz*50+100,2,RED);

    tft.setTextColor(WHITE, BLACK);
    tft.setTextSize(2);
    tft.setCursor(-10, 205);
    tft.print(" G.B:");tft.print(gx,1);tft.print(" ");
    tft.setCursor(-10, 225);
    tft.print(" G.S:");tft.print(gy,1);tft.print(" ");
    tft.setCursor(120, 205);
    tft.print("G.F:");tft.print(gz,1);tft.print(" ");

  }
void gkalibrate(){// Gsensor Kalibrieren//
  float kali=0;
  for(kali=1;kali<100;kali++){
    gkx+=analogRead(A0);
    gky+=analogRead(A1);
    gkz+=analogRead(A2);
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
      T1=141-sensors.getTempCByIndex(0);
      T2=141-sensors.getTempCByIndex(1);
      timesincerequest=millis();  
      }
    
  if ((timesincerequest+2500)<millis()){
    Ta1=T1;
    Ta2=T2;
    T1=141-sensors.getTempCByIndex(0);
    T2=141-sensors.getTempCByIndex(1);
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
   boolean out=0;
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
    
    if (!digitalRead(0)){
      out=1;
      while (!digitalRead(0));
      while (out){
       
        if (!digitalRead(2)){omenu++;}
        if (!digitalRead(1)){omenu--;}
        if(omenu>4)omenu=1;
        if(omenu<1)omenu=4;
          
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
          if (!digitalRead(0)) if (omenu==4){out=0;while(!digitalRead(0));}// user wählt EXIT
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
                if (!digitalRead(2))if (Daylight > 0)Daylight--;
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
                if (!digitalRead(2))if (Nightlight> 0)Nightlight--;
                analogWrite(3,Nightlight);
                }
                EEPROM.put(2,Nightlight);
            }
          }
        }   
      }
  
void loop() {
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
  if (analogRead(A7)>500) analogWrite(3,Nightlight);else analogWrite(3,Daylight);
 /* showInnen();
  showgf();
  showTime();
  showGPS();
  showTemp();*/
}
