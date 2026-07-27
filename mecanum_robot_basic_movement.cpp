//=========================================================
// Robot Movement Demonstration
//=========================================================
//
// This program demonstrates the basic robot movements:
//
// 1. Move Forward
// 2. Move Backward
// 3. Turn Left 90°
// 4. Turn Right 90°
// 5. Turn Left 180°
// 6. Turn Right 180°
// 7. Grab Object
// 8. Drop Object
//
// The code is intended for movement testing only.
// No mission or autonomous navigation logic is included.
//
// Hardware:
// - Mecanum Robot
// - 3 IR Line Sensors
// - Ultrasonic Sensor
// - Servo Gripper
// - Color Sensor
//=========================================================
// LIBRARIES
//=========================================================
#include <Arduino.h>
#include <MecanumCar_v2.h>
#include <IRremote.h>
#include <Servo.h>
//=========================================================
// OBJECTS
//=========================================================
mecanumCar mecanumCar(3, 2);
Servo myservo;
IRrecv irrecv(A3);
decode_results results;
//=========================================================
// PIN DEFINITIONS
//=========================================================
// Line Tracking Sensors
#define SENSOR_LEFT      A0
#define SENSOR_MIDDLE    A1
#define SENSOR_RIGHT     A2
// Color Sensor (TCS3200)
#define S0               4
#define S1               5
#define S2               7
#define S3               6
#define SENSOR_OUT       8
// Servo Motor
#define SERVO_PIN        9
// Ultrasonic Sensor
#define TRIG_PIN         12
#define ECHO_PIN         13
// IR Receiver
#define RECV_PIN         A3
//=========================================================
// ROBOT SETTINGS
//=========================================================
// Turning calibration (milliseconds)
#define TURN_90_TIME           300
#define TURN_LEFT_180_TIME     460
#define TURN_RIGHT_180_TIME    420
// Motion settings
#define FOLLOW_SPEED           40
#define TURN_SPEED             60
#define BRAKE_TIME             30
//=========================================================
// MOTOR SPEED VARIABLES
//=========================================================
extern uint8_t speed_Upper_L;
extern uint8_t speed_Lower_L;
extern uint8_t speed_Upper_R;
extern uint8_t speed_Lower_R;
//=========================================================
// REMOTE CONTROL BUTTONS
//=========================================================
const unsigned long BTN_OK      = 0xFF02FD;
const unsigned long BTN_RED     = 0xFF30CF;
const unsigned long BTN_YELLOW  = 0xFFB04F;
//=========================================================
// COLOR SENSOR VARIABLES
//=========================================================
int redFrequency = 0;
int greenFrequency = 0;
int blueFrequency = 0;
//=========================================================
// SYSTEM STATUS
//=========================================================
bool missionRunning = false;
//=========================================================
// FUNCTION PROTOTYPES
//=========================================================
// Move forward while following the line.
// Stops after reaching the specified number of intersections.
void forward(int target);
// Move backward for a specified duration (milliseconds)
// while maintaining line alignment.
void moveback(int durationMs);
// Rotate robot 90° to the left.
void turnLeft90();
// Rotate robot 90° to the right.
void turnRight90();
// Rotate robot 180° clockwise.
void turnRight180();
// Rotate robot 180° counter-clockwise.
void turnLeft180();
// Detect and grab an object with the requested color.
// Returns true if the correct object is successfully grabbed.
bool grab(String targetColor);
// Move to the drop-off location and release the object.
void dropoff(int target);
//=========================================================
// setup()
//=========================================================
// Description:
//   Initializes all hardware components before the robot
//   starts running.
//
// Initializes:
//   - Serial communication
//   - Mecanum robot
//   - Line tracking sensors
//   - Servo gripper
//   - Color sensor
//   - Ultrasonic sensor
//   - IR remote receiver
//=========================================================
void setup()
{
    Serial.begin(9600);
    // Initialize robot
    mecanumCar.Init();
    // Configure line tracking sensors
    pinMode(SENSOR_LEFT, INPUT);
    pinMode(SENSOR_MIDDLE, INPUT);
    pinMode(SENSOR_RIGHT, INPUT);
    // Initialize servo gripper
    myservo.attach(SERVO_PIN);
    myservo.write(7);           // Open gripper
    // Configure color sensor
    pinMode(S0, OUTPUT);
    pinMode(S1, OUTPUT);
    pinMode(S2, OUTPUT);
    pinMode(S3, OUTPUT);
    pinMode(SENSOR_OUT, INPUT);
    // Set color sensor frequency scaling (20%)
    digitalWrite(S0, HIGH);
    digitalWrite(S1, LOW);
    // Configure ultrasonic sensor
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    // Initialize IR remote receiver
    irrecv.enableIRIn();
    Serial.println("Robot Ready");
}
//=========================================================
// loop()
//=========================================================
// Description:
//   Waits for commands from the IR remote.
//
// Button Functions:
//   BTN_OK      : Execute selected movement.
//   BTN_RED     : Execute selected movement.
//   BTN_YELLOW  : Execute selected movement.
//
// Note:
//   Replace the placeholder comments with the movement
//   function you wish to demonstrate.
//=========================================================
void loop()
{
    if (!irrecv.decode(&results))
        return;
    if (missionRunning)
    {
        irrecv.resume();
        return;
    }
    missionRunning = true;
    switch (results.value)
    {
        case BTN_OK:
            // Example:
            // forward(3);
            break;
        case BTN_RED:
            // Example:
            // turnLeft180();
            break;
        case BTN_YELLOW:
            // Example:
            // grab("RED");
            break;
        default:
            break;
    }
    missionRunning = false;
    irrecv.resume();
}
//=========================================================
// getDistanceCM()
//=========================================================
// Description:
//   Measures the distance to the nearest object using the
//   HC-SR04 ultrasonic sensor.
//
// Return:
//   Distance in centimeters.
//
// Note:
//   Returns 999 cm if no object is detected within the
//   timeout period.
//=========================================================
float getDistanceCM()
{
    // Send trigger pulse
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    // Measure echo duration
    long duration = pulseIn(ECHO_PIN, HIGH, 30000);
    // No echo detected
    if (duration == 0)
        return 999.0;
    // Convert time to distance (cm)
    return duration * 0.034f / 2.0f;
}
//=========================================================
// forward()
//=========================================================
// Description:
//   Performs line following until the robot reaches the
//   requested number of intersections.
//
// Parameter:
//   target
//      Number of intersections to travel.
//
// Return:
//   None.
//=========================================================
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
//=========================================================
// moveback()
//=========================================================
// Description:
//   Moves the robot backward while using the line sensors
//   for alignment.
//
// Parameter:
//   durationMs
//      Backward movement time in milliseconds.
//
// Return:
//   None.
//=========================================================
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
//=========================================================
// grab()
//=========================================================
// Description:
//   Searches for an object using the ultrasonic sensor.
//   Approaches the object while reducing speed based on
//   distance, detects its color, and closes the gripper if
//   the detected color matches the requested color.
//
// Parameter:
//   targetColor
//      Desired object color.
//
// Return:
//   true  -> Correct object successfully grabbed.
//   false -> Object not found or color mismatch.
//=========================================================
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
//=========================================================
// turnRight180()
//=========================================================
// Description:
//   Rotates the robot approximately 180° clockwise,
//   aligns with the line using the sensors, and performs
//   a small drift correction.
//
// Return:
//   None.
//=========================================================
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
//=========================================================
// turnLeft180()
//=========================================================
// Description:
//   Rotates the robot approximately 180° counter-clockwise,
//   aligns with the line using the sensors, and performs
//   a small drift correction.
//
// Return:
//   None.
//=========================================================
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
//=========================================================
// dropoff()
//=========================================================
// Description:
//   Moves to the requested drop-off point, opens the
//   gripper to release the object, and backs away from
//   the drop zone.
//
// Parameter:
//   target
//      Number of intersections before releasing the object.
//
// Return:
//   None.
//=========================================================
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
}
