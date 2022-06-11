#include <CodeGenerator.h>          //Random seed
#include <Arduino.h>                
/*
   IR-Receiver pin layout left -> right: 
   rs  ->  pin 2
   gnd ->  gnd
   vcc ->  5V
 */
#include "PinDefinitionsAndMore.h"  //pindefinations for ir-receiver
#include <IRremote.h>               //Library for IR
#include <Wire.h>                   //I2C for random seed input
/*
   --------------------------------------------------------------------------------------------------------------------
   Modification by Thomas Anseeuw of the:
   Example sketch/program showing An Arduino Door Access Control featuring RFID, EEPROM, Relay
   --------------------------------------------------------------------------------------------------------------------
  *Simple Work Flow (not limited to) :
                                     +-----------+
  +---------------------------------->RECEIVE I2C^------------------------------------------+
  |                                        |                                                |
  |                                        |                                                |
  |                              +---------v----------+                                     |
  |                              |Generate random seed|                                     |
  |                              +---------+----------+                                     |
  |                                        |                                                |
  |                                        |                                                |
  |                              +---------v----------+                                     |
  |                              | Guess correct pass |<------------+                       |
  |                              +---------+----------+             |                       |
  |                                        |          |             |                       |
  |                                        |          +--> False ---+                       |
  |                             +----------v------------+                                   |
  |                             |ir-password is correct,|                                   |
  |                             +----------+------------+                                   |
  |                                        |                                                |
  |                                        |                                                | 
  |                                   +----v-----+                                          |
  |                                    READ TAGS+^                                          |
  |                              +--------------------+                                     |
  |                              |                    |                                     |
  |                              |                    |                                     |
  |                         +----v-----+        +-----v----+                                |
  |                         |MASTER TAG|        |OTHER TAGS|                                |
  |                         +--+-------+        ++-------------+                            |
  |                            |                 |             |                            |
  |                            |                 |             |                            |
  |                      +-----v---+        +----v----+   +----v------+                     |
  |         +------------+READ TAGS+---+    |KNOWN TAG|   |UNKNOWN TAG|                     |
  |         |            +-+-------+   |    +-----------+ +------------------+              |
  |         |              |           |                |                    |              |
  |    +----v-----+   +----v----+   +--v--------+     +-v----------+  +------v----+         |
  |    |MASTER TAG|   |KNOWN TAG|   |UNKNOWN TAG|     |GRANT ACCESS|  |DENY ACCESS|         |
  |    +----------+   +---+-----+   +-----+-----+     +-----+------+  +-----+-----+         |
  |                       |               |                 |               |               |
  |       +----+     +----v------+     +--v---+             |               +--------------->
  +-------+EXIT|     |DELETE FROM|     |ADD TO|             |                               |
          +----+     |  EEPROM   |     |EEPROM|             |                               |
                     +-----------+     +------+             +-------------------------------+


   Use a Master Card which is act as Programmer then you can able to choose card holders who will granted access or not

 * **Easy User Interface**

   Just one RFID tag needed whether Delete or Add Tags. You can choose to use Leds for output or Serial LCD module to inform users.

 * **Stores Information on EEPROM**

   Information stored on non volatile Arduino's EEPROM memory to preserve Users' tag and Master Card. No Information lost
   if power lost. EEPROM has unlimited Read cycle but roughly 100,000 limited Write cycle.

 * **Security**
   To keep it simple we are going to use Tag's Unique IDs. It's simple and not hacker proof.

   @license Released into the public domain.

   Typical pin layout used:
   -----------------------------------------------------------------------------------------
               MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
               Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
   Signal      Pin          Pin           Pin       Pin        Pin              Pin
   -----------------------------------------------------------------------------------------
   RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
   SPI SS      SDA(SS)      10            53        D10        10               10
   SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
   SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
   SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
*/

#include <EEPROM.h>                 // We are going to read and write PICC's UIDs from/to EEPROM
#include <SPI.h>                    // RC522 Module uses SPI protocol
#include <MFRC522.h>                // Library for Mifare RC522 Devices

#include <Servo.h>

#define LED_ON HIGH
#define LED_OFF LOW

#define redLed 7           // Set Led Pins
#define greenLed 6         
#define blueLed 5          
                           
#define servo 8            // Set Servo Pin
                           
bool programMode = false;  // initialize programming mode to false
                           
uint8_t successRead;       // Variable integer to keep if we have Successful Read from Reader
                           
