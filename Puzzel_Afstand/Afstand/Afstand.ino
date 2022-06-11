#include <CodeGenerator.h>
#include <Servo.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

#define echoPin 11 // attach pin D2 Arduino to pin Echo of HC-SR04
#define trigPin 10 //attach pin D3 Arduino to pin Trig of HC-SR04

LiquidCrystal_I2C lcd(0x27,20,4);

// defines variables
long duration; // variable for the duration of sound wave travel
int distance; // variable for the distance measurement

int t=0;

const int ROW_NUM = 4; //four rows
const int COLUMN_NUM = 3; //three columns

char keys[ROW_NUM][COLUMN_NUM] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

byte pin_rows[ROW_NUM] = {9, 8, 7, 6}; //connect to the row pinouts of the keypad
byte pin_column[COLUMN_NUM] = {5, 4, 3}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );

String password = ""; // change your password here
String SubStrng;
String input_password;

Servo myservo;  // create servo object to control a servo
int pos = 180;    // variable to store the servo position

void setup() {
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an OUTPUT
  pinMode(echoPin, INPUT); // Sets the echoPin as an INPUT
  Serial.begin(9600); // // Serial Communication is starting with 9600 of baudrate speed
  input_password.reserve(32); // maximum input characters is 33, change if needed

  lcd.init(); // initialize the lcd
  lcd.backlight();

  lcd.setCursor(0, 0);         // move cursor to   (0, 0)
  lcd.print("Welkom bij de");  // print message at (0, 0)
  lcd.setCursor(2, 1);         // move cursor to   (2, 1)
  lcd.print("2de opdracht");   // print message at (2, 1)
  delay(2000);
  int* array = CodeGenerator::getRandomCode(36);
    for (int i = 0; i < 4; i++) {
      password += array[i];
    }

   myservo.attach(12);  // attaches the servo on pin 12 to the servo object
   myservo.write(pos);
}




void loop() {
  while(t==0){
  // Clears the trigPin condition
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  distance = duration * 0.034 / 2; // Speed of sound wave divided by 2 (go and back)

  Serial.println(password);
  
  if(distance > 3 && distance < 5){
    String SubStrng = password.substring(0,1);
    lcd.clear();
    lcd.setCursor(0, 0);         // move cursor to   (0, 0)
    lcd.print("Het 1ste cijfer");        // print message at (0, 0)
    lcd.setCursor(5,1);
    lcd.print("is "+SubStrng);
    delay(100);
    }
    else if(distance > 9&& distance < 11){
    String SubStrng = password.substring(4,3);
    lcd.clear();
    lcd.setCursor(0, 0);           // move cursor to   (0, 0)
    lcd.print("Het 4de cijfer ");  // print message at (0, 0)
    lcd.setCursor(5,1);
    lcd.print("is "+SubStrng);     
    delay(100);
    }
     else if(distance > 15 && distance < 17){
    String SubStrng = password.substring(2,3);
    lcd.clear();
    lcd.setCursor(0, 0);         // move cursor to   (0, 0)
    lcd.print("Het 2de cijfer ");        // print message at (0, 0)
    lcd.setCursor(5,1);
    lcd.print("is "+SubStrng);        
    delay(100);
    }
     else if(distance > 21 && distance < 23){
    String SubStrng = password.substring(3,4);
    lcd.clear();
    lcd.setCursor(0, 0);         // move cursor to   (0, 0)
    lcd.print("Het 3de cijfer ");        // print message at (0, 0)
    lcd.setCursor(5,1);
    lcd.print("is "+SubStrng);       
    delay(100);
    }
     else{
    Serial.println("0");
    lcd.clear();
     }

    char key = keypad.getKey();

  if (key){
    Serial.println(key);

    if(key == '*') {
      input_password = ""; // clear input password
    } else if(key == '#') {
      if(password == input_password) {
        Serial.println("password is correct");
        t=1;
        lcd.setCursor(0, 0); 
        lcd.clear();
        lcd.println("password juist!!");
        lcd.setCursor(0,1);
        lcd.println("Neem het bakje!!");
        for (pos = 180; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
            myservo.write(pos);              // tell servo to go to position in variable 'pos'
            delay(15);                       // waits 15ms for the servo to reach the position
           }
        
        
        // DO YOUR WORK HERE
        
      } else {
        lcd.clear();
        lcd.setCursor(0, 0); 
        lcd.println("password fout!!!");
        Serial.println("password is incorrect, try again");
        delay(2000);
      }

      input_password = ""; // clear input password
    } else {
      input_password += key; // append new character to input password string
    }
  }
  }
  
} 




  
