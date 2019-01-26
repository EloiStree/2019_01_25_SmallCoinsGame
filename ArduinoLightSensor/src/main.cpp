#include <Arduino.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct L {
  int analogPort, digitalPort;
} Led;

Led led1 = {A0, 8};
Led led2 = {A1, 8};
Led led3 = {A0, 9};

Led leds[] = { led1, led2, led3 };

int analog[] = { A0, A1 };
int digital[] = { 8, 9 };

bool win = true;
Led led;

time_t t;

void setup() {
  //Serial.begin(9600);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  
  srand((unsigned) time(&t));
}

void loop() {
  if (win) {
    // launch new coin
    // choose random led
 
		led = leds[(rand())%3];

    win = false;
  }

  if (!win) {
    pinMode(A0, OUTPUT);
    pinMode(A1, OUTPUT);
    analogWrite(A0, 0);
    analogWrite(A1, 0);
    digitalWrite(8, HIGH);
    digitalWrite(9, HIGH);

    // led on
    pinMode(led.analogPort, OUTPUT);
    analogWrite(led.analogPort, 255);
    digitalWrite(led.digitalPort, LOW);
    delay(1);

    digitalWrite(8, LOW);
    digitalWrite(9, LOW);

    digitalWrite(led.digitalPort, HIGH);
    // led off
  }
    
  
  pinMode(led.analogPort, INPUT);

  int sensor = analogRead(led.analogPort);

  if (sensor < 300) {
    Serial.println("WIN !");
    win = true;
  }

  Serial.print(led.analogPort);
  Serial.print(" - ");
  Serial.print(led.digitalPort);
  Serial.print(" : ");
  Serial.println(sensor);
}