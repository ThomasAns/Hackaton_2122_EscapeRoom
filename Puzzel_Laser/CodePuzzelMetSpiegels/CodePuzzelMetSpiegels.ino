#include <CodeGenerator.h>

#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

SoftwareSerial mySerial(10, 11); // RX, TX

Servo servo1;
Servo servo2;

int pot1 = 0;
int pot2 = 1;
int potVal1;
int potVal2;
int lichtsensor = 2;
String code = "";

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  lcd.init();
  lcd.backlight();
  
  servo1.attach(3); 
  servo2.attach(5);
  
  Serial.begin(9600);
    int* array = CodeGenerator::getRandomCode(38);
    for (int i = 0; i < 4; i++) {
      code += array[i];
    }


}

void loop() {
  Serial.println(mySerial.read());
  char number = Serial.read();
  Serial.println(number);
  potVal1 = analogRead(pot1);
  
  potVal1 = map(potVal1, 0, 1023, 0, 180);
  servo1.write(potVal1);

  potVal2 = analogRead(pot2);
  potVal2 = map(potVal2, 0, 1023, 0, 180);
  servo2.write(potVal2);

  if (analogRead(lichtsensor) > 350) {
    lcd.setCursor(0,0);
    lcd.print("De code is:");
    lcd.setCursor(0,1);
    lcd.print(code);
  }

  if  (analogRead(lichtsensor) < 350) {
    lcd.clear();
  }
}
