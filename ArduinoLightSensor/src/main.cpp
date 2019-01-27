#include <Arduino.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SoftwareSerial.h>

typedef struct L {
  uint8_t analogPort, digitalPort;
  float calibrated;
  bool isCalibrated;
} Led;

float checkLight(Led);
void driveMotor();
void startDemo();
void stopDemo();
void calibrate();
void randomMotorDrive(uint32_t duration);
void randomMotorStop();

// timing
uint32_t delayBeforeMotor = 8000;
uint32_t delayAfterMotor = 4000;

uint32_t randomMotorDuration = 0;
uint32_t randomMotorStart = 0;
bool randomMotor = false;

uint8_t currentStep = 0;
uint64_t currentStepMillis = 0;

// BT
SoftwareSerial BTSerial (2, 3); // RX, TX

// LED
Led led1 = {A0, 8, 0, false};
Led led2 = {A1, 9, 0, false};
Led led3 = {A2, 10, 0, false};
Led led4 = {A3, 11, 0, false};
Led led5 = {A4, 12, 0, false};
Led led6 = {A5, 13, 0, false};

Led leds[] = { led1, led2, led3, led4, led5, led6 };
Led led;

// LED PINS
uint8_t analog[] = { A0, A1, A2, A3, A4, A5 };
uint8_t digital[] = { 8, 9, 10, 11, 12, 13 };

// MOTOR
bool motor = false;
uint8_t motorPin = 7;
uint32_t lastMotorStart = 0;
uint16_t motorDuration = 750;
uint16_t motorDelay = 10000;

// SCORE
bool win = false;
bool reset = true;
uint64_t lastWin = 0;
uint32_t score = 0;

// LIGHT SENSORS CALIBRATION
bool calibrated = false;
bool calibrating = false;
uint64_t calDelay = 60000;
uint64_t lastCal = 0;

// DEMO MODE
bool demo = false;
uint32_t demoDelay = __UINT32_MAX__;
uint32_t demoStart = 0;
uint32_t lastCoin = 0;
Led demoLeds[3];

time_t t;

void setup() {
  Serial.begin(9600);
  BTSerial.begin(9600);

  BTSerial.println("Setup");

  // set all the LED digital pins to output, low
  for (uint8_t i = 0; i < sizeof(digital)/sizeof(digital[0]); i++) {
    pinMode(digital[i], OUTPUT);
    digitalWrite(digital[i], LOW);
  }

  // set all the light sensors analog pins to input
  for (uint8_t i = 0; i < sizeof(analog)/sizeof(analog[0]); i++) {
    pinMode(analog[i], INPUT);
  }

  // set motor pin to output, low
  pinMode(motorPin, OUTPUT);
  digitalWrite(motorPin, LOW);

  // init random
  srand((unsigned) time(&t));
}

void loop() {
  // read BT serial
  if(BTSerial.available() > 0) {
    char data = (char) BTSerial.read();

    Serial.print("RECEIVED ");
    Serial.println(data);

    switch(data) {
      case 'l' : // calibration
        calibrate();
      break;

      case 's' : // shake motors
        randomMotorDrive(750);
      break;

      case 'd' : // debug logs
        // TBI
      break;

      case 'n' : // next color
        reset = true;
      break;

      case 'r' :
        reset = true;
        score = 0;
      break;
    }
  }

  // use motor
  driveMotor();

  if (randomMotor && millis() - randomMotorStart > randomMotorDuration) {
    randomMotorStop();
  }

  // demo only code
  if (millis() - lastCoin > demoDelay) {
      if (!demo) {
        startDemo();
        Serial.println("DEMO MODE IS STARTING !");
      }

      if (millis() - demoStart > demoDelay) {
        // change leds
        demoStart = millis();
      }
  }

  // calibration
  if ((calibrating || !calibrated || ( demo && millis() - lastCal > calDelay)))
  {
    calibrate();
  }

  else {
    // if player won, restart the game by choosing a new led
    if (win) {
      // add 10 to the score when a led is hit
      score += 10;
      lastWin = millis();

      // display new score on serial
      Serial.print("SCORE : ");
      Serial.println(score);

      BTSerial.print("S:");
      BTSerial.println(score);

      // reset the win flag
      win = false;
      reset = true;
    }

    // choose a new led
    if (reset) {
      // choose random led, except previous one
      uint8_t oldLedAnalog = led.analogPort;

      while(oldLedAnalog == led.analogPort) {
        led = leds[(rand())%sizeof(leds)/sizeof(leds[0])];
      }

      Serial.print("NEW LED : ");
      Serial.println(led.analogPort);

      // power off all the leds
      for (uint8_t i = 0; i < sizeof(digital)/sizeof(digital[0]); i++) {
        digitalWrite(digital[i], LOW);
      }

      // power on the newly chosen led
      digitalWrite(led.digitalPort, HIGH);

      reset = false;
      currentStep = 1;
    }

    for (uint8_t i = 0; i < sizeof(digital)/sizeof(digital[0]); i++) {
      digitalWrite(digital[i], LOW);
    }

    digitalWrite(led.digitalPort, HIGH);

    // led off
    for (uint8_t i = 0; i < sizeof(digital)/sizeof(digital[0]); i++) {
      digitalWrite(digital[i], LOW);
    }

    float sensor = checkLight(led);

    /*Serial.print(sensor);
    Serial.print(" - ");
    Serial.println(led.digitalPort);*/

    // check if something blocks the ambiant light
    if (sensor < (led.calibrated * 0.75)) {
      digitalWrite(led.digitalPort, LOW);
      delay(1000);
      Serial.println(sensor);
      Serial.println("YOU WON : ");
      win = true;
    }
  }
}

