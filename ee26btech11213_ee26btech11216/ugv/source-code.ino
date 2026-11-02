#define CUSTOM_SETTINGS
#define INCLUDE_GAMEPAD_MODULE
#include <DabbleESP32.h>

// Brownout reset suppression registers
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// ================= YOUR PIN ASSIGNMENTS =================
// Right Motor (L293D Channel 1 & 2)
const int RIGHT_MOTOR_ENA = 33; // L293D Pin 1 (PWM Speed)
const int RIGHT_MOTOR_IN1 = 13; // L293D Pin 2 (Direction 1)
const int RIGHT_MOTOR_IN2 = 27; // L293D Pin 7 (Direction 2) -- Change to 14 if wired to GPIO 14

// Left Motor (L293D Channel 3 & 4)
const int LEFT_MOTOR_ENB  = 25; // L293D Pin 9 (PWM Speed)
const int LEFT_MOTOR_IN1  = 26; // L293D Pin 10 (Direction 1)
const int LEFT_MOTOR_IN2  = 32; // L293D Pin 15 (Direction 2)

// ================= PWM SETTINGS ====================
const int PWM_FREQ     = 1000; // 1 kHz PWM frequency
const int PWM_RES      = 8;    // 8-bit resolution (0 - 255)
const int RIGHT_PWM_CH = 0;
const int LEFT_PWM_CH  = 1;

// ================= SPEED PROFILES ==================
const int SPEED_SLOW   = 130;  // Square
const int SPEED_NORMAL = 180;  // Cross (Default Boot Speed)
const int SPEED_FAST   = 220;  // Triangle
const int SPEED_MAX    = 255;  // Circle (100% Uncapped DC)

int currentSpeed = SPEED_NORMAL;
const float leftMotorTrim = 0.95; // Drift correction factor

// ================= FORWARD DECLARATIONS ============
void setMotorPWM(int leftPwm, int rightPwm);
void moveForward();
void moveBackward();
void spinLeft();
void spinRight();
void stopMotors();
void handleSpeedMultipliers();
void handleMovement();

// ================= SETUP ===========================
void setup() {
  // Disable software brownout detector so transient motor noise doesn't reset CPU
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  Serial.begin(115200);

  // Initialize Motor Direction Pins
  pinMode(RIGHT_MOTOR_IN1, OUTPUT);
  pinMode(RIGHT_MOTOR_IN2, OUTPUT);
  pinMode(LEFT_MOTOR_IN1, OUTPUT);
  pinMode(LEFT_MOTOR_IN2, OUTPUT);

  // Configure PWM Channels (compatible with ESP32 Arduino Core 2.x and 3.x)
  #if ESP_ARDUINO_VERSION_MAJOR >= 3
    ledcAttach(RIGHT_MOTOR_ENA, PWM_FREQ, PWM_RES);
    ledcAttach(LEFT_MOTOR_ENB, PWM_FREQ, PWM_RES);
  #else
    ledcSetup(RIGHT_PWM_CH, PWM_FREQ, PWM_RES);
    ledcSetup(LEFT_PWM_CH, PWM_FREQ, PWM_RES);
    ledcAttachPin(RIGHT_MOTOR_ENA, RIGHT_PWM_CH);
    ledcAttachPin(LEFT_MOTOR_ENB, LEFT_PWM_CH);
  #endif

  stopMotors();

  // Initialize Dabble Bluetooth
  Dabble.begin("UGV-ESP32");
  Serial.println("UGV initialized with verified pins. Ready for Dabble.");
}

// ================= LOOP ============================
void loop() {
  Dabble.processInput();

  handleSpeedMultipliers();
  handleMovement();
}

// ================= MOTOR DRIVE FUNCTIONS ===========

void setMotorPWM(int leftPwm, int rightPwm) {
  leftPwm  = constrain(leftPwm, 0, 255);
  rightPwm = constrain(rightPwm, 0, 255);

  #if ESP_ARDUINO_VERSION_MAJOR >= 3
    ledcWrite(RIGHT_MOTOR_ENA, rightPwm);
    ledcWrite(LEFT_MOTOR_ENB, leftPwm);
  #else
    ledcWrite(RIGHT_PWM_CH, rightPwm);
    ledcWrite(LEFT_PWM_CH, leftPwm);
  #endif
}

void moveForward() {
  int rSpeed = currentSpeed;
  int lSpeed = (int)(currentSpeed * leftMotorTrim);

  // Right motor forward
  digitalWrite(RIGHT_MOTOR_IN1, HIGH);
  digitalWrite(RIGHT_MOTOR_IN2, LOW);

  // Left motor forward
  digitalWrite(LEFT_MOTOR_IN1, HIGH);
  digitalWrite(LEFT_MOTOR_IN2, LOW);

  setMotorPWM(lSpeed, rSpeed);
}

void moveBackward() {
  int rSpeed = currentSpeed;
  int lSpeed = (int)(currentSpeed * leftMotorTrim);

  // Right motor backward
  digitalWrite(RIGHT_MOTOR_IN1, LOW);
  digitalWrite(RIGHT_MOTOR_IN2, HIGH);

  // Left motor backward
  digitalWrite(LEFT_MOTOR_IN1, LOW);
  digitalWrite(LEFT_MOTOR_IN2, HIGH);

  setMotorPWM(lSpeed, rSpeed);
}

void spinLeft() {
  // Pivot counter-clockwise (Right forward, Left backward)
  digitalWrite(RIGHT_MOTOR_IN1, HIGH);
  digitalWrite(RIGHT_MOTOR_IN2, LOW);
  digitalWrite(LEFT_MOTOR_IN1, LOW);
  digitalWrite(LEFT_MOTOR_IN2, HIGH);

  setMotorPWM(currentSpeed, currentSpeed);
}

void spinRight() {
  // Pivot clockwise (Right backward, Left forward)
  digitalWrite(RIGHT_MOTOR_IN1, LOW);
  digitalWrite(RIGHT_MOTOR_IN2, HIGH);
  digitalWrite(LEFT_MOTOR_IN1, HIGH);
  digitalWrite(LEFT_MOTOR_IN2, LOW);

  setMotorPWM(currentSpeed, currentSpeed);
}

void stopMotors() {
  digitalWrite(RIGHT_MOTOR_IN1, LOW);
  digitalWrite(RIGHT_MOTOR_IN2, LOW);
  digitalWrite(LEFT_MOTOR_IN1, LOW);
  digitalWrite(LEFT_MOTOR_IN2, LOW);

  setMotorPWM(0, 0);
}

// ================= DABBLE INPUT HANDLERS ===========

void handleSpeedMultipliers() {
  if (GamePad.isSquarePressed()) {
    currentSpeed = SPEED_SLOW;
    Serial.println("Speed: SLOW (130)");
  } 
  else if (GamePad.isCrossPressed()) {
    currentSpeed = SPEED_NORMAL;
    Serial.println("Speed: NORMAL (180)");
  } 
  else if (GamePad.isTrianglePressed()) {
    currentSpeed = SPEED_FAST;
    Serial.println("Speed: FAST (220)");
  } 
  else if (GamePad.isCirclePressed()) {
    currentSpeed = SPEED_MAX;
    Serial.println("Speed: MAX (255)");
  }
}

void handleMovement() {
  if (GamePad.isUpPressed()) {
    moveForward();
  } 
  else if (GamePad.isDownPressed()) {
    moveBackward();
  } 
  else if (GamePad.isLeftPressed()) {
    spinLeft();
  } 
  else if (GamePad.isRightPressed()) {
    spinRight();
  } 
  else {
    stopMotors();
  }
}