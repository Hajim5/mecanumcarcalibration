#include "mecanum.h"
#include <Arduino.h>
#include <MecanumCar_v2.h>

// extern variables
extern mecanumCar mecanumCar;
extern Servo myservo;
extern mecanumCar mecanumCar;
extern uint8_t speed_Upper_L;
extern uint8_t speed_Lower_L;
extern uint8_t speed_Upper_R;
extern uint8_t speed_Lower_R;

void moveForwardIntersections(int target)
{
    int counter = 0;
    bool onJunction = false;

    speed_Upper_L = 50;
    speed_Lower_L = 50;
    speed_Upper_R = 40;
    speed_Lower_R = 40;

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
    delay(800);
}

void turnRight90()
{
    //dulu 60
    speed_Upper_L = 65;
    speed_Lower_L = 65;
    speed_Upper_R = 65;
    speed_Lower_R = 65;

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
    delay(800);
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
    delay(800);
}

void turn180_R()
{
    turnRight90();
    turnRight90();
}

void turn180_RO()
{
    turnRight90_O();
    turnRight90_O();
}

void moveback(int target)
{
    int counter = 0;
    bool onJunction = false;
    speed_Upper_L = 60;
    speed_Lower_L = 45;
    speed_Upper_R = 47;
    speed_Lower_R = 43;

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
    delay(800);
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
    delay(800);
}

void turnLeft90_O()
{
    speed_Upper_L = 69;
    speed_Lower_L = 69;
    speed_Upper_R = 69;
    speed_Lower_R = 69;

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
    delay(800);
}

void turn180_L()
{
    turnLeft90();
    turnLeft90();
}

void turn180_LO()
{
    turnLeft90_O();
    turnLeft90_O();
}

void leftmove(int target)
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
    delay(800);
}


void rightmove(int target)
{
 int counter = 0;
    bool onMarker = false;
    //speed_Upper_L = 45;
    //speed_Lower_L = 45;
    //speed_Upper_R = 60;
    //speed_Lower_R = 50;
    speed_Upper_L = 55;
    speed_Lower_L = 60;
    speed_Upper_R = 55;
    speed_Lower_R = 45;

    while(counter < target)
    {
        bool left   = digitalRead(SensorLeft);
        bool middle = digitalRead(SensorMiddle);
        bool right  = digitalRead(SensorRight);

        mecanumCar.R_Move();

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
    delay(800);
}

bool grab(String targetColor)
{
    // Open gripper
    myservo.write(7);
    delay(300);

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
        delay(1000);

        return true;
    }
    else
    {
        Serial.println("Wrong color!");

        myservo.write(7);      // Keep gripper open
        delay(300);

        return false;
    }
}
