//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> ส่วนการควบคุมจอ LCD แสดงผล <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
#include <Wire.h>
//#include <LCD.h>
//#include <LiquidCrystal_I2C.h>
/*
  //#define I2C_ADDR 0x26
  #define I2C_ADDR 0x27 //----- TEST
  #define BACKLIGHT_PIN 3
  LiquidCrystal_I2C lcd(I2C_ADDR, 2, 1, 0, 4, 5, 6, 7);*/

#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>><<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> ส่วนเพิ่มเติม โปรแกรม ENCODER <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
#include <EnableInterrupt.h>
#include <Rotary.h>
//----------- Voor aanpassen meting alleen onderstaande diameter wijzigen -----
//----------- สำหรับ aanpassen meting alleen onderstaande เส้นผ่าศูนย์กลาง wijzigen
//float diameter = 31.8; // gemeten diameter  เส้นผ่านศูนย์กลางอัญมณี
float diameter = 5; // gemeten diameter  เส้นผ่านศูนย์กลางอัญมณี
//-----------------------------------------------------------------------------
volatile long encoderPos = 0;
float distance_mm = 0;
float distance_inch = 0;
float omtrek = diameter; //outline โครงร่าง 3.14159265358979
float distance_corr = omtrek / 60;

//Distance_corr 1.666667 = omtrek wiel 100 mm /60 pulsen/rotatie
//gemeten diam wiel = 31,8 mm >> omtrek = 99,9026464 >> correctie = 1.66504421

const int encoderButton = 40;
const int resetButton = 41;

const int encoderButton_OUT = 46;
const int resetButton_OUT = 47;

int resetbuttonState = 0;
int encoderState = 0;
int lastencoderState = 0;
int encoderCounter = 0;
int dir_upState = 0;
int dir_downState = 0;

Rotary r = Rotary(2, 3);

//----------Create-Arrow-up-sign------------------------------
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
//----------Create-Arrow-down-sign------------------------------
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


int Distance_OOK = 0;
float DistanceENCODER = 0; // ระยะทางที่ต้องการเคลื่อนที่
int MOTOR_EN =  8;
int MOTOR_Dir = 9;
int STOP = 0;
int Encoder_Start = 0;
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> ส่วนเพิ่มเติม โปรแกรม ENCODER <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> ส่วนกำหนดตัวแปร KEY PAD <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
#include <Keypad.h>
const byte ROWS = 4;
const byte COLS = 3;
char keys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', ' '}
};
byte rowPins[ROWS] = {26, 27, 32, 31};
byte colPins[COLS] = {29, 28, 30};

//byte rowPins[ROWS] = {30, 28, 26, 27}; //----TEST
//byte colPins[COLS] = {31, 29, 32};

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
#include <Bounce2.h>
#define BUTTON_START 36
#define BUTTON_STOP 37
Bounce Sensor_A = Bounce();
Bounce Sensor_B = Bounce();
#define BUTTON_A 40
#define BUTTON_B 42
#define BUTTON_C 48
#define BUTTON_D 43
Bounce Sensor_AA = Bounce();
Bounce Sensor_BB = Bounce();
Bounce Sensor_CC = Bounce();
Bounce Sensor_DD = Bounce();

int BUTTON_BlackA = 0;
int BUTTON_BlackB = 0;
int BUTTON_BlackC = 0;
int BUTTON_BlackD = 0;
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>><<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
int BUTTON_STEP = 0;
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> ส่วนกำหนด Limit Switch <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
int Limit_Switch_A   = 38; // ติดตั้งยังตำแหน่งเริ่มต้น
int Limit_Switch_B   = 39; // ติดตั้งยังตำแหน่งท้ายสุด
int currentButtonStateA  = HIGH; // ค่าสถานะปัจจุบนของปุ่ม
int currentButtonStateB  = HIGH; // ค่าสถานะปัจจุบนของปุ่ม
int previousButtonStateA = HIGH;// ค่าสถานะของปุ่มกดครั้งที่แล้ว
int previousButtonStateB = HIGH;// ค่าสถานะของปุ่มกดครั้งที่แล้ว
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>><<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

