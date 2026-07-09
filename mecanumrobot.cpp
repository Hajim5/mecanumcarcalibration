#include <Arduino.h>
#include <MecanumCar_v2.h>
#include <IRremote.h>
#include <Servo.h>
#include <SoftwareSerial.h>

SoftwareSerial espSerial(11, 10);

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

void runMissionBlue();
void runMissionRed();
void runMissionYellow();
void forward(int target);
void moveleft(int target);
void moveleft_O(int target);
void moveright(int target);
void moveright_O(int target);
void turnRight90();
void turnRight90_O();
void moveback(int target);
void turnLeft90();
bool grab(String targetColor);
void forward2(int target);

void setup()
{
    espSerial.begin(9600);

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
    if (espSerial.available())
    {
        String command = espSerial.readStringUntil('\n');
        command.trim();

        if (command == "START" && !missionRunning)
        {
            missionRunning = true;
            runMissionBlue();
            missionRunning = false;
        }
    }
}

void runMissionBlue()
{
}

void runMissionRed()
{
}

void runMissionYellow()
{
}

long getDistanceCM() {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    long duration = pulseIn(ECHO_PIN, HIGH, 30000); // 30ms timeout
    if (duration == 0) return 999;                  // Return large number if no echo
    long distance = duration * 0.034 / 2;           // Convert time to cm
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
    delay(400);
}

void moveleft(int target)
{
    int counter = 0;
    bool onMarker = false;
    //speed_Upper_L = 57;
    //speed_Lower_L = 50;
    //speed_Upper_R = 60;
    //speed_Lower_R = 60;
/*     speed_Upper_L = 47;
    speed_Lower_L = 40;
    speed_Upper_R = 50;
    speed_Lower_R = 50; */
    speed_Upper_L = 59; //57
    speed_Lower_L = 52;//50
    speed_Upper_R = 60;
    speed_Lower_R = 60;

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
    delay(400);
}

void turnRight90()
{
    //dulu 60
    speed_Upper_L = 66;
    speed_Lower_L = 66;
    speed_Upper_R = 66;
    speed_Lower_R = 66;

    unsigned long start = millis();

    while (millis() - start < TURN_BLIND_MS)
        mecanumCar.Turn_Right();

    start = millis();
    //150 dulu
    while (millis() - start < 150)
    {
        bool left   = digitalRead(SensorLeft);
        bool middle = digitalRead(SensorMiddle);
        bool right  = digitalRead(SensorRight);

        mecanumCar.Turn_Right();

        if (!left && !middle && right)
            break;
    }

    mecanumCar.Stop();
    delay(400);
}

void moveright(int target)
{
    int counter = 0;
    bool onMarker = false;

    speed_Upper_L = 64;
    speed_Lower_L = 52;//52
    speed_Upper_R = 64;
    speed_Lower_R = 46;

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
    delay(400);
}

void turnRight90_O()
{
//dulu 60
    speed_Upper_L = 66;
    speed_Lower_L = 66;
    speed_Upper_R = 66;
    speed_Lower_R = 66;

    unsigned long start = millis();

    while (millis() - start < TURN_BLIND_MS)
        mecanumCar.Turn_Right();

    start = millis();
    //150 dulu
    while (millis() - start < 200)
    {
        bool left   = digitalRead(SensorLeft);
        bool middle = digitalRead(SensorMiddle);
        bool right  = digitalRead(SensorRight);

        mecanumCar.Turn_Right();

        if (!left && !middle && right)
            break;
    }

    mecanumCar.Stop();
    delay(400);
}

void moveleft_O(int target)
{
    int counter = 0;
    bool onMarker = false;
    speed_Upper_L = 52;
    speed_Lower_L = 45;
    speed_Upper_R = 55;
    speed_Lower_R = 55;

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
    delay(400);
}

void moveback(int target)
{
    int counter = 0;
    bool onJunction = false;
    speed_Upper_L = 39; //38
    speed_Lower_L = 44; //43
    speed_Upper_R = 30;
    speed_Lower_R = 35;
    
    // Record the exact time the movement started
    unsigned long startTime = millis();
    
    // The loop will exit if it hits the target junctions OR if 200 milliseconds (0.2s) pass
    while((counter < target) && (millis() - startTime < 1200))
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

            mecanumCar.Back();
        }
        else
        {
            onJunction = false;

            if(!left && middle && !right)
                mecanumCar.Back();

            else if(left && !middle && !right)
                mecanumCar.Turn_Left();

            else if(!left && !middle && right)
                mecanumCar.Turn_Right();

            else if(left && middle && !right)
                mecanumCar.Turn_Left();

            else if(!left && middle && right)
                mecanumCar.Turn_Right();
                
            else if(left && !middle && right)
                mecanumCar.Back();
                
            else if(!left && !middle && !right)
                mecanumCar.Back();
        }
    }

    mecanumCar.Stop();
    delay(400);
}

