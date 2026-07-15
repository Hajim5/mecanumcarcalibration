//additional code
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
        turnRight180();
        forward(2);
        return;
    }
    else {moveback(900);
    turnRight180();
    forward(2);
    turnRight180();}
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
        turnRight180();
        forward(2);
        return;
    }
    else {moveback(900);
    turnRight180();
    moveright(2);
    forward(2);
    turnRight180();}
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
        turnRight180();
        forward(2);
        return;

    }
    else {moveback(900);
    turnRight180();
    moveleft(2);
    forward(2);
    turnRight180();}
}

void searchBlue()
{
    forward(1);
    if (grab("BLUE"))
    {
        turnLeft180();
        forward(2);
        dropoff(5);
        turnRight180();
        forward(2);
        return;

    }
    else {moveback(900);
    turnRight180();
    forward(2);
    turnRight180();}
    moveright(2);
    forward(1);
    if (grab("BLUE"))
    {
        turnLeft180();
        moveright(2);
        forward(2);
        dropoff(5);
        turnRight180();
        forward(2);
        return;

    }
    else {moveback(900);
    turnRight180();
    moveright(2);
    forward(2);
    turnRight180();}
    moveleft(2);
    forward(1);
    if (grab("BLUE"))
    {
        turnLeft180();
        moveleft(2);
        forward(2);
        dropoff(5);
        turnRight180();
        forward(2);
        return;

    }
    else {moveback(900);
    turnRight180();
    moveleft(2);
    forward(2);
    turnRight180();}
}

void searchYellow()
{
    forward(1);
    if (grab("YELLOW"))
    {
        turnLeft180();
        forward(2);
        dropoff(5);
        turnRight180();
        forward(2);
        return;

    }
    else {moveback(900);
    turnRight180();
    forward(2);
    turnRight180();}
    moveright(2);
    forward(1);
    if (grab("YELLOW"))
    {
        turnLeft180();
        moveright(2);
        forward(2);
        dropoff(5);
        turnRight180();
        forward(2);
        return;

    }
    else {moveback(900);
    turnRight180();
    moveright(2);
    forward(2);
    turnRight180();}
    moveleft(2);
    forward(1);
    if (grab("YELLOW"))
    {
        turnLeft180();
        moveleft(2);
        forward(2);
        dropoff(5);
        turnRight180();
        forward(2);
        return;

    }
    else {moveback(900);
    turnRight180();
    moveleft(2);
    forward(2);
    turnRight180();}
}