byte storedCard[4];        // Stores an ID read from EEPROM
byte readCard[4];          // Stores scanned ID read from RFID Module
byte masterCard[4];        // Stores master card's ID read from EEPROM

// Create MFRC522 instance.
#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);

boolean BadgePermission = false; // initialize Badge-permission to false, if true, user can input the ir-code.
int waarde;                // Value ir-input
int count = 0;             //lengt input
int test1, test2;
int results[4];            // array for the input values.
int oplossing[4] = {1,2,3,4}; //test solution
int testing[4];            //Solution filled with CodeGenerator, yes i know the names are flipped.
boolean correct = false;

///////////////////////////////////////// Setup ///////////////////////////////////
void setup() {
  //Arduino Pin Configuration
  pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(blueLed, OUTPUT);
  pinMode(servo, OUTPUT);

  digitalWrite(servo, HIGH);        // Make sure door is locked
  digitalWrite(redLed, LED_OFF);    // Make sure led is off
  digitalWrite(greenLed, LED_OFF);  // Make sure led is off
  digitalWrite(blueLed, LED_OFF);   // Make sure led is off

  //Protocol Configuration
  Serial.begin(9600);
  Wire.begin(4);                    // join i2c bus with address #4
  Wire.onReceive(receiveEvent);     // register event
  SPI.begin();           // MFRC522 Hardware uses SPI protocol
  mfrc522.PCD_Init();    // Initialize MFRC522 Hardware

  // Check if master card defined, if not let user choose a master card
  // This also useful to just redefine the Master Card
  // You can keep other EEPROM records just write other than 143 to EEPROM address 1
  // EEPROM address 1 should hold magical number which is '143'
  if (EEPROM.read(1) != 143) {
    Serial.println(F("No Master Card Defined"));
    Serial.println(F("Scan A PICC to Define as Master Card"));
    do {
      successRead = getID();            // sets successRead to 1 when we get read from reader otherwise 0
      digitalWrite(blueLed, LED_ON);    // Visualize Master Card need to be defined
      delay(200);
      digitalWrite(blueLed, LED_OFF);
      delay(200);
    }
    while (!successRead);                  // Program will not go further while you not get a successful read
    for ( uint8_t j = 0; j < 4; j++ ) {        // Loop 4 times
      EEPROM.write( 2 + j, readCard[j] );  // Write scanned PICC's UID to EEPROM, start from address 3
    }
    EEPROM.write(1, 143);                  // Write to EEPROM we defined Master Card.
    Serial.println(F("Master Card Defined"));
  }
  Serial.println(F("-------------------"));
  Serial.println(F("Master Card's UID"));
  for ( uint8_t i = 0; i < 4; i++ ) {          // Read Master Card's UID from EEPROM
    masterCard[i] = EEPROM.read(2 + i);    // Write it to masterCard
    Serial.print(masterCard[i], HEX);
  }
  Serial.println("");
  Serial.println(F("-------------------"));
  Serial.println(F("Everything is ready"));
  Serial.println(F("Waiting PICCs to be scanned"));
  cycleLeds();    // Everything ready lets give user some feedback by cycling leds
  
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK, USE_DEFAULT_FEEDBACK_LED_PIN);
  Serial.print("Ready to receive IR signals at pin ");
  Serial.println(IR_RECEIVE_PIN);
}

