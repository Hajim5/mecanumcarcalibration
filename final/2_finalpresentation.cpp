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
#define TURN_BLIND_MS_LEFT180  460
#define TURN_BLIND_MS_RIGHT180 420
#define TURN_SPEED           60
#define FOLLOW_SPEED         40
#define BRAKE                30
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
const unsigned long OK_BUTTON = 0xFF02FD;//BLUE   #ok
const unsigned long NO0_BUTTON = 0xFF30CF;//red  #4
const unsigned long NO1_BUTTON = 0xFFB04F;//yellow   #3
//const unsigned long RESET_BUTTON = 0xFFB04F;//yellow   #3

IRrecv irrecv(RECV_PIN);
decode_results results;

int redFrequency;
int greenFrequency;
int blueFrequency;
int diffColor  = 200;
int requestedColorState = 0;
//=========================================================
//sequence
//=========================================================
void sequence_OK();
void sequence_0();
void sequence_1();
//=========================================================
//movement
//=========================================================
void forward(int target);
void moveback(int durationMs);
void moveleft(int target);
void moveright(int target);
void turnLeft90(); //Tak calibrate & x include lagi
void turnRight90(); //Tak calibrate & x include lagi
bool grab(String targetColor);
void turnRight180(); //without object
void turnLeft180(); //with object
void dropoff(int target);
void moveleft_O(int target); //KIV
void moveright_O(int target); //KIV
void turnRight90_O(); //KIV
void searchRed();
void searchBlue();
void searchYellow();

// last minute 
bool objectDetected = false;

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
                    sequence_OK();
                    missionRunning = false;
                }
                break;

            case NO0_BUTTON:
                if (!missionRunning)
                {
                    missionRunning = true;
                    sequence_0();
                    missionRunning = false;
                }
                break;
            case NO1_BUTTON:
                if (!missionRunning)
                {
                    missionRunning = true;
                    sequence_1();
                    missionRunning = false;
                }
                break;
            default:
                break;
        }
        irrecv.resume();
    }
}

void sequence_OK()
{
}

void sequence_0()  //block order from left to right: Y,B,R
{
}

void sequence_1() //order from left to right: B,Y,R
{
    forward(4);
    searchRed();
    turnRight180();
    forward(4);
    searchBlue();
    turnRight180();
    forward(4);
    searchYellow();
    turnRight180();
}

void searchRed()
{
    // --------------------------
    // Location 2
    // --------------------------
    forward(1);
    if (grab("RED"))
    {
        turnLeft180();
        forward(2);
        dropoff(5);
        return;
    }
    else
    {
        if (objectDetected)
        {
            // Wrong colour
            moveback(900);
            turnRight180();
            forward(2);
            turnRight180();
        }
        else
        {
            // No object
            turnRight180();
            forward(1);
            turnRight180();
        }
    }
    // --------------------------
    // Location 1
    // --------------------------
    moveright(2);
    forward(1);
    if (grab("RED"))
    {
        turnLeft180();
        moveright(2);
        forward(2);
        dropoff(5);
        return;
    }
    else
    {
        if (objectDetected)
        {
            moveback(900);
            turnRight180();
            moveright(2);
            forward(2);
            turnRight180();
        }
        else
        {
            turnRight180();
            moveright(2);
            forward(1);
            turnRight180();
        }
    }
    // --------------------------
    // Location 3
    // --------------------------
    moveleft(2);
    forward(1);
    if (grab("RED"))
    {
        turnLeft180();
        moveleft(2);
        forward(2);
        dropoff(5);
        return;
    }
    else
    {
        if (objectDetected)
        {
            moveback(900);
            turnRight180();
            moveleft(2);
            forward(2);
            turnRight180();
        }
        else
        {
            turnRight180();
            moveleft(2);
            forward(1);
            turnRight180();
        }
    }
}

