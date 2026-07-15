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

//#define TURN_BLIND_MS        300
#define TURN_BLIND_MS        400
#define TURN_OBJECT          450
#define TURN_OBJECT_right    450
#define TURN_BLIND_LEFT180   470
#define TURN_SPEED           60
#define FOLLOW_SPEED         40
#define BRAKE                20

#define GRIPPER_OPEN    7 // was 7 -- 7 drives past the mechanical stop and strains the servo.
                           // Start at 30 (~halfway between 7 fully-open and 60 closed) and
                           // nudge lower/higher until it opens fully without straining.
#define GRIPPER_CLOSE   60

// Choose which sensor should detect the new line
// Usually the center sensor
#define STOP_SENSOR SensorMiddle

bool missionRunning = false;
// USER ADJUSTABLE JUNCTION ALIGNMENT TIMES (CHANGE THESE)
bool grab(String targetColor);
void approachDrop();

bool searchTarget(String targetColor,int startBlock);
void nextObject();


void moveTopToBottom();
void moveMiddleToTop();
void moveBottomToMiddle();

int currentBlock = 0;   // 0 = first, 1 = second, 2 = third


// =====================
// MOTOR SPEED
// =====================
extern uint8_t speed_Upper_L;
extern uint8_t speed_Lower_L;
extern uint8_t speed_Upper_R;
extern uint8_t speed_Lower_R;

// Inner-wheel speed as a percentage of outer-wheel speed during line-follow
// correction. A full pivot turn (Turn_Left/Turn_Right) on every correction
// overshoots into the opposite sensor and oscillates — softSteer keeps the
// outer side at whatever speed is already set (speeds are left untouched)
// and only slows (not reverses) the inner side.
//
// Two strengths instead of one fixed percentage: a shallow drift (outer +
// middle sensor both still on the line) only needs a light nudge, but the
// old code applied the same correction strength regardless of how far off
// the line actually was — even a tiny drift got a hard correction, which
// overshot to the other side and triggered an equally hard correction back,
// i.e. the zigzag. Proportioning the correction to the actual error damps
// that out without touching any of the base speed_* values.
#define SOFT_TURN_PCT_GENTLE 80   // outer + middle both on line — slight drift, light correction
#define SOFT_TURN_PCT_SHARP  60   // higher == less turn

void softSteer(bool steerLeft, uint8_t pct = SOFT_TURN_PCT_SHARP)
{
    if (steerLeft)
    {
        mecanumCar.Motor_Upper_L(1, (uint8_t)((uint32_t)speed_Upper_L * pct / 100));
        mecanumCar.Motor_Lower_L(1, (uint8_t)((uint32_t)speed_Lower_L * pct / 100));
        mecanumCar.Motor_Upper_R(1, speed_Upper_R);
        mecanumCar.Motor_Lower_R(1, speed_Lower_R);
    }
    else
    {
        mecanumCar.Motor_Upper_L(1, speed_Upper_L);
        mecanumCar.Motor_Lower_L(1, speed_Lower_L);
        mecanumCar.Motor_Upper_R(1, (uint8_t)((uint32_t)speed_Upper_R * pct / 100));
        mecanumCar.Motor_Lower_R(1, (uint8_t)((uint32_t)speed_Lower_R * pct / 100));
    }
}

const unsigned long OK_BUTTON = 0xFF02FD;//BLUE   #ok
const unsigned long NO0_BUTTON = 0xFF4AB5;
const unsigned long NO8_BUTTON = 0xFF38C7;
const unsigned long NO2_BUTTON = 0xFF5AA5;
const unsigned long NO3_BUTTON = 0xFFB04F;
const unsigned long NO4_BUTTON = 0xFF30CF;
const unsigned long NO5_BUTTON = 0xFF18E7;
const unsigned long NO6_BUTTON = 0xFF7A85;
const unsigned long NO7_BUTTON = 0xFF10EF;
const unsigned long NO1_BUTTON = 0xFF7A85;

IRrecv irrecv(RECV_PIN);
decode_results results;

void forward(int target); //NO7_Button
void forward_5cm(int target, bool withBlock = false);
void turnLeft180();
void turnLeft180_v1();
void turnRight180_v1();
void moveback(int target); //NO8_Button
void moveright(int target); //NO3_Button
void moveleft(int target); //NO4_Button
void turnRight90(); //NO9_Button
void turnRight90_Object();
void turnLeft90(); //NO0_Button
void turnLeft90_Object();

