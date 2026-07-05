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