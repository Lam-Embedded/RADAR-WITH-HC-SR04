#include <Arduino.h>
#include <Servo.h>

const uint8_t trig1 = 8;
const uint8_t echo1 = 7;

const uint8_t trig2 = 10;
const uint8_t echo2 = 9;

const uint8_t trig3 = 12;
const uint8_t echo3 = 11;

const uint8_t SERVO_PIN = 6;

const uint8_t BUTTON_START = 5;
const uint8_t BUTTON_STOP = 2;
const uint8_t BUTTON_CHANGE = 4;
const uint8_t speaker = 3;

Servo myservo;
uint8_t angle = 0;
int8_t direction = 1;

uint8_t buttonChangeState = HIGH;
uint8_t lastButtonChangeState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

volatile boolean flag = false;
volatile uint8_t count = 0;

// Ham Do khoang cach
long readDistance(int trigPin, int echoPin);
// Ham hoat dong
void action (long &dist1, long &dist2, long &dist3, uint8_t countStatus);

void setup() {
    Serial.begin(115200);
    pinMode(trig1, OUTPUT);
    pinMode(echo1, INPUT);

    pinMode(trig2, OUTPUT);
    pinMode(echo2, INPUT);

    pinMode(trig3, OUTPUT);
    pinMode(echo3, INPUT);

    pinMode(speaker, OUTPUT);

    myservo.attach(SERVO_PIN);

    pinMode(BUTTON_STOP, INPUT_PULLUP);
    pinMode(BUTTON_START, INPUT_PULLUP);
    pinMode(BUTTON_CHANGE, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(BUTTON_STOP), stopSystem, CHANGE);

    myservo.write(0);
    analogWrite(speaker, 0);
}

void loop() {
    long d1 = 0, d2 = 0, d3 = 0;

    int reading = digitalRead(BUTTON_CHANGE);
    if (digitalRead(BUTTON_START) == LOW) { 
        delay(50);
        if (digitalRead(BUTTON_START) == LOW) {
            flag = true;
            analogWrite(speaker, 100);
            analogWrite(speaker, 0);
        }
    }

    if (reading != lastButtonChangeState) {
        lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > debounceDelay) {
        if (reading != buttonChangeState) {
            buttonChangeState = reading;
            
            if (buttonChangeState == HIGH) {
                count++;
                analogWrite(speaker, 100);
                analogWrite(speaker, 0);
            }
        }
    }
    if (count > 3) count = 0;
    lastButtonChangeState = reading;

    // 3 sensor
    if (flag == true && count == 0) {
        action(d1, d2, d3, count);
    }

    // only sensor 1
    if (count == 1) {
        action(d1, d2, d3, count);
    }

    //only sensor 2
    if (count == 2) {
        action(d1, d2, d3, count);
    }

    // only sensor 3
    if (count == 3) {
        action(d1, d2, d3, count);
    }
    String data = String(angle) + "," + String(d1) + ";" +
                  String(angle) + "," + String(d2) + ";" +
                  String(angle) + "," + String(d3) + ".";
    Serial.println(data);

}

void stopSystem() {
    flag = false;
    count = 0;
}

void action (long &dist1, long &dist2, long &dist3, uint8_t countStatus) {
    angle += direction;
    if (angle >= 179 || angle <= 1) {
        direction = -direction;
    }
    myservo.write(angle);
    delay(30);
    dist1 = readDistance(trig1, echo1);
    dist1 = (dist1 > 30) ? 100 : dist1;
    delay(50);
    dist2 = readDistance(trig2, echo2);
    dist2 = (dist2 > 60) ? 100 : dist2;
    delay(50);
    dist3 = readDistance(trig3, echo3);   
    dist3 = (dist3 > 90) ? 100 : dist3;

    switch (countStatus) {
    case 1:
        dist2 = dist2 * 2;
        dist3 = dist3 * 2;
        break;
    case 2:
        dist1 = dist1 * 2;
        dist3 = dist3 * 2;
        break;
    case 3:
        dist1 = dist1 * 2;
        dist2 = dist2 * 2;
        break;
    default:
        break;
    }
}

long readDistance(int trigPin, int echoPin) {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(5);
    digitalWrite(trigPin, LOW);

    long duration = pulseIn(echoPin, HIGH, 30000);
    long distance = duration / 2 / 29.412;  // cm
    return distance;
}