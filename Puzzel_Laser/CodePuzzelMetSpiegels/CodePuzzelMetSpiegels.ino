#include <CodeGenerator.h>

#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

//SoftwareSerial mySerial(10, 11); // RX, TX //Serial no longer used -> I2C
 
Servo servo1;
Servo servo2;

int pot1 = 0;
int pot2 = 1;
int potVal1;
int potVal2;
int lichtsensor = 2;
String code = "";
boolean received = false; //initialisatiewaarde

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();

  servo1.attach(3); 
  servo2.attach(5);

  Wire.begin(3);                // join i2c bus with address #3
  Wire.onReceive(receiveEvent); // register event
    //int* array = CodeGenerator::getRandomCode(38);
    //for (int i = 0; i < 4; i++) {
    //  code += array[i];
    //}
}

void receiveEvent(int howMany){
  if  (!received) {
    int x = Wire.read();    // receive byte as an integer
  Serial.println(x);         // print the integer
  int* array = CodeGenerator::getRandomCode(x);
  Serial.print("UwU Henlo YaY: ");
  for (int x=0; x<4; x++){
    code += array[x];
    Serial.print(array[x]);
  }
  received = true;
  }

}

void loop() {
  potVal1 = analogRead(pot1);

  potVal1 = map(potVal1, 0, 1023, 0, 120);
  servo1.write(potVal1);

  potVal2 = analogRead(pot2);
  potVal2 = map(potVal2, 0, 1023, 0, 120);
  servo2.write(potVal2);

  if (analogRead(lichtsensor) > 350) {
    lcd.setCursor(0,0);
    lcd.print("De code is:");
    lcd.setCursor(0,1);
    lcd.print(code);
    delay(100);
  }

  if  (analogRead(lichtsensor) < 350) {
    lcd.clear();
  }
}