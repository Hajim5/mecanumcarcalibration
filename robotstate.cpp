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
#define TURN_BLIND_MS_LEFT180  450
#define TURN_BLIND_MS_RIGHT180 400
#define TURN_SPEED           60
#define FOLLOW_SPEED         40
#define BRAKE                30
#define STOP_SENSOR SensorMiddle

// =====================
// STATE MACHINE SETUP
// =====================
enum State {
  STATE_INIT,   // Setup & hardware sanity checks
  STATE_AUDIT,  // Mechanical self-diagnostic movement checks
  STATE_IDLE,   // Standby; waiting for user button command
  STATE_BUSY    // Actively executing driving sequences
};

State currentState = STATE_INIT;
State previousState = STATE_INIT;

enum ActiveSequence {
  SEQ_NONE,
  SEQ_OK,
  SEQ_0,
  SEQ_1
};
ActiveSequence selectedSequence = SEQ_NONE;

// Status LED Timers
unsigned long ledTimer = 0;
bool ledBlinkState = false;

// =====================
// MOTOR SPEED
// =====================
extern uint8_t speed_Upper_L;
extern uint8_t speed_Lower_L;
extern uint8_t speed_Upper_R;
extern uint8_t speed_Lower_R;

const unsigned long OK_BUTTON = 0xFF02FD;  // OK -> Trigger sequence_OK
const unsigned long NO0_BUTTON = 0xFF30CF; // #4 -> Trigger sequence_0
const unsigned long NO1_BUTTON = 0xFFB04F; // #3 -> Trigger sequence_1

IRrecv irrecv(RECV_PIN);
decode_results results;

int redFrequency;
int greenFrequency;
int blueFrequency;
int diffColor  = 200;

// =====================
// PROTOTYPES
// =====================
void checkIRRemote();
void updateLEDs();
void onStateEnter(State state);
void runInitPhase();
void runAuditPhase();
void flushIRBuffer();

void sequence_OK();
void sequence_0();
void sequence_1();

void forward(int target);
void moveback(int target);
void moveleft(int target);
void moveright(int target);
void turnLeft90(); 
void turnRight90(); 
bool grab(String targetColor);
void turnRight180(); 
void turnLeft180(); 
void dropoff(int target);
void moveleft_O(int target); 
void moveright_O(int target); 
void turnRight90_O(); 
float getDistanceCM();

// =====================
// SETUP
// =====================
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
    Serial.println("System Startup: State Machine Initialized.");
    Serial.println("Entering: STATE_INIT");
}

// =====================
// MAIN LOOP
// =====================
void loop()
{
    checkIRRemote();
    updateLEDs();

    // Catch state changes
    if (currentState != previousState)
    {
        onStateEnter(currentState);
        previousState = currentState;
    }

    // Process Active States
    switch (currentState)
    {
        case STATE_INIT:
            runInitPhase();
            currentState = STATE_AUDIT;
            break;

        case STATE_AUDIT:
            runAuditPhase();
            currentState = STATE_IDLE;
            break;

        case STATE_IDLE:
            // Sits here waiting for checkIRRemote() to select a sequence
            break;

        case STATE_BUSY:
            // Execute the selected remote control routine
            if (selectedSequence == SEQ_OK) {
                sequence_OK();
            } 
            else if (selectedSequence == SEQ_0) {
                sequence_0();
            } 
            else if (selectedSequence == SEQ_1) {
                sequence_1();
            }
            
            // Auto-return to idle once mission sequence completes
            selectedSequence = SEQ_NONE;
            currentState = STATE_IDLE;
            break;
    }
}

// =====================
// FSM SYSTEM CONTROL
// =====================
void onStateEnter(State state)
{
    switch (state)
    {
        case STATE_INIT:
            Serial.println("\n>>> Entering State: STATE_INIT <<<");
            break;

        case STATE_AUDIT:
            Serial.println("\n>>> Entering State: STATE_AUDIT (Diagnostics) <<<");
            break;

        case STATE_IDLE:
            mecanumCar.Stop();
            mecanumCar.left_led(0);
            mecanumCar.right_led(0);
            Serial.println("\n>>> Entering State: STATE_IDLE (Waiting for Command) <<<");
            break;

        case STATE_BUSY:
            mecanumCar.left_led(1);
            mecanumCar.right_led(1);
            Serial.println("\n>>> Entering State: STATE_BUSY (Running Sequence) <<<");
            break;
    }
}