///////////////////////////////////////// Main Loop ///////////////////////////////////
void loop() {
  // do {
  //   successRead = getID();  // sets successRead to 1 when we get read from reader otherwise 0
  //   if (programMode) {
  //     cycleLeds();              // Program Mode cycles through Red Green Blue waiting to read a new card
  //   }
  //   else {
  //     normalModeOn();     // Normal mode, blue Power LED is on, all others are off
  //   }
  // } while (!successRead);   //the program will not go further while you are not getting a successful read
  // if (programMode) {
  //   if ( isMaster(readCard) ) { //When in program mode check First If master card scanned again to exit program mode
  //     Serial.println(F("Master Card Scanned"));
  //     Serial.println(F("Exiting Program Mode"));
  //     Serial.println(F("-----------------------------"));
  //     programMode = false;
  //     return;
  //   } else {
  //     if ( findID(readCard) ) { // If scanned card is known delete it
  //       Serial.println(F("I know this PICC, removing..."));
  //       deleteID(readCard);
  //       Serial.println("-----------------------------");
  //       Serial.println(F("Scan a PICC to ADD or REMOVE to EEPROM"));
  //     }
  //     else {                    // If scanned card is not known add it
  //       Serial.println(F("I do not know this PICC, adding..."));
  //       writeID(readCard);
  //       Serial.println(F("-----------------------------"));
  //       Serial.println(F("Scan a PICC to ADD or REMOVE to EEPROM"));
  //     }
  //   }
  // } else {
  //   if ( isMaster(readCard)) {    // If scanned card's ID matches Master Card's ID - enter program mode
  //     programMode = true;
  //     Serial.println(F("Hello Master - Entered Program Mode"));
  //     uint8_t count = EEPROM.read(0);   // Read the first Byte of EEPROM that
  //     Serial.print(F("I have "));     // stores the number of ID's in EEPROM
  //     Serial.print(count);
  //     Serial.print(F(" record(s) on EEPROM"));
  //     Serial.println("");
  //     Serial.println(F("Scan a PICC to ADD or REMOVE to EEPROM"));
  //     Serial.println(F("Scan Master Card again to Exit Program Mode"));
  //     Serial.println(F("-----------------------------"));
  //   }
  //   else {
  //     if ( findID(readCard) ) { // If not, see if the card is in the EEPROM
  //       Serial.println(F("Welcome, You shall pass"));
  //       granted(300);         // Open the door lock for 300 ms
  //       Badge-permission = true;
  //     }
  //     else {      // If not, show that the ID was not valid
  //       Serial.println(F("You shall not pass"));
  //       denied();
  //     }
  //   }
  // }
    /*
     * leest ir-inputs in tot er 4 zijn om te vergeijken met de oplossing.
     */
    while (count < 4){
      if (IrReceiver.decode()) {
        waardes();
        results[count] = waarde;
        Serial.print(results[count]);  //Serial.print(waarde);
        count++;
        delay(1000);
        /*
        * !!!Important!!! Enable receiving of the next value,
        * since receiving has stopped after the end of the current received data packet.
        */ 
        IrReceiver.resume(); // Enable receiving of the next value
      } 
    }  
    /*
     * Als er 4 waardes zijn vergelijk elk deel met oplossing.
     */
    if (count == 4){
      Serial.println("Check input:");
      for (int x=0; x<4; x++){
        test1 = results[x];// asign each index of arrays to test, one by one and compare
        Serial.print((String)"Input: " + test1 + " ");
        //test2= oplossing[x];
        test2= testing[x];
        Serial.print((String)"Oplossing: " + test2 + " ");
        //Serial.print(x);
        /*
         * Als er een deel niet overeenkomt break en reset om op nieuw te proberen.
         */
        if (test1 != test2) {
          Serial.println("Fout!");
          correct = false;
          count=0; // reset om op nieuw waardes in te lezen
          break;
        } else {
          Serial.println("Correct");
          correct = true;
          if(x == 3){ 
            count++;
          }
        }
      }
    } else {
      count = 0;
    }
    if(correct) {
      Serial.println("Password is correct");
      delay(1000);
      do {
        //exit(0); //temp exit loop
        //TODO code: Sweep servo open final door
        do {
        successRead = getID();  // sets successRead to 1 when we get read from reader otherwise 0
        if (programMode) {
          cycleLeds();              // Program Mode cycles through Red Green Blue waiting to read a new card
        }
        else {
          normalModeOn();     // Normal mode, blue Power LED is on, all others are off
        }
        } while (!successRead);   //the program will not go further while you are not getting a successful read
        if (programMode) {
          if ( isMaster(readCard) ) { //When in program mode check First If master card scanned again to exit program mode
            Serial.println(F("Master Card Scanned"));
            Serial.println(F("Exiting Program Mode"));
            Serial.println(F("-----------------------------"));
            programMode = false;
            return;
          } else {
            if ( findID(readCard) ) { // If scanned card is known delete it
              Serial.println(F("I know this PICC, removing..."));
              deleteID(readCard);
              Serial.println("-----------------------------");
              Serial.println(F("Scan a PICC to ADD or REMOVE to EEPROM"));
            }
            else {                    // If scanned card is not known add it
              Serial.println(F("I do not know this PICC, adding..."));
              writeID(readCard);
              Serial.println(F("-----------------------------"));
              Serial.println(F("Scan a PICC to ADD or REMOVE to EEPROM"));
            }
          }
        } else {
          if ( isMaster(readCard)) {    // If scanned card's ID matches Master Card's ID - enter program mode
            programMode = true;
            Serial.println(F("Hello Master - Entered Program Mode"));
            uint8_t count = EEPROM.read(0);   // Read the first Byte of EEPROM that
            Serial.print(F("I have "));     // stores the number of ID's in EEPROM
            Serial.print(count);
            Serial.print(F(" record(s) on EEPROM"));
            Serial.println("");
            Serial.println(F("Scan a PICC to ADD or REMOVE to EEPROM"));
            Serial.println(F("Scan Master Card again to Exit Program Mode"));
            Serial.println(F("-----------------------------"));
          }
          else {
            if ( findID(readCard) ) { // If not, see if the card is in the EEPROM
              Serial.println(F("Welcome, You shall pass"));
              granted(300);         // Open the door lock for 300 ms
            }
            else {      // If not, show that the ID was not valid
              Serial.println(F("You shall not pass"));
              denied();
            }
          }
        }
      } while(!BadgePermission);
    } else {
      Serial.println("Wrong password");
    } 
}

