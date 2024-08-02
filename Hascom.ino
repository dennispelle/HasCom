
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 10
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);


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
int err;  
SimpleDHT11 dht11(2);

/*
 * Initialisiere Display
 */
#include <LiquidCrystal.h>
LiquidCrystal lcd(3,4,5,6,7,8);
byte jahr,monat,tag,stunde,mimute,sekunde,zeitzone=2;//Zeitzone für sommer/winterzeit usw
byte uzeit=zeitzone+1;
double uMin,uMax,uMinabs=1023,uMaxabs,uAvr,uNow,uRip,uRipabs;
void setup() {
  ss.begin(GPSBaud);
  lcd.begin(8, 2);
  sensors.begin();

}
void lcdclean(){ // LcdBildschirm Löschen
  lcd.setCursor(0,0);
  lcd.print("        ");
  lcd.setCursor(0,1);
  lcd.print("        ");
  }
void getTemp(){ // Hole die Tempereatur und die Luftfeuchtigkeit und speichere sie in Var: temperature und humidity
   err = dht11.read(&temperature, &humidity, NULL);
  }
void showClima(){// Zeige Temperatur und Luftfeuchte im Fahrzeug an
  lcdclean();
  getTemp();
  lcd.setCursor(0,0);
 
  lcd.print((int)temperature);
  lcd.print("C bei");
  lcd.setCursor(0,1);
  lcd.print((int)humidity);
  lcd.print("% rel");
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
   
    if (stunde<uzeit)//überlaufzeit
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
    lcdclean();  
// Kein Datum nur die Uhrzeit   
    lcd.setCursor(0,0);    
  //  lcd.print("Uhrzeit:");
   
    if (tag<10) lcd.print("0");
    lcd.print(tag); lcd.print(".");
    
    if (monat<10) lcd.print("0");
    lcd.print(monat);lcd.print(".");
        
    lcd.print(jahr);
 
    lcd.setCursor(1, 1); 
//  lcd.setCursor(0,1); //falls sekunden angezeigt werden sollen
    if (stunde<10) lcd.print("0");
    lcd.print(stunde); lcd.print(":");
    
    if (mimute<10) lcd.print("0");
    lcd.print(mimute);
    
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
  for(x=0;x<200;x++){
      uNow=5.0*analogRead(A1)/1023.0;
      uAvr+=uNow;
      if (uNow>uMax)uMax=uNow;
      if (uNow<uMin)uMin=uNow;
    }
   uAvr/=200;
   uRip=uMax-uMin;
   if (uRipabs<uRip)uRipabs=uRip;
   if (uMaxabs<uMax)uMaxabs=uMax;
   if (uMinabs>uMin)uMinabs=uMin;
  }
void showVolt(){
  lcdclean();
  lcd.setCursor(0,0);
  lcd.print("Um=");lcd.print(uAvr);lcd.print("V");
  lcd.setCursor(0,1);
  lcd.print("Ud=");lcd.print(uRip);lcd.print("V");
  }
void showTemp(){
  lcdclean();
  lcd.setCursor(0,0);
  lcd.print("T1=");lcd.print(sensors.getTempCByIndex(0));
  lcd.setCursor(0,1);
  lcd.print("T2=");lcd.print(sensors.getTempCByIndex(1));
}
void loop() {
  sensors.requestTemperatures();// Hole Temperaturen der Tempsensoren
 
  showClima();
  delay(2000);
  showClock();
  delay(2000);
  getVolt();
  showVolt();
  delay(2000);
  showTemp();
  delay(2000);
}
