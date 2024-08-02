
#include <EEPROM.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 10
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
boolean Sommerzeit;

#include <SoftwareSerial.h>
#include <TinyGPS++.h>
// The TinyGPS++ object
TinyGPSPlus gps;
static const int RXPin = 11, TXPin = 12;
static const uint32_t GPSBaud = 9600;
const int UTC_offset = 2;
SoftwareSerial ss(RXPin, TXPin);
  /*initialisiere den dht11 Sensor (Temperatur 0-50°C;Luftfeuechte 20%-95%RH)
   * dht11.read(&temperature, &humidity, NULL)
   */
#include <SimpleDHT.h>
byte temperature = 0;
byte humidity = 0; 
float T1,T2;
int err;  
SimpleDHT11 dht11(2);

/*
 * Initialisiere Display
 */
#include <LiquidCrystal.h>
LiquidCrystal lcd(3,4,5,6,7,8);
byte jahr,monat,tag,stunde,mimute,sekunde,zeitzone  ;//Zeitzone für sommer/winterzeit usw
byte Menu,Nightlight,Daylight;
byte L,M,R;
double uMin,uMax,uMinabs=1023,uMaxabs,uAvr,uNow,uRip,uRipabs;



void setup() {

  digitalWrite(13,HIGH);
  pinMode(9,OUTPUT);
  EEPROM.get(0, Sommerzeit);
  EEPROM.get(1, Daylight);
  EEPROM.get(2, Nightlight);
  zeitzone=1+Sommerzeit;
  digitalWrite(A1,HIGH);
  digitalWrite(A2,HIGH);
  digitalWrite(A3,HIGH);
  
  ss.begin(GPSBaud);
  lcd.begin(8, 2);
  sensors.begin();

}
void getTemp(){ // Hole die Tempereatur und die Luftfeuchtigkeit und speichere sie in Var: temperature und humidity
   err = dht11.read(&temperature, &humidity, NULL);
  }
void showClima(){// Zeige Temperatur und Luftfeuchte im Fahrzeug an
  lcd.setCursor(0,0);
 
  lcd.print((int)temperature);
  lcd.print("C bei ");
  lcd.setCursor(0,1);
  lcd.print((int)humidity);
  lcd.print("% r.Lf");
  }
void getClock(){
    while (ss.available() > 0)
    if (gps.encode(ss.read()))

    //hole die aktuelle Zeit über GPS 

    jahr=gps.date.year()-2000;
    //jahr=19;//schaltjahr
    monat=gps.date.month();
    //monat=12;//testmonat
    tag=gps.date.day();
    //tag=31;
    stunde=gps.time.hour();
    //stunde=23;//überlaufstunde
    mimute=gps.time.minute();
    sekunde=gps.time.second();

    //korrigiere die Zeit nach der Zeitzone
    stunde=stunde+zeitzone;
    stunde=stunde%24;
   
    if (stunde<zeitzone)//überlaufzeit
    {
      tag++;     
      if (tag==32){       tag=1;  monat++;} //wenn der monat 32tage hat
      if (tag==31) //möglicher überlauftag
      {
        if (monat==4){    tag=1;  monat++;}
        if (monat==6){    tag=1;  monat++;}
        if (monat==9){    tag=1;  monat++;}
        if (monat==11){   tag=1;  monat++;}
      }
      if (monat==2){
        if (tag==30){     tag=1;  monat++;}
        if (tag==29){if ((jahr%4)!=0){tag=1;monat++;}}
        }   //wenn februar und kein schaltjahr
      //if ((monat==2)&&((jahr%4)==0)&&(tag==30)){tag=1; monat++;}  //wenn februar und schaltjahr

      if (monat==13){monat=1;jahr++;}
          
    }


          
}
void showClock(){
  getClock();
  
// Kein Datum nur die Uhrzeit   
    lcd.setCursor(0,0);    
  //  lcd.print("Uhrzeit:");
   
    if (tag<10) lcd.print("0");
    lcd.print(tag); lcd.print(".");
    
    if (monat<10) lcd.print("0");
    lcd.print(monat);lcd.print(".");
        
    lcd.print(jahr);
 
    lcd.setCursor(0, 1); 
    lcd.print(" ");
//  lcd.setCursor(0,1); //falls sekunden angezeigt werden sollen
    if (stunde<10) lcd.print("0");
    lcd.print(stunde); lcd.print(":");
    
    if (mimute<10) lcd.print("0");
    lcd.print(mimute);
    lcd.print("  ");
    
/*
    lcd.print(":");
    if (sekunde<10) lcd.print("0");
    lcd.print(sekunde);
*/
  
  }  
