# STM32 Autonomous Emergency Braking

<img width="1600" height="1101" alt="image" src="https://github.com/user-attachments/assets/e3e766ed-456e-48a3-be51-9964f34e627d" />

Bare-metal emergency braking system on two STM32F446RE Nucleo boards that communicate over CAN. All drivers are written at register level, without HAL.

## How it works

ECU1 (sensor node) reads the distance from a VL53L0X sensor over I2C, shows it on an OLED display and sends it to ECU2 over CAN.

ECU2 (actuator node) receives the distance and decides the state:
- SAFE: green LED, brake released
- WARNING (under 50 cm): yellow LED and buzzer
- DANGER (under 20 cm): red LED, buzzer and the servo applies the brake
- LINK LOST (no message from ECU1 for 300 ms): the brake is applied as a fail-safe

## Drivers written from scratch

CAN, I2C, SPI, PWM, GPIO, SysTick, UART and the SSD1306 OLED display. The VL53L0X driver is a C port of the Pololu Arduino library.

## Hardware

2 STM32F446RE Nucleo boards, 2 CAN transceivers, a VL53L0X distance sensor, 2 SSD1306 OLED displays, an SG90 servo, 3 LEDs and a buzzer.

## In progress

Restructuring the project into AUTOSAR-style layers.