void checkIRRemote()
{
    if (irrecv.decode(&results))
    {
        unsigned long code = results.value;
        Serial.print("IR Code: 0x");
        Serial.println(code, HEX);

        // We only accept start triggers if the robot is currently idle
        if (currentState == STATE_IDLE)
        {
            if (code == OK_BUTTON) {
                selectedSequence = SEQ_OK;
                currentState = STATE_BUSY;
            } 
            else if (code == NO0_BUTTON) {
                selectedSequence = SEQ_0;
                currentState = STATE_BUSY;
            } 
            else if (code == NO1_BUTTON) {
                selectedSequence = SEQ_1;
                currentState = STATE_BUSY;
            }
        }
        // Force break back to IDLE if the OK button is pressed while busy
        else if (currentState == STATE_BUSY && code == OK_BUTTON)
        {
            Serial.println("FORCE HALT RECEIVED!");
            mecanumCar.Stop();
            selectedSequence = SEQ_NONE;
            currentState = STATE_IDLE;
        }
        irrecv.resume();
    }
}

void updateLEDs()
{
    // Quick blink during diagnostic audit run
    if (currentState == STATE_AUDIT)
    {
        if (millis() - ledTimer >= 150)
        {
            ledTimer = millis();
            ledBlinkState = !ledBlinkState;
            mecanumCar.left_led(ledBlinkState);
            mecanumCar.right_led(ledBlinkState);
        }
    }
    // Slower, breathing-style standby flash when idle
    else if (currentState == STATE_IDLE)
    {
        if (millis() - ledTimer >= 750)
        {
            ledTimer = millis();
            ledBlinkState = !ledBlinkState;
            mecanumCar.left_led(ledBlinkState);
            mecanumCar.right_led(ledBlinkState);
        }
    }
}

void flushIRBuffer()
{
    Serial.println("Flushing IR Hardware Buffer...");
    delay(200);
    while (irrecv.decode(&results))
    {
        irrecv.resume();
        delay(10);
    }
    irrecv.resume();
}

// =====================
// PHASE IMPLEMENTATIONS
// =====================
void runInitPhase()
{
    Serial.println("[Init] Testing clamp physical limits...");
    myservo.write(45); // Partial close
    delay(400);
    myservo.write(7);  // Reset open
    delay(400);

    Serial.print("[Init] IR Sensor Reading L/M/R: ");
    Serial.print(digitalRead(SensorLeft));   Serial.print(" / ");
    Serial.print(digitalRead(SensorMiddle)); Serial.print(" / ");
    Serial.println(digitalRead(SensorRight));

    Serial.print("[Init] Ultrasonic Distance: ");
    Serial.print(getDistanceCM());
    Serial.println(" cm");
    
    delay(500);
}

void runAuditPhase()
{
    Serial.println("[Audit] Initiating brief chassis calibration run...");
    
    // Set a safe speed for dry motor checks
    speed_Upper_L = 40; speed_Lower_L = 40;
    speed_Upper_R = 40; speed_Lower_R = 40;

    mecanumCar.Advance();    delay(250);
    mecanumCar.Back();       delay(250);
    mecanumCar.L_Move();     delay(250);
    mecanumCar.R_Move();     delay(250);
    mecanumCar.Stop();       delay(400);

    flushIRBuffer();
    Serial.println("[Audit] Calibration sequence successfully verified!");
}

// =====================
// COMPLETED SEQUENCES
// =====================
void sequence_OK()
{
    forward(5);
    grab("BLUE");
    turnLeft180();
    forward(4);
    dropoff(3);
    delay(5000);
    turnRight180();
    forward(3);
    moveright(2);
    forward(2);
    grab("RED");
    turnLeft180();
    forward(2);
    moveleft_O(2);
    forward(4);
    dropoff(3);
    delay(5000);
    turnRight180();
    forward(3);
    moveleft(2);
    forward(2);
    grab("YELLOW");
    turnLeft180();
    forward(2);
    moveleft_O(2);
    forward(4);
    dropoff(3);
    delay(5000);
    turnRight180();
}

void sequence_0()
{
    forward(5);
    grab("BLUE");
    turnLeft180();
    forward(5);
    moveright(2); 
    grab("RED");
    turnLeft180();
    forward(2);
    moveleft_O(2);
    forward(4);
    dropoff(3);
    delay(5000);
    turnRight180();
    forward(3);
    moveleft(2);
    forward(2);
    grab("YELLOW");
    turnLeft180();
    forward(2);
    moveleft_O(2);
    forward(4);
    dropoff(3);
    delay(5000);
    turnRight180();
}

