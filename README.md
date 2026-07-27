# 🤖 Mecanum Car Calibration

> Research & Development project for calibrating a mecanum-wheeled mobile robot using a grid map to achieve accurate and repeatable autonomous movement.

---

## 📖 Overview

**Mecanum Car Calibration** is an R&D project that focuses on calibrating a mecanum-wheeled mobile robot by testing its movements on a predefined **grid map**. The robot follows the grid paths while individual wheel PWM values are adjusted to minimize movement errors and improve navigation accuracy.

Each movement is repeatedly tested, analyzed, and calibrated until the robot can follow the intended path consistently.

---

## 🎯 Objectives

- 📌 Validate mecanum wheel movement on a grid map.
- 📌 Identify movement offsets for each motion.
- 📌 Calibrate individual motor PWM values.
- 📌 Improve movement repeatability.
- 📌 Reduce accumulated navigation errors.
- 📌 Build a reference for future autonomous robot development.

---

## 🗺️ Test Environment

All experiments are performed using a **grid map**.

<div align="center">

**gridmap.jpg**

Robot movements are evaluated based on how accurately they follow the grid.

</div>

---

## 🚗 Movement Calibration

The following movements are calibrated throughout this project:

- ⬆️ Forward
- ⬇️ Backward
- ↩️ Turn Left (90°)
- ↪️ Turn Right (90°)
- 🔄 Turn 180°

Each movement is tested independently to determine the appropriate PWM compensation for every motor.

---

## 🧪 Calibration Procedure

For every movement:

1. ▶️ Execute the movement.
2. 👀 Observe the robot trajectory.
3. 📏 Measure any movement offset.
4. 🔧 Adjust the corresponding wheel PWM values.
5. 🔁 Repeat the experiment.
6. ✅ Record the final calibrated PWM values.

---

## 📂 Project Structure

```text
MecanumCarCalibration/
│
├── README.md
│
├── src/
│   ├── mecanum_robot_basic_movement.cpp
│
├── calibration/
│   ├── Mecanum_Movement_Calibration_RnD_Blank_Template.pdf
│   ├── Turn_Calibration_RnD_Blank_Template.pdf
│
├── results/
│   ├── 1)forwardmovement.png
│   └── 2)backwardmovement.png
│   ├── 3)turn180(left_direction).png
│   └── 4)turn180(right_direction).png
│
└── gridmap.jpg
```

---

## 📑 Experiment Template

Every calibration experiment follows the same documentation format.

The experiment template can be found in:

```text
calibration/
└── blank_Template.md
│   ├── Mecanum_Movement_Calibration_RnD_Blank_Template.pdf
│   ├── Turn_Calibration_RnD_Blank_Template.pdf
```

Use this template to record:

- Experiment objective
- Initial PWM values
- Test observations
- Offset direction
- PWM adjustments
- Final calibrated values
- Conclusions

---

## 📊 Results

The calibration results for every experiment are stored inside the **results** folder.

```text
results/
│   ├── 1)forwardmovement.png
│   └── 2)backwardmovement.png
│   ├── 3)turn180(left_direction).png
│   └── 4)turn180(right_direction).png
```
---

## 🚀 Future Improvements

- 🎯 PID-based heading correction
- 🧭 IMU-assisted orientation control
- ⚙️ Wheel encoder feedback
- 📷 Vision-based localization
- 🤖 Automatic PWM optimization
- 📈 Automated calibration algorithm

---

## 👨‍💻 Author

**Hajim5**
**syamilshafie22-tech**
**SyukranSJ**

Research & Development on Mecanum Wheel Motion Calibration

---
⭐ This repository documents the calibration process of a mecanum mobile robot and serves as a reference for future robotics research and autonomous navigation projects.
