# TEL_RobotControl Repository README

This repository contains the firmware for a robot control system built on Arduino Mega, designed for a competition involving autonomous navigation and object manipulation missions.

## Overview

The system implements a polling-based architecture where the main controller (Arduino Mega) receives commands from a Jetson Nano for high-level navigation and executes mission-specific tasks using sensors and actuators.

## Architecture

### Main Components

**Communication System** (`Communicate` class)
- Manages serial communication with three devices:
  - Jetson Nano (Serial2, 115200 baud) - receives movement and action commands
  - JY61 IMU (Serial3, 115200 baud) - provides orientation data
  - Control Mega (motor controller) - sends processed commands 

**Event Processing**
- Uses callback functions for handling incoming data
- Three main callbacks:
  - `recieve_jetson_nano()` - processes movement commands
  - `recieve_jetson_nano_action()` - executes mission actions
  - `recieve_jy61_now_angle()` - updates current orientation 

### Command Protocol

**Movement Commands** (from Jetson Nano)<cite />
```
Format: [DSSTAAAVVW
- D: direction code (single char)
- SS: speed (2 digits)
- TAA: target angle (3 digits)
- VV: servo0 angle (3 digits)
- W: servo1 angle (3 digits)
```

**Action Commands** (from Jetson Nano)<cite />
```
Format: <X
- X: action code (single char)
```

### Mission Actions

The system supports various mission-specific actions triggered by action codes:

| Code | Function |
|------|----------|
| `a` | Activate suction pump |
| `b` | Deactivate suction pump |
| `c` | Enable storage output |
| `d` | Disable storage output |
| `e` | Position to start box (front) |
| `f` | Position to start box (back) |
| `g` | Navigate to target from front |
| `s` | Disable IMU correction |
| `t` | Enable IMU correction |
| `z` | Rotate to target angle |

### Debug Interface

Serial monitor commands for manual testing:
- `a-f`: Positioning commands
- `g`: Toggle debug sensor output
- `h-j`: Target navigation
- `k`: Execute missions 2 and 3
- `l`: Execute mission 3
- `z`: Angle correction

## Main Loop

The main loop continuously polls serial buffers and processes incoming commands:

```cpp
void loop() {
    // Debug output (if enabled)
    // Serial monitor command processing
    commu.read_serial_buffer();  // Poll all serial interfaces
}
```

**Note**: The loop operates on a best-effort polling basis with no guaranteed timing.<cite /> Mission execution blocks the main loop until completion, which can take several seconds for complex maneuvers.<cite />

## Notes

This is a competition robot firmware with mission-specific logic hardcoded for autonomous navigation tasks.<cite /> The system uses a polling architecture rather than interrupts, meaning command processing is sequential and blocking.<cite /> The codebase includes Chinese comments indicating this was developed for a Taiwanese robotics competition. [14](#0-13) 

Wiki pages you might want to explore:
- [Main Loop and Event Processing (Bacon9629/TEL_RobotControl)](/wiki/Bacon9629/TEL_RobotControl#4.2)

