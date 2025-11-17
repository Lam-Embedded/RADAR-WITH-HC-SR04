#include <Arduino.h>
#include <Servo.h>

// ------------------------- PIN CONFIG -------------------------
const uint8_t trig1 = 8;
const uint8_t echo1 = 7;

const uint8_t trig2 = 10;
const uint8_t echo2 = 9;

const uint8_t trig3 = 12;
const uint8_t echo3 = 11;

const uint8_t SERVO_PIN = 6;

const uint8_t BUTTON_START  = 5;
const uint8_t BUTTON_STOP   = 2;
const uint8_t BUTTON_CHANGE = 4;
const uint8_t speaker       = 3;

// ------------------------- VARIABLES -------------------------
Servo myservo;

uint8_t angle = 0;
int8_t direction = 1; 
uint8_t stepAngle = 3;   // TỐC ĐỘ QUÉT SERVO
volatile bool running = false;
volatile uint8_t mode = 0;

// Debounce nút CHANGE
uint8_t buttonChangeState = HIGH;
uint8_t lastButtonChangeState = HIGH;
unsigned long lastDebounce = 0;
const unsigned long debounceDelay = 50;

// ------------------------ FUNCTION DECLARE --------------------
long readDistance(int trigPin, int echoPin);
void runRadar();
void stopSystem();

// ===============================================================
//                           SETUP
// ===============================================================
void setup() {
    Serial.begin(115200);

    pinMode(trig1, OUTPUT);
    pinMode(echo1, INPUT);

    pinMode(trig2, OUTPUT);
    pinMode(echo2, INPUT);

    pinMode(trig3, OUTPUT);
    pinMode(echo3, INPUT);

    pinMode(BUTTON_START, INPUT_PULLUP);
    pinMode(BUTTON_STOP, INPUT_PULLUP);
    pinMode(BUTTON_CHANGE, INPUT_PULLUP);

    pinMode(speaker, OUTPUT);

    myservo.attach(SERVO_PIN);
    myservo.write(0);

    attachInterrupt(digitalPinToInterrupt(BUTTON_STOP), stopSystem, FALLING);
}

// ===============================================================
//                              LOOP
// ===============================================================
void loop() {

    // ----------- NÚT START -----------
    if (digitalRead(BUTTON_START) == LOW) {
        delay(20);
        if (digitalRead(BUTTON_START) == LOW) {
            running = true;
            tone(speaker, 3000, 50);
        }
    }

    // ----------- NÚT CHANGE MODE ----------- 
    int reading = digitalRead(BUTTON_CHANGE);

    if (reading != lastButtonChangeState) {
        lastDebounce = millis();
    }

    if ((millis() - lastDebounce) > debounceDelay) {
        if (reading != buttonChangeState) {
            buttonChangeState = reading;

            if (buttonChangeState == HIGH) {
                mode++;
                if (mode > 3) mode = 0;
                tone(speaker, 2000, 50);
            }
        }
    }
    lastButtonChangeState = reading;

    // ----------- CHẠY RADAR -----------
    if (running) {
        runRadar();
    }
}

// ===============================================================
//                           STOP INTERRUPT
// ===============================================================
void stopSystem() {
    running = false;
    mode = 0;
}

// ===============================================================
//                         RADAR ENGINE
// ===============================================================
void runRadar() {
    static unsigned long lastServoMove = 0;
    const unsigned long servoPeriod = 20;  // mỗi 20ms cập nhật góc

    long d1 = 100, d2 = 100, d3 = 100;

    // ---- SERVO MOVE NON-BLOCKING ----
    if (millis() - lastServoMove >= servoPeriod) {
        lastServoMove = millis();

        if (angle >= 180) direction = -1;
        else if (angle == 0) direction = 1;

        angle += stepAngle * direction;
        angle = constrain(angle, 0, 180);
        myservo.write(angle);
    }

    // ---- READ SENSORS ----
    d1 = readDistance(trig1, echo1);
    d2 = readDistance(trig2, echo2);
    d3 = readDistance(trig3, echo3);

    // ---- MODE FILTER ----
    switch (mode) {
        case 1:
            d2 = 100; d3 = 100;
            break;
        case 2:
            d1 = 100; d3 = 100;
            break;
        case 3:
            d1 = 100; d2 = 100;
            break;
        default:
            break;
    }

    // ---- SEND DATA ----
    String data =
        String(angle) + "," + d1 + ";" +
        String(angle) + "," + d2 + ";" +
        String(angle) + "," + d3 + ".";

    Serial.println(data);
}

// ===============================================================
//                    ULTRASONIC - FAST VERSION
// ===============================================================
long readDistance(int trigPin, int echoPin) {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(5);
    digitalWrite(trigPin, LOW);

    long duration = pulseIn(echoPin, HIGH, 15000);  // 15ms timeout nhanh
    if (duration == 0) return 100;                 // không phản hồi

    long dist = duration / 58;   // cm
    if (dist > 200) dist = 100;  // chặn nhiễu
    return dist;
}
