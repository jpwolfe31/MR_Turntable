/*
Turntable Controller

US Digital H3 Optical Rotary Encoder
  - Uses INT0 (pin 2) for channel A
  - Uses INT0 (pin 3) for channel B
  - Z index pulse read via polling
  - 2000 cycles on A and B per rotation encoder
  - enables  8000 counts or steps
  - B leads A for clockwise shaft rotation, and A leads
      B for counterclockwise rotation when viewed from the 
      shaft side of the encoder
  - Z is high only at index when when A and B are low 

Stepper motor 
  - 5 volt 
  - 200 steps per revolution 1.8 degrees
  - driven with TIP41As npn darlingtons
*/

// for stepper
#include <Stepper.h>
#define STEPS_PER_REV 800  // Adjust for your motor - not necessarily for actual steps
// Motor pins (to driver inputs) 
Stepper stepperMotor(STEPS_PER_REV, A2, A4, A3, A5);  // no D10 avail on lcd kyb shield

// for LCD and buttons
// Reads voltages on A0 to decode the 5 switches
#include <LiquidCrystal.h>
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);           // select the pins used on the LCD panel
unsigned long int lastMillis = 0;
int lcd_key     = 5; // none
int old_lcd_key = 5; // none
int adc_key_in  = 0;
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5

// for encoder
#define ENCODER_PIN_A 2   // INT0 must be 2 or 3 for interupt
#define ENCODER_PIN_B 3   // INT1 must be 2 or 3 for interupt
#define ENCODER_PIN_Z 11   // Index pulse (polled)  Note - D0 and D1 did not work with Z
#define RELAY_PIN 12   // Relay output for changinhg track polarity based on position.
int encoderCount = 7999;
int oldEC = encoderCount;
bool indexDetected = false;
const int CPR = 8000; 
int posNum = 0;
int posMax = 7; // one less given 0 start
const int pos[8] =     {500,  1000,  2500,  3000,  4510,  5000,  6510,  7000}; 
const int posRel[8] = {   0,     0,     0,     0,     0,     1,     1,     1}; 
char posCh[8][8] =   {"P0 ", "P1 ", "P2 ", "P3 ", "P4 ", "P5 ", "P6 ", "P7 "};

int gotoCount = 0;
int oldGoto = 0;
const int max = 7900;
const int min = 100;

// for USB Serial commands
#define CMD_BUFFER 32
char cmdBuffer[CMD_BUFFER];
byte cmdIndex = 0;

// Interrupt for channel A
void handleEncoderA() {
  bool a = digitalRead(ENCODER_PIN_A);
  bool b = digitalRead(ENCODER_PIN_B);
  if (a == b) {
    encoderCount++; // CW
  } else {
    encoderCount--; // CCW
  }
bool z = digitalRead(ENCODER_PIN_Z);
if (z == HIGH && a == LOW && b == LOW) {// index found   
  encoderCount = 0;
  indexDetected = true;
  Serial.println("Index Found");
  } 
//Serial.print("EC: ");
//Serial.println(encoderCount);
}
// Interrupt for channel B
void handleEncoderB() 
{
bool a = digitalRead(ENCODER_PIN_A);
bool b = digitalRead(ENCODER_PIN_B);
if (a != b) {
  encoderCount++; // CW
  } 
else {
  encoderCount--; // CCW
  }
bool z = digitalRead(ENCODER_PIN_Z);
if (z == HIGH && a == LOW && b == LOW) {// index found   
  encoderCount = 0;
  indexDetected = true;
  Serial.println("Index Found");
  }
// cannot go negative or over 7999
//Serial.print("EC: ");
//Serial.println(encoderCount);
}  

// software reset
void(* resetFunc) (void) = 0; //declare reset function @ address 0

