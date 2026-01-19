/*
 /\_/\  /\_/\  /\_/\  /\_/\        School_Tour_FTC.ino:
( o.o )( o.o )( o.o )( o.o )       |___CONFIGURATION: Constants and channels
 > ^ <  > ^ <  > ^ <  > ^ <        |___PWM DRIVER: Adafruit PWM Servo Driver
#######              #######       |___HARDWARE API: dc_control(), servo_control(), CRservo_control()
 /\_/\    ghelopax    /\_/\        |___SUBSYSTEMS: Drivetrain, Arm, Claw
( o.o )              ( o.o )       |___ARDUINO FUNCTIONS: setup(), loop()
 > ^ <   @itsmevjnk   > ^ <
#######              #######
 /\_/\  /\_/\  /\_/\  /\_/\
( o.o )( o.o )( o.o )( o.o )
 > ^ <  > ^ <  > ^ <  > ^ <
*/

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <PS2X_lib.h>

// #############
// CONFIGURATION
// #############

/* CONSTANTS */
// Drive motor speed
#define SPD_DRIVE           3072
#define SPD_DEAD            80
// Servo PW
#define PW_MIN              440
#define PW_MAX              2270
#define CR_PW_MIN           400
#define CR_PW_MID           1400

/* PWM channels */
// DC Motor
#define MOTOR_DRIVE_LEFT_A  8 
#define MOTOR_DRIVE_LEFT_B  9
#define MOTOR_DRIVE_RIGHT_A 14
#define MOTOR_DRIVE_RIGHT_B 15
// Servo
#define SERVO_ARM_X         2
#define SERVO_ARM_Y         3
#define SERVO_CLAW          4

/* PS2 pins */
#define PS2_DAT             12
#define PS2_CMD             13
#define PS2_ATT             15
#define PS2_CLK             14


// ##########
// PWM DRIVER
// ##########

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

void PWMDriver_init() {
  Serial.print(F("Initializing PWM controller..."));

  // PWM Init
  pwm.begin();
  pwm.setOscillatorFrequency(27000000);
  pwm.setPWMFreq(50);
  // Wire Init
  Wire.setClock(400000);

  Serial.println(F("done."));
}


// ############
// HARDWARE API
// ############

/* PS2 Controller */
PS2X ps2;

void PS2_init() {
  Serial.print(F("Initializing PS2 controller..."));

  uint8_t error = ps2.config_gamepad(PS2_CLK, PS2_CMD, PS2_ATT, PS2_DAT);
  while (error != 0) {
    switch (error) {
      case 1:
        Serial.println("Error code 1: No controller found, check wiring.");
        break;
      case 2:
        Serial.println("Error code 2: Controller found but not accepting commands.");
        break;
      case 3:
        Serial.println("Error code 3: Controller refusing to enter Pressures mode, may not support it. ");
        break;
    }

    error = ps2.config_gamepad(PS2_CLK, PS2_CMD, PS2_ATT, PS2_DAT);
  }

  Serial.println(F("done."));
}

/* Control */
void dc_control(uint8_t channelA, uint8_t channelB, int16_t speed) {
  pwm.setPWM(channelA, 0, ((speed > 0) ?   speed  : 0));
  pwm.setPWM(channelB, 0, ((speed < 0) ? (-speed) : 0));
}

void servo_control(uint8_t channel, float value) {
  pwm.writeMicroseconds(channel, PW_MIN + value * (PW_MAX - PW_MIN));
}

void CRservo_control(uint8_t channel, float value) {
  pwm.writeMicroseconds(channel, CR_PW_MID + value * (CR_PW_MID - CR_PW_MIN));
}


// ##########
// SUBSYSTEMS
// ##########

void pos_update(float &pos, float value, float _min, float _max) {
  if (value < 0) {
    if (pos - value >= _min) pos -= value;
  }
  
  if (value > 0) {
    if (pos + value <= _max) pos += value;
  }
}

/* DRIVETRAIN */
void drivetrain_update() {
  int16_t speed_l = map(ps2.Analog(PSS_LY), 0, 255, SPD_DRIVE, -SPD_DRIVE);
  dc_control(MOTOR_DRIVE_LEFT_A, MOTOR_DRIVE_LEFT_B, speed_l);

  int16_t speed_r = map(ps2.Analog(PSS_RY), 0, 255, -SPD_DRIVE, SPD_DRIVE);
  dc_control(MOTOR_DRIVE_RIGHT_A, MOTOR_DRIVE_RIGHT_B, speed_r);
}

/* ARM */
void arm_update() {
  if (ps2.Button(PSB_TRIANGLE)) CRservo_control(SERVO_ARM_X,  0.5);
  if (ps2.Button(PSB_CROSS))    CRservo_control(SERVO_ARM_X, -0.5);
  if (ps2.Button(PSB_CIRCLE))   CRservo_control(SERVO_ARM_Y,  0.5);
  if (ps2.Button(PSB_SQUARE))   CRservo_control(SERVO_ARM_Y, -0.5);
}

/* CLAW */
void claw_update() {
  if (ps2.Button(PSB_PAD_UP))   CRservo_control(SERVO_CLAW,  0.5);
  if (ps2.Button(PSB_PAD_DOWN)) CRservo_control(SERVO_CLAW, -0.5);
}


// #################
// ARDUINO FUNCTIONS
// #################

void setup() {
  Serial.begin(115200);  // ESP32 baud rate (bps)

  PWMDriver_init();
  PS2_init();

  CRservo_control(SERVO_ARM_X, 0.0);
  CRservo_control(SERVO_ARM_Y, 0.0);
  CRservo_control(SERVO_CLAW , 0.0);
}

void loop() {
  ps2.read_gamepad();  // update from controller

  drivetrain_update();
  
  // ARM_X = SQUARE, ARM_Y = CROSS  
  arm_update();
  // CLAW = TRIANGLE
  claw_update();
}