int redFrequency;
int greenFrequency;
int blueFrequency;
int diffColor  = 200;
int requestedColorState = 0;

/*
 * @brief Initializes all hardware used by the robot.
 *
 * Configures:
 * - Mecanum car driver
 * - Line tracking sensors
 * - Servo motor
 * - Color sensor
 * - Ultrasonic sensor
 * - IR receiver
 *
 * Runs only once when the Arduino starts.
 */
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

/*
 * @brief Main program loop.
 *
 * Continuously waits for an IR remote command.
 * Once a button is received, executes the corresponding
 * movement function while preventing multiple commands
 * from running simultaneously.
 */
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
                    mecanumCar.Stop();
                    missionRunning = false;
                }
                break;
            case NO1_BUTTON:
                if(!missionRunning)
                {
                    missionRunning = true;
                    turnRight180_v1();
                    missionRunning = false;
                }
                break;
            case NO8_BUTTON: //button 8
                if (!missionRunning)
                {
                    missionRunning = true;
                    //forward(2); 
                    forward_5cm(5);
                   // grab("BLUE"); // waits (re-sampling in place) until a blue object is seen
                    searchTarget("BLUE",1);
                    turnLeft180_v1(); //o stand for object
                    switch(currentBlock)
                    {
                        case 0:   // First block
                            forward(5);
                            approachDrop(); // slow final intersection so i;t drops as straight as possible
                            mecanumCar.Stop();
                            myservo.write(GRIPPER_OPEN);
                            delay(3000);
                            turnRight180_v1();
                            break;

                        case 1:   // Second block
                            forward_5cm(2, true);
                            turnRight90_Object();
                            forward_5cm(2, true);
                            turnLeft90_Object();
                            forward_5cm(3, true);
                            approachDrop();
                            mecanumCar.Stop();
                            myservo.write(GRIPPER_OPEN);
                            delay(3000);
                            turnRight180_v1();
                            break;

                        case 2:   // Third block
                            forward_5cm(1, true);
                            turnLeft90_Object();
                            forward_5cm(2, true);
                            turnRight90_Object();
                            forward_5cm(4, true);
                            approachDrop();
                            mecanumCar.Stop();
                            myservo.write(GRIPPER_OPEN);
                            delay(3000);
                            turnRight180_v1();
                            break;
                    }
                    //forward(5);
                    //approachDrop(); // slow final intersection so i;t drops as straight as possible
                    //mecanumCar.Stop();
                    //myservo.write(GRIPPER_OPEN);
                    //delay(3000);
                    //turnRight180_v1();
                    missionRunning = false;
                }
                break;
            case NO2_BUTTON: //button 9
                if (!missionRunning)
                {
                    missionRunning = true;
                    forward_5cm(3);
                    //turnLeft90();
                    turnLeft90_Object();
                    forward_5cm(2);
                    turnRight90_Object();
                    //turnRight90();
                    forward_5cm(2);
                    //grab("YELLOW");
                    searchTarget("YELLOW",2);
                    turnLeft180_v1();
                    switch(currentBlock)
                    {
                        case 0:   // First block
                            // Original return path
                            forward_5cm(1, true);
                            turnLeft90_Object();
                            //turnLeft90();
                            forward_5cm(2, true);
                            turnRight90_Object();
                            //turnRight90();
                            forward_5cm(4, true);
                            approachDrop();
                            mecanumCar.Stop();
                            myservo.write(GRIPPER_OPEN);
                            delay(3000);
                            turnRight180_v1();
                            break;

                        case 1:   // Second block
                            forward(5);
                            approachDrop(); // slow final intersection so i;t drops as straight as possible
                            mecanumCar.Stop();
                            myservo.write(GRIPPER_OPEN);
                            delay(3000);
                            turnRight180_v1();
                            break;

                        case 2:   // Third block
                            forward_5cm(2, true);
                            turnRight90_Object();
                            //turnRight90();
                            forward_5cm(2, true);
                            turnLeft90_Object();
                            //turnLeft90();
                            forward_5cm(3, true);
                            approachDrop();
                            mecanumCar.Stop();
                            myservo.write(GRIPPER_OPEN);
                            delay(3000);
                            turnRight180_v1();  
                            break;
                    }
                   // forward_5cm(1, true);
                    //turnLeft90_Object();
                    //turnLeft90();
                   // forward_5cm(2, true);
                   // turnRight90_Object();
                    //turnRight90();
                   // forward_5cm(4, true);
                   // approachDrop();
                    //mecanumCar.Stop();
                   // myservo.write(GRIPPER_OPEN);
                    //delay(3000);
                    //turnRight180_v1();
                   // moveback(2);
                    missionRunning = false;
                }
                break;
            case NO3_BUTTON: //correct
                if (!missionRunning)
                {
                    missionRunning = true;
                    //moveright(1);
                    forward_5cm(3);
                    //delay(1000);
                    turnRight90();
                    //delay(1000);
                    forward_5cm(2);
                    //delay(1000);
                    turnLeft90();
                   // delay(1000);
                    forward_5cm(2);
                   // grab("RED");
                    searchTarget ("RED",0);
                    turnLeft180_v1();
                    switch(currentBlock)
                    {
                        case 0:   // First block
                            // Original return path
                            forward_5cm(2, true);
                            turnRight90_Object();
                            forward_5cm(2, true);
                            turnLeft90_Object();
                            forward_5cm(3, true);
                            approachDrop();
                            mecanumCar.Stop();
                            myservo.write(GRIPPER_OPEN);
                            delay(3000);
                            turnRight180_v1();
                            break;

                        case 1:   // Second block
                            forward_5cm(1, true);
                            turnLeft90_Object();
                            forward_5cm(2, true);
                            turnRight90_Object();
                            forward_5cm(4, true);
                            approachDrop();
                            mecanumCar.Stop();
                            myservo.write(GRIPPER_OPEN);
                            delay(3000);
                            turnRight180_v1();
                            break;

                        case 2:   // Third block
                            forward(5);
                            approachDrop(); // slow final intersection so i;t drops as straight as possible
                            mecanumCar.Stop();
                            myservo.write(GRIPPER_OPEN);
                            delay(3000);
                            turnRight180_v1();
                            break;
                    }
                   // forward_5cm(2, true);
                    //turnRight90_Object();
                   // forward_5cm(2, true);
                  //  turnLeft90_Object();
                    //forward_5cm(3, true);
                    //approachDrop();
                    //mecanumCar.Stop();
                    //myservo.write(GRIPPER_OPEN);
                    //delay(3000);
                    //turnRight180_v1();  
                    missionRunning = false;
                }
                break;
            case NO4_BUTTON: //correct
                if (!missionRunning)
                {
                    missionRunning = true;
                    moveleft(1); 
                    missionRunning = false;
                }
                break;
            case NO7_BUTTON: //turn right
                if (!missionRunning)
                {
                    missionRunning = true;
                    turnRight90();   
                    missionRunning = false;
                }
                break;
            case NO0_BUTTON: //turn left
                if (!missionRunning)
                {
                    missionRunning = true;
                    turnLeft90();   
                    missionRunning = false;
                }
                break;
            case NO5_BUTTON:
                if(!missionRunning)
                {
                    missionRunning = true;
                    //turnLeft180();
                    turnLeft180_v1();
                    missionRunning = false;
                }
            default:
                break;
        }
        irrecv.resume();
    }
}