void setup() 
{
// for serial
Serial.begin(115200);

// for stepper
//works with STEPS_PER_REV now set at 800  // Adjust for each motor- not necessarily for actual steps
//speed is in RPM of motor
// 3 seems to provide more control and a good rumble sound
// 5 works too.  RPM 10 to high as motor drive slips on wheel RPM
stepperMotor.setSpeed(3); //

//for turntable
pinMode(ENCODER_PIN_A, INPUT); // TTL outputs from encoder
pinMode(ENCODER_PIN_B, INPUT);
pinMode(ENCODER_PIN_Z, INPUT);
pinMode(RELAY_PIN, OUTPUT); // output for track polarity relay
digitalWrite(RELAY_PIN, 0); // off to start  
Serial.println("Rotary Encoder Initialized");

// Attach interrupts
attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_A), handleEncoderA, CHANGE); 
attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_B), handleEncoderB, CHANGE); 

// for LCD
lcd.begin(16, 2);               // start the library
// lcd.setCursor(0,0);             // set the LCD cursor   position 
// lcd.print("Push the buttons");  // print a simple message on the LCD
} // end of setup

void loop() 
{
// wait for index
if (indexDetected == false){
  Serial.println("Finding Index");
  lcd.clear();
  lcd.setCursor(1,0);             // move cursor to second line "1" and 9 spaces over
  lcd.print("Finding Index"); 
  while(indexDetected == false){
    stepperMotor.step(1); // move reverse
    }
  lcd.clear();
  lcd.setCursor(1,0);             // move cursor to second line "1" and 9 spaces over
  lcd.print("Index Found"); 
  delay(2000);
  // move to start 
  posNum = 0;
  gotoCount = pos[posNum];
  digitalWrite(RELAY_PIN, posRel[posNum]);
  }
if (lastMillis + 1000 < millis()){
  lcd.clear();
  if (encoderCount < 1000){lcd.setCursor(7,0);}
  else {lcd.setCursor(6,0);}
  lcd.print(encoderCount); 
  lcd.setCursor(3,1);
  lcd.print(posCh[posNum]); 
  if (gotoCount < 1000){lcd.setCursor(7,1);}
  else {lcd.setCursor(6,1);}
  lcd.print(gotoCount); 
  lastMillis = millis();
  }
// check for serial command
while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') { // End of command
      if (cmdIndex > 0) {
        cmdBuffer[cmdIndex] = '\0'; // Null-terminate
        processCommand(cmdBuffer);
        cmdIndex = 0; // Reset buffer
      }
    }
    else if (cmdIndex < CMD_BUFFER - 1) {
      cmdBuffer[cmdIndex++] = c;
    }
  }
// for movement
if (encoderCount + 1 < gotoCount) {
  stepperMotor.step(-2); // Move forward
  if (encoderCount - oldEC > 1){Serial.println("skipped forward encoder count");}
  oldEC = encoderCount;
  }
if (encoderCount - 2 > gotoCount) {
  stepperMotor.step(1); // Move reverse
  if (oldEC - encoderCount > 1){Serial.println("skipped reverse encoder count");}
  oldEC = encoderCount;
  }
// check pushbuttons
lcd.setCursor(9,1);             // move cursor to second line "1" and 9 spaces over
lcd_key = read_LCD_buttons();   // read the buttons
if (lcd_key != old_lcd_key){  // check if new key for debounce
  switch (lcd_key){               // depending on which button was pushed, we perform an action
    case btnRIGHT:{             //  push button "RIGHT" and show the word on the screen
      Serial.println("RIGHT ");
      gotoCount = gotoCount - 3; 
      break;
      }
    case btnLEFT:{
      Serial.println("LEFT   "); //  push button "LEFT" and show the word on the screen
      gotoCount = gotoCount + 3; 
      break;
      }    
    case btnUP:{
      Serial.println("UP    ");  //  push button "UP" and show the word on the screen
      posNum++;
      if (posNum > posMax) {posNum = posMax;}
      gotoCount = pos[posNum];
      digitalWrite(RELAY_PIN, posRel[posNum]);
      break;
      }
    case btnDOWN:{
      Serial.println("DOWN  ");  //  push button "DOWN" and show the word on the screen
      posNum--;
      if (posNum < 0) {posNum = 0;}
      gotoCount = pos[posNum];
      digitalWrite(RELAY_PIN, posRel[posNum]);   
      break;
      }
    case btnSELECT:{ // not used
      Serial.println("SELECT");  //  push button "SELECT" and show the word on the screen
      break;
      }
    case btnNONE:{ // used for debouncing
      Serial.println("NONE  ");  //  No action  will show "None" on the screen
      break;
      }
    }
  } 