/////////////////////////////////////////  Access Granted    ///////////////////////////////////
void granted ( uint16_t setDelay) {
  digitalWrite(blueLed, LED_OFF);   // Turn off blue LED
  digitalWrite(redLed, LED_OFF);  // Turn off red LED
  digitalWrite(greenLed, LED_ON);   // Turn on green LED
  digitalWrite(servo, LOW);     // Unlock door!
  BadgePermission = true;
  delay(setDelay);          // Hold door lock open for given seconds
  digitalWrite(servo, HIGH);    // Relock door
  delay(1000);            // Hold green LED on for a second
}

///////////////////////////////////////// Access Denied  ///////////////////////////////////
void denied() {
  digitalWrite(greenLed, LED_OFF);  // Make sure green LED is off
  digitalWrite(blueLed, LED_OFF);   // Make sure blue LED is off
  digitalWrite(redLed, LED_ON);   // Turn on red LED
  delay(1000);
}


///////////////////////////////////////// Get PICC's UID ///////////////////////////////////
uint8_t getID() {
  // Getting ready for Reading PICCs
  if ( ! mfrc522.PICC_IsNewCardPresent()) { //If a new PICC placed to RFID reader continue
    return 0;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) {   //Since a PICC placed get Serial and continue
    return 0;
  }
  // There are Mifare PICCs which have 4 byte or 7 byte UID care if you use 7 byte PICC
  // I think we should assume every PICC as they have 4 byte UID
  // Until we support 7 byte PICCs
  Serial.println(F("Scanned PICC's UID:"));
  for ( uint8_t i = 0; i < 4; i++) {  //
    readCard[i] = mfrc522.uid.uidByte[i];
    Serial.print(readCard[i], HEX);
  }
  Serial.println("");
  mfrc522.PICC_HaltA(); // Stop reading
  return 1;
}

///////////////////////////////////////// Cycle Leds (Program Mode) ///////////////////////////////////
void cycleLeds() {
  digitalWrite(redLed, LED_OFF);  // Make sure red LED is off
  digitalWrite(greenLed, LED_ON);   // Make sure green LED is on
  digitalWrite(blueLed, LED_OFF);   // Make sure blue LED is off
  delay(200);
  digitalWrite(redLed, LED_OFF);  // Make sure red LED is off
  digitalWrite(greenLed, LED_OFF);  // Make sure green LED is off
  digitalWrite(blueLed, LED_ON);  // Make sure blue LED is on
  delay(200);
  digitalWrite(redLed, LED_ON);   // Make sure red LED is on
  digitalWrite(greenLed, LED_OFF);  // Make sure green LED is off
  digitalWrite(blueLed, LED_OFF);   // Make sure blue LED is off
  delay(200);
}

//////////////////////////////////////// Normal Mode Led  ///////////////////////////////////
void normalModeOn () {
  digitalWrite(blueLed, LED_ON);  // Blue LED ON and ready to read card
  digitalWrite(redLed, LED_OFF);  // Make sure Red LED is off
  digitalWrite(greenLed, LED_OFF);  // Make sure Green LED is off
  digitalWrite(servo, HIGH);    // Make sure Door is Locked
}

//////////////////////////////////////// Read an ID from EEPROM //////////////////////////////
void readID( uint8_t number ) {
  uint8_t start = (number * 4 ) + 2;    // Figure out starting position
  for ( uint8_t i = 0; i < 4; i++ ) {     // Loop 4 times to get the 4 Bytes
    storedCard[i] = EEPROM.read(start + i);   // Assign values read from EEPROM to array
  }
}

