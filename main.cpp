#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h> // Library for LCD

#define flowSensorPin A14    //sensor input flow rate
#define iOutPin 12           //controller output for steady flow
#define buttonDownPin A2          // Down button
#define buttonUpPin A3            // Up button
#define buttonLeftPin A4          // Back button
#define buttonRightPin A5         // Enter button

LiquidCrystal_I2C lcd(0x27,20,4);

uint32_t intervalStart;
uint16_t setPoint = 70;           // 0...1023
uint16_t flowMeasure = 140;       // 0...1023, 0-20mA
uint16_t lastflowMeasure = 100;   // 0...1023, 0-20mA
uint16_t flowRate = 70;           //
uint8_t iOutRaw = 20;             // 0- 250  (4 ... 20 mA)
uint8_t iOut;                     // 0-100 %
bool outUp = LOW;
bool outDown = LOW;
uint16_t loopInterval = 500;      // Controll loop delay
uint16_t buttonDelay = 180;       // delay between button press


void setup() {
  Serial.begin(9600);
  pinMode(flowSensorPin, INPUT);
  pinMode(iOutPin, OUTPUT);
  pinMode(buttonLeftPin, INPUT_PULLUP);
  pinMode(buttonDownPin, INPUT_PULLUP);
  pinMode(buttonUpPin, INPUT_PULLUP);
  pinMode(buttonRightPin, INPUT_PULLUP);

  Serial.print("Hello World!");
 // lcd.begin();
 lcd.init();
 lcd.backlight();
 lcd.setCursor(1,0);
 lcd.print("Hello, world!");
 lcd.setCursor(1,3);
 lcd.print("Hallo Filter ");
 delay(2000);
 lcd.clear();
}

void loop() {

  if (digitalRead(buttonUpPin)) {
    delay(buttonDelay);
    if(setPoint < 400) setPoint += 1; // max setpoint is full range 20 mA
    else setPoint = 400;
  }
  else if (digitalRead(buttonDownPin)) {
    delay(buttonDelay);
    if(setPoint > 10) setPoint -= 1;  // min setpoint is min range 4 mA
    else setPoint = 10;
  }
  else if (digitalRead(buttonRightPin)) { // right button
    delay(buttonDelay);
    if (loopInterval < 2000) loopInterval += 10;
    else loopInterval = 2000;
  }
  else if (digitalRead(buttonLeftPin)) { // left button
    delay(buttonDelay);
    if (loopInterval > 100) loopInterval -=10;
    else loopInterval = 100;
  }

  if(millis() > loopInterval + intervalStart)  {
    lastflowMeasure = flowMeasure;
    flowMeasure = analogRead(flowSensorPin);      //0...1023 [0 - 20]
    if (flowMeasure > 1023) flowMeasure = 1023;
    if (flowMeasure < 1) flowMeasure = 1;
    flowRate = (flowMeasure + lastflowMeasure) / 4;   // 0 ... 512

    if (flowRate < setPoint - 2) {
      iOutRaw += 1;
      outUp = HIGH;
      if (flowRate < setPoint - 16) iOutRaw += 3;
    }
    else outUp = LOW;

    if (flowRate > setPoint + 2) {
      iOutRaw -= 1;
      outDown = HIGH;
      if (flowRate > setPoint + 16) iOutRaw -= 3;
    }
    else outDown = LOW;

    if(iOutRaw > 249) iOutRaw = 249 ;  // AO max 20 mA
    if(iOutRaw < 55) iOutRaw = 55;      // AO min 4 mA
    analogWrite(iOutPin, iOutRaw );
    iOut = (iOutRaw - 53) / 2;

    lcd.setCursor(0,0);
    lcd.print("FlowRate:  ");
    if (flowRate < 100)  lcd.print(" ");
    if (flowRate < 10)  lcd.print(" ");
    lcd.print(flowRate);
    lcd.print("ml/min");

    lcd.setCursor(0,1);
    lcd.print("SetPoint:  ");
    if (setPoint < 100)  lcd.print(" ");
    lcd.print(setPoint);
    lcd.print("ml/min");

    lcd.setCursor(0,2);
    lcd.print("Interval: ");
    if (loopInterval < 1000)  lcd.print(" ");
    lcd.print(loopInterval);
    lcd.print(" ms");

    lcd.setCursor(0,3);
    lcd.print("Output:        %    ");
    lcd.setCursor(11,3);
    if (iOut < 100)  lcd.print(" ");
    lcd.print(iOut);          // 50 .. 255
    lcd.setCursor(18,3);
    if (outUp)   lcd.print("P");
    if (outDown)    lcd.print("b");

    intervalStart = millis();
  }

}