void searchBlue()
{
    // --------------------------
    // Location 2
    // --------------------------
    forward(1);
    if (grab("BLUE"))
    {
        turnLeft180();
        forward(2);
        dropoff(5);
        return;
    }
    else
    {
        if (objectDetected)
        {
            moveback(900);
            turnRight180();
            forward(2);
            turnRight180();
        }
        else
        {
            turnRight180();
            forward(1);
            turnRight180();
        }
    }
    // --------------------------
    // Location 1
    // --------------------------
    moveright(2);
    forward(1);
    if (grab("BLUE"))
    {
        turnLeft180();
        moveright(2);
        forward(2);
        dropoff(5);
        return;
    }
    else
    {
        if (objectDetected)
        {
            moveback(900);
            turnRight180();
            moveright(2);
            forward(2);
            turnRight180();
        }
        else
        {
            turnRight180();
            moveright(2);
            forward(1);
            turnRight180();
        }
    }
    // --------------------------
    // Location 3
    // --------------------------
    moveleft(2);
    forward(1);
    if (grab("BLUE"))
    {
        turnLeft180();
        moveleft(2);
        forward(2);
        dropoff(5);
        return;
    }
    else
    {
        if (objectDetected)
        {
            moveback(900);
            turnRight180();
            moveleft(2);
            forward(2);
            turnRight180();
        }
        else
        {
            turnRight180();
            moveleft(2);
            forward(1);
            turnRight180();
        }
    }
}

void searchYellow()
{
    // --------------------------
    // Location 2
    // --------------------------
    forward(1);
    if (grab("YELLOW"))
    {
        turnLeft180();
        forward(2);
        dropoff(5);
        return;
    }
    else
    {
        if (objectDetected)
        {
            moveback(900);
            turnRight180();
            forward(2);
            turnRight180();
        }
        else
        {
            turnRight180();
            forward(1);
            turnRight180();
        }
    }
    // --------------------------
    // Location 1
    // --------------------------
    moveright(2);
    forward(1);
    if (grab("YELLOW"))
    {
        turnLeft180();
        moveright(2);
        forward(2);
        dropoff(5);
        return;
    }
    else
    {
        if (objectDetected)
        {
            moveback(900);
            turnRight180();
            moveright(2);
            forward(2);
            turnRight180();
        }
        else
        {
            turnRight180();
            moveright(2);
            forward(1);
            turnRight180();
        }
    }
    // --------------------------
    // Location 3
    // --------------------------
    moveleft(2);
    forward(1);
    if (grab("YELLOW"))
    {
        turnLeft180();
        moveleft(2);
        forward(2);
        dropoff(5);
        return;
    }
    else
    {
        if (objectDetected)
        {
            moveback(900);
            turnRight180();
            moveleft(2);
            forward(2);
            turnRight180();
        }
        else
        {
            turnRight180();
            moveleft(2);
            forward(1);
            turnRight180();
        }
    }
}

float getDistanceCM() 
{
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    long duration = pulseIn(ECHO_PIN, HIGH, 30000); // 30ms timeout
    if (duration == 0) return 999;                  // Return large number if no echo
    float distance = duration * 0.034f / 2.0f;           // Convert time to cm
    return distance;
}

void forward(int target)
{
    int counter = 0;
    bool onJunction = false;
    speed_Upper_L = 50;
    speed_Lower_L = 50;
    speed_Upper_R = 41;
    speed_Lower_R = 41;
    while(counter < target)
    {
        bool left   = digitalRead(SensorLeft);
        bool middle = digitalRead(SensorMiddle);
        bool right  = digitalRead(SensorRight);
        if(left && middle && right)
        {
            if(!onJunction)
            {
                counter++;
                Serial.print("Intersection: ");
                Serial.println(counter);
                onJunction = true;
            }
            mecanumCar.Advance();
        }
        else
        {
            onJunction = false;
            if(!left && middle && !right)
                mecanumCar.Advance();
            else if(left && !middle && !right)
                mecanumCar.Turn_Left();
            else if(!left && !middle && right)
                mecanumCar.Turn_Right();
            else if(left && middle && !right)
                mecanumCar.Turn_Left();
            else if(!left && middle && right)
                mecanumCar.Turn_Right();
            else if(left && !middle && right)
                mecanumCar.Advance();
            else if(!left && !middle && !right)
                mecanumCar.Advance();
        }
    }
    mecanumCar.Stop();
    delay(1000);
}

