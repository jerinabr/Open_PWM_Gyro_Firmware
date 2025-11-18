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
- [ ] Create SPI library for ICM-42605
  - [ ] Write SPI initialization
  - [ ] Write ICM-42605 driver
- [ ] Create PWM library for channels
  - [ ] Write timer initialization
  - [ ] Write PWM driver
- [ ] Create UART library for RX serial input
  - [ ] Write UART initialization
  - [ ] Write UART decoder for different RX protocols
    - [ ] Write IBUS decoder
    - [ ] Write SBUS decoder
    - [ ] Write CRSF decoder
- [ ] Create USB library for device configuration
  - [ ] Implement DFU
  - [ ] Create configuration registers
## Data Processing
- [ ] Add sensor fusion library/algorithm
- [ ] Implement gyro modes
  - [ ] Implement gyro off
  - [ ] Implement gyro stabilize
  - [ ] Implement gyro angle limit
- [ ] Think of more stuff to implement