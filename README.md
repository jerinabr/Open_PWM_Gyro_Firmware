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

The data input to the gyro is via IBUS, SBUS, or CRSF through the top (SER) header

# Task List
## Hardware Interfaces
- [ ] Create ICM-42605 library
  - [ ] Create SPI driver
  - [ ] Create ICM-42605 driver
- [ ] Create PWM library for channels
  - [ ] Create timer driver
  - [ ] Create PWM driver
- [ ] Create receiver serial library
  - [x] Create UART driver
  - [ ] Create serial decoder for different RX protocols
    - [ ] Create IBUS decoder
    - [ ] Create SBUS decoder
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