void getVolt(){
  int x;
  uAvr=0;
  uMin=1023;
  uMax=0;
  for(x=0;x<50;x++){
      uNow=5.0*analogRead(A0)/1023.0;
      uAvr+=uNow;
      if (uNow>uMax)uMax=uNow;
      if (uNow<uMin)uMin=uNow;
    }
   uAvr/=50;
   uRip=uMax-uMin;
   if (uRipabs<uRip)uRipabs=uRip;
   if (uMaxabs<uMax)uMaxabs=uMax;
   if (uMinabs>uMin)uMinabs=uMin;
  }
void showVolt(){
  getVolt();// Messe Boardspannung
  lcd.setCursor(0,0);
  lcd.print("Um=");lcd.print(uAvr);lcd.print("V");
  lcd.setCursor(0,1);
  lcd.print("Ud=");lcd.print(uRip);lcd.print("V");
  }
void showTemp(){
  if((millis()/1000)%5){
  T1=sensors.getTempCByIndex(0);
  T2=sensors.getTempCByIndex(1);
  lcd.setCursor(0,0);
  lcd.print("T1=");lcd.print(T1);
  lcd.setCursor(0,1);
  lcd.print("T2=");lcd.print(T2);}
}
void setNightlight(){
    lcd.setCursor(0,0);
    lcd.print("LCD Nacht");
    lcd.setCursor(0,1);
    lcd.print("        ");
    while (digitalRead(A2)){
      lcd.setCursor(2,1);
      if ((Nightlight/2.55)<100) lcd.print(" ");
      lcd.print(Nightlight/2.55);
      lcd.print("%");
      if (!digitalRead(A1)){ Nightlight--;delay(50);}
      if (!digitalRead(A3)){ Nightlight++;delay(50);}
      analogWrite(9,Nightlight);
      }
      EEPROM.put(2, Nightlight);
    
  }
void setDaylight(){
    lcd.setCursor(0,0);
    lcd.print("LCD  Tag");
    lcd.setCursor(0,1);
    lcd.print("        ");
    while (digitalRead(A2)){
      lcd.setCursor(2,1);
      if ((Nightlight/2.55)<100) lcd.print(" ");
      lcd.print(Nightlight/2.55);
      lcd.print("%");
      if (!digitalRead(A1)){ Daylight--;delay(50);}
      if (!digitalRead(A3)){ Daylight++;delay(50);}
      analogWrite(9,Daylight);
      }
      EEPROM.put(1,Daylight);
    
  }

byte getButton(){
  byte doit=0;
  unsigned long timer=millis();
if ((!digitalRead(A3))&&(millis()-timer>2000)){
    lcd.setCursor(0,1);
    lcd.print("Tag%????");
    }
  if (!digitalRead(A2)&&(millis()-timer>2000)){
    lcd.setCursor(0,1);
    lcd.print("So/Wi? ");
    }
  if (!digitalRead(A1)&&(millis()-timer>2000)){
    lcd.setCursor(0,1);
    lcd.print("Nacht%???");
    }
  while(!digitalRead(A2)){
    
    if ((Menu==0)&&((millis()-timer)>5000)){
      if (Sommerzeit)Sommerzeit=0; else Sommerzeit=1;
      zeitzone=1+Sommerzeit;
      lcd.print("!");
      EEPROM.put(0, Sommerzeit);
      delay(1000);
      break;}
    }
  while (!digitalRead(A1)or!digitalRead(A3)){
      if (digitalRead(A1))L=0; else {L=1;doit=64;}
      if (digitalRead(A3))M=0; else {M=1;doit=192;}
      if ((!digitalRead(A1))&&((millis()-timer)>5000)){setDaylight();break;}
      if ((!digitalRead(A3))&&((millis()-timer)>5000)){setNightlight();break;}
    }
    return doit; 
  }

  
void loop() { 
    if((millis()/1000)%5){
      T1=sensors.getTempCByIndex(0);}
      if((millis()/1000+3)%5){
      T2=sensors.getTempCByIndex(1);}
      getTemp();
    Menu+=getButton();
    if (digitalRead(13)) analogWrite(9,Nightlight); 
    else analogWrite(9,Daylight);
   
    
    if (Menu==0)showClock();
    if (Menu==64)showClima();
    if (Menu==128)showTemp();
    if (Menu==192)showVolt();
  
  
  

}
