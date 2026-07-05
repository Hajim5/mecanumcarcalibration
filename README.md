# MecanumCarCalibration

## Overview

**MecanumCarCalibration** is an R&D project to calibrate a mecanum-wheeled mobile robot by analyzing movement offsets and tuning individual wheel PWM values. Each movement is tested against its expected trajectory, and wheel speeds are adjusted to compensate for drift, resulting in more accurate and repeatable autonomous navigation.

---

## Objectives

- Verify the movement theory of a mecanum drive through practical testing.
- Identify movement offsets in each direction.
- Calibrate individual motor PWM values.
- Improve movement accuracy for grid-based navigation.
- Reduce accumulated positioning errors.

---

## Movement Calibration

The following movements are calibrated:

- Forward
- Backward
- Left Strafe
- Right Strafe
- Forward Left
- Forward Right
- Backward Left
- Backward Right
- Rotate Left (CCW)
- Rotate Right (CW)

---

## Calibration Method

1. Execute a predefined movement.
2. Observe the robot trajectory.
3. Measure the movement offset.
4. Determine which wheel(s) require compensation.
5. Adjust the PWM values.
6. Repeat until the robot follows the intended path.

---

## Example

### Forward Movement

**Expected Result**

```
Start
  │
  │
  │
  ▼
Finish
```

**Actual Result**

```
Start
  │
   \
    \
     ▼
   Finish
```

The robot drifts to the **right**.

Possible correction:

- Increase PWM of the weaker wheel(s), or
- Reduce PWM of the stronger wheel(s),

until the robot moves straight.

---

## Repository Structure

```
MecanumCarCalibration/
│
├── README.md
├── src/
│   ├── main.cpp
│   ├── mecanum.cpp
│   └── mecanum.h
│
├── calibration/
│   ├── forward.md
│   ├── backward.md
│   ├── left_strafe.md
│   ├── right_strafe.md
│   └── rotation.md
│
└── results/
    ├── images/
    └── videos/
```

---

## Future Improvements

- Automatic PID-based calibration
- IMU-assisted heading correction
- Encoder-based distance calibration
- Vision-assisted localization
- Automatic PWM tuning algorithm

---

## Author

Ahmad Hazim

Research & Development on Mecanum Wheel Motion Calibration