#define Relay_drill 45 //ควบคุมการเจาะสว่าน

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> ตัวแปรกำหนดขั้นตอนการทำงาน <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
int STARTGO = 0;
int STARTGO2 = 0;
int STARTGO3 = 0;
float Distance = 0; // ระยะทางที่ต้องการเคลื่อนที่
int Distance_Chack = 0; // ระยะทางที่ต้องการเคลื่อนที่
float DistanceA = 0;
float Distance_AUTO = 0; // ระยะทางที่ต้องการเคลื่อนที่
float Distance_AUTO1[30]; // ระยะทางที่กำลังเคลื่อนที่
float Distance_Part = 0; // ระยะทางที่กำลังเคลื่อนที่
int Distance_NEXT_Part = 0; //ระยะทางที่ต้องการเคลื่อนที่ Part ถัดไป
int Distance_Former_Part = 0; //ระยะทาง former
float DistanceOK = 0;
float STEP = 0;
int New_Part = 0; // ใช้เก็บข้อมูลการเคลื่อนที่กรณีการเลือก Part ใหม่ ให้มอเตอร์เริ่มจากจุดนี้
int START = 0;
int STARTM = 0;
int Display = 0;
int STRAT_SYSTEMS = 0;
int End_SYSTEMS = 0;
int clearLCD = 0;
int counter = 0;
int ConfirmMM = 0;
//////////////////////////////////////////////// การทำงานขั้นตอนที่ 1 ///////////////////////////////////////////////////////////
/////////////////////////////////////// กำหนดตัวแปรการควบคุมมอเตอร์ ไปยังตำแหน่ง 0 ////////////////////////////////////////////////
int MOTOR_SET_POIRT_0 = 0;
int Motor_Turn = 0;
int KEY11 = 0;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////// ตัวแปรโหมด AUTO Part //////////////////////////////////////////////////////////////////////////////
int Select_Part_Mode = 0;
int Select_Monitor = 0;
int set_Systems = 0;
int Display_SELECT_Confirm = 0;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////// การทำงานขั้นตอนที่ 2 ///////////////////////////////////////////////////////////
//////////////////////////////////////////// กำหนดตัวแปรการเลือกโหมดการทำงาน ///////////////////////////////////////////////////
int START_SELECT_MODE = 0;
int Display_SELECT_MODE = 0;
int AUTO_MODE = 0;
int MANUAL_MODE = 0;
int MANUAL_NEW_MODE = 0;  //<<<<<<<<<<<<< โหมดใหม่
int SET_MOTOR_MODE = 0;
int Confirm = 0;
int Cancel = 0;
//////////////////////////////////////////////// การทำงานขั้นตอนที่ 2.1 ///////////////////////////////////////////////////////////
//////////////////////////////////////////// กำหนดตัวแปรโหมดการทำงาน AUTO ////////////////////////////////////////////////////
int SELECT_PART_MODE = 0; // ขั้นตอนการเลือก Part ผลิตภัณฑ์
int PART = 0; // ตัวแปรเก็บการทำงาน PART ที่กำหนด
int PART_OK = 0; // ยืนยันการเลือก Part
int Display_SELECT_PART = 0; // หน้าจอเลือกใช้งาน Part
int LL = 0;
float STEP_Former = 1; // เก็บตำแหน่งล่าสุดที่ผ่านมา
float STEP_i = 0;
int STEP_Next_Part = 1;
int CF = 0;
int CF2 = 0;
int LCD_Monotor = 0;
int Display_Start = 0;
int CUT_OK = 0;
int Cahk_Distance = 0;
/////////////////////////////////////////////////////// เก็บข้อมูลแต่ละ Part ////////////////////////////////////////////////////
//int WD_08_2LA29_25N00[15] = {0, 15, 25}; // ทดสอบ
int Part1 = 0;
int Part2 = 0;
int Part3 = 0;
int Part4 = 0;
int Part_Confirm = 0;
int Part_Step = 0;
float Distances[300];
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< กรณีฝังระยะเจาะในโปรแกรม >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
float WD_08_2LA29_25N00[15] = {15, 215, 200, 70, 55, 45, 600, 45, 125, 200, 215, 20};
float WD_08_2LA29_25M00[15] = {15, 230, 200, 55, 55, 345, 345, 110, 200, 230, 15};
float WD_08_2LA29_25400[15] = {142.5, 193, 243.5, 294, 344.5, 395, 445.5, 496, 17600};
float WD_08_2LA29_25L00[15] = {142.5, 193, 243.5, 294, 344.5, 395, 445.5, 496, 546.5, 1700};
//////////////////////////////////////////////////// สิ้นสุดเก็บข้อมูลแต่ละ Part ////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////// การทำงานขั้นตอนที่ 3 ///////////////////////////////////////////////////////////
//////////////////////////////////////////// กำหนดตัวแปรการเลือกโหมดการทำงาน ///////////////////////////////////////////////////
int STEP_is = -1;
int STEP_Next_Parts = 0;
int STEP_Formers = -1; //เก็บตำแหน่งล่าสุดที่ผ่านมา -1
int STEP_Formers_2 = 0; //เก็บตำแหน่งล่าสุดที่ผ่านมา -1
int RESET_COUTN = 0;
char key;
char val[10];
int k = 0;
float value = 0;
int START_MANUAL = 0;
int value_A = 0;
int Distance_value = 0;
int COUNT_ENTER = 8;
int ENTER = 0;
int i = 0;
int count_Point = 0;
int Display_SET_Point = 0;
int Point_No1 = 1; // ตัวแปรเก็บตำแหน่ง Point No.
int Point_No2 = 2;
int Point_No3 = 3;
int Point_No4 = 4;
int STEP_Point[200]; // 32,767
int clear_LCD = 0;
int Control_Motor = 0;
int COUNT = 0;
int updet_display = 1;
int Edit_display = 0;
int SET_SYSTEMS = 0;
int End = 0;
int Data_Before = 0;
int New_Data = 0;
int SYSTEMS_COUNT = 0;
int MOTOR_START = 0;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int Display_SET_MOTOR_MODE = 0;
int SpeedMotor = 0;
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>><<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
float Go_To_Start_Point = 0;
float Go_To_Start_PointA = 0;
int Pause = 0;
int STOP_Chack = 0;

