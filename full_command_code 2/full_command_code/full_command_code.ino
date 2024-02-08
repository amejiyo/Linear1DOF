/*
  Pul+ goes to +5V
  Pul- goes to Arduino Mega Pin which is PWM output
  Dir+ goes to +5V
  Dir- goes to to Arduino Pin
  Enable+ to nothing
  Enable- to nothing
*/

#include <AccelStepper.h>
#include <Keypad.h>
#include <Bounce2.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EnableInterrupt.h>
LiquidCrystal_I2C lcd(0x26, 20, 4);

#define SPEED_NORMAL 3000
#define FORWARD 1
#define BACKWARD 0

// limit switch button pin
const int limitStartPin = 38;
const int limitEndPin = 39;

// Define driver pin connections
const int stepPin = 5;
const int dirPin = 6;
const int enPin = 7;

AccelStepper stepper(1, stepPin, dirPin);     // motorInterfaceType = 1 (driver)

// main board connection pin
const int reach_goalPin = 8;
const int des_posPin = 9;

// Relay Driller pin
const int drillerPin = 45;
//----------Create-Arrow-up-sign---------------
byte Arrow_down[] = {
  B00100,
  B00100,
  B00100,
  B00100,
  B00100,
  B10101,
  B01110,
  B00100
};
//----------Create-Arrow-down-sign--------------
byte Arrow_up[] = {
  B00100,
  B01110,
  B10101,
  B00100,
  B00100,
  B00100,
  B00100,
  B00100
};

// Keyboard setpup
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {26, 27, 32, 31};
byte colPins[COLS] = {29, 28, 30, 33};
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

// Command button configuration
const int start_buttonPin = 36;
const int stop_buttonPin = 34;
Bounce start_button = Bounce();
Bounce stop_button = Bounce();
const int decimal_buttonPin = 40;
const int delete_buttonPin = 48;
const int dir_buttonPin = 42;
const int reset_buttonPin = 43;
Bounce decimal_button = Bounce();
Bounce delete_button = Bounce();
Bounce dir_button = Bounce();
Bounce reset_button = Bounce();


// auto mode default position
float WD_08_2LA29_25N00[15] = {15, 215, 200, 70, 55, 45, 600, 45, 125, 200, 215, 20};
float WD_08_2LA29_25M00[15] = {15, 230, 200, 55, 55, 345, 345, 110, 200, 230, 15};
float WD_08_2LA29_25400[15] = {142.5, 193, 243.5, 294, 344.5, 395, 445.5, 496, 17600};
float WD_08_2LA29_25L00[15] = {142.5, 193, 243.5, 294, 344.5, 395, 445.5, 496, 546.5, 1700};

// Define variables
const float leadscrewPitch = 5;   // pitch = 5 mm
const float microsteps = 400.00;
float current_step = 0;
int stepperState = 0;
int substate = 0;
int state = 0;
int mode = 0;
float desire_pos[100] = {0};
int numStation = 0;
char value[10];
int value_ind = 0;
bool case3 = false;
int conti = 0;
bool running = false;
bool dir = FORWARD;
bool case6=0;

void setHome(){
  stepper.setSpeed(-SPEED_NORMAL); 
  // digitalWrite(dirPin, BACKWARD);
  while (digitalRead(limitStartPin)) {
    stepper.runSpeed();
  }
  stepper.stop();
}

void autoModeLCD(){
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("1.WD_08_2LA29_25N00"); 
  lcd.setCursor(0, 1); lcd.print("2.WD_08_2LA29_25M00"); 
  lcd.setCursor(0, 2); lcd.print("3.WD_08_2LA29_25400"); 
  lcd.setCursor(0, 3); lcd.print("4.WD_08_2LA29_25L00"); 
}

void selectModeLCD(char mode_name[]){
  char s [80];
  strcpy (s, "<<< ");
  strcat (s, mode_name);
  strcat (s, " >>>");
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("SYSTEM SELECT MODE"); 
  lcd.setCursor(1, 2); lcd.print(s); 
  lcd.setCursor(0, 3); lcd.print("*=Confirm #=Cancel");
}

