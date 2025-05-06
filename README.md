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