/*
 * @brief Moves the robot forward while following a line.
 *
 * The robot continuously follows the center line using
 * three IR sensors. Every time a junction is detected,
 * the junction counter increases.
 *
 * Movement stops after reaching the requested number
 * of junctions.
 *
 * @param target Number of junctions to pass before stopping.
 */
void forward(int target)
{
    int counter = 0;
    bool onJunction = false;

    speed_Upper_L = 45;
    speed_Lower_L = 45;
    speed_Upper_R = 45;
    speed_Lower_R = 45;

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

void forward_5cm(int target, bool withBlock)
{
    int counter = 0;
    bool onJunction = false;

   // speed_Upper_L = 50;
   //speed_Lower_L = 50;
   // speed_Upper_R = 41;
   // speed_Lower_R = 41;

    //battery full
    speed_Upper_L = 45;
    speed_Lower_L = 45;
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
                onJunction = true;

                // Final intersection detected
                if(counter >= target)
                {
                    // Continue forward slightly
                    // Calibrate this delay for ~5 cm movement
                    mecanumCar.Advance();
                    if (withBlock) {
                        delay(175);
                    } else {delay(150);}
                    
                    mecanumCar.Back();
                    

                    mecanumCar.Stop();
                    delay(400);
                    return;
                }
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

void turnLeft180()
{
    // Equal speed to keep rotation center fixed
    speed_Upper_L = 67;
    speed_Lower_L = 65;
    speed_Upper_R = 65;
    speed_Lower_R = 67;

    // First 90 degrees
    unsigned long start = millis();

    while (millis() - start < TURN_BLIND_MS)
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

    // Continue past first line so it doesn't detect
    // the same 90-degree line again
    start = millis();

    while (millis() - start < TURN_BLIND_MS)
    {
        mecanumCar.Turn_Left();
    }

    // Find the opposite line = 180 degrees
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

    // Active brake
    mecanumCar.Turn_Right();
    delay(BRAKE);

    mecanumCar.Stop();
    delay(400);

}

void turnLeft180_v1()
{
    speed_Upper_L = 66;
    speed_Lower_L = 66;
    speed_Upper_R = 66;
    speed_Lower_R = 66;

    unsigned long start = millis();

    // =====================================
    // FIRST 90 DEGREE SECTION
    // =====================================

    // Blind turn first
    while (millis() - start < TURN_BLIND_LEFT180)
    {
        mecanumCar.Turn_Left();
    }

    start = millis();

    // Detect first line (~90 degrees)
    while (millis() - start < 150)
    {
        bool left   = digitalRead(SensorLeft);
        bool middle = digitalRead(SensorMiddle);
        bool right  = digitalRead(SensorRight);

        mecanumCar.Turn_Left();

        if (left && !middle && !right)
            break;
    }

    // =====================================
    // CONTINUE PAST FIRST LINE
    // =====================================

    start = millis();

    while (millis() - start < TURN_BLIND_LEFT180)
    {
        mecanumCar.Turn_Left();
    }

    start = millis();

    // =====================================
    // DETECT SECOND LINE (~180 degrees)
    // =====================================

    while (millis() - start < 150)
    {
        bool left   = digitalRead(SensorLeft);
        bool middle = digitalRead(SensorMiddle);
        bool right  = digitalRead(SensorRight);

        mecanumCar.Turn_Left();

        if (left && !middle && !right)
            break;
    }

    // =====================================
    // ACTIVE BRAKE
    // =====================================

    mecanumCar.Turn_Right();
    delay(BRAKE);

    mecanumCar.Stop();
    delay(100);

    // =====================================
    // DRIFT LEFT
    // =====================================

    speed_Lower_L = 66;
    speed_Lower_R = 64;

    mecanumCar.drift_right();
    delay(300);   // Tune this for required left shift

    mecanumCar.Stop();
    delay(400);
}

void turnRight180_v1()
{
    speed_Upper_L = 65;
    speed_Lower_L = 65;
    speed_Upper_R = 65;
    speed_Lower_R = 65;

    unsigned long start = millis();

    // =====================================
    // FIRST 90 DEGREE SECTION
    // =====================================

    // Blind turn first
    while (millis() - start < TURN_BLIND_MS)
    {
        mecanumCar.Turn_Right();
    }

    start = millis();

    // Detect first line (~90 degrees)
    while (millis() - start < 150)
    {
        bool left   = digitalRead(SensorLeft);
        bool middle = digitalRead(SensorMiddle);
        bool right  = digitalRead(SensorRight);

        mecanumCar.Turn_Right();

        if (!left && !middle && right)
            break;
    }

    // =====================================
    // CONTINUE PAST FIRST LINE
    // =====================================

    start = millis();

    while (millis() - start < TURN_BLIND_MS)
    {
        mecanumCar.Turn_Right();
    }

    start = millis();

    // =====================================
    // DETECT SECOND LINE (~180 degrees)
    // =====================================

    while (millis() - start < 150)
    {
        bool left   = digitalRead(SensorLeft);
        bool middle = digitalRead(SensorMiddle);
        bool right  = digitalRead(SensorRight);

        // FIX: Continue turning RIGHT
        mecanumCar.Turn_Right();

        if (!left && !middle && right)
            break;
    }

    // =====================================
    // ACTIVE BRAKE
    // =====================================

    // Correct: opposite direction to brake
    mecanumCar.Turn_Left();
    delay(BRAKE);

    mecanumCar.Stop();
    delay(100);

    // =====================================
    // DRIFT LEFT
    // =====================================

    speed_Lower_L = 64;
    speed_Lower_R = 66;

    mecanumCar.drift_left();
    delay(230);

    mecanumCar.Stop();
    delay(400);
}
/*
 * @brief Moves the robot backward.
 *
 * The robot travels backward while counting junctions.
 * A timeout is included to prevent the robot from
 * reversing forever if a junction cannot be detected.
 *
 * @param target Number of junctions to move backward.
 */
void moveback(int target)
{
    int counter = 0;
    bool onJunction = false;

   // speed_Upper_L = 36; //39
    //speed_Lower_L = 40; //44
    //speed_Upper_R = 37; //30 //32
    //speed_Lower_R = 39; //35 //37


    speed_Upper_L = 45;
    speed_Lower_L = 45;
    speed_Upper_R = 45;
    speed_Lower_R = 45;
    
    // Record the exact time the movement started
    //unsigned long startTime = millis();
    // The loop will exit if it hits the target junctions OR if 200 milliseconds (0.2s) pass
    /*while((counter < target) && (millis() - startTime < 1200))
    {
        bool left   = digitalRead(SensorLeft);
        bool middle = digitalRead(SensorMiddle);
        bool right  = digitalRead(SensorRight);
        if(left && middle && right)
        {
            if(!onJunction)
            {
                counter++;
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
    delay(400);*/
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

/*
 * @brief Strafes the robot to the right.
 *
 * Uses the mecanum wheels to move sideways while
 * counting line markers detected by the sensors.
 *
 * Stops after reaching the requested number of markers.
 *
 * @param target Number of markers to pass.
 */
void moveright(int target)
{
    int counter = 0;
    bool onMarker = false;

    speed_Upper_L = 64;
    speed_Lower_L = 52;
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

/*
 * @brief Strafes the robot to the left.
 *
 * Uses the mecanum wheels to move sideways while
 * counting line markers detected by the sensors.
 *
 * Stops after reaching the requested number of markers.
 *
 * @param target Number of markers to pass.
 */
void moveleft(int target)
{
    int counter = 0;
    bool onMarker = false;

    speed_Upper_L = 52; 
    speed_Lower_L = 52;
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

/*
 * @brief Rotates the robot approximately 90° clockwise.
 *
 * The robot first performs a blind turn for a fixed
 * duration, then uses the line sensors to accurately
 * align itself with the next line.
 */
void turnRight90()
{
    speed_Upper_L = 66;
    speed_Lower_L = 66;
    speed_Upper_R = 66;
    speed_Lower_R = 66;

    unsigned long start = millis();

    while (millis() - start < TURN_BLIND_MS)
        mecanumCar.Turn_Right();

    start = millis();
    while (millis() - start < 150)
    {
        bool left   = digitalRead(SensorLeft);
        bool middle = digitalRead(SensorMiddle);
        bool right  = digitalRead(SensorRight);

        mecanumCar.Turn_Right();

        if (!left && !middle && right)
            break;
    }
    // Active brake: briefly rotate opposite direction
    mecanumCar.Turn_Left();
    delay(BRAKE);
    mecanumCar.Stop();
    delay(400);
}

void turnRight90_Object()
{
    speed_Upper_L = 66;
    speed_Lower_L = 66;
    speed_Upper_R = 66;
    speed_Lower_R = 66;

    unsigned long start = millis();

    while (millis() - start < (TURN_OBJECT_right))
        mecanumCar.Turn_Right();

    start = millis();
    while (millis() - start < 150)
    {
        bool left   = digitalRead(SensorLeft);
        bool middle = digitalRead(SensorMiddle);
        bool right  = digitalRead(SensorRight);

        mecanumCar.Turn_Right();

        if (!left && !middle && right)
            break;
    }

    // Active brake: briefly rotate opposite direction
    mecanumCar.Turn_Left();
    delay(BRAKE);
    mecanumCar.Stop();
    delay(400);

}


/*
 * @brief Rotates the robot approximately 90° counter-clockwise.
 *
 * The robot first performs a blind turn for a fixed
 * duration, then fine-tunes its alignment using the
 * line sensors until the new line is detected.
 */
/*void turnLeft90()
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
}*/

void turnLeft90()
{
    speed_Upper_L = 67;
    speed_Lower_L = 67;
    speed_Upper_R = 67;
    speed_Lower_R = 67;

    unsigned long start = millis();

    // Blind turn first
    while (millis() - start < TURN_BLIND_MS)
    {
        mecanumCar.Turn_Left();
    }

    start = millis();

    // Continue turning until left sensor detects line
    while (millis() - start < 150)
    {
        bool left   = digitalRead(SensorLeft);
        bool middle = digitalRead(SensorMiddle);
        bool right  = digitalRead(SensorRight);

        mecanumCar.Turn_Left();

        if (left && !middle && !right)
            break;
    }

    // Active brake: briefly rotate opposite direction
    mecanumCar.Turn_Right();
    delay(BRAKE);
    mecanumCar.Stop();
    delay(400);
}

void turnLeft90_Object()
{
    speed_Upper_L = 67;
    speed_Lower_L = 67;
    speed_Upper_R = 67;
    speed_Lower_R = 67;

    unsigned long start = millis();

    // Blind turn first
    while (millis() - start < TURN_OBJECT)
    {
        mecanumCar.Turn_Left();
    }

    start = millis();

    // Continue turning until left sensor detects line
    while (millis() - start < 150)
    {
        bool left   = digitalRead(SensorLeft);
        bool middle = digitalRead(SensorMiddle);
        bool right  = digitalRead(SensorRight);

        mecanumCar.Turn_Left();

        if (left && !middle && !right)
            break;
    }

    // Active brake: briefly rotate opposite direction
    mecanumCar.Turn_Right();
    delay(BRAKE);
    mecanumCar.Stop();
    delay(400);

}



//========================================
// Read one colour channel
//========================================
int readColor(bool s2, bool s3)
{
    digitalWrite(S2, s2);
    digitalWrite(S3, s3);

    delay(20);

    return pulseIn(SENSOR_OUT, LOW, 30000);
}

//========================================
// Detect colour once
//========================================
String detectColour()
{
    redFrequency   = readColor(LOW, LOW);
    blueFrequency  = readColor(LOW, HIGH);
    greenFrequency = readColor(HIGH, HIGH);

    Serial.print("R:");
    Serial.print(redFrequency);

    Serial.print(" G:");
    Serial.print(greenFrequency);

    Serial.print(" B:");
    Serial.println(blueFrequency);

    int diff = greenFrequency - redFrequency;

    if(diff > 400)
        return "RED";

    else if(diff > 0)
        return "YELLOW";

    else
        return "BLUE";
}

// Approach tuning
#define APPROACH_START_CM 10   // start slowing down once object is this close
#define GRAB_AT_CM        4.8 // stop here, ready to check colour

// Speed while still far from the object (matches moveForwardIntersections' cruise speed)
const uint8_t APPROACH_NORMAL_UL = 40;
const uint8_t APPROACH_NORMAL_LL = 40;
const uint8_t APPROACH_NORMAL_UR = 34;
const uint8_t APPROACH_NORMAL_LR = 34;


// Speed once inside APPROACH_START_CM, easing toward the stop (your existing grab() speed)
const uint8_t APPROACH_SLOW_UL = 35;
const uint8_t APPROACH_SLOW_LL = 35;
const uint8_t APPROACH_SLOW_UR = 31;
const uint8_t APPROACH_SLOW_LR = 31;

uint8_t rampSpeed(uint8_t normalSpd, uint8_t slowSpd, long distance)
{
    if (distance >= APPROACH_START_CM) return normalSpd;
    if (distance <= GRAB_AT_CM)        return slowSpd;
    return (uint8_t)map(distance, GRAB_AT_CM, APPROACH_START_CM, slowSpd, normalSpd);
}

bool grab(String targetColor)
{
    // Open gripper
    myservo.write(GRIPPER_OPEN);
    delay(300);

    // Start at normal (far) speed -- ramps down automatically as we close in
    speed_Upper_L = 35;
    speed_Lower_L = 35;
    speed_Upper_R = 32;
    speed_Lower_R = 32;

    while (true)
    {
        // -----------------------------
        // ULTRASONIC (read first -- decides this iteration's speed)
        // -----------------------------
        digitalWrite(TRIG_PIN, LOW);
        delayMicroseconds(2);
        digitalWrite(TRIG_PIN, HIGH);
        delayMicroseconds(10);
        digitalWrite(TRIG_PIN, LOW);

        long duration = pulseIn(ECHO_PIN, HIGH, 30000);
        long distance = (duration == 0) ? 999 : (long)(duration * 0.034 / 2);

        Serial.print("Distance: ");
        Serial.print(distance);
        Serial.println(" cm");

        if (distance <= 7)
        {
            speed_Upper_L = 35 * 0.9 * 0.9;
            speed_Lower_L = 35 * 0.9 * 0.9;
            speed_Upper_R = 35 * 0.9 * 0.9;
            speed_Lower_R = 35 * 0.9 * 0.9;
        }
        else if (distance <= 15)
        {
            speed_Upper_L = 35 * 0.9;
            speed_Lower_L = 35 * 0.9;
            speed_Upper_R = 35 * 0.9;
            speed_Lower_R = 35 * 0.9;
        }
        else
        {
            speed_Upper_L = 35;
            speed_Lower_L = 35;
            speed_Upper_R = 35;
            speed_Lower_R = 35;
        }
        if (distance <= 3.8)
        {
            mecanumCar.Stop();
            Serial.println("Object detected!");
            break;
        }


        if (distance <= GRAB_AT_CM)
        {
            mecanumCar.Stop();
            break;
        }

        //speed_Upper_L = 35;
        //speed_Lower_L = 35;
        //speed_Upper_R = 35;
        //speed_Lower_R = 35;


        speed_Upper_L = rampSpeed(APPROACH_NORMAL_UL, APPROACH_SLOW_UL, distance);
        speed_Lower_L = rampSpeed(APPROACH_NORMAL_LL, APPROACH_SLOW_LL, distance);
        speed_Upper_R = rampSpeed(APPROACH_NORMAL_UR, APPROACH_SLOW_UR, distance);
        speed_Lower_R = rampSpeed(APPROACH_NORMAL_LR, APPROACH_SLOW_LR, distance);

        // -----------------------------
        // LINE FOLLOWING (uses the speed just set above)
        // -----------------------------
        bool left   = digitalRead(SensorLeft);
        bool middle = digitalRead(SensorMiddle);
        bool right  = digitalRead(SensorRight);

        if(!left && middle && !right)
            mecanumCar.Advance();

        else if(left && !middle && !right)
            softSteer(true, SOFT_TURN_PCT_SHARP);   // fully off — outer sensor only

        else if(!left && !middle && right)
            softSteer(false, SOFT_TURN_PCT_SHARP);

        else if(left && middle && !right)
            softSteer(true, SOFT_TURN_PCT_GENTLE);  // slight drift — outer + middle still on line

        else if(!left && middle && right)
            softSteer(false, SOFT_TURN_PCT_GENTLE);
        else if(left && !middle && right)
            mecanumCar.Advance();
        else if(!left && !middle && !right)
            //findLine(); // lost the line entirely — sweep left/right until it's found again
            //alignToLine(); // lost the line entirely — sweep left/right until it's found again
            mecanumCar.Advance();
        delay(30);
    }

    // ---- colour detection / grab loop below: UNCHANGED from your file ----
    while (true)
    {
                // ---- Colour detection ----
        int redCount = 0;
        int yellowCount = 0;
        int blueCount = 0;

        Serial.println();
        Serial.println("Taking 10 colour readings...");

        for(int i = 0; i < 10; i++)
        {
            String colour = detectColour();

            if(colour == "RED")
                redCount++;
            else if(colour == "YELLOW")
                yellowCount++;
            else
                blueCount++;

            delay(100);
        }

        Serial.println("----------------------");
        Serial.print("RED    : "); Serial.println(redCount);
        Serial.print("YELLOW : "); Serial.println(yellowCount);
        Serial.print("BLUE   : "); Serial.println(blueCount);
        Serial.println("----------------------");

        String detected = "UNCERTAIN";

        if(redCount >= 8)
            detected = "RED";
        else if(yellowCount >= 8)
            detected = "YELLOW";
        else if(blueCount >= 8)
            detected = "BLUE";

        Serial.print("FINAL : ");
        Serial.println(detected);

        if(detected == targetColor)
        {
            Serial.println("Target colour matched!");

            myservo.write(GRIPPER_CLOSE);
            delay(1000);

            return true;
        }

        // Wrong colour
        Serial.println("Wrong colour.");
        return false;
}
}

/*bool searchTarget(String targetColor, int startBlock)
{
    currentBlock = 0;

    while(true)
    {
        Serial.print("Checking block ");
        Serial.println(currentBlock + 1);

        if(grab(targetColor))
        {
            Serial.print("Found at block ");
            Serial.println(currentBlock + 1);
            return true;
        }

        // Already checked all 3 blocks
        if(currentBlock >= 2)
        {
            Serial.println("Target not found!");
            return false;
        }

        currentBlock++;

        nextObject();
    }
}*/

bool searchTarget(String targetColor, int startBlock)
{
    currentBlock = startBlock;

    while (true)
    {
        Serial.print("Checking block ");
        Serial.println(currentBlock);

        if (grab(targetColor))
            return true;

        // ---------- Decide next move ----------

        if (startBlock == 0)   // Started at TOP (RED mission)
        {
            if(currentBlock == 0)
            {
                moveTopToBottom();
                currentBlock = 1;
            }
            else if(currentBlock == 1)
            {
                moveBottomToMiddle();
                currentBlock = 2;
            }
            else
            {
                return false;
            }
        }

        else if(startBlock == 1)   // Started at MIDDLE (BLUE mission)
        {
            if(currentBlock == 1)
            {
                moveMiddleToTop();
                currentBlock = 2;
            }
            else if(currentBlock == 2)
            {
                moveTopToBottom();
                moveMiddleToTop();
                currentBlock = 0;
            }
            else
            {
                return false;
            }
        }

        else if(startBlock == 2)   // Started at BOTTOM (YELLOW mission)
        {
            if(currentBlock == 2)
            {
                moveBottomToMiddle();
                currentBlock = 1;
            }
            else if(currentBlock == 1)
            {
                moveMiddleToTop();
                currentBlock = 0;
            }
            else
            {
                return false;
            }
        }
    }
}

void moveTopToBottom()
{
    //mecanumCar.Back();
    moveback(1);
    delay(150);
    mecanumCar.Advance();
    mecanumCar.Stop();
    turnLeft90();
    forward_5cm(4);
    turnRight90();
}
void moveMiddleToTop()
{
    moveback(1);
    delay(150);
    mecanumCar.Advance();
    mecanumCar.Stop();
    turnRight90();
    forward_5cm(2);
    turnLeft90();
}
void moveBottomToMiddle()
{
    moveback(1);
    delay(150);
    mecanumCar.Advance();
    mecanumCar.Stop();
   // moveback(1);
    turnRight90();
    forward_5cm(2);
    turnLeft90();
}



/*void nextObject()
{
    Serial.println("Moving to next block...");

    myservo.write(GRIPPER_OPEN);
    delay(300);

    mecanumCar.Back();
    delay(250);
    mecanumCar.Stop();
    delay(200);

    moveback(1);
    turnRight90();
    forward_5cm(1);
    turnLeft90();
}*/

void approachDrop()
{
    // Slow approach speed (same profile as grab()'s approach)
    speed_Upper_L = 40;
    speed_Lower_L = 36;
    speed_Upper_R = 37;
    speed_Lower_R = 37;

    int counter = 0;
    bool onJunction = false;

    while (counter < 2)
    {
        bool left   = digitalRead(SensorLeft);
        bool middle = digitalRead(SensorMiddle);
        bool right  = digitalRead(SensorRight);

        if (left && middle && right)
        {
            if (!onJunction)
            {
                counter++;
                Serial.print("Drop approach — final intersection: ");
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
                softSteer(true, SOFT_TURN_PCT_SHARP);   // fully off — outer sensor only

            else if(!left && !middle && right)
                softSteer(false, SOFT_TURN_PCT_SHARP);

            else if(left && middle && !right)
                softSteer(true, SOFT_TURN_PCT_GENTLE);  // slight drift — outer + middle still on line

            else if(!left && middle && right)
                softSteer(false, SOFT_TURN_PCT_GENTLE);
            else if(left && !middle && right)
                mecanumCar.Advance();
            else if(!left && !middle && !right)
                //findLine(); // lost the line entirely — sweep left/right until it's found again
                //alignToLine(); // lost the line entirely — sweep left/right until it's found again
                mecanumCar.Advance();
        }
    }

    mecanumCar.Stop();
    delay(300);
}

// Sequence for middle and right side.

//ultrasonic detection if there is no object move to the next one and if really there is nothing go home.

// 