void turnLeft90()
{
    speed_Upper_L = 67;
    speed_Lower_L = 67;
    speed_Upper_R = 67;
    speed_Lower_R = 67;

    unsigned long start = millis();

    while (millis() - start < TURN_BLIND_MS)
        mecanumCar.Turn_Left();

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

    mecanumCar.Stop();
    delay(400);
}

int readColor(bool s2, bool s3)
{
    digitalWrite(S2, s2);
    digitalWrite(S3, s3);

    delay(20);

    return pulseIn(SENSOR_OUT, LOW, 30000);
}

bool grab(String targetColor)

{
    // Open gripper
    myservo.write(7);
    delay(400);

    speed_Upper_L = 37;
    speed_Lower_L = 37;
    speed_Upper_R = 28;
    speed_Lower_R = 29;

    while (true)
    {
        bool left   = digitalRead(SensorLeft);
        bool middle = digitalRead(SensorMiddle);
        bool right  = digitalRead(SensorRight);

        // -----------------------------
        // LINE FOLLOWING
        // -----------------------------
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

        // -----------------------------
        // ULTRASONIC
        // -----------------------------
        digitalWrite(TRIG_PIN, LOW);
        delayMicroseconds(2);

        digitalWrite(TRIG_PIN, HIGH);
        delayMicroseconds(10);

        digitalWrite(TRIG_PIN, LOW);

        long duration = pulseIn(ECHO_PIN, HIGH);

        if (duration == 0)
            continue;

        long distance = duration * 0.034 / 2;

        Serial.print("Distance: ");
        Serial.print(distance);
        Serial.println(" cm");

        if (distance <= 4)
        {
            mecanumCar.Stop();
            Serial.println("Object detected!");
            break;
        }

        delay(30);
    }

    // -----------------------------
    // COLOR DETECTION
    // -----------------------------
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

    Serial.print("RED   : ");
    Serial.println(redFrequency);

    Serial.print("GREEN : ");
    Serial.println(greenFrequency);

    Serial.print("BLUE : ");
    Serial.println(blueFrequency);

    // -----------------------------
    // COLOR CLASSIFICATION
    // -----------------------------
    String detectColour = "UNKNOWN";

    // No object / ambient light
    if (redFrequency > 4000 &&
        greenFrequency > 4000 &&
        blueFrequency > 4000)
    {
        detectColour = "UNKNOWN";
    }
    // Blue
    else if (blueFrequency < redFrequency &&
             blueFrequency < greenFrequency)
    {
        detectColour = "BLUE";
    }
    // Red or Yellow
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

    Serial.print("Detected Color : ");
    Serial.println(detectColour);

    Serial.print("Target Color   : ");
    Serial.println(targetColor);

    // -----------------------------
    // CHECK TARGET COLOR
    // -----------------------------
    if (detectColour == targetColor)
    {
        Serial.println("Correct color!");

        myservo.write(60);     // Close gripper
        delay(400);

        return true;
    }
    else
    {
        Serial.println("Wrong color!");

        myservo.write(7);      // Keep gripper open
        delay(400);

        return false;
    }
}

void forward2(int target)
{
    int counter = 0;
    bool onJunction = false;

    // Normal travelling speed
    speed_Upper_L = 50;
    speed_Lower_L = 50;
    speed_Upper_R = 41;
    speed_Lower_R = 41;

    while(counter < target)
    {
        // Slow down when approaching the final target
        if(counter == target - 1)
        {
            speed_Upper_L = 30;
            speed_Lower_L = 30;
            speed_Upper_R = 25;
            speed_Lower_R = 25;
        }

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

            else
                mecanumCar.Advance();
        }
    }

    mecanumCar.Stop();
    delay(400);

    // Release object slowly
    for(int pos = 80; pos >= 7; pos--)
    {
        myservo.write(pos);
        delay(20);
    }

    delay(400);
}

void moveright_O(int target)
{
    int counter = 0;
    bool onMarker = false;
    speed_Upper_L = 54;
    speed_Lower_L = 41;
    speed_Upper_R = 57;
    speed_Lower_R = 46;

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

                Serial.print("Strafe Right Count: ");
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
    delay(400);
}