void modeMenuLCD(){
  lcd.clear();
  lcd.setCursor(3, 0); lcd.print("1.AUTO MODE"); 
  lcd.setCursor(3, 1); lcd.print("2.MANUAL MODE"); 
  lcd.setCursor(3, 2); lcd.print("3.SET MOTOR MODE"); 
}

void manualModeLCD(){
  lcd.clear(); 
  for (int i=0; i<4;i++){
    lcd.setCursor(0, i); lcd.print("Part"); lcd.print(numStation + i + 1); lcd.print(" = "); lcd.setCursor(17, i); lcd.print("mm.");
  }
}

void motorModeLCD(){
  if (!running) {
    lcd.clear();
    lcd.setCursor(4, 0);
    lcd.print("Please press");
    lcd.setCursor(2, 2);
    lcd.print("The START button");
    lcd.setCursor(0, 3);
    if (dir) lcd.print("MOVING FORWARD");
    else lcd.print("MOVING BACKWARD");
    lcd.noBlink();
  }
  else{
    lcd.clear();
    lcd.setCursor(0, 2);
    if (dir) lcd.print("MOVING FORWARD");
    else lcd.print("MOVING BACKWARD");
    lcd.setCursor(0, 0); lcd.print("SYSTEM MOTOR MODE"); 
    lcd.setCursor(0, 3); lcd.print("#=QUIT");
  }
}

void Drill() { // สั่งให้เจาะส่วาน
  digitalWrite(drillerPin, 1);
  delay(500);
  digitalWrite(drillerPin , 0);
  delay(500);
}

void selectPart(float B[], char part_name[]){
  for (int i=0; i<sizeof(B);i++) desire_pos[i] = B[i];
  char s [80];
  strcpy (s, ">");
  strcat (s, part_name);
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("SYSTEM SELECT PART"); 
  lcd.setCursor(0, 2); lcd.print(s); 
  lcd.setCursor(0, 3); lcd.print("*=Confirm #=Cancel");
}

void stepperMotor(){
  switch (stepperState){
    case 0:
      lcd.clear();
      lcd.setCursor(6, 0);
      lcd.print("SYSTEM");
      lcd.setCursor(4, 1);
      lcd.print("*** Go To ***");
      lcd.setCursor(0, 2);
      lcd.print("Point NO.= ");
      lcd.print(substate + 1);
      lcd.print(" / ");
      lcd.print(numStation);
      // Move to desired position
      Serial.print("Going to station ");
      Serial.println(substate + 1);
      Serial.println(desire_pos[substate]);
      Serial.println((int)(((desire_pos[substate]*microsteps)/leadscrewPitch) - current_step));
      stepper.move((int)(((desire_pos[substate]*microsteps)/leadscrewPitch) - current_step));
      Serial.print(current_step);
      Serial.print(" ");
      Serial.print(desire_pos[substate]*microsteps/leadscrewPitch);
      Serial.print(" ");
      Serial.println((desire_pos[substate]*microsteps/leadscrewPitch) - current_step);
      stepper.runToPosition();
      current_step += (desire_pos[substate]*microsteps/leadscrewPitch) - current_step;
      delay(100);
      substate += 1;
      stepperState = 1;
      break;
    case 1:
      // Driller acivate
      Serial.println("Driller Activated!");
      delay(100);
      lcd.clear();
      Drill();
      lcd.setCursor(6, 0);
      lcd.print("SYSTEM");
      lcd.setCursor(4, 1);
      lcd.print("*** CUT ***");
      lcd.setCursor(0, 2);
      lcd.print("Point NO.= ");
      lcd.print(substate);
      lcd.print(" OK. >> ");
      delay(1000);
      stepperState = 2;
      break;
    case 2:
      if (start_button.fell()){
        stepperState = 0;
        if (substate+1 > numStation){
          numStation = 0;
          substate = 0;
          setHome();
          lcd.clear();
          lcd.setCursor(4, 0); 
          lcd.print("End SYSTEM"); 
          lcd.setCursor(0, 1); 
          lcd.print("1.Part Before"); 
          lcd.setCursor(0, 2); 
          lcd.print("2.New Part");
          state = 5;
          conti = 0;
        }
      }
  }
}

