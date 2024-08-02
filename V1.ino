/*Initialisire gpsmodul
 * 
 */
#include <SoftwareSerial.h>
#include <TinyGPS++.h>
// The TinyGPS++ object
TinyGPSPlus gps;
static const int RXPin = 12, TXPin = 11;
static const uint32_t GPSBaud = 4800;

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
void setup() {
  ss.begin(GPSBaud);
  lcd.begin(8, 2);
  lcd.setCursor(0,0);
  lcd.print("START");
}
void lcdClean(){ // LcdBildschirm Löschen
  lcd.setCursor(0,0);
  lcd.print("        ");
  lcd.setCursor(0,1);
  lcd.print("        ");
  }
void getTemp(){ // Hole die Tempereatur und die Luftfeuchtigkeit und speichere sie in Var: temperature und humidity
   err = dht11.read(&temperature, &humidity, NULL);
  }

void showClima(){// Zeige Temperatur und Luftfeuchte im Fahrzeug an
  lcdClean();
  getTemp();
  lcd.setCursor(0,0);
 
  lcd.print((int)temperature);
  lcd.print("C bei");
  lcd.setCursor(0,1);
  lcd.print((int)humidity);
  lcd.print("% rel");
  }

void showClock(){
  lcdClean();
  while (ss.available() > 0)
    if (gps.encode(ss.read()))
    lcd.setCursor(0, 0);
    if (gps.time.hour() < 10) lcd.print(F("0"));
    lcd.print(gps.time.hour()+2);
    lcd.print(F(":"));
    if (gps.time.minute() < 10) lcd.print(F("0"));
    lcd.print(gps.time.minute());
    lcd.print(F(":"));
    if (gps.time.second() < 10) lcd.print(F("0"));
    lcd.print(gps.time.second());
  
  }
void loop() {
  showClock();
  delay(5000);
  

}
