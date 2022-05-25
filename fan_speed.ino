#include <Math.h>
#define PI 3.1415926535897932384626433832795

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width,  in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

//Declare an SSD1306 display object connected to I2C
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define USE_TIMER_3     true
#include "TimerInterrupt.h"
#include "ISR_Timer.h"
#define HW_TIMER_INTERVAL_MS         50L
#define TIMER_INTERVAL_500MS        500L
#define TIMER_INTERVAL_1000MS      1000L
ISR_Timer ISR_timer;

//HW pin definition
#define FANc 10 //PWM
#define P2 A2   //Potenciometro
#define TAC 7   //Tachometer

//Global var
volatile long pulseCount = 0;
unsigned long RPMv = 0;
unsigned long lastTime = millis();

int Hz2RPM = 60;
int pulsesBYrev = 2;

void setup() {
  // put your setup code here, to run once:
  pinMode(FANc, OUTPUT);
  analogWrite(FANc, 0);
   
  pinMode(TAC, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(TAC), onPulse, RISING);

  // initialize OLED display with address 0x3C for 128x64
  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (true);
  }

  oled.setTextSize(1);          // text size
  oled.setTextColor(WHITE);     // text color

  ITimer3.init();
  ITimer3.attachInterruptInterval(HW_TIMER_INTERVAL_MS, TimerHandler);

  ISR_timer.setInterval(TIMER_INTERVAL_500MS, measureRPM);
  ISR_timer.setInterval(TIMER_INTERVAL_1000MS, OLEDTimerHandler);
  
  //Serial.begin(9600);
}

void TimerHandler()
{
  ISR_timer.run();
}

void measureRPM()
{
    RPMv = 1000*((float)pulseCount/(millis() - lastTime))*(Hz2RPM/pulsesBYrev);
    pulseCount = 0;
    lastTime = millis();
}

void OLEDTimerHandler()
{
    //OLEDupdate();
    //Serial.print("RPM: "); Serial.println(RPMv);
    //Serial.print("millis: "); Serial.println(millis());
}

void OLEDupdate(){
  int input = analogRead(P2);
  int DutyCycle = map(input, 0, 1023, 0, 255);
  analogWrite(FANc, DutyCycle);

  int DutyCycle_p = map(DutyCycle, 0, 255, 0, 100);
  int RPM_p = map(RPMv, 0, 2070, 0, 100);
    
  oled.clearDisplay(); // clear display
  oled.setCursor(0, 5);// position to display
  oled.println("FAN SPEED!"); 
   
  if(RPMv <10){
    oled.drawRoundRect(SCREEN_WIDTH-17, 5, 15, 8, 3, WHITE);
    oled.setCursor(SCREEN_WIDTH-40, 5);// position to display
    oled.println("OFF");
  }
  if(RPMv >10){
    oled.fillRoundRect(SCREEN_WIDTH-17, 5, 15, 8, 3, WHITE);
    oled.setCursor(SCREEN_WIDTH-40, 5);// position to display
    oled.println("ON!");
  }

  oled.fillCircle(SCREEN_WIDTH/2, SCREEN_HEIGHT, SCREEN_WIDTH*5/16, WHITE);
  oled.fillCircle(SCREEN_WIDTH/2, SCREEN_HEIGHT, SCREEN_WIDTH*5/16*0.99, BLACK);
  oled.fillCircle(SCREEN_WIDTH/2, SCREEN_HEIGHT-2, 3, WHITE);
  oled.drawLine(SCREEN_WIDTH/2, SCREEN_HEIGHT-2, SCREEN_WIDTH/2-SCREEN_WIDTH*5/16*cos(PI*DutyCycle_p/100), SCREEN_HEIGHT-max(2,SCREEN_WIDTH*5/16*sin(PI*DutyCycle_p/100)), WHITE);
//  oled.fillCircle(SCREEN_WIDTH/2-SCREEN_WIDTH*5/16*cos(PI*DutyCycle_p/100), SCREEN_HEIGHT-max(2,SCREEN_WIDTH*5/16*sin(PI*DutyCycle_p/100)), 2, WHITE);
  oled.fillTriangle(
    SCREEN_WIDTH/2-1*SCREEN_WIDTH*5/16*cos(PI*RPM_p/100), SCREEN_HEIGHT-max(2,1*SCREEN_WIDTH*5/16*sin(PI*RPM_p/100)), 
    SCREEN_WIDTH/2-1.2*SCREEN_WIDTH*5/16*cos(PI*RPM_p/100+PI/50), SCREEN_HEIGHT-max(2,1.2*SCREEN_WIDTH*5/16*sin(PI*RPM_p/100+PI/50)), 
    SCREEN_WIDTH/2-1.2*SCREEN_WIDTH*5/16*cos(PI*RPM_p/100-PI/50), SCREEN_HEIGHT-max(2,1.2*SCREEN_WIDTH*5/16*sin(PI*RPM_p/100-PI/50)), 
    WHITE);

  oled.fillRoundRect(0, 19, SCREEN_WIDTH/2*0.9, 22, 0, BLACK);
  char buffer[40];
  oled.setCursor(0, 20);// position to display
  sprintf(buffer, "%%   %d", DutyCycle_p);
  oled.println(buffer); // text to display
  oled.setCursor(0, 30);// position to display
  sprintf(buffer, "RPM %d", RPMv);
  oled.println(buffer); // text to display
  oled.setCursor(0, 40);// position to display
    
  oled.display();       // show on OLED
      
//  Serial.print("Entrada: "); Serial.print(input); Serial.print(" Duty Cycle: "); Serial.print(DutyCycle); Serial.print(" Tac: "); Serial.print(RPMv); 
//  Serial.println();
}

void loop()
{
  OLEDupdate();
}

void onPulse() {
  pulseCount++;
}
