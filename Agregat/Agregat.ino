#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include <SPI.h>
#include <SD.h>

RTC_DS1307 rtc;
File dataFile;
//alarme
int alarma1=22;
int alarma2=28;
//durata--> rpm
const int pulsePin = 2;  // Pin to which the pulses are connected
volatile unsigned long prevPulseTime = 0;
volatile unsigned long lastPulseTime = 0;
volatile boolean pulseDetected = false;
double rpm=0 ;
unsigned long rpm_pompa=0 ;
const int pulsesPerRevolution = 2;
double frequency;
unsigned long pulseInterval;
//scalare debit
const int Pistonare = 25; // buton
int starep=0;
double LperC=4.7123;
int Piston=100;
volatile int count = 0;
int debitinst=0;


//analog debit
int edebit=5 ;//iesire analog debit
int iesire1;
double Dmax= 612.6;
//analog presiune
int sensorPin = A0;
int sensorValue = 0; //valoare citita intrare
int limita=6; //iesire alarma
int val=0; //valoare scalata
int memorie=250; //limita setata
const int buton=4;//buton alarma
int stare;//stare intrare
//volum total
int reset=7;
int resetare;
int verif;
int scal;
int total;
float y;
//verif
int c=0;
int b=0;
int i;
float memo; 
//ecran
LiquidCrystal_I2C lcd(0x27, 20, 4);
int a=0;
//sd
const int chipSelect = 53;


void setup() {
 //alarma
 pinMode(22,OUTPUT);
 pinMode(28,OUTPUT);
   //ecran
  lcd.init();                     
  lcd.backlight();
  //sd
  if (!SD.begin(chipSelect)) {
    lcd.setCursor(3, 2);
    lcd.print("Eroare card SD");
    digitalWrite(22,HIGH);
  }
  pinMode(chipSelect, OUTPUT);

  Serial.print("Date,Time,Presiune[bar],Debit[l/min],Volum[litri]");

  //rtc
   if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    digitalWrite(28,HIGH);
  }
  if (!rtc.isrunning()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }


  //intrare reset total
 pinMode(reset,INPUT);
  //iesire alarma
  pinMode(limita,OUTPUT);
  //buton alarma
  pinMode(buton, INPUT_PULLUP);
  //iesire
  pinMode(edebit, OUTPUT);
  //selectare pistonare
  pinMode(Pistonare, INPUT_PULLUP);
 // attachInterrupt(digitalPinToInterrupt(Pistonare), risingEdgeInterrupt1, RISING);
  //impulsuri
  pinMode(pulsePin, INPUT);  // Set the pulsePin as an input with pull-up resistor
  attachInterrupt(digitalPinToInterrupt(pulsePin), pulseInterrupt, FALLING);  // Attach interrupt to the falling edge of the pulse
  Serial.begin(9600);
}