old_lcd_key = lcd_key;  // save old key for debounce
}// end of loop

int read_LCD_buttons()
{
adc_key_in = analogRead(0);       // read the value from the sensor 
// V1.1 base case.  V1.0 is different - note modified as specified below including &&
if (adc_key_in < 200)  return btnRIGHT;  // was 50
if (adc_key_in < 250 && adc_key_in > 200)  return btnUP; // was just 250
if (adc_key_in < 450 && adc_key_in > 250)  return btnDOWN; // was just 450
if (adc_key_in < 650 && adc_key_in > 450)  return btnLEFT; // was just 650
if (adc_key_in < 850 && adc_key_in > 650)  return btnSELECT; // was just 850
if (adc_key_in > 1000) return btnNONE; 
return btnNONE; // when all others fail, return this.
}
 
void processCommand(char *cmd)
{
if (strcmp(cmd, "r") == 0) {
  resetFunc();  //call reset
  }
// save gotoCount
oldGoto = gotoCount;
  if (strcmp(cmd, "0") == 0) {
    Serial.println("zero");
    posNum = 0;
    gotoCount = pos[posNum];
    digitalWrite(RELAY_PIN, posRel[posNum]);
    } 
  if (strcmp(cmd, "1") == 0) {
    Serial.println("one");
    posNum = 1;
    gotoCount = pos[posNum];
    digitalWrite(RELAY_PIN, posRel[posNum]);
    }
  if (strcmp(cmd, "2") == 0) {
    Serial.println("two");
    posNum = 2;
    gotoCount = pos[posNum];
    digitalWrite(RELAY_PIN, posRel[posNum]);
    }
  if (strcmp(cmd, "3") == 0) {
    Serial.println("three");
    posNum = 3;
    gotoCount = pos[posNum];
    digitalWrite(RELAY_PIN, posRel[posNum]);
    } 
  if (strcmp(cmd, "4") == 0) {
    Serial.println("four");
    posNum = 4;
    gotoCount = pos[posNum];
    digitalWrite(RELAY_PIN, posRel[posNum]);
    } 
  if (strcmp(cmd, "5") == 0) {
    Serial.println("five");
    posNum = 5;
    gotoCount = pos[posNum];
    digitalWrite(RELAY_PIN, posRel[posNum]);
    }
  if (strcmp(cmd, "6") == 0) {
    Serial.println("six");
    posNum = 6;
    gotoCount = pos[posNum];
    digitalWrite(RELAY_PIN, posRel[posNum]);
    }
  if (strcmp(cmd, "7") == 0) {
    Serial.println("seven");
    posNum = 7;
    gotoCount = pos[posNum];
    digitalWrite(RELAY_PIN, posRel[posNum]);
    }
  if (strcmp(cmd, "+") == 0) {
    gotoCount = gotoCount + 3; 
    }
  if (strcmp(cmd, "-") == 0) {
    gotoCount = gotoCount - 3; 
    }
  if (strcmp(cmd, "++") == 0) {
    gotoCount = gotoCount + 100;  
    }
  if (strcmp(cmd, "--") == 0) {
    gotoCount = gotoCount - 100;  
    }
  if(strlen(cmd) == 4) {
    gotoCount = atoi(cmd);
    }
// cannot go less than min or more than max
if (gotoCount < min) {gotoCount = oldGoto;}
if (gotoCount > max) {gotoCount = oldGoto;}
strcpy(cmd, " "); // clear command
Serial.print("gotoCount ");
Serial.println(gotoCount);
}

