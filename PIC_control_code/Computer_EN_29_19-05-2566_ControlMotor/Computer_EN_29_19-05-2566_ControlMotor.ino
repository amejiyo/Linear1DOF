#define PUL 5
#define DIR 6
#define EN 7

int PWM = 400; //สัญญาณพลัสที่ตั้งจากกล่องควบคุม
int MAX_SPEED = 1000;
int TOP_SPEED = 140;

int MOTOR_EN =  2;
int MOTOR_Dir = 3;

int DATA1 = 0;
int DATA2 = 0;

void setup() {
  Serial.begin(9600);
  pinMode(MOTOR_EN, INPUT_PULLUP);
  pinMode(MOTOR_Dir, INPUT_PULLUP);

  pinMode(PUL , OUTPUT); //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< ควบคุมมอเตอร์
  pinMode(DIR, OUTPUT);
  pinMode(EN, OUTPUT);
}

void loop() {
  DATA1 = digitalRead(MOTOR_EN);
  DATA2 = digitalRead(MOTOR_Dir);

  if (DATA2 == HIGH) {
    //Serial.println("มอเตอร์เข้า");
    digitalWrite(DIR, HIGH); //----------------------------------------- สั่งให้มอเตอร์เข้า
  }
  if (DATA2 == LOW) {
    //Serial.println("มอเตอร์อออก");
    digitalWrite(DIR, LOW); //----------------------------------------- สั่งให้มอเตอร์ออก
  }

  if (DATA1 == HIGH) {
    MAX_SPEED = 500;
    // Serial.println("มอเตอร์หยุดทำงาน");
    digitalWrite(EN, HIGH); // สั่งให้มอเตอร์หยุดทำงาน
  }

  if (DATA1 == LOW) {
    //Serial.println("มอเตอร์ทำงาน");
    MAX_SPEED--;
    delayMicroseconds(200);
    MAX_SPEED--;
    if (MAX_SPEED <= TOP_SPEED ) {
      MAX_SPEED = TOP_SPEED;
    }
    digitalWrite(EN, LOW); // สั่งให้มอเตอร์ทำงาน
    digitalWrite(PUL, HIGH);
    delayMicroseconds(MAX_SPEED); //SpeedMotor
    digitalWrite(PUL, LOW);
    delayMicroseconds(MAX_SPEED);
  }
}
