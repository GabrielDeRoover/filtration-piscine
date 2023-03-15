#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <SimpleKeypad.h>
#include <LiquidCrystal_I2C.h>
#include <iarduino_RTC.h> 

LiquidCrystal_I2C lcd(0x27,16,2);

//keypad
const byte nb_rows = 4;//nombre de lignes
const byte nb_cols = 4;//nombre de colonnes
char key_chars[nb_rows][nb_cols] = {
  {1, 2, 3, 'A'},
  {4, 5, 6, 'B'},
  {7, 8, 9, 'C'},
  {'*', 10, '#', 'D'}
};
byte rowPins[nb_rows] = {2, 3, 4, 5};//entrées numériques où sont branchées les lignes
byte colPins[nb_cols] = {6, 7, 8, 9};//entrées numériques où sont branchées les colonnes

SimpleKeypad kpd((char*)key_chars, rowPins, colPins, nb_rows, nb_cols);   // New keypad called kpd

#define WATER_TEMP_BUS 13                     
OneWire oneWire(WATER_TEMP_BUS); 
DallasTemperature sensors(&oneWire);

//horloge RTC
iarduino_RTC time (RTC_DS3231);

//def timer
int SEC = 0;
int MIN = 0;
int HOUR = 0;
unsigned long temps_filtration_commence;
float temps_de_filtration;
char* H_actuelle;

//def minuteur
long int SEC1 = 0;
long int MIN1 = 0;
long int HOUR1 = 0;
unsigned long temps_filtration_commence1;
float temps_de_filtration1;

const char * degree = "\337C"; // °C

void setup() 
{
  pinMode(12, OUTPUT);
  //Init Serial USB
  Serial.begin(9600);
  Serial.println(F("Initialize System"));
  delay(500);
  //Init RTC
  time.begin();
  //         sec, min, hour, date, month, year, wed
  time.settime(0, 0, 21, 26, 2, 23, 0);
  //Init lcd
  lcd.init();
  lcd.backlight();
  lcd.setCursor(3,0);
  lcd.print("Initialize");
  lcd.setCursor(5,1);
  lcd.print("System");
  delay(5000);
  menu();    
}

void menu()
{
  lcd.clear();
  lcd.print("1: tps restant");
  lcd.setCursor(0,1);
  lcd.print("2: filtra manu");
}

void choix_utilisateur()
{
switch(kpd.getKey())
{
    
  case 'C':
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("1: tps restant");
    lcd.setCursor(0,1);
    lcd.print("2: filtra manu");//
    return 0;
  break;
  
  case 'D':
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("3: temperature");
    lcd.setCursor(0,1);
    lcd.print("4: heure");
    return 0;
  break;

  case 1://temps de filtration restant
    lcd.clear();
    if(temps_de_filtration != 0)
    {
      lcd.setCursor(0,0);
      lcd.print("filtration");
      lcd.setCursor(0,1);
      lcd.print("manuelle en cour");
    }else
    {
      lcd.setCursor(0,0);
      lcd.print("il reste");
      lcd.setCursor(0,1);
      lcd.print(String(HOUR1) + String(" : ") + String(MIN1) + String(" : ") + String(SEC1));
    }    
   return 0;
  break;
  
  case 2://filtration manuelle
    lcd.clear();
    if (digitalRead(12) == HIGH)
    {
      lcd.print("filtre depuis");
      lcd.scrollDisplayRight();
      lcd.setCursor(0,1);
      lcd.print(String(HOUR) + String(" : ") + String(MIN) + String(" : ") + String(SEC));
    }else
    {
      lcd.print("filtration arret");
    }
    return 0;
  break;
  
  case 4://heure affichage
    lcd.clear();
    lcd.print(time.gettime("H"));
    //lcd.print(Hour);
    lcd.print(":");
    lcd.print(time.gettime("i"));
    lcd.print(":");
    lcd.print(time.gettime("s"));
    return 0;
  break;

  case 10:
    digitalWrite (12, LOW);
    temps_de_filtration = 0;
    lcd.clear();
    lcd.print("filtration stop");
    return 0;
  break;

  case 9:
    if(digitalRead(12) == LOW){HOUR = 0; MIN = 0; SEC = 0; temps_de_filtration = 0;}
    digitalWrite(12, HIGH);

    temps_filtration_commence = millis();
    lcd.clear();
    lcd.print("filtration start");
    return 0;
  break;

  case 3://température affichage
    lcd.clear();
    lcd.print("il fait :");
    sensors.requestTemperatures();
    float dTempWater = sensors.getTempCByIndex(0);
    lcd.print(dTempWater);
    lcd.print(degree);
    return 0;
  break;  

  default:
   lcd.clear();
   lcd.print("erreur");
   delay(3000);
   menu();
   return 0;
  break;  
}
}

void loop() 
{
  choix_utilisateur();

 //temps filtration manuel
 if(digitalRead(12) == HIGH)
 { 
    if(temps_de_filtration1 == 0)
   {  
      if (millis() - temps_filtration_commence > 995)
     { 
      temps_filtration_commence = millis();
      SEC ++;
      temps_de_filtration ++;
      if (SEC > 60) { SEC = 1; MIN ++; }
       if (MIN > 60) { MIN = 0; HOUR ++; }
     }
   }
 }

 H_actuelle = time.gettime("H");
 int h = atoi(H_actuelle);
 int H_alarme = 22;
 if (h == H_alarme){digitalWrite(12, HIGH);}
 char* M_actuelle = time.gettime("i");
 int m = atoi(M_actuelle);
  int M_alarme = 20;

 //temps de filtration nécessaire
  if (h == H_alarme && m == M_alarme && temps_de_filtration1 == 0 )
 {
    sensors.requestTemperatures();
    float dTempWater = round(sensors.getTempCByIndex(0));
    temps_de_filtration1 = round(dTempWater / 2 * 3600);
    SEC1 = temps_de_filtration1;
    MIN1 = SEC1 / 60;
    SEC1 = SEC1 - (MIN1 * 60);
    HOUR1 = MIN1 / 60;
    MIN1 = MIN1 - (HOUR1 * 60);
 }

  if(temps_de_filtration1 != 0)
 {  
    if (millis() - temps_filtration_commence1 > 995)
   {
      temps_filtration_commence1 = millis();
      SEC1 --;
     temps_de_filtration1 --;
     if (SEC1 <= 0) { SEC1 = 59; MIN1 --; }
       if (MIN1 <= 0) { MIN1 = 59; HOUR1 --; }
       if(HOUR1 <= 0 && MIN1 <=0){digitalWrite(12, LOW);}
   }
 }   

}