///////////////////////////////////////// Add ID to EEPROM   ///////////////////////////////////
void writeID( byte a[] ) {
  if ( !findID( a ) ) {     // Before we write to the EEPROM, check to see if we have seen this card before!
    uint8_t num = EEPROM.read(0);     // Get the numer of used spaces, position 0 stores the number of ID cards
    uint8_t start = ( num * 4 ) + 6;  // Figure out where the next slot starts
    num++;                // Increment the counter by one
    EEPROM.write( 0, num );     // Write the new count to the counter
    for ( uint8_t j = 0; j < 4; j++ ) {   // Loop 4 times
      EEPROM.write( start + j, a[j] );  // Write the array values to EEPROM in the right position
    }
    successWrite();
    Serial.println(F("Succesfully added ID record to EEPROM"));
  }
  else {
    failedWrite();
    Serial.println(F("Failed! There is something wrong with ID or bad EEPROM"));
  }
}

///////////////////////////////////////// Remove ID from EEPROM   ///////////////////////////////////
void deleteID( byte a[] ) {
  if ( !findID( a ) ) {     // Before we delete from the EEPROM, check to see if we have this card!
    failedWrite();      // If not
    Serial.println(F("Failed! There is something wrong with ID or bad EEPROM"));
  }
  else {
    uint8_t num = EEPROM.read(0);   // Get the numer of used spaces, position 0 stores the number of ID cards
    uint8_t slot;       // Figure out the slot number of the card
    uint8_t start;      // = ( num * 4 ) + 6; // Figure out where the next slot starts
    uint8_t looping;    // The number of times the loop repeats
    uint8_t j;
    uint8_t count = EEPROM.read(0); // Read the first Byte of EEPROM that stores number of cards
    slot = findIDSLOT( a );   // Figure out the slot number of the card to delete
    start = (slot * 4) + 2;
    looping = ((num - slot) * 4);
    num--;      // Decrement the counter by one
    EEPROM.write( 0, num );   // Write the new count to the counter
    for ( j = 0; j < looping; j++ ) {         // Loop the card shift times
      EEPROM.write( start + j, EEPROM.read(start + 4 + j));   // Shift the array values to 4 places earlier in the EEPROM
    }
    for ( uint8_t k = 0; k < 4; k++ ) {         // Shifting loop
      EEPROM.write( start + j + k, 0);
    }
    successDelete();
    Serial.println(F("Succesfully removed ID record from EEPROM"));
  }
}

///////////////////////////////////////// Check Bytes   ///////////////////////////////////
bool checkTwo ( byte a[], byte b[] ) {   
  for ( uint8_t k = 0; k < 4; k++ ) {   // Loop 4 times
    if ( a[k] != b[k] ) {     // IF a != b then false, because: one fails, all fail
       return false;
    }
  }
  return true;  
}

///////////////////////////////////////// Find Slot   ///////////////////////////////////
uint8_t findIDSLOT( byte find[] ) {
  uint8_t count = EEPROM.read(0);       // Read the first Byte of EEPROM that
  for ( uint8_t i = 1; i <= count; i++ ) {    // Loop once for each EEPROM entry
    readID(i);                // Read an ID from EEPROM, it is stored in storedCard[4]
    if ( checkTwo( find, storedCard ) ) {   // Check to see if the storedCard read from EEPROM
      // is the same as the find[] ID card passed
      return i;         // The slot number of the card
    }
  }
}

///////////////////////////////////////// Find ID From EEPROM   ///////////////////////////////////
bool findID( byte find[] ) {
  uint8_t count = EEPROM.read(0);     // Read the first Byte of EEPROM that
  for ( uint8_t i = 1; i < count; i++ ) {    // Loop once for each EEPROM entry
    readID(i);          // Read an ID from EEPROM, it is stored in storedCard[4]
    if ( checkTwo( find, storedCard ) ) {   // Check to see if the storedCard read from EEPROM
      return true;
    }
    else {    // If not, return false
    }
  }
  return false;
}

///////////////////////////////////////// Write Success to EEPROM   ///////////////////////////////////
// Flashes the green LED 3 times to indicate a successful write to EEPROM
void successWrite() {
  digitalWrite(blueLed, LED_OFF);   // Make sure blue LED is off
  digitalWrite(redLed, LED_OFF);  // Make sure red LED is off
  digitalWrite(greenLed, LED_OFF);  // Make sure green LED is on
  delay(200);
  digitalWrite(greenLed, LED_ON);   // Make sure green LED is on
  delay(200);
  digitalWrite(greenLed, LED_OFF);  // Make sure green LED is off
  delay(200);
  digitalWrite(greenLed, LED_ON);   // Make sure green LED is on
  delay(200);
  digitalWrite(greenLed, LED_OFF);  // Make sure green LED is off
  delay(200);
  digitalWrite(greenLed, LED_ON);   // Make sure green LED is on
  delay(200);
}