void setup () {
  Serial.begin(9600);
  //lcd.begin (20, 4); //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< จอLCD
  //lcd.setBacklightPin(BACKLIGHT_PIN, POSITIVE);
  //lcd.setBacklight(HIGH);
  //lcd.home (); // ไปที่ตัวอักษรที่ 0 แถวที่ 1
  lcd.init();
  lcd.createChar(0, Arrow_down);
  lcd.createChar(1, Arrow_up);

  lcd.setCursor(3, 0);
  lcd.print("Welcome ALL To");
  lcd.setCursor(2, 2);
  lcd.print("POWER UTAH GROUP");
  delay(500);
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("Motor is moving to");
  lcd.setCursor(0, 2);
  lcd.print("the initial position");
  lcd.setCursor(9, 3);
  lcd.blink();

  //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> ส่วนเพิ่มเติม โปรแกรม ENCODER <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
  enableInterrupt(2, interruptFunction, CHANGE);
  enableInterrupt(3, interruptFunction, CHANGE);

  pinMode(encoderButton_OUT, OUTPUT);
  pinMode(resetButton_OUT, OUTPUT);

  pinMode(encoderButton, INPUT_PULLUP);
  pinMode(resetButton, INPUT_PULLUP);

  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> ส่วนเพิ่มเติม โปรแกรม ENCODER <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

  Sensor_A.attach(BUTTON_START, INPUT_PULLUP);//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< ปุ่มกด START STOP
  Sensor_B.attach(BUTTON_STOP, INPUT_PULLUP);
  Sensor_A.interval(5);
  Sensor_B.interval(5);

  Sensor_AA.attach(BUTTON_A, INPUT_PULLUP);//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< ปุ่มกด
  Sensor_BB.attach(BUTTON_B, INPUT_PULLUP);
  Sensor_CC.attach(BUTTON_C, INPUT_PULLUP);
  Sensor_DD.attach(BUTTON_D, INPUT_PULLUP);
  Sensor_AA.interval(25);
  Sensor_BB.interval(25);
  Sensor_CC.interval(25);
  Sensor_DD.interval(25);

  pinMode(Limit_Switch_A, INPUT_PULLUP);
  pinMode(Limit_Switch_B, INPUT_PULLUP);
  pinMode(Relay_drill, OUTPUT);

  pinMode(MOTOR_EN, OUTPUT);
  pinMode(MOTOR_Dir, OUTPUT);
  
  MOTOR_SET_POIRT_0 = 1; //>>>>>>> สั่งให้ระบบทำงานในขั้นตอนที่ 1 นำมอเตอร์ไปยังตำแหน่งที่ 0
}


