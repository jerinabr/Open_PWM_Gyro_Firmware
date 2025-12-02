![Static Badge](https://img.shields.io/badge/STATUS-WIP-b388eb?style=for-the-badge&logoSize=auto)

# Overview
Firmware for the Open_PWM_Gyro

The Open_PWM_Gyro board design can be found [here](https://github.com/jerinabr/Open_PWM_Gyro#)

This gyro will have a few modes including but not limited to:
- Off
  - Directly pass the RX output to the channels without any stabilization input
- Stabilize
  - Counteract any unwanted rotational forces that act on the plane due to wind or aircraft instability
- Angle Limit
  - Limit the roll and pitch angles of the aircraft and return to level flight when the pitch and roll inputs are centered

Each mode will be configrable via USB with an app that I need to make lol

The data input to the gyro is via IBUS or CRSF through the top (SER) header

# Design
The hardware interfaces in this firmware are written with as little HAL and LL usage as possible

This is for two main reasons:
  1. This is my first project with a "bare" microcontroller instead of the typical dev board and I've never used an STM32 before so I want to get familiar with the underlying hardware
  2. Unlike a company, I have no deadlines so I don't need to speed up my workflow by using HAL

# Task List
## Hardware Interfaces
- [ ] Create ICM-42605 library
  - [ ] Create SPI driver
  - [ ] Create ICM-42605 driver
- [ ] Create PWM library for channels
  - [ ] Create timer driver
  - [ ] Create PWM driver
- [x] Create receiver serial library
  - [x] Create UART driver
  - [x] Create serial decoder for different RX protocols
    - [x] Create IBUS decoder
    - [ ] Create CRSF decoder
- [ ] Create USB library for device configuration
  - [ ] Implement DFU
  - [ ] Create configuration registers
- [ ] Create flash library to store configuration
  - [ ] Create flash driver
## Data Processing
- [ ] Add sensor fusion library/algorithm
- [ ] Implement gyro modes
  - [ ] Implement gyro off
  - [ ] Implement gyro stabilize
  - [ ] Implement gyro angle limit
- [ ] Think of more stuff to implement