///////////////////////////////////////// Write Failed to EEPROM   ///////////////////////////////////
// Flashes the red LED 3 times to indicate a failed write to EEPROM
void failedWrite() {
  digitalWrite(blueLed, LED_OFF);   // Make sure blue LED is off
  digitalWrite(redLed, LED_OFF);  // Make sure red LED is off
  digitalWrite(greenLed, LED_OFF);  // Make sure green LED is off
  delay(200);
  digitalWrite(redLed, LED_ON);   // Make sure red LED is on
  delay(200);
  digitalWrite(redLed, LED_OFF);  // Make sure red LED is off
  delay(200);
  digitalWrite(redLed, LED_ON);   // Make sure red LED is on
  delay(200);
  digitalWrite(redLed, LED_OFF);  // Make sure red LED is off
  delay(200);
  digitalWrite(redLed, LED_ON);   // Make sure red LED is on
  delay(200);
}

///////////////////////////////////////// Success Remove UID From EEPROM  ///////////////////////////////////
// Flashes the blue LED 3 times to indicate a success delete to EEPROM
void successDelete() {
  digitalWrite(blueLed, LED_OFF);   // Make sure blue LED is off
  digitalWrite(redLed, LED_OFF);  // Make sure red LED is off
  digitalWrite(greenLed, LED_OFF);  // Make sure green LED is off
  delay(200);
  digitalWrite(blueLed, LED_ON);  // Make sure blue LED is on
  delay(200);
  digitalWrite(blueLed, LED_OFF);   // Make sure blue LED is off
  delay(200);
  digitalWrite(blueLed, LED_ON);  // Make sure blue LED is on
  delay(200);
  digitalWrite(blueLed, LED_OFF);   // Make sure blue LED is off
  delay(200);
  digitalWrite(blueLed, LED_ON);  // Make sure blue LED is on
  delay(200);
}

////////////////////// Check readCard IF is masterCard   ///////////////////////////////////
// Check to see if the ID passed is the master programing card
bool isMaster( byte test[] ) {
  return checkTwo(test, masterCard);
}

bool monitorWipeButton(uint32_t interval) {
  uint32_t now = (uint32_t)millis();
  while ((uint32_t)millis() - now < interval)  {
    // check on every half a second
    if (((uint32_t)millis() % 500) == 0) {
      //if (digitalRead(wipeB) != LOW)
        return false;
    }
  }
  return true;
}

///////////////////////////////////////// Receive I2C Input ///////////////////////////////////
void receiveEvent(int howMany)
{
  int x = Wire.read();       // receive byte as an integer
  Serial.println(x);         // print the integer
  int* array = CodeGenerator::getRandomCode(x);
  Serial.print("Solution From codegenerator: ");
  for (int x=0; x<4; x++){
    testing[x] = array[x];
    Serial.print(testing[x]);  
  }
  Serial.println();
}

///////////////////////////////////////// ir-receiver value check ///////////////////////////////////
void waardes(){
  /*
  * Finally, check the received data and perform actions according to the received command
  */
  if (IrReceiver.decodedIRData.command == 0x4A) {
    //Serial.println("9");
    waarde = 9;
  }else if (IrReceiver.decodedIRData.command == 0x52) {
    //Serial.println("8");
    waarde = 8;
  }else if (IrReceiver.decodedIRData.command == 0x42) {
    //Serial.println("7");
    waarde = 7;
  }else if (IrReceiver.decodedIRData.command == 0x5A) {
    //Serial.println("6");
    waarde = 6;
  }else if (IrReceiver.decodedIRData.command == 0x1C) {
    //Serial.println("5");
    waarde= 5;
  }else if (IrReceiver.decodedIRData.command == 0x8) {
    //Serial.println("4");
    waarde = 4;
  }else if (IrReceiver.decodedIRData.command == 0x5E) {
    //Serial.println("3");
    waarde = 3;
  }else if (IrReceiver.decodedIRData.command == 0x18) {
    //Serial.println("2");
    waarde = 2;
  }else if (IrReceiver.decodedIRData.command == 0xC) {
    //Serial.println("1");
    waarde = 1;
  }else if (IrReceiver.decodedIRData.command == 0x16) {
    //Serial.println("0");
    waarde = 0;
  }
}