void loop() {
  //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> ส่วนเพิ่มเติม โปรแกรม ENCODER <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
  //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> ส่วนเพิ่มเติม โปรแกรม ENCODER <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
  currentButtonStateA = digitalRead(Limit_Switch_A);// อ่านค่าสถานะของ push switch (ว่ากด หรือ ไม่กด)
  currentButtonStateB = digitalRead(Limit_Switch_B);// อ่านค่าสถานะของ push switch (ว่ากด หรือ ไม่กด)
  Sensor_A.update(); // หากต้องการเช็คสถานะจาก LOW เป็น HIGH ให้แทนที่ฟังก์ชั่น fell() ด้วยฟังก์ชั่น rose()
  Sensor_B.update();
  Sensor_AA.update();
  Sensor_BB.update();
  Sensor_CC.update();
  Sensor_DD.update();
  if (Pause == 0) { //<<<<<<<<<<<< สั่งให้ระบบหยุดชั่วคราว
    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< เมื่อระบบเริ่มทำงานให้มอเตอร์กลับไปยังตำแหน่งเริ่มต้น >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< เมื่อระบบเริ่มทำงานให้มอเตอร์กลับไปยังตำแหน่งเริ่มต้น >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< เมื่อระบบเริ่มทำงานให้มอเตอร์กลับไปยังตำแหน่งเริ่มต้น >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< เมื่อระบบเริ่มทำงานให้มอเตอร์กลับไปยังตำแหน่งเริ่มต้น >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> ทำงานในขั้นตอนที่ 1 <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    if (MOTOR_SET_POIRT_0 == 1) {
      digitalWrite(MOTOR_Dir, HIGH);
      digitalWrite(MOTOR_EN, HIGH);
      if ((currentButtonStateA != previousButtonStateA) && previousButtonStateA == HIGH) { //<<<<<<<<<<<<<<<<< ชนลิมิตสวิทซ์แล้วให้หยุด
        Serial.println("Limit Switch จุดเริ่มต้น: ทำงาน");
        STOP_Chack = 1;
        digitalWrite(MOTOR_EN, LOW); // สั่งให้มอเตอร์"หยุด"ทำงาน
        digitalWrite(MOTOR_Dir, LOW); // สั่งให้มอเตอร์หมุนไปทางซ้าย เพื่อเตรียทพร้อมในการทำงาน //<<<<<<<<<<<<<<<<<<<<<<<<<< สั่งให้มอเตอร์เตรียมหมุนซ้าย
        MOTOR_SET_POIRT_0 = 0; // ออกจากโหมดการทำงาน MOTOR_SET_POIRT_0
        START_SELECT_MODE = 1; // สั่งให้เข้าสู่โหมดการทำงาน START_SELECT_MODE
        Display_SELECT_MODE = 1; // สั่งให้เข้าสู่ Display การเลือกโหมดการทำงาน
        lcd.noBlink(); // สั่งให้เคอร์เซอร์หยุดทำงาน
      }
      if (digitalRead(BUTTON_STOP) == 0) { //<<<<<<<<<<< กดปุ่ม STOP เพื่อหยุดการทำงาน
        STOP = 1;
        digitalWrite(MOTOR_EN, LOW); // สั่งให้มอเตอร์"หยุด"ทำงาน// สั่งให้มอเตอร์"หยุด"ทำงาน
        Serial.println("STOP");
        SET_MOTOR_MODE = 0;
        MANUAL_MODE = 0;
        AUTO_MODE = 0;
        MOTOR_SET_POIRT_0 = 0;
        STARTGO = 0;
        lcd.clear();
      }
    }
    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> สิ้นสุดการทำงานในขั้นตอนที่ 1 <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< เมื่อระบบเริ่มทำงานให้มอเตอร์กลับไปยังตำแหน่งเริ่มต้น >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< เมื่อระบบเริ่มทำงานให้มอเตอร์กลับไปยังตำแหน่งเริ่มต้น >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< เมื่อระบบเริ่มทำงานให้มอเตอร์กลับไปยังตำแหน่งเริ่มต้น >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< เมื่อระบบเริ่มทำงานให้มอเตอร์กลับไปยังตำแหน่งเริ่มต้น >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< เลือกโหมดการทำงาน >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< เลือกโหมดการทำงาน >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< เลือกโหมดการทำงาน >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< เลือกโหมดการทำงาน >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> ทำงานในขั้นตอนที่ 2 <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    /////////////////////////////////////////////////////////// เริ่มทำงานในขั้นตอนที่ 2.1 ////////////////////////////////////////////////////////
    if (START_SELECT_MODE == 1) {
      char key1 = keypad.getKey(); // อ่านค่าจากการกด KEYPAD
      if (key1 == '1') { // กดเลข 1 บน KEYPAD เพื่อเลือกใช้งานในโหมด AUTO
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("SYSTEMS SELECT MODE");
        lcd.setCursor(1, 2);
        lcd.print("<<< AUTO MODE >>>");
        lcd.setCursor(0, 3);
        lcd.print("*=Confirm  #=Cancel");
        set_Systems = 1;
        Display_SELECT_MODE = 0; // สั่งให้ออกจาก Display_SELECT_MODE
        MANUAL_MODE = 0; // ให้ออกจากโหมดการทำงาน MANUAL_MODE
        SET_MOTOR_MODE = 0; // ให้ออกจากโหมดการทำงาน SET MOTOR
        MANUAL_NEW_MODE = 0; // ให้ทำงานในโหมด MANUAL
        AUTO_MODE = 1; // ให้ทำงานในโหมด AUTO
        SELECT_PART_MODE = 1; // สั่งให้ไปทำงานในส่วนการเลือก Part โหมด AUTO
        Display_SELECT_PART = 1; // สั่งให้ไปที่หน้าต่าง การเลือก Part
      }

      if (key1 == '2') { // กดเลข 2 บน KEYPAD เพื่อเลือกใช้งานในโหมด MANUAL
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("SYSTEMS SELECT MODE");
        lcd.setCursor(0, 2);
        lcd.print("<<< MANUAL MODE >>>");
        lcd.setCursor(0, 3);
        lcd.print("*=Confirm  #=Cancel");
        Display_SELECT_MODE = 0; // สั่งให้ออกจาก Display_SELECT_MODE
        AUTO_MODE = 0; // ให้ออกจากโหมดการทำงาน AUTO
        MANUAL_NEW_MODE = 0; // ให้ทำงานในโหมด MANUAL
        SET_MOTOR_MODE = 0; // ให้ออกจากโหมดการทำงาน SET MOTOR
        START_MANUAL = 1;
        MANUAL_MODE = 1; // ให้ทำงานในโหมด MANUAL
      }

      if (key1 == '3') { // กดเลข 2 บน KEYPAD เพื่อเลือกใช้งานในโหมด MANUAL
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("SYSTEMS SELECT MODE");
        lcd.setCursor(0, 2);
        lcd.print("<<< MANUAL NEW MODE >>>");
        lcd.setCursor(0, 3);
        lcd.print("*=Confirm  #=Cancel");
        Display_SELECT_MODE = 0; // สั่งให้ออกจาก Display_SELECT_MODE
        AUTO_MODE = 0; // ให้ออกจากโหมดการทำงาน AUTO
        SET_MOTOR_MODE = 0; // ให้ออกจากโหมดการทำงาน SET MOTOR
        MANUAL_MODE = 0; // ให้ออกจากโหมดการทำงาน MANUAL_MODE

        START_MANUAL = 1;
        MANUAL_NEW_MODE = 1; // ให้ทำงานในโหมด MANUAL
      }

      if (key1 == '4') { // กดเลข 3 บน KEYPAD เพื่อเลือกใช้งานในโหมด SET MOTOR MODE
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("SYSTEMS SELECT MODE");
        lcd.setCursor(0, 2);
        lcd.print("<<<SET MOTOR MODE>>>");
        lcd.setCursor(0, 3);
        lcd.print("*=Confirm  #=Cancel");
        Display_SELECT_MODE = 0; // สั่งให้ออกจาก Display_SELECT_MODE
        AUTO_MODE = 0; // ให้ออกจากโหมดการทำงาน AUTO
        MANUAL_MODE = 0; // ให้ออกจากโหมดการทำงาน MANUAL_MODE
        MANUAL_NEW_MODE = 0; // ให้ทำงานในโหมด MANUAL
        Display_SET_MOTOR_MODE = 1;
        SET_MOTOR_MODE = 1;  // ให้ทำงานในโหมด SET MOTOR


        digitalWrite(MOTOR_Dir, HIGH); //<<<<<<<<<<<<<<<<<<<<<<<<<< สั่งให้มอเตอร์เตรียมหมุนซ้าย


      }
      if (key1 == '*') { // ยืนยันการเลือกโหมดการทำงาน
        Display_SELECT_MODE = 0; // สั่งให้ออกจาก Display_SELECT_MODE
        START_SELECT_MODE = 0; // สั่งให้ออกจาก START_SELECT_MODE
        Confirm = 1; // ยืนยันการทำงานการเลือกโหมด
        lcd.clear(); // เคลียร์หน้าจอ LCD
      }
      if (key1 == ' ') { // # เลือกโหมดการทำงานใหม่
        AUTO_MODE = 0; // ให้ออกจากโหมดการทำงาน AUTO
        MANUAL_MODE = 0; // ให้ออกจากโหมดการทำงาน MANUAL_MODE
        SET_MOTOR_MODE = 0;  // ให้ออกจากโหมดการทำงาน SET MOTOR
        Display_SELECT_MODE = 1; // เลือกโหมดการทำงานใหม่
      }

      if (Display_SELECT_MODE == 1) { //ส่วนควบคุมการแสดงในส่วนการเลือกโหมดการทำงานของระบบ
        lcd.clear();
        lcd.setCursor(3, 0);
        lcd.print("1.AUTO MODE");
        lcd.setCursor(3, 1);
        lcd.print("2.MANUAL MODE");
        lcd.setCursor(3, 2);
        lcd.print("3.MANUAL NEW MODE ");
        lcd.setCursor(3, 3);
        lcd.print("4.SET MOTOR MODE");
        Display_SELECT_MODE = 0;
      }
    }
    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< เลือกโหมดการทำงาน >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< เลือกโหมดการทำงาน >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< เลือกโหมดการทำงาน >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< เลือกโหมดการทำงาน >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>





    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< โหมดทำงานแบบ AUTO Part >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< โหมดทำงานแบบ AUTO Part >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< โหมดทำงานแบบ AUTO Part >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< โหมดทำงานแบบ AUTO Part >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< โหมดทำงานแบบ AUTO Part >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< โหมดทำงานแบบ AUTO Part >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< โหมดทำงานแบบ AUTO Part >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< โหมดทำงานแบบ AUTO Part >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>





    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< โหมดทำงานแบบ MANUL >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< โหมดทำงานแบบ MANUL >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< โหมดทำงานแบบ MANUL >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< โหมดทำงานแบบ MANUL >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    if ((MANUAL_MODE == 1) && (Confirm == 1)) {
      char key3 = keypad.getKey(); // อ่านค่าจากการกด KEYPAD
      if (START_MANUAL == 1) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Part");
        lcd.print(Point_No1);
        lcd.print(" = ");
        lcd.setCursor(17, 0);
        lcd.print("mm.");

        lcd.setCursor(0, 1);
        lcd.print("Part");
        lcd.print(Point_No2);
        lcd.print(" = ");
        lcd.setCursor(17, 1);
        lcd.print("mm.");

        lcd.setCursor(0, 2);
        lcd.print("Part");
        lcd.print(Point_No3);
        lcd.print(" = ");
        lcd.setCursor(17, 2);
        lcd.print("mm.");

        lcd.setCursor(0, 3);
        lcd.print("Part");
        lcd.print(Point_No4);
        lcd.print(" = ");
        lcd.setCursor(17, 3);
        lcd.print("mm.");
        delay(500);

        SET_SYSTEMS = 1;
        START_MANUAL = 0;
      }

      if (SET_SYSTEMS == 1) {
        ////////////////////// ตรวจจับการกด KEY //////////////////////////////////
        if (Sensor_AA.fell()) {
          key3 = '.';
          Serial.println("BUTTON_Black 1");
        }

        if (key3 != NO_KEY) {
          //////////////////////// ส่วนควบคุม Cursor /////////////////////////////////
          COUNT_ENTER++;
          val[k] = key3; //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< เก็บค่าจากการกด KEYPAD
          k++;
          updet_display = 1;
          Edit_display = 0;
          //Serial.println(key3);
        }
        if (Sensor_BB.fell()) {
          BUTTON_BlackB = 1;
          Serial.println("BUTTON_Black 2");
        }
        if (BUTTON_BlackB == 1) { //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< กดเพื่อลบตัวเลขที่พิมพ์ผิด
          COUNT_ENTER--;
          k--;
          updet_display = 1;
          Edit_display = 1;
        }
        if (updet_display == 1) {
          if (Edit_display == 0 ) {
            lcd.setCursor(COUNT_ENTER, ENTER);
            lcd.blink();
            lcd.print(key3);
            updet_display = 0;
          }
          if (Edit_display == 1 ) { //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< ลบแก้ไข้ค่าที่กรอกมาทีละ 1 ตำแหน่ง
            k--;
            lcd.setCursor(COUNT_ENTER--, ENTER);
            lcd.print(" ");
            lcd.blink();
            updet_display = 0;
          }
        }
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        if (key3 == ' ') { //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< กด # เพื่อบันทึกข้อมูล
          lcd.noBlink();
          ENTER++; // สั่งให้เปลี่ยนบรรทัดใหม่
          COUNT_ENTER = 8;
          for (int i = 0; i < k; i++) {
            if (val[i] == 'C') {
              val[i] = '.';
            }
          }
          value = atof(val);
          Distances[Distance_value] = value;
          Distance_value++;
          k = 0;
          if (ENTER > 3) { // ถ้าพิมพ์ครบ 4 บรรทัดแล้วให้ไปเริ่มพิมพ์ใหม่
            ENTER = 0;
          }
          clear_LCD++;
          if (clear_LCD > 3 ) {
            clear_LCD = 0;
            lcd.clear();
            Display_SET_Point = 1;
          }
        }

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        if (Display_SET_Point == 1) { //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< แสดงผลการกำหนดค่าบน LCD
          for (int j = 1; j < 5; j++) {
            Point_No1++;
            Point_No2++;
            Point_No3++;
            Point_No4++;
          };
          lcd.setCursor(0, 0);
          lcd.print("Part");
          lcd.print(Point_No1);
          lcd.print(" = ");
          lcd.setCursor(17, 0);
          lcd.print("mm.");
          lcd.setCursor(0, 1);
          lcd.print("Part");
          lcd.print(Point_No2);
          lcd.print(" = ");
          lcd.setCursor(17, 1);
          lcd.print("mm.");
          lcd.setCursor(0, 2);
          lcd.print("Part");
          lcd.print(Point_No3);
          lcd.print(" = ");
          lcd.setCursor(17, 2);
          lcd.print("mm.");
          lcd.setCursor(0, 3);
          lcd.print("Part");
          lcd.print(Point_No4);
          lcd.print(" = ");
          lcd.setCursor(17, 3);
          lcd.print("mm.");
          Display_SET_Point = 0;
        }

        if (key3 == '*') { //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< กด * เพื่อสรุปค่าที่กรอก
          for (i = 0; i < Distance_value; i++) {
            Distance = Distances[i];
            count_Point++;
            STEP_Point[count_Point] = count_Point;
            Serial.print("Part ");
            Serial.print(STEP_Point[count_Point]);
            Serial.print(" = ");
            Serial.println(Distances[i]);
          }

          lcd.clear();
          lcd.setCursor(4, 0);
          lcd.print("Please press");
          lcd.setCursor(2, 2);
          lcd.print("The START button");
          lcd.noBlink();

          updet_display = 0;
          k = 0;
          Distance_value = 0;
          Control_Motor = 1; //<<<<<<<<<<<<<<<<<<<<<<<<<<<< สั่งให้ไปทำงานต่อในส่วนควบคุมมอเตอร์
          SET_SYSTEMS = 0; //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< สั่งให้ออกจากโหมด SET SYSTEM
        }
      }

      ////////////////////////////////////////////////// ///////////////// การควบคุมมอเตอร์ /////////////////////////////////////////////////
      if (Control_Motor == 1) {
        if (CUT_OK == 0) {
          if (Sensor_A.fell()) { //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< ปุ่มกด Start เริ่มทำงาน
            START = 1;
            LL++;
            STEP_is++; // นับ STEP ปัจจุบัน
            STEP_Next_Parts++;  // นับ STEP ถัดไป
            Distance_Part = Distances[STEP_is]; // เก็บค่า STEP ปัจจุบัน
            Distance_NEXT_Part = Distances[STEP_Next_Parts]; // เก็บค่า STEP ถัดไป
            if (New_Part == 0 ) {
              if (STEP_is == 0 && Distance_Former_Part <= 0) {
                Distance = Distance_Part; // Part ที่ 1
                Serial.println("STEP 1 ");
              }
              if (STEP_is > 0 || Distance_Former_Part > 0) {
                Serial.println("STEP 2 ");
                if (SYSTEMS_COUNT <= 0) {
                  STEP_Formers++; // นับ STEP ที่ผ่านมา
                  Distance_Former_Part = Distances[STEP_Formers];
                  Serial.println("STEP 3 ");
                }
                if (SYSTEMS_COUNT  >= 1) {
                  COUNT++;
                  if (COUNT == 1) {
                    SYSTEMS_COUNT = 0;
                    COUNT = 0;
                    Serial.println("OKOKOK");
                  }
                }

                if (Distance_Former_Part == 0) {
                  Distance = Distance_Part;
                  Serial.println("STEP 6 ");
                }
                if (Distance_Former_Part > 0) {
                  Distance = Distance_Part - Distance_Former_Part;
                  Serial.println("STEP 7 ");
                }
              }
            }
            if (New_Part == 1 ) {
              if (STEP_is == 0 && Distance_Former_Part <= 0) {
                Distance = Distance_Part; // Part ที่ 1
                Serial.println("STEP 1 ");
              }
              if (STEP_is >= 0 || Distance_Former_Part >= 0) {
                Serial.println("STEP 2 ");
                if (SYSTEMS_COUNT <= 0) {
                  STEP_Formers++; // นับ STEP ที่ผ่านมา
                  Distance_Former_Part = Distances[STEP_Formers];
                  Serial.println("STEP 3 ");
                }
                if (SYSTEMS_COUNT  >= 1) {
                  COUNT++;
                  if (COUNT == 1) {
                    SYSTEMS_COUNT = 0;
                    COUNT = 0;
                    Serial.println("OKOKOK");
                  }
                }
                if (Distance_Former_Part >= 0) {
                  Distance = Distance_Part - Distance_Former_Part;
                  Serial.println("STEP 7 ");
                }
              }
            }
            Serial.println(" ");
            Serial.print("SYSTEMS_COUNT = "); Serial.print(SYSTEMS_COUNT);
            Serial.print(" > LL = "); Serial.print(LL);
            Serial.print(" > CUT_OK = "); Serial.print(CUT_OK);
            Serial.print(" > STEP_is = "); Serial.print(STEP_is);
            Serial.print(" > End_SYSTEMS = "); Serial.print(End_SYSTEMS);

            Serial.print(" > >> จุดตัดปัจจุบัน = "); Serial.print(Distance_Part);
            Serial.print(" > จุดตัดถัดไป = "); Serial.print(STEP_Next_Parts);
            Serial.print(" > จุดตัดที่ผ่านมา = "); Serial.print(STEP_Formers);
            Serial.println(" ");
            Display_Start = 1;
            delay(200);
          }
        }

        if (START == 1) {
          if (Distance < 0) {
            digitalWrite(MOTOR_Dir, HIGH); //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< สั่งให้มอเตอร์หมุนซ้าย
            Distance;
            Serial.println("STEP TEST 1");
            Motor_Turn = 1;
          }
          if (Distance > 0) {
            digitalWrite(MOTOR_Dir, LOW);  //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< สั่งให้มอเตอร์หมุนขวา
            Distance;
            Serial.println("STEP TEST 2");
            Motor_Turn = 2;
          }
          STEP = (abs(Distance)) / 5; //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<  คำนวณการเคลื่อนที่มอเตอร์
          DistanceOK = STEP * 400;
          if (Display_Start == 1) {
            Serial.println(" ");
            Serial.print("Distance = ");
            Serial.print(Distance);
            Serial.print(" > STEP = ");
            Serial.print(STEP);
            Serial.print(" > DistanceOK = ");
            Serial.print(DistanceOK);
            Serial.print(" > จุดตัดที่ผ่านมา = ");
            Serial.print(Distance_Former_Part);
            Serial.print(" > จุดตัดปัจจุบัน = ");
            Serial.print(Distance_Part);
            Serial.print(" > จุดตัดถัดไป = ");
            Serial.print(Distance_NEXT_Part);
            Serial.print(" > ระยะการเคลื่อนที่ = ");
            Serial.print(Distance);
            Serial.print(" mm. ");
            Serial.print(" จุดตัดที่ = ");
            Serial.print(LL);
            Serial.print(" STEPPER = ");
            Serial.print(DistanceOK);
            Serial.print(" SPR. ");
            //Serial.print(" DELAY MOTOR = ");
            //Serial.println(SpeedMotor);
            Serial.println(" ");
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Distance = ");
            lcd.print(abs(Distance));
            lcd.print(" mm.");
            lcd.setCursor(0, 1);
            lcd.print("Part NO.= ");
            lcd.print(LL);
            lcd.setCursor(0, 2);
            //lcd.print("SPEED MOTOR= ");
            // lcd.print(SpeedMotor);
            if (Motor_Turn == 1) {
              lcd.setCursor(2, 3);
              lcd.print("    Motor IN");
              Motor_Turn = 0;
            }
            if (Motor_Turn == 2) {
              lcd.setCursor(2, 3);
              lcd.print("    Motor OUT");
              Motor_Turn = 0;
            }
            delay(100);
            Display_Start = 0;
          }
          STARTGO = 1;
        }

        if (CF == 1) { // เมื่อมอเตอร์ทำงานเสร็จให้ทำงานในกระบวนการต่อไป
          START = 0;
          delay(100);
          lcd.clear();
          Drill(); //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< สั่งสว่านเจาะ
          lcd.setCursor(6, 0);
          lcd.print("SYSTEMS");
          lcd.setCursor(4, 1);
          lcd.print("*** CUT ***");
          lcd.setCursor(0, 2);
          lcd.print("Point NO.= ");
          lcd.print(LL);
          lcd.print(" OK. >> ");
          delay(100);
          CF = 0;

          if (Distance_NEXT_Part <= 0) { // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< ถ้าจุดถัดไปมีค่าน้อยกว่าจุดปัจจุบันให้ ที่โหมดสิ้นสุดการทำงาน
            lcd.clear();
            End_SYSTEMS = 1;
            CUT_OK = 1;
          }
        }

        if (CUT_OK == 1) {
          START = 0;
          CF = 0;

          if (End_SYSTEMS == 1) {
            lcd.setCursor(6, 0);
            lcd.print("End_SYSTEMS");
            lcd.setCursor(0, 1);
            lcd.print("1.Part Before");
            lcd.setCursor(0, 2);
            lcd.print("2.New Part");
            if (key3 == '1') {
              Distance_Former_Part = Distance_Part; // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< ให้จุดตัดถัดไป มีค่าเท่ากับ จุดตัดปัจจุบัน ณ ตอนนี้
              LL = 0;
              CUT_OK = 0;
              End_SYSTEMS = 0;
              STEP_is = -1;
              STEP_Next_Parts = 0;
              STEP_Formers = -1; // เก็บตำแหน่งล่าสุดที่ผ่านมา
              SYSTEMS_COUNT++;
              Serial.print("LL = "); Serial.print(LL);
              Serial.print(" > CUT_OK = "); Serial.print(CUT_OK);
              Serial.print(" > STEP_is = "); Serial.print(STEP_is);
              Serial.print(" > End_SYSTEMS = "); Serial.println(End_SYSTEMS);

              Serial.print("จุดตัดปัจจุบัน = "); Serial.print(Distance_Part);
              Serial.print(" > จุดตัดถัดไป = "); Serial.print(STEP_Next_Parts);
              Serial.print(" > จุดตัดที่ผ่านมา = "); Serial.print(STEP_Formers);
              Serial.println(" ");

              lcd.clear();
              lcd.setCursor(4, 0);
              lcd.print("Please press");
              lcd.setCursor(2, 1);
              lcd.print("The START button");
            }

            if (key3 == '2') { // กด 2 เพื่อเลือกใหม่
              Point_No1 = 1;
              Point_No2 = 2;
              Point_No3 = 3;
              Point_No4 = 4;
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("Part");
              lcd.print(Point_No1);
              lcd.print(" = ");
              lcd.setCursor(17, 0);
              lcd.print("mm.");

              lcd.setCursor(0, 1);
              lcd.print("Part");
              lcd.print(Point_No2);
              lcd.print(" = ");
              lcd.setCursor(17, 1);
              lcd.print("mm.");

              lcd.setCursor(0, 2);
              lcd.print("Part");
              lcd.print(Point_No3);
              lcd.print(" = ");
              lcd.setCursor(17, 2);
              lcd.print("mm.");

              lcd.setCursor(0, 3);
              lcd.print("Part");
              lcd.print(Point_No4);
              lcd.print(" = ");
              lcd.setCursor(17, 3);
              lcd.print("mm.");

              SET_SYSTEMS = 1;
              COUNT_ENTER = 8;
              ENTER = 0;
              CUT_OK = 0;
              End_SYSTEMS = 0;
              STEP_is = -1;
              STEP_Next_Parts = 0;
              //STEP_Formers = -2;
              RESET_COUTN = 0;
              LL = 0;
              Control_Motor = 0;
              Display_SET_Point = 0;
              value = 01;
              Distance_value = 0;
              Distance = 0;
              Distance_Part = 0;
              Distances[0];
              STEP_Point[0];
              count_Point = 0;
              clear_LCD = 0;
              New_Part = 1;
            }
          }
        }
      }
    } // สิ้นสุด โหมด MANUAL

    if (STARTGO == 1) {
      START = 0;
      Distance_OOK = 1;
      digitalWrite(MOTOR_EN, HIGH);//-----------------------------------------สั่งมอเตอร์ทำงาน
    }
    
    if (Distance_OOK == 1) {
      encoderState = digitalRead(encoderButton);// compare the buttonState to its previous state เปรียบเทียบ buttonState กับสถานะก่อนหน้า
      if (encoderState != lastencoderState) { // if the state has changed, increment the counter หากสถานะมีการเปลี่ยนแปลง ให้เพิ่มตัวนับ
        if (encoderState == HIGH ) { // if the current state is HIGH then the button went from off to on: หากสถานะปัจจุบันเป็น HIGH ปุ่มจะเปลี่ยนจากปิดเป็นเปิด:
          encoderCounter++;
        }
      }
      lastencoderState = encoderState;
      //---------counter-pushbutton-encoder----------- ตัวเข้ารหัสปุ่มกดเคาน์เตอร์

      resetbuttonState = digitalRead(resetButton);
      if (encoderCounter > 2) {
        encoderCounter = 0;
      }
      if (resetbuttonState == LOW) {
        encoderPos = 0.0;
      }
      distance_mm = (encoderPos / 10) * distance_corr; //encoderPos/10; ------- ตัวเข้ารหัสPos/10
      distance_inch = distance_mm / 25.4;
      //Serial.print("ระยะทางการเคลื่อนที่: ");
      //Serial.print(distance_mm);
      //Serial.println(" mm.");

      //Distance_Part - 3.5;

      if (distance_mm >= Distance_Part) { //----------------------------------------------------------- ถ้าระ Encoder วัดระยะทางการเคลื่อนที่ได้ตามที่กำหนดให้มอเตอร์หยุดทำงาน
        digitalWrite(MOTOR_EN, LOW);//-----------------------------------------สั่งมอเตอร์หยุดทำงาน
        //Serial.print("55555555555555");
        Serial.print("ระยะทางการเคลื่อนที่: ");
        Serial.print(distance_mm);
        Serial.println(" mm.");
        //delay(500);
        Drill(); //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< สั่งสว่านเจาะ
        CF = 1;
        MANUAL_MODE = 1;
        STARTGO = 0;
        Distance_OOK = 0;
        Distance = 0;
      }
    }
    /////////////////////////////////////////////////////////// สิ้นสุดการทำงานในขั้นตอนที่ 3 /////////////////////////////////////////////////////
    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< โหมดทำงานแบบ MANUL >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< โหมดทำงานแบบ MANUL >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< โหมดทำงานแบบ MANUL >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< โหมดทำงานแบบ MANUL >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>



    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< โหมดใหม่เพิ่มเติมจากบันทึกภายในล่าสุด >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< โหมดใหม่เพิ่มเติมจากบันทึกภายในล่าสุด >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< โหมดใหม่เพิ่มเติมจากบันทึกภายในล่าสุด >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< โหมดใหม่เพิ่มเติมจากบันทึกภายในล่าสุด >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> เพิ่มเติม <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< โหมดใหม่เพิ่มเติมจากบันทึกภายในล่าสุด >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< โหมดใหม่เพิ่มเติมจากบันทึกภายในล่าสุด >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< โหมดใหม่เพิ่มเติมจากบันทึกภายในล่าสุด >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< โหมดใหม่เพิ่มเติมจากบันทึกภายในล่าสุด >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< โหมดการทำงานทดสอบมอเตอร์ >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< โหมดการทำงานทดสอบมอเตอร์ >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< โหมดการทำงานทดสอบมอเตอร์ >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< โหมดการทำงานทดสอบมอเตอร์ >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< โหมดการทำงานทดสอบมอเตอร์ >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< โหมดการทำงานทดสอบมอเตอร์ >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< โหมดการทำงานทดสอบมอเตอร์ >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< โหมดการทำงานทดสอบมอเตอร์ >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
  } //<<<<<<<<<<<< สั่งให้ระบบหยุดชั่วคราว
  previousButtonStateA = currentButtonStateA;
  previousButtonStateB = currentButtonStateB;
}

//---------Interupt-function-for-encoder--------------- Interupt-ฟังก์ชั่นสำหรับตัวเข้ารหัส
void interruptFunction() {
  unsigned char result = r.getPosition();
  if (result == DIR_NONE) { // do nothing
  }
  //-----------------position-encoder-0--------------------------------------
  else if (result == DIR_CW && encoderPos < 150000 && encoderCounter >= 1 ) {
    encoderPos++;
    dir_upState = 1;
    dir_downState = 0;
  }
  else if (result == DIR_CCW && encoderPos > 1 && encoderCounter >= 1) {
    encoderPos--;
    dir_downState = 1;
    dir_upState = 0;
  }
}

void Drill() { // สั่งให้เจาะส่วาน
  digitalWrite(Relay_drill, 1);
  delay(500);
  digitalWrite(Relay_drill , 0);
  delay(500);
}
