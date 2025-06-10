# Amateur-GPS
Hyperbolic Navigation system using NRF24lo1 and Arduino Nano

## Components and Connections

The table below summarizes the pin connections for each component with Arduino Nano:

| **Device**         | **Pin Name**    | **Pin Number** |
|--------------------|-----------------|----------------|
| **NRF24L01**       | IRQ pin         | 2              |
|                    | CE pin          | 10             |
|                    | CSN pin         | 9              |
|                    | SCK pin         | 13             |
|                    | MISO pin        | 12             |
|                    | MOSI pin        | 11             |
| **TFT Display**    | Reset pin       | -1             |
|                    | CSN pin         | 4              |
|                    | Data Pin        | 4              |
| **Rotary Encoder** | Push Button     | 3              |
|                    | Output A        | A0             |
|                    | Output B        | A1             |

## Usage

- **NRF24L01**: This is a popular 2.4 GHz wireless module used for communication in Arduino-based projects. It connects to the Arduino via SPI.
- **TFT Display**: A small color display used to show information. The connections for the TFT are mostly for control and data transmission.
- **Rotary Encoder**: This device is used to detect the rotation of a knob. It has two output pins for rotational data and one for the push button.


## Libraries Required

- **RF24 Library** for communicating with the NRF24L01 module
- **TFT and SPI** (or any compatible TFT library)

## Program flow
The project features an Object Oriented approach to reduce code repetition and for consistency.

- **NRF24Radio.h**: Contains all configaration functions ie. Radio initialisation, interrupt functions, protocol functions such as send and receive message

- **Config.h**: contains pin definitions, global structures and preprocessor directives. 

- **Eeprom_config.h**: contains preprocessor directives for programming the Arduino eeprom and turning LCD on and off.
  (#define IS_MASTER true) Comment out to program radio as slave

- **Utils.h**:  Contains generic receive and send function logic. This code is used by both master and slave with finer usage controlled in MasterDevice.h and SlaveDevice.h files.

- **LCD.h** : Contains controller code for the TFT Display

- **rotary_encoder.h**:  Contains code that controls cursor movement on the TFT and to select different menu items.
- **Main.ino** : Contains main program and event loop




