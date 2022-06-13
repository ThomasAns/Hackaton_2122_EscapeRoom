#include <CodeGenerator.h>
#include <Servo.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>

Servo myservo;
int bieper = 13; //Pin voor buzzer
int Eenheid = 200; //De tijd tussen elke actie
int freq = 300; //freq in Hertz
int button = 9;
String code = "";
String mijncode;
int teller = 0;
int meeTeGevenNummer; //dit nummer meegeven naar de volgende opdracht als input voor de random seed
int* array;


const byte ROWS = 4; // 4 rijen
const byte COLS = 3; // 3 kolommen
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

byte rowPins[ROWS] = { 2, 3, 4, 5 }; //Verbinding van rowpinsa
byte colPins[COLS] = { 6,7,8 }; // Verbinding van kolompins
boolean received = false;

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
LiquidCrystal_I2C lcd(0x23, 20, 4);


void setup() {
    myservo.attach(12);

    myservo.write(140);

    Serial.begin(9600);
    Wire.begin(1);                    // join i2c bus with address #1
    Wire.onReceive(receiveEvent);     // register event
  
    lcd.init();
    lcd.backlight();
    
    pinMode(bieper, OUTPUT);
    pinMode(button, INPUT_PULLUP);
    

    //int input = analogRead(A0); //verander dit door een random waarde (Serial.read)

    /*array = CodeGenerator::getRandomCode(input);
    for (int i = 0; i < 4; i++) {
      code += array[i];
    }
    Serial.println(code);
    */
    
    Serial.println("Voer de code in:");
    lcd.print("Voer de code in:");
}
void receiveEvent(int howMany)
{
  if (!received) {
    int x = Wire.read();       // receive byte as an integer
    Serial.println(x);         // print the integer
    int* array = CodeGenerator::getRandomCode(x);
    Serial.print("Solution From codegenerator: ");
    for (int x=0; x<4; x++){
      code += array[x];
      Serial.print(array[x]);  
    }
    received = true;
    Serial.println();
    Serial.println(code);
  }
}

void loop(){
  char key = keypad.getKey();

    if (key) {
        teller++;
        lcd.setCursor(teller, 1);
        lcd.print("*");
        if (key == '*') checkCode();
        else {
          int num = key - 48; //1 als char - 48 geeft 1 als nummer
          mijncode += num;
        }
    }

    if  (digitalRead(button) == 0) {
      speelMorse(array[0]);
      delay(400);
      speelMorse(array[1]);
      delay(400);
      speelMorse(array[2]);
      delay(400);
      speelMorse(array[3]);
    }
}

void checkCode() {
  Serial.println(mijncode);
    if (code == mijncode)
    {
        lcd.clear();
        lcd.println("Goed gedaan!");
        lcd.setCursor(0, 1);
        lcd.println("De code is juist");
        myservo.write(0);
        delay(5000);
    }
    
    else
    {
        Serial.println("Niet juist");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("De code is fout!");
        delay(3000);
    }

    mijncode = "";
    teller = 0;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Voer de code in:");
}

void speelMorse(int num) {  
    switch (num)
    {
    case 0:
        speelStreep();
        speelStreep();
        speelStreep();
        speelStreep();
        speelStreep();
        break;
    case 1:
        speelPunt();
        speelStreep();
        speelStreep();
        speelStreep();
        speelStreep();
        break;
    case 2:
        speelPunt();
        speelPunt();
        speelStreep();
        speelStreep();
        speelStreep();
        break;
    case 3:
        speelPunt();
        speelPunt();
        speelPunt();
        speelStreep();
        speelStreep();
        break;
    case 4:
        speelPunt();
        speelPunt();
        speelPunt();
        speelPunt();
        speelStreep();
        break;
    case 5:
        speelPunt();
        speelPunt();
        speelPunt();
        speelPunt();
        speelPunt();
        break;
    case 6:
        speelStreep();
        speelPunt();
        speelPunt();
        speelPunt();
        speelPunt();
        break;
    case 7:
        speelStreep();
        speelStreep();
        speelPunt();
        speelPunt();
        speelPunt();
        break;
    case 8:
        speelStreep();
        speelStreep();
        speelStreep();
        speelPunt();
        speelPunt();
        break;
    case 9:
        speelStreep();
        speelStreep();
        speelStreep();
        speelStreep();
        speelPunt();
    default:
        //doe niks
        break;
    }
}

void speelPunt() {
    tone(bieper, freq);
    delay(Eenheid);
    noTone(bieper);
    delay(Eenheid);
}
void speelStreep() {
    tone(bieper, freq);
    delay(Eenheid * 3);
    noTone(bieper);
    delay(Eenheid);
}