void moveback(int durationMs)
{
    const int NORMAL_UL = 40;
    const int NORMAL_LL = 45;
    const int NORMAL_UR = 30;
    const int NORMAL_LR = 35;
    speed_Upper_L = NORMAL_UL;
    speed_Lower_L = NORMAL_LL;
    speed_Upper_R = NORMAL_UR;
    speed_Lower_R = NORMAL_LR;
    unsigned long startTime = millis();
    // Loop until the specified duration (in milliseconds) has passed
    while (millis() - startTime < (unsigned long)durationMs)
    {
        bool left   = digitalRead(SensorLeft);
        bool middle = digitalRead(SensorMiddle);
        bool right  = digitalRead(SensorRight);
        if (!left && middle && !right)
            mecanumCar.Back();
        else if (left && !middle && !right)
            mecanumCar.Turn_Left();
        else if (!left && !middle && right)
            mecanumCar.Turn_Right();
        else if (left && middle && !right)
            mecanumCar.Turn_Left();
        else if (!left && middle && right)
            mecanumCar.Turn_Right();
        else if (left && !middle && right)
            mecanumCar.Back();
        else if (!left && !middle && !right)
            mecanumCar.Back();
        else if (left && middle && right)
            mecanumCar.Back(); // Keep backing up through intersections
    }
    mecanumCar.Stop();
    delay(1000);
}

void moveleft(int target)
{
    int counter = 0;
    bool onMarker = false;
    speed_Upper_L = 47;
    speed_Lower_L = 40;
    speed_Upper_R = 50;
    speed_Lower_R = 50;
    while(counter < target)
    {
        bool left   = digitalRead(SensorLeft);
        bool middle = digitalRead(SensorMiddle);
        bool right  = digitalRead(SensorRight);
        mecanumCar.L_Move();
        // Detect center line crossing
        if(left && !middle && !right)
        {
            if(!onMarker)
            {
                counter++;
                Serial.print("Strafe Left Count: ");
                Serial.println(counter);
                onMarker = true;
                if(counter >= target)
                    break;
            }
        }
        else
        {
            onMarker = false;
        }
    }
    mecanumCar.Stop();
    delay(1000);
}

void moveright(int target)
{
    int counter = 0;
    bool onMarker = false;
    speed_Upper_L = 64;
    speed_Lower_L = 41;
    speed_Upper_R = 65;
    speed_Lower_R = 49;
    while(counter < target)
    {
        bool left   = digitalRead(SensorLeft);
        bool middle = digitalRead(SensorMiddle);
        bool right  = digitalRead(SensorRight);
        mecanumCar.R_Move();
        // Detect center line crossing
        if(!left && !middle && right)
        {
            if(!onMarker)
            {
                counter++;
                onMarker = true;
                if(counter >= target)
                    break;
            }
        }
        else
        {
            onMarker = false;
        }
    }
    mecanumCar.Stop();
    delay(1000);
}