void sequence_1()
{
    forward(5);
    bool targetFound = grab("BLUE"); 
    
    if (!targetFound) {
        moveback(2); 
        moveleft(2); 
        grab("BLUE"); 
        turnLeft180();
        forward(2);
        moveleft_O(2); 
        forward(4);
        dropoff(3);
        delay(5000);
        turnRight180();
    }

    forward(3);
    moveright(2);
    forward(2);
    grab("RED");
    turnLeft180();
    forward(2);
    moveleft_O(2);
    forward(4);
    dropoff(3);
    delay(5000);
    turnRight180();

    forward(4);
    grab("YELLOW");
    turnLeft180();
    forward(2);
    moveleft_O(2);
    forward(4);
    dropoff(3);
    delay(5000);
    turnRight180();
}

// =====================
// MOVEMENT FUNCTIONS
// =====================
float getDistanceCM() 
{
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    long duration = pulseIn(ECHO_PIN, HIGH, 30000); 
    if (duration == 0) return 999;                  
    float distance = duration * 0.034f / 2.0f;           
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
        // Fail-safe check during execution loop to exit instantly
        checkIRRemote();
        if (currentState == STATE_IDLE) return;

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

void moveback(int target)
{
    int counter = 0;
    bool onJunction = false;
    speed_Upper_L = 40;
    speed_Lower_L = 45;
    speed_Upper_R = 30;
    speed_Lower_R = 35;
    while(counter < target)
    {
        checkIRRemote();
        if (currentState == STATE_IDLE) return;

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
        checkIRRemote();
        if (currentState == STATE_IDLE) return;

        bool left   = digitalRead(SensorLeft);
        bool middle = digitalRead(SensorMiddle);
        bool right  = digitalRead(SensorRight);
        mecanumCar.L_Move();
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

void moveright(int target)
{
    int counter = 0;
    bool onMarker = false;
    speed_Upper_L = 66;
    speed_Lower_L = 41;
    speed_Upper_R = 67;
    speed_Lower_R = 49;
    while(counter < target)
    {
        checkIRRemote();
        if (currentState == STATE_IDLE) return;

        bool left   = digitalRead(SensorLeft);
        bool middle = digitalRead(SensorMiddle);
        bool right  = digitalRead(SensorRight);
        mecanumCar.R_Move();
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

bool grab(String targetColor)
{
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
    while (true)
    {
        checkIRRemote();
        if (currentState == STATE_IDLE) return false;

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
        
        digitalWrite(TRIG_PIN, LOW);
        delayMicroseconds(2);
        digitalWrite(TRIG_PIN, HIGH);
        delayMicroseconds(10);
        digitalWrite(TRIG_PIN, LOW);
        long duration = pulseIn(ECHO_PIN, HIGH);
        if (duration == 0)
            continue;
        long distance = duration * 0.034 / 2;
        
        if (distance <= 7)
        {
            speed_Upper_L = NORMAL_UL * 0.9 * 0.9;
            speed_Lower_L = NORMAL_LL * 0.9 * 0.9;
            speed_Upper_R = NORMAL_UR * 0.9 * 0.9;
            speed_Lower_R = NORMAL_LR * 0.9 * 0.9;
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

    if (redFrequency > 4000 && greenFrequency > 4000 && blueFrequency > 4000)
    {
        detectColour = "UNKNOWN";
    }
    else if (blueFrequency < redFrequency && blueFrequency < greenFrequency)
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
        myservo.write(60);   
        delay(400);
        return true;
    }
    else
    {
        myservo.write(7);    
        delay(400);
        return false;
    }
}

void turnRight180()
{
    speed_Upper_L = 63;
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
    mecanumCar.Turn_Left(); 
    delay(BRAKE);
    mecanumCar.Stop(); 
    delay(100);
    speed_Lower_L = 66;
    speed_Lower_R = 66;
    mecanumCar.drift_left();
    delay(200);   
    mecanumCar.Stop();
    delay(600);
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
    delay(350);   
    mecanumCar.Stop();
    delay(600);
}

void dropoff(int target)
{
    forward(target);
    myservo.write(7); // Open gripper
    delay(400);
    moveback(target);
}

void moveleft_O(int target) { moveleft(target); }
void moveright_O(int target) { moveright(target); }
