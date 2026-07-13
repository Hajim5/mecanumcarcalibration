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
#define TURN_BLIND_MS_LEFT   400
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

void forward(int target); //NO1_Button
void moveback(int target); //NO2_Button
void moveright(int target); //NO3_Button
void moveleft(int target); //NO4_Button
void diagonalUpperRight(int target); //NO5_Button
void diagonalUpperLeft(int target); //NO6_Button
void diagonalLowerRight(int target); //NO7_Button
void diagonalLowerLeft(int target); //NO8_Button
void turnRight90(); //NO9_Button
void turnLeft90(); //NO0_Button
void turn180_R();
void turn180_L();

bool grab(String targetColor);
void alignRobot();
bool verifyAligment();
void dropoff(int target);

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
                if (!missionRunning)
                {
                    missionRunning = true;
                    forward(); 
                    missionRunning = false;
                }
                break;
            case NO2_BUTTON:
                if (!missionRunning)
                {
                    missionRunning = true;
                    moveback();
                    missionRunning = false;
                }
                break;
            case NO3_BUTTON:
                if (!missionRunning)
                {
                    missionRunning = true;
                    moveright();
                    missionRunning = false;
                }
                break;
            case NO4_BUTTON:
                if (!missionRunning)
                {
                    missionRunning = true;
                    moveleft();  
                    missionRunning = false;
                }
                break;
            case NO5_BUTTON:
                if (!missionRunning)
                {
                    missionRunning = true;
                    diagonalUpperRight();  
                    missionRunning = false;
                }
                break;
            case NO6_BUTTON:
                if (!missionRunning)
                {
                    missionRunning = true;
                    diagonalUpperLeft();   
                    missionRunning = false;
                }
                break;
            case NO7_BUTTON:
                if (!missionRunning)
                {
                    missionRunning = true;
                    diagonalLowerRight(); 
                    missionRunning = false;
                }
                break;
            case NO8_BUTTON:
                if (!missionRunning)
                {
                    missionRunning = true;
                    diagonalLowerLeft();  
                    missionRunning = false;
                }
                break;
            case NO9_BUTTON:
                if (!missionRunning)
                {
                    missionRunning = true;
                    turnRight90();   
                    missionRunning = false;
                }
                break;
            case NO0_BUTTON:
                if (!missionRunning)
                {
                    missionRunning = true;
                    turnLeft90();   
                    missionRunning = false;
                }
                break;
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
 * @brief Moves the robot diagonally toward the upper-right direction.
 *
 * This function uses the mecanum wheels to move the robot diagonally
 * while counting line markers using the line tracking sensors.
 *
 * During the movement, the robot continuously reads the left, middle,
 * and right sensors. When the right sensor detects a new line marker,
 * the marker counter is incremented. The function continues moving
 * until the specified number of markers has been reached.
 *
 * The 'onMarker' flag prevents the same marker from being counted
 * multiple times while the robot is still passing over it.
 *
 * @param target Number of line markers to detect before stopping.
 */
