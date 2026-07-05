#include "mecanum.h"
#include <Arduino.h>
#include <MecanumCar_v2.h>
#include <IRremote.h>
#include <Servo.h>

mecanumCar mecanumCar(3, 2);
Servo myservo;
// =====================
// PINS
// =====================
#define SensorLeft    A0
#define SensorMiddle  A1
#define SensorRight   A2

#define S0 4
#define S1 5
#define S2 7
#define S3 6
#define SENSOR_OUT 8
#define servo_pin 9

int RECV_PIN = A3;

#define TRIG_PIN 12
#define ECHO_PIN 13


#define TURN_BLIND_MS        300
#define TURN_SPEED           60
#define FOLLOW_SPEED         40

// Choose which sensor should detect the new line
// Usually the center sensor
#define STOP_SENSOR SensorMiddle

// =====================
// NEW COUUNTER & TURN VARIABLES
// =====================
bool missionRunning = false;
// USER ADJUSTABLE JUNCTION ALIGNMENT TIMES (CHANGE THESE)

// =====================
// MOTOR SPEED
// =====================
extern uint8_t speed_Upper_L;
extern uint8_t speed_Lower_L;
extern uint8_t speed_Upper_R;
extern uint8_t speed_Lower_R;

// =====================
// Button
// =====================
const unsigned long OK_BUTTON = 0xFF02FD;//BLUE
const unsigned long NO0_BUTTON = 0xFF30CF;//red
const unsigned long NO1_BUTTON = 0xFFB04F;//yellow

IRrecv irrecv(RECV_PIN);
decode_results results;

void setup()
{
    Serial.begin(9600);

    mecanumCar.Init();

    pinMode(SensorLeft, INPUT);
    pinMode(SensorMiddle, INPUT);
    pinMode(SensorRight, INPUT);

    myservo.attach(servo_pin);
    myservo.write(7); // Initialize open

    pinMode(S0, OUTPUT); pinMode(S1, OUTPUT);
    pinMode(S2, OUTPUT); pinMode(S3, OUTPUT);
    pinMode(SENSOR_OUT, INPUT);
    
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    // TCS3200 Frequency scaling = 20%
    digitalWrite(S0, HIGH);
    digitalWrite(S1, LOW);

    irrecv.enableIRIn();

    Serial.println("Ready");
}

void loop()
{
    if (irrecv.decode(&results))
    {
        switch (results.value)
        {
            case OK_BUTTON:
                if (!missionRunning)
                {
                    missionRunning = true;
                    runMissionBlue();
                    missionRunning = false;
                }
                break;

            case NO0_BUTTON:
                if (!missionRunning)
                {
                    missionRunning = true;
                    runMissionRed();
                    missionRunning = false;
                }
                break;
            case NO1_BUTTON:
                if (!missionRunning)
                {
                    missionRunning = true;
                    runMissionYellow();
                    missionRunning = false;
                }
                break;

            default:
                break;
        }

        irrecv.resume();
    }
}
void runMissionBlue()
{
    moveForwardIntersections(5);
    grab("BLUE");
    turn180_RO();
    moveForwardIntersections(7);
    mecanumCar.Stop();
    myservo.write(7);
    moveback(1);
    turn180_R();
}