bool grab(String targetColor)
{
    objectDetected = false;
    myservo.write(7);
    delay(400);
    const int NORMAL_UL = 37;
    const int NORMAL_LL = 37;
    const int NORMAL_UR = 28;
    const int NORMAL_LR = 29;
    speed_Upper_L = NORMAL_UL;
    speed_Lower_L = NORMAL_LL;
    speed_Upper_R = NORMAL_UR;
    speed_Lower_R = NORMAL_LR;
    float distance = 999;
    unsigned long startTime = millis();
    int detectCount = 0;
    while (millis() - startTime < 5000)
    {
        distance = getDistanceCM();

        if (distance <= 19)
        {
            detectCount++;

            if (detectCount >= 2)
            {
                objectDetected = true; // Last minute change
                break;      // Stable object detected
            }
        }
        else
        {
            detectCount = 0;    // Reset if one reading fails
        }
        delay(50);
    }
    if (detectCount < 2)
    {
        return false;
    }
    while (true)
    {
        bool left   = digitalRead(SensorLeft);
        bool middle = digitalRead(SensorMiddle);
        bool right  = digitalRead(SensorRight);
        if (!left && middle && !right)
        {
            mecanumCar.Advance();
        }
        else if (left)
        {
            mecanumCar.Turn_Left();
        }
        else if (right)
        {
            mecanumCar.Turn_Right();
        }
        else
        {
            mecanumCar.Advance();
        }

        distance = getDistanceCM();
        if (distance <= 7)
        {
            speed_Upper_L = NORMAL_UL * 0.85;
            speed_Lower_L = NORMAL_LL * 0.85;
            speed_Upper_R = NORMAL_UR * 0.85;
            speed_Lower_R = NORMAL_LR * 0.85;
        }
        else if (distance <= 15)
        {
            speed_Upper_L = NORMAL_UL * 0.9;
            speed_Lower_L = NORMAL_LL * 0.9;
            speed_Upper_R = NORMAL_UR * 0.9;
            speed_Lower_R = NORMAL_LR * 0.9;
        }
        else
        {
            speed_Upper_L = NORMAL_UL;
            speed_Lower_L = NORMAL_LL;
            speed_Upper_R = NORMAL_UR;
            speed_Lower_R = NORMAL_LR;
        }
        if (distance <= 3.8)
        {
            mecanumCar.Stop();
            break;
        }
        delay(30);
    }
    digitalWrite(S2, LOW);
    digitalWrite(S3, LOW);
    redFrequency = pulseIn(SENSOR_OUT, LOW);
    delay(20);
    digitalWrite(S2, HIGH);
    digitalWrite(S3, HIGH);
    greenFrequency = pulseIn(SENSOR_OUT, LOW);
    delay(20);
    digitalWrite(S2, LOW);
    digitalWrite(S3, HIGH);
    blueFrequency = pulseIn(SENSOR_OUT, LOW);
    delay(20);
    String detectColour = "UNKNOWN";
    if (redFrequency > 4000 &&
        greenFrequency > 4000 &&
        blueFrequency > 4000)
    {
        detectColour = "UNKNOWN";
    }
    else if (blueFrequency < redFrequency &&
             blueFrequency < greenFrequency)
    {
        detectColour = "BLUE";
    }
    else if (redFrequency < blueFrequency)
    {
        if (abs(redFrequency - greenFrequency) < 250)
        {
            detectColour = "YELLOW";
        }
        else if (redFrequency < greenFrequency)
        {
            detectColour = "RED";
        }
    }
    if (detectColour == targetColor)
    {
        for (int pos = 7; pos <= 60; pos++)
        {
            myservo.write(pos);
            delay(20);
        }
        return true;
    }
    myservo.write(7);
    delay(1000);
    return false;
}

void turnRight180()
{
    speed_Upper_L = 63;//66
    speed_Lower_L = 63;
    speed_Upper_R = 63;
    speed_Lower_R = 63;
    unsigned long start = millis();
    while (millis() - start < TURN_BLIND_MS_RIGHT180)
    {
        mecanumCar.Turn_Right();
    }
    start = millis();
    while (millis() - start < 150)
    {
        bool left   = digitalRead(SensorLeft);
        bool middle = digitalRead(SensorMiddle);
        bool right  = digitalRead(SensorRight);
        mecanumCar.Turn_Right();
        if (left && !middle && !right)
            break;
    }
    start = millis();
    while (millis() - start < TURN_BLIND_MS_RIGHT180)
    {
        mecanumCar.Turn_Right();
    }
    start = millis();
    while (millis() - start < 150)
    {
        bool left   = digitalRead(SensorLeft);
        bool middle = digitalRead(SensorMiddle);
        bool right  = digitalRead(SensorRight);
        mecanumCar.Turn_Right();
        if (left && !middle && !right)
            break;
    }
    mecanumCar.Turn_Left(); //
    delay(BRAKE);
    mecanumCar.Stop(); 
    delay(100);
    speed_Lower_L = 66;
    speed_Lower_R = 66;
    mecanumCar.drift_left();
    delay(150);   // Tune this for required left shift
    mecanumCar.Stop();
    delay(1000);
}

