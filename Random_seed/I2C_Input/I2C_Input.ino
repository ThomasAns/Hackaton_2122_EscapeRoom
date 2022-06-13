#include <CodeGenerator.h>
#include <Wire.h>

//byte x = 30; // random start waarde
int randomVal;
void setup(){
  Wire.begin(); // join i2c bus (address optional for master)
  randomVal = analogRead(A0);
}

void loop(){
  int* output = CodeGenerator::getRandomCode(randomVal);
    Wire.beginTransmission(1); // transmit to device #1 Morse
    Wire.write(output[4]);         // sends one byte  
    Wire.endTransmission();    // stop transmitting
    output = CodeGenerator::getRandomCode(output[4]);
    
    Wire.beginTransmission(2); // transmit to device #2 Afstand
    Wire.write(output[4]);         // sends one byte  
    Wire.endTransmission();    // stop transmitting

    //use the same output for 3 and 4 because 3 gives the solution for 4
    output = CodeGenerator::getRandomCode(output[4]);
        
    Wire.beginTransmission(3); // transmit to device #3 Laser
    Wire.write(output[4]);         // sends one byte  
    Wire.endTransmission();    // stop transmitting
    delay(500);
    
    Wire.beginTransmission(4); // transmit to device #4 Kluis
    Wire.write(output[4]);         // sends one byte  
    Wire.endTransmission();    // stop transmitting
    delay(500);
}