float checkLight(Led checkLed) {
    // prepare sensor reading
    digitalWrite(checkLed.digitalPort, HIGH);
    pinMode(checkLed.analogPort, INPUT);
    // read sensor
    int sensorValue = analogRead(checkLed.analogPort);
    float voltage = sensorValue * (5.0 / 1023.0);

    return voltage;
}

void driveMotor() {
  if (!demo && !motor && millis() - lastMotorStart - motorDuration > motorDelay) {
    Serial.println("MOTOR START");
    BTSerial.print("M:");
    BTSerial.println(motorDelay + motorDuration);
    digitalWrite(motorPin, HIGH);
    lastMotorStart = millis();
    motor = true;

  }

  else if (motor && millis() - lastMotorStart > motorDuration) {
    Serial.println("MOTOR STOPPED");
    digitalWrite(motorPin, LOW);
    motor = false;
  }
}

void randomMotorDrive(uint32_t duration) {
  digitalWrite(motorPin, HIGH);
  randomMotorDuration = duration;
  randomMotorStart = millis();
  randomMotor = true;
  Serial.println("MOTOR RANDOM STARTED");
}

void randomMotorStop() {
  digitalWrite(motorPin, LOW);
  randomMotor = false;
  Serial.println("MOTOR RANDOM STOPPED");
}

void startDemo() {
  demoStart = millis();
  demo = true;
}

void stopDemo() {
  lastCoin = millis();
  demo = false;
}

void calibrate() {
  calibrated = false;

  if (!calibrating) {
    for (uint8_t i = 0; i < sizeof(leds)/sizeof(leds[0]); i++) {
      float value = checkLight(leds[i]);
      leds[i].isCalibrated = false;
      leds[i].calibrated = checkLight(leds[i]);
      Serial.print(i);
      Serial.print(". ");
      Serial.println(value);
    }
  }

  else {
    for (uint8_t i = 0; i < sizeof(leds)/sizeof(leds[0]); i++) {
      if (!leds[i].isCalibrated) {
        float value = checkLight(leds[i]);

        if(fabs(leds[i].calibrated - value) > 0.3f) {
          leds[i].isCalibrated = false;
          leds[i].calibrated = value;
        }

        else {
          leds[i].calibrated = (leds[i].calibrated + value) / 2;
          leds[i].isCalibrated = true;
        }

        Serial.print(i);
        Serial.print(". ");
        Serial.println(value);
      }
    }
  }
  
  Serial.println("==");

  if (calibrating) {
    calibrated = true;
    for (uint8_t i = 0; i < sizeof(leds)/sizeof(leds[0]); i++) {
      Serial.print(i);
      Serial.print("/ ");
      Serial.println(leds[i].calibrated);

      if (leds[i].isCalibrated == false) {
        calibrated = false;
      }
    }

    if (calibrated) {
      calibrated = true;
      calibrating = false;

      lastCal = millis();
      Serial.println("END OF CALIBRATION");
    }
  }

  else {
    calibrating = true;
  }
}

void gameLoop() {
  
  // reset (state 0)
  if (currentStep == 0) {
    reset = true;
    currentStepMillis = millis();
  }

  // wait 6 seconds (state 1)
  if (currentStep == 1 && millis() - currentStepMillis > delayBeforeMotor) {
    currentStep++;
  }

  // shake (state 2)
  if (currentStep == 2) {
    motor = true;
  }
}