void loop() {
 if (pulseDetected) {
    // Calculate the time between the most recent pulse and the second-most recent pulse
    pulseInterval = lastPulseTime - prevPulseTime;

    // Print the time interval to the serial monitor
   frequency = 1.0 / (pulseInterval / 1000000.0);
   rpm=(frequency*60);
   
    pulseDetected = false;
  }

 //piston
 starep= digitalRead(Pistonare);
 if(starep==0){
   count++;

   }
 if(count>=2){
  count=0;
 }
 
 switch (count){
  case 1:
  LperC=6.232;
  Piston=115;
 
  break;
  case 0:
  LperC=4.7123;
  Piston=100;

  break;
 }
 

//scalare
 rpm_pompa=rpm/3.04;
//4.85
//iesire debit
 debitinst=((rpm_pompa)*(LperC));
 if(rpm_pompa<=150){
   iesire1=map(debitinst,0,935,0,255);
 } 
 else{
   iesire1=255;
 }
 analogWrite(edebit,iesire1);
 //volum total
 y=LperC/3.04;
 total=verif*y;     
  //senzor presiune
  sensorValue = analogRead(sensorPin);
  val= map(sensorValue,0,1023,-175,700); 
  //alarma
  stare=digitalRead(buton);
  if(stare==LOW){
   memorie=memorie+5;
    if(memorie>=701){
       memorie=0;
    }
  }
  if(val<=memorie){
   digitalWrite(limita,HIGH);
  }
  else{
    digitalWrite(limita,LOW);
  }

 //reset
  resetare= digitalRead(reset);
  if(resetare==HIGH){
    verif=0;
  }
  //idle
  i++;
  if(i==1){
    memo=rpm_pompa;
  }
 if (i>=8 && memo==rpm_pompa ){
  rpm=0;
 }
 //rtc
 DateTime now = rtc.now();
 //initiere
 while(c !=1){
   c++;
    dataFile = SD.open("LOGGER.txt", FILE_WRITE); 
   if(dataFile){
    digitalWrite(22,HIGH);
    digitalWrite(28,HIGH);
    delay(30);
    digitalWrite(22,LOW);
    digitalWrite(28,LOW);
    delay(40);
   //
    delay(10);
   dataFile.println("Date,Time,Presiune[bar],Debit[l/min],Volum[litri]");
   delay(10);
   dataFile.close();
   //
   digitalWrite(22,HIGH);
    delay(30);
    digitalWrite(22,LOW);
    digitalWrite(28,HIGH);
    delay(40);
   digitalWrite(28,LOW);
   delay(100);
  
   
   }
   else{
    lcd.setCursor(1, 2);
    lcd.print("Error open card SD");
    digitalWrite(22,HIGH);
   }
 }
 b++;
  //sd
 if(b>15){
  dataFile = SD.open("LOGGER.txt", FILE_WRITE);
  if(dataFile){

  dataFile.print(now.year()); 
  dataFile.print("/");
  dataFile.print(now.month());
  dataFile.print("/");
  dataFile.print(now.day());
  dataFile.print(",");
  dataFile.print(now.hour());
  dataFile.print(":");
  dataFile.print(now.minute());
  dataFile.print(":");
  dataFile.print(now.second());
  dataFile.print(",");
  dataFile.print(val);
  dataFile.print(",");
  dataFile.print(debitinst);
  dataFile.print(",");
  dataFile.println(total);
  dataFile.close();
  }
  else{ 
    lcd.setCursor(1,2);
    digitalWrite(22,HIGH);
  }
 }

 //ecran lcd
 a++;
 if(a>21){
  a=0;
 }
 if(a==20){
    lcd.clear();
  }
 lcd.setCursor(0,0);
 lcd.print("Piston:");
 lcd.setCursor(8,0);
 lcd.print(Piston);
 lcd.setCursor(12, 0);
 lcd.print("mm");
 lcd.setCursor(0, 1);
 lcd.print("Alarma:");
 lcd.setCursor(8, 1);
 lcd.print(memorie);
 lcd.setCursor(12, 1);
 lcd.print("bar");
 lcd.setCursor(5, 2);
 lcd.print(now.day());
 lcd.setCursor(7, 2);
 lcd.print("/");
 lcd.setCursor(8, 2);
 lcd.print(now.month());
 lcd.setCursor(10, 2);
 lcd.print("/");
 lcd.setCursor(11, 2);
 lcd.print(now.year());
 lcd.setCursor(6, 3);
 lcd.print(now.hour());
 lcd.setCursor(8 , 3);
 lcd.print(":");
 lcd.setCursor(9, 3);
 lcd.print(now.minute());
 lcd.setCursor(11, 3);
 lcd.print(":");
 lcd.setCursor(12, 3);
 lcd.print(now.second());


 Serial.print(now.day());
 Serial.print("/");
 Serial.print(now.month());
 Serial.print("/");
 Serial.print(now.year());
 Serial.print(",");
 Serial.print(val);
 Serial.print(";");
  Serial.print(debitinst);
  Serial.print(";");
 Serial.println(total);
   Serial.print(";");
 Serial.println(rpm);
  Serial.print(";");
  Serial.print(verif);
   Serial.print(";");
  Serial.print(y);
 delay(500);
}

void pulseInterrupt() {

  verif++;
  i=0;
  prevPulseTime = lastPulseTime;
  lastPulseTime = micros();
  pulseDetected = true;
}

