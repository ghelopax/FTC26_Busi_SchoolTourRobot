/*
 /\_/\  /\_/\  /\_/\  /\_/\        SchoolTour_ArduinoUno.ino:
( o.o )( o.o )( o.o )( o.o )       |___CONFIGURATION: Constants and channels
 > ^ <  > ^ <  > ^ <  > ^ <        |___PWM DRIVER: Adafruit PWM Servo Driver
#######              #######       |___HARDWARE API: dc_control()
 /\_/\    ghelopax    /\_/\        |___SUBSYSTEMS: Drivetrain
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

/* PWM channels */
// DC Motor
#define MOTOR_DRIVE_LEFT_A  3
#define MOTOR_DRIVE_LEFT_B  5
#define MOTOR_DRIVE_RIGHT_A 6
#define MOTOR_DRIVE_RIGHT_B 9

/* PS2 pins */
#define PS2_DAT             12
#define PS2_CMD             11
#define PS2_ATT             10
#define PS2_CLK             13


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
        Serial.println("\nError code 1: No controller found, check wiring.");
        break;
      case 2:
        Serial.println("\nError code 2: Controller found but not accepting commands.");
        break;
      case 3:
        Serial.println("\nError code 3: Controller refusing to enter Pressures mode, may not support it. ");
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


// ##########
// SUBSYSTEMS
// ##########

/* DRIVETRAIN */
void drivetrain_update() {
  int16_t speed_l = map(ps2.Analog(PSS_LY), 0, 255, SPD_DRIVE, -SPD_DRIVE);
  dc_control(MOTOR_DRIVE_LEFT_A, MOTOR_DRIVE_LEFT_B, speed_l);

  int16_t speed_r = map(ps2.Analog(PSS_RY), 0, 255, -SPD_DRIVE, SPD_DRIVE);
  dc_control(MOTOR_DRIVE_RIGHT_A, MOTOR_DRIVE_RIGHT_B, speed_r);
}


// #################
// ARDUINO FUNCTIONS
// #################

void setup() {
  Serial.begin(115200);  // baud rate (bps)

  // Serial.println("START");

  PWMDriver_init();
  PS2_init();
}

void loop() {
  ps2.read_gamepad();  // update from controller

  drivetrain_update();

  // Serial.print("PSS_LY: ");
  // Serial.print(ps2.Analog(PSS_LY));
  // Serial.print(", PSS_RY: ");
  // Serial.println(ps2.Analog(PSS_RY));
}
