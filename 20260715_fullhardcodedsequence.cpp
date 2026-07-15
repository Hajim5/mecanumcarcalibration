void fullsequence()
{
    // Leave the starting home
    forward(4);
    // Pick in sequence
    searchRed();
    searchBlue();
    searchYellow();
    // Mission complete
    dropoff(5);      // Return to 1st Home
}

void searchRed()
{
    // --------------------------
    // Location 2
    // --------------------------
    forward(1);
    if (grab("RED"))
    {
        rotate180();
        forward(2);
        dropoff(5);
        return;
    }
    rotate180();
    forward(2);
    rotate180();
    // --------------------------
    // Location 1
    // --------------------------
    moveright(2);
    forward(1);
    if (grab("RED"))
    {
        rotate180();
        moveright(2);
        forward(2);
        dropoff(5);
        return;
    }
    rotate180();
    moveright(2);
    forward(2);
    rotate180();
    // --------------------------
    // Location 3
    // --------------------------
    moveleft(2);
    forward(1);
    if (grab("RED"))
    {
        rotate180();
        moveleft(2);
        forward(2);
        dropoff(5);
        return;
    }
    rotate180();
    moveleft(2);
    forward(2);
}

void searchBlue()
{
    forward(1);
    if (grab("BLUE"))
    {
        rotate180();
        forward(2);
        dropoff(5);
        return;
    }
    rotate180();
    forward(2);
    rotate180();
    moveright(2);
    forward(1);
    if (grab("BLUE"))
    {
        rotate180();
        moveright(2);
        forward(2);
        dropoff(5);
        return;
    }

    rotate180();
    moveright(2);
    forward(2);
    rotate180();
    moveleft(2);
    forward(1);
    if (grab("BLUE"))
    {
        rotate180();
        moveleft(2);
        forward(2);
        dropoff(5);
        return;
    }
    rotate180();
    moveleft(2);
    forward(2);
}

void searchYellow()
{
    forward(1);
    if (grab("YELLOW"))
    {
        rotate180();
        forward(2);
        dropoff(5);
        return;
    }
    rotate180();
    forward(2);
    rotate180();
    moveright(2);
    forward(1);
    if (grab("YELLOW"))
    {
        rotate180();
        moveright(2);
        forward(2);
        dropoff(5);
        return;
    }
    rotate180();
    moveright(2);
    forward(2);
    rotate180();
    moveleft(2);
    forward(1);
    if (grab("YELLOW"))
    {
        rotate180();
        moveleft(2);
        forward(2);
        dropoff(5);
        return;
    }
    rotate180();
    moveleft(2);
    forward(2);
}