void diagonalUpperRight(int target)
{
    int counter = 0;
    bool onMarker = false;

    // Motor speed calibration
    speed_Upper_L = 60;
    speed_Lower_L = 60;
    speed_Upper_R = 60;
    speed_Lower_R = 60;

    while(counter < target)
    {
        bool left   = digitalRead(SensorLeft);
        bool middle = digitalRead(SensorMiddle);
        bool right  = digitalRead(SensorRight);

        // Move diagonally to the upper-right
        mecanumCar.RU_Move();

        // Detect a new marker
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
 * @brief Moves the robot diagonally toward the upper-left direction.
 *
 * This function uses the mecanum wheels to move the robot diagonally
 * while counting line markers using the line tracking sensors.
 *
 * During the movement, the robot continuously reads the left, middle,
 * and right sensors. When the left sensor detects a new line marker,
 * the marker counter is incremented. The function continues moving
 * until the specified number of markers has been reached.
 *
 * The 'onMarker' flag prevents the same marker from being counted
 * multiple times while the robot is still passing over it.
 *
 * @param target Number of line markers to detect before stopping.
 */
void diagonalUpperLeft(int target)
{
    int counter = 0;
    bool onMarker = false;

    speed_Upper_L = 60;
    speed_Lower_L = 60;
    speed_Upper_R = 60;
    speed_Lower_R = 60;

    while(counter < target)
    {
        bool left   = digitalRead(SensorLeft);
        bool middle = digitalRead(SensorMiddle);
        bool right  = digitalRead(SensorRight);

        mecanumCar.LU_Move();

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
 * @brief Moves the robot diagonally toward the lower-right direction.
 *
 * This function uses the mecanum wheels to move the robot diagonally
 * in reverse while counting line markers using the line tracking sensors.
 *
 * During the movement, the robot continuously reads the left, middle,
 * and right sensors. When the right sensor detects a new line marker,
 * the marker counter is incremented. The function continues moving
 * until the specified number of markers has been reached.
 *
 * The 'onMarker' flag prevents the same marker from being counted
 * multiple times while the robot is still passing over it.
 *
 * @param target Number of line markers to detect before stopping.
 */
void diagonalLowerRight(int target)
{
    int counter = 0;
    bool onMarker = false;

    speed_Upper_L = 60;
    speed_Lower_L = 60;
    speed_Upper_R = 60;
    speed_Lower_R = 60;

    while(counter < target)
    {
        bool left   = digitalRead(SensorLeft);
        bool middle = digitalRead(SensorMiddle);
        bool right  = digitalRead(SensorRight);

        mecanumCar.RD_Move();

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
 * @brief Moves the robot diagonally toward the lower-left direction.
 *
 * This function uses the mecanum wheels to move the robot diagonally
 * in reverse while counting line markers using the line tracking sensors.
 *
 * During the movement, the robot continuously reads the left, middle,
 * and right sensors. When the left sensor detects a new line marker,
 * the marker counter is incremented. The function continues moving
 * until the specified number of markers has been reached.
 *
 * The 'onMarker' flag prevents the same marker from being counted
 * multiple times while the robot is still passing over it.
 *
 * @param target Number of line markers to detect before stopping.
 */
void diagonalLowerLeft(int target)
{
    int counter = 0;
    bool onMarker = false;

    speed_Upper_L = 60;
    speed_Lower_L = 60;
    speed_Upper_R = 60;
    speed_Lower_R = 60;

    while(counter < target)
    {
        bool left   = digitalRead(SensorLeft);
        bool middle = digitalRead(SensorMiddle);
        bool right  = digitalRead(SensorRight);

        mecanumCar.LD_Move();

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
    mecanumCar.Turn_Left();
    delay(BRAKE);

    speed_Lower_L = 66;
    speed_Lower_R = 64;

    mecanumCar.drift_right();
    delay(100);

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
    mecanumCar.Turn_Right();
    delay(BRAKE);

    speed_Lower_L = 66;
    speed_Lower_R = 64;

    mecanum.drift_left();
    delay(100);
    
    mecanumCar.Stop();
    delay(400);
}

#define ALIGN_STABLE_TIME 100   // milliseconds

void alignRobot()
{
    Serial.println("Aligning Robot...");

    unsigned long stableStart = 0;

    // Slow speed for fine alignment
    speed_Upper_L = 35;
    speed_Lower_L = 35;
    speed_Upper_R = 30;
    speed_Lower_R = 30;

    while (true)
    {
        bool left   = digitalRead(SensorLeft);
        bool middle = digitalRead(SensorMiddle);
        bool right  = digitalRead(SensorRight);

        // =====================================
        // Perfect Alignment (Center Sensor Only)
        // =====================================
        if (!left && middle && !right)
        {
            // Start timing once alignment is achieved
            if (stableStart == 0)
            {
                stableStart = millis();
            }

            // Alignment has remained stable
            if (millis() - stableStart >= ALIGN_STABLE_TIME)
            {
                mecanumCar.Stop();

                Serial.println("Robot Aligned");

                delay(100);

                return;
            }
        }
        else
        {
            // Lost alignment -> restart timer
            stableStart = 0;
            // =====================================
            // Robot shifted to the RIGHT
            // (Line appears under LEFT sensor)
            // =====================================
            if (left)
            {
                mecanumCar.drift_left();
            }
            // =====================================
            // Robot shifted to the LEFT
            // (Line appears under RIGHT sensor)
            // =====================================
            else if (right)
            {
                mecanumCar.drift_right();
            }
            // =====================================
            // Lost Line
            // =====================================
            else
            {
                mecanumCar.Stop();
            }
        }

        delay(10);
    }
}

bool verifyAlignment()
{
    int goodReadings = 0;

    while(goodReadings < 10)
    {
        bool left   = digitalRead(SensorLeft);
        bool middle = digitalRead(SensorMiddle);
        bool right  = digitalRead(SensorRight);

        if(!left && middle && !right)
            goodReadings++;
        else
            goodReadings = 0;

        delay(10);
    }

    return true;
}

void dropoff(int target)
{
    int counter = 0;
    bool onJunction = false;

    Serial.println("===========================");
    Serial.println("DROP-OFF MISSION");
    Serial.println("===========================");

    // ----------------------------------
    // Driving Speed
    // ----------------------------------

    speed_Upper_L = 40;
    speed_Lower_L = 40;
    speed_Upper_R = 40;
    speed_Lower_R = 40;

    // ----------------------------------
    // Move Until Target Junction
    // ----------------------------------

    while(counter < target)
    {
        bool left   = digitalRead(SensorLeft);
        bool middle = digitalRead(SensorMiddle);
        bool right  = digitalRead(SensorRight);

        // -----------------------------
        // Junction Detection
        // -----------------------------

        if(left && middle && right)
        {
            if(!onJunction)
            {
                counter++;

                Serial.print("Junction : ");
                Serial.println(counter);

                onJunction = true;

                if(counter >= target)
                {
                    mecanumCar.Stop();
                    break;
                }
            }

            mecanumCar.Advance();
        }
        else
        {
            onJunction = false;

            // -----------------------------
            // Line Following
            // -----------------------------

            if(!left && middle && !right)
            {
                mecanumCar.Advance();
            }
            else if(left)
            {
                mecanumCar.Turn_Left();
            }
            else if(right)
            {
                mecanumCar.Turn_Right();
            }
            else
            {
                mecanumCar.Advance();
            }
        }

        delay(10);
    }

    // ----------------------------------
    // Fine Alignment
    // ----------------------------------

    Serial.println("Aligning...");

    alignRobot();
    verifyAlignment();

    delay(100);

    // ----------------------------------
    // Open Gripper
    // ----------------------------------

    Serial.println("Dropping Object");

    for(int pos = 80; pos >= 7; pos--)
    {
        myservo.write(pos);
        delay(20);
    }

    delay(500);

    Serial.println("===========================");
    Serial.println("DROP-OFF COMPLETE");
    Serial.println("===========================");
}