void setup() {
  Serial.begin(9600);
  Serial.println("Hi User");
  // set up LCD
  lcd.init();
  lcd.backlight();

  lcd.setCursor(3, 0);
  lcd.print("Welcome ALL To");
  lcd.setCursor(2, 2);
  lcd.print("POWER UTAH GROUP");
  delay(500);
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("Motor is moving to");
  lcd.setCursor(0, 2);
  lcd.print("The initial position");
  lcd.setCursor(9, 3);

  // command button
  start_button.attach(start_buttonPin, INPUT_PULLUP);
  stop_button.attach(stop_buttonPin, INPUT_PULLUP);
  reset_button.attach(reset_buttonPin, INPUT_PULLUP);
  start_button.interval(5);
  stop_button.interval(5);
  reset_button.interval(5);
  decimal_button.attach(decimal_buttonPin, INPUT_PULLUP);
  delete_button.attach(delete_buttonPin, INPUT_PULLUP);
  dir_button.attach(dir_buttonPin, INPUT_PULLUP);
  decimal_button.interval(25);
  delete_button.interval(25);
  dir_button.interval(25);

  pinMode(limitStartPin, INPUT_PULLUP);
  pinMode(limitEndPin, INPUT_PULLUP);
  pinMode(drillerPin, OUTPUT);
  pinMode(37, INPUT);
  pinMode(des_posPin, OUTPUT);

  Serial.begin(9600);
  // driver pin configuration
  pinMode(enPin, OUTPUT);
  stepper.setAcceleration(500);
  stepper.setMaxSpeed(2000);
  digitalWrite(enPin, LOW);

  // enable interrupt
  enableInterrupt(stop_buttonPin, interruptFunction, CHANGE); 
  enableInterrupt(reset_buttonPin, resetFunction, CHANGE); 

}

void loop() {
  // if (digitalRead(stop_buttonPin) == 1) Serial.println("Stop");
  decimal_button.update(); 
  delete_button.update();
  dir_button.update();
  reset_button.update();
  start_button.update();
  stop_button.update(); 

  char input_key = keypad.getKey();
  if (reset_button.fell()) resetFunction();
  if (stop_button.fell()) interruptFunction();
  if (reset_button.fell()) resetFunction();
  switch (state){
    case 0:
      // initial process: Set home;
      setHome();
      modeMenuLCD();
      state = 1;
      break;
    case 1:
      if (input_key == '*'){
        lcd.clear();
        state = 2;
        if (mode == 1) autoModeLCD();
        else if (mode == 2) manualModeLCD();
        else if (mode == 3) motorModeLCD();
      }
      else if (input_key == '#'){
        modeMenuLCD();
        mode = 0;
      }
      else if (input_key == '1' || mode == 1) {
        if (mode != 1) selectModeLCD("AUTO MODE");
        mode = 1;         // AUTO mode
      }
      else if (input_key == '2' || mode == 2) {
        if (mode !=2) selectModeLCD("MANUAL MODE");
        mode = 2;   // Manual mode
      }
      else if (input_key == '3' || mode == 3){
        if (mode != 3) selectModeLCD("SET MOTOR MODE");
        mode =3;
      }
      break;
    case 2:
      lcd.setCursor(value_ind+9, numStation%4);     // for mmode 2; manual mode
      switch (mode){
        case 1:     // auto mode
          if (input_key == '1') selectPart(WD_08_2LA29_25N00, "WD_08_2LA29_25N00");
          else if (input_key == '2') selectPart(WD_08_2LA29_25M00, "WD_08_2LA29_25M00");
          else if (input_key == '3') selectPart(WD_08_2LA29_25400, "WD_08_2LA29_25400");
          else if (input_key == '4') selectPart(WD_08_2LA29_25L00, "WD_08_2LA29_25L00");
          else if (input_key == '*') {
            state = 3;
            value_ind = 0;
            numStation = 15;
          }
          else if (input_key == '#') autoModeLCD();
          break;
        case 2:     // manual mode
          if (decimal_button.fell() || input_key == 'B'){
            input_key = '.';
          }
          if (delete_button.fell() || input_key == 'A'){
            lcd.setCursor(value_ind+8, numStation%4);     // number of station = total line number
            value_ind --;
            lcd.blink();
            lcd.print(" ");
          }
          else if (input_key == '#') {
            float point = atof(value);              // convert array of char to float
            desire_pos[numStation] = point;         // update destination position array
            numStation ++;
            value_ind = 0;                        // reset value array index to 0
            memset(value, 0, sizeof(value));      // reset value array to 0
            if (numStation%4 == 0){
              manualModeLCD();
            }
          }
          else if (input_key == '*'){
            float point = atof(value);              // convert array of char to float
            desire_pos[numStation] = point; 
            numStation ++;
            state = 3;
            value_ind = 0;
            memset(value, 0, sizeof(value));
          }
          else if (input_key) {
            value[value_ind] = input_key;
            value_ind ++;
            lcd.setCursor(value_ind+8, numStation%4);
            lcd.blink();
            lcd.print(input_key);
          }
          break;
        case 3:     // set motor mode
          if (!digitalRead(limitStartPin) && running && !dir) interruptFunction();
          if (dir_button.fell()) {
            dir = !dir;
            motorModeLCD();
          }
          if (input_key == '#'){
            state = 0;
            mode = 0;
            stepper.stop();
            running = false;
          }
          if (start_button.fell()) {
            running = true;
            if (dir) stepper.setSpeed(SPEED_NORMAL);
            else stepper.setSpeed(-SPEED_NORMAL);
            motorModeLCD();
          }
          if (running){
            stepper.runSpeed();
          }
        break;
      }
      break;
    case 3:     // confirm command
      if (!case3){    // first enter case 3; case3 = false
        lcd.clear();
        lcd.setCursor(4, 0);
        lcd.print("Please press");
        lcd.setCursor(2, 2);
        lcd.print("The START button");
        lcd.noBlink();
      }
      case3 = true;
      if (start_button.fell()) {
        state = 4;
        running = true;
      }
      break;
    case 4:      // move to desire position
      stepperMotor();
      break;
    case 5:     // after finish all tasks
      case3 = false;
      if (input_key == '1') {
        conti = 1;
        selectModeLCD("Part Before");
      }
      else if(input_key == '2') {
        conti = 2;
        mode = 0;
        selectModeLCD("New Part");
      }
      else if (input_key == '#'){
        lcd.clear();
        lcd.setCursor(4, 0); 
        lcd.print("End_SYSTEM"); 
        lcd.setCursor(0, 1); 
        lcd.print("1.Part Before"); 
        lcd.setCursor(0, 2); 
        lcd.print("2.New Part");
      }
      else if (input_key == '*'){
        if (conti == 1) state = 3;
        else if (conti == 2) state = 0;
      }
      break;
    case 6:     // after stopped all process
      if(case6){
        lcd.clear();
        lcd.setCursor(0, 0); 
        lcd.print("THE PROCESS STOPPED"); 
        lcd.setCursor(0, 2); 
        lcd.print("PRESS RESET TO CONT."); 
      }
      case6=0;
      break;
    }
}

void interruptFunction(){
  Serial.print("Stop");
  running = false;
  stepper.stop();
  if (state == 2 && mode == 3){
    motorModeLCD();
  }
  else state = 6;
  case6=1;
}

void resetFunction(){
  if (state == 6){
    state =0;
    mode = 0;
    current_step = 0;
    stepperState = 0;
    substate = 0;
    state = 0;
    mode = 0;
    memset(desire_pos, 0, sizeof(desire_pos)); 
    memset(value, 0, sizeof(value)); 
    numStation = 0;
    value_ind = 0;
    case3 = false;
  }
}

void stopMotor(){
  stepper.stop();
  setHome();
}