void turnLeft180()
{
    speed_Upper_L = 67;
    speed_Lower_L = 66;
    speed_Upper_R = 62;
    speed_Lower_R = 64;
    unsigned long start = millis();
    while (millis() - start < TURN_BLIND_MS_LEFT180)
    {
        mecanumCar.Turn_Left();
    }
    start = millis();
    while (millis() - start < 150)
    {
        bool left   = digitalRead(SensorLeft);
        bool middle = digitalRead(SensorMiddle);
        bool right  = digitalRead(SensorRight);
        mecanumCar.Turn_Left();
        if (left && !middle && !right)
            break;
    }
    start = millis();
    while (millis() - start < TURN_BLIND_MS_LEFT180)
    {
        mecanumCar.Turn_Left();
    }
    start = millis();
    while (millis() - start < 150)
    {
        bool left   = digitalRead(SensorLeft);
        bool middle = digitalRead(SensorMiddle);
        bool right  = digitalRead(SensorRight);
        mecanumCar.Turn_Left();
        if (left && !middle && !right)
            break;
    }
    mecanumCar.Turn_Right();
    delay(BRAKE);
    mecanumCar.Stop();
    delay(100);
    speed_Lower_L = 66;
    speed_Lower_R = 66;
    mecanumCar.drift_right();
    delay(350);   // Tune this for required left shift
    mecanumCar.Stop();
    delay(1000);
}

void dropoff(int target)
{
    int counter = 0;
    bool onJunction = false;
    const int NORMAL_UL = 50;
    const int NORMAL_LL = 50;
    const int NORMAL_UR = 41;
    const int NORMAL_LR = 41;
    speed_Upper_L = NORMAL_UL;
    speed_Lower_L = NORMAL_LL;
    speed_Upper_R = NORMAL_UR;
    speed_Lower_R = NORMAL_LR;
    while(counter < target)
    {
        bool left   = digitalRead(SensorLeft);
        bool middle = digitalRead(SensorMiddle);
        bool right  = digitalRead(SensorRight);
        if(left && middle && right)
        {
            if(!onJunction)
            {
                counter++;
                speed_Upper_L = round(speed_Upper_L * 0.90);
                speed_Lower_L = round(speed_Lower_L * 0.90);
                speed_Upper_R = round(speed_Upper_R * 0.90);
                speed_Lower_R = round(speed_Lower_R * 0.90);
                onJunction = true;
            }
            mecanumCar.Advance();
        }
        else
        {
            onJunction = false;
            if(!left && middle && !right)
                mecanumCar.Advance();
            else if(left && !middle && !right)
                mecanumCar.Turn_Left();
            else if(!left && !middle && right)
                mecanumCar.Turn_Right();
            else if(left && middle && !right)
                mecanumCar.Turn_Left();
            else if(!left && middle && right)
                mecanumCar.Turn_Right();
            else if(left && !middle && right)
                mecanumCar.Advance();
            else if(!left && !middle && !right)
                mecanumCar.Advance();
        }
    }
    mecanumCar.Stop();
    delay(300);
    for(int pos = 80; pos >= 7; pos--)
    {
        myservo.write(pos);
        delay(20);
    }
    moveback(600);
    delay(12000); // Depend on Nazri
}
