****************************************************************************************************
* This is the firmware for the MilliOhm Meter (described herein)                                   *
****************************************************************************************************

Firmware for MilliOhm Meter using the following hardware;
    Scullcom milliohm meter circuit
    Arduino Pro Mini
    ADS1115 breakout board
    2.2" 240x320 SPI TFT LCD with ILI9341 driver
    Micro USB single LiIon battery charger module
    LiIon single cell boost converter module
    18650 2500mAh LiIon cyclindrical cell

Hardware setup.
Using an Arduino Pro Mini @ 16MHz / 5V.

Description & Function              Arduino Arduino
======================              Pin #   Pin Name
                                    ======= ========
ADS1115 16b A/D Converter I2C
    SDA   (Serial Data)             A4      ADC4 (SDA)
    SCL   (Serial Clock)            A5      ADC5 (SCL)

2.2" TFT RGB 240x320 Display (ILI9341)
    MISO  (Serial Out)              12      PB4 (MISO) (Serial In)
    MOSI  (Serial In)               11      PB3 (MOSI) (Serial Out)
    SCK   (Serial Clock)            13      PB5 (SCK)  (Serial Clock)
    LED   (Backlight)               2       PD2
    DC/RS (SD Card Select)          3       PD3
    RST   (Reset)                   4       PD4
    CS    (LCD Select)              5       PD5

ADS1115 Module Analog Inputs
    0,1   Differential              MilliOhm meter output
    2     Single ended              LiIon battery voltage

Check that the archive contains the following files;

MilliOhmMeter.ino       Arduino C++ source
MilliOhmMeter.h         C++ header
MilliOhmMeter.chm       The HTML documentation
MilliOhmMeter.vcxproj   Visual Studio project file
Filter.h                Exponential Filter source
printf.h                Formatted print source
README.txt              This file
Doxyfile                Doxygen documentation configuration

MilliOhmMeter.chm contains the documentation for the firmware.

Written by John Fitter, Eagle Air Australia Pty. Ltd.
BSD license. All text above must be included in any redistribution

