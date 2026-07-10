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
#define TURN_BLIND_MS        450
#define TURN_SPEED           60
#define FOLLOW_SPEED         40
#define BRAKE                20

// Choose which sensor should detect the new line
// Usually the center sensor
#define STOP_SENSOR SensorMiddle

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
const unsigned long NO0_BUTTON = 0xFF4AB5;
const unsigned long NO1_BUTTON = 0xFF38C7;
const unsigned long NO2_BUTTON = 0xFF5AA5;
const unsigned long NO3_BUTTON = 0xFFB04F;
const unsigned long NO4_BUTTON = 0xFF30CF;
const unsigned long NO5_BUTTON = 0xFF18E7;
const unsigned long NO6_BUTTON = 0xFF7A85;
const unsigned long NO7_BUTTON = 0xFF10EF;


//unassigned button
const unsigned long NO8_BUTTON = 0xFFB04F;
const unsigned long NO9_BUTTON = 0xFFB04F;

IRrecv irrecv(RECV_PIN);
decode_results results;

void forward(int target); //NO7_Button
void forward_5cm(int target);
void turnLeft180();
void turnLeft180_v1();
void moveback(int target); //NO8_Button
void moveright(int target); //NO3_Button
void moveleft(int target); //NO4_Button
void turnRight90(); //NO9_Button
void turnLeft90(); //NO0_Button

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
            case NO1_BUTTON: //button 8
                if (!missionRunning)
                {
                    missionRunning = true;
                    //forward(2); 
                    forward_5cm(2);
                    missionRunning = false;
                }
                break;
            case NO2_BUTTON: //button 9
                if (!missionRunning)
                {
                    missionRunning = true;
                    moveback(2);
                    missionRunning = false;
                }
                break;
            case NO3_BUTTON: //correct
                if (!missionRunning)
                {
                    missionRunning = true;
                    moveright(1);
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

void forward_5cm(int target)
{
    int counter = 0;
    bool onJunction = false;

    speed_Upper_L = 50;
    speed_Lower_L = 50;
    speed_Upper_R = 42;
    speed_Lower_R = 42;

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
                    delay(130);
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
    speed_Upper_L = 67;
    speed_Lower_L = 66;
    speed_Upper_R = 62;
    speed_Lower_R = 64;

    unsigned long start = millis();

    // =====================================
    // FIRST 90 DEGREE SECTION
    // =====================================

    // Blind turn first
    while (millis() - start < TURN_BLIND_MS)
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

    while (millis() - start < TURN_BLIND_MS)
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

    speed_Upper_L = 45;
    speed_Lower_L = 45;
    speed_Upper_R = 45;
    speed_Lower_R = 45;

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

    speed_Upper_L = 45; 
    speed_Lower_L = 45;
    speed_Upper_R = 45;
    speed_Lower_R = 45;

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
    speed_Upper_L = 45;
    speed_Lower_L = 45;
    speed_Upper_R = 45;
    speed_Lower_R = 45;

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
    speed_Lower_L = 66;
    speed_Upper_R = 62;
    speed_Lower_R = 64;

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

