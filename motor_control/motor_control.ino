/*
  Pul+ goes to +5V
  Pul- goes to Arduino Mega Pin which is PWM output
  Dir+ goes to +5V
  Dir- goes to to Arduino Pin
  Enable+ to nothing
  Enable- to nothing
*/

#include <AccelStepper.h>

// Define main board pin connections
const int reach_goalPin = 2;
const int input_posPin = 3;

// Define driver pin connections
const int stepPin = 5;
const int dirPin = 6;
const int enPin = 7;

// Define Driller pin connection
const int drillerPin = 45;

// Define variables
const float leadscrewPitch = 5;   // pitch = 5 mm
const float microsteps = 1600;
long int current_step = 0;
float desire_pos[4];
int stepperState = 0;
int substate = 0;
int numStation = 4;

// Therefore, if the motor runs 6400 steps, the machine will move 5 mm.

AccelStepper stepper(1, stepPin, dirPin);     // motorInterfaceType = 1 (driver)

void setup() {
  Serial.begin(9600);
  // main board communication
  pinMode(reach_goalPin, OUTPUT);
  pinMode(input_posPin, INPUT);
  // driver pin configuration
  pinMode(enPin, OUTPUT);
  stepper.setAcceleration(500);
  stepper.setMaxSpeed(2000);
  digitalWrite(enPin, LOW);
}

void loop() {
  
  switch (stepperState){
    case 0:
      float input;
      while (1){
        if (Serial.available() > 0) 
        {
          input = Serial.parseFloat();
          if (input != 0){
            desire_pos[substate] = input;
            break;
          }
        }
      }
      Serial.print("SubState ");
      Serial.print(substate);
      Serial.print(" ");
      Serial.println(desire_pos[substate]);
      substate += 1;
      if (substate >= numStation) {
        stepperState = 1;
        substate = 0;
      }
      break;
    case 1:
      // Move to desired position
      Serial.print("Going to station ");
      Serial.println(substate + 1);
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
    case 2:
      // Driller acivate
      Serial.println("Driller Activated!");
      digitalWrite(reach_goalPin, HIGH); 
      delay(50); 
      digitalWrite(reach_goalPin , LOW); 
      delay(5000);
      if (substate >= numStation){
        substate = 0;
        stepperState = 0;
        Serial.println("Reach all goals");
        break;
      }
      stepperState = 1;
      break;
  }
}
