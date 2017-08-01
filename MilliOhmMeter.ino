#define _MILLIOHMMETER_C
/***********************************************************************************************//**
 *  \par        MilliOhmMeter Control Program - Source File.
 *
 *  \par        Details
 *              Firmware for MilliOhm Meter using the following hardware;
 *  \li             Scullcom milliohm meter circuit
 *  \li             Arduino Pro Mini
 *  \li             ADS1115 breakout board
 *  \li             2.2" 240x320 SPI TFT LCD with ILI9341 driver
 *  \li             Micro USB single LiIon battery charger module
 *  \li             LiIon single cell boost converter module
 *  \li             18650 2500mAh LiIon cyclindrical cell
 *
 *  \file       MILLIOHMMETER.INO
 *  \author     J. F. Fitter <jfitter@eagleairaust.com.au>
 *  \version    1.0
 *  \date       July 2017
 *  \copyright  Copyright (c) 2017 John Fitter.  All right reserved.
 *
 *  \par        Changelog
 *              23jul2017  Initial release
 *
 *  \par        License
 *              This program is free software; you can redistribute it and/or modify it under
 *              the terms of the GNU Lesser General Public License as published by the Free
 *              Software Foundation; either version 2.1 of the License, or (at your option)
 *              any later version.
 *  \par
 *              This Program is distributed in the hope that it will be useful, but WITHOUT ANY
 *              WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 *              PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details
 *              at http://www.gnu.org/copyleft/gpl.html
 *  \par
 *              You should have received a copy of the GNU Lesser General Public License along
 *              with this library; if not, write to the Free Software Foundation, Inc.,
 *              51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *  \par
 *  \par        Hardware setup.
 *              Using an Arduino Pro Mini @ 16MHz / 5V.
 *  \verbatim
    Description & Function                     Arduino    Arduino
    ======================                     Pin #      Pin Name
                                               =======    ========
    ADS1115 16b A/D Converter I2C
        SDA   (Serial Data)                    A4         ADC4 (SDA)
        SCL   (Serial Clock)                   A5         ADC5 (SCL)

    2.2" TFT RGB 240x320 Display (ILI9341)
        MISO  (Serial Out)                     12         PB4 (MISO) (Serial In)
        MOSI  (Serial In)                      11         PB3 (MOSI) (Serial Out)
        SCK   (Serial Clock)                   13         PB5 (SCK)  (Serial Clock)
        LED   (Backlight)                      2          PD2
        DC/RS (SD Card Select)                 3          PD3
        RST   (Reset)                          4          PD4
        CS    (LCD Select)                     5          PD5

    ADS1115 Analog Inputs
        0,1   Differential                     MilliOhm meter output
        2     Single ended                     LiIon battery voltage
    \endverbatim
 *
 *//***********************************************************************************************/

#include <avr/pgmspace.h>
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <gfxfont.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_ADS1015.h>
#include <TimerUp.h>
#include "Filter.h"
#include "printf.h"

#include "MilliOhmMeter.h"

/**
 *  \brief    Program setup.
 *  \remarks  Performed once only at powerup.
 */
void setup() {

    #ifdef _DEBUG_
    startDebug();                                   // Start the debug output.
    #endif // _DEBUG_

    printf_begin();                                 // Start the printf formatter.
    Wire.begin();                                   // Start the serial bus.
    initBattery();                                  // Initialize battery monitoring hardware.
    initAds1115();                                  // Initialize ADS1115.
    initLcd();                                      // Initialize the LCD hardware.
}

/**
 *  \brief    Main program loop.
 *  \remarks  The main loop is merely a scheduler relying on the rollover of several timers to
 *            initiate functions.
 */
void loop() {
    if (procTimer[0].rollOver()) getResistance();
    if (procTimer[1].rollOver()) updateDisplay();
    if (procTimer[2].rollOver()) getBattVolts(); 
}

/**
 *  \brief                   Convert a double value to a formatted string.
 *  \remarks                 Unlike dtostrf() this procedure returns a string of only the specified
 *                           length, leading edge padded with spaces or truncated.
 *  \param [in] double val   Value to be converted.
 *  \param [in] uint8_t len  Length of the string to return.
 *  \param [in] uint8_t prec Precision (ie. number of decimals).
 *  \param [in] char *buf    String buffer to receive the result.
 *  \return     char*        Pointer to the formatted string buffer.
 */
char* dToStringFmt(double val, uint8_t len, uint8_t prec, char *buf) {
    uint8_t n = strlen(dtostrf(val, len, prec, buf));
    return &buf[n-len];
}

/**
 *  \brief    Update the values displayed on the LCD.
 *  \remarks  This procedure writes all of the variable data to the LCD. It performs
 *            Implement a simple linear recursive exponential filter. This is
 *            scaling of the value and selection of the appropriate units.
 */
void updateDisplay() {

    char buf[31];
    uint8_t dec = 0;
    double mul = 1000000.0;

    // If the resistance is over 5 then that is an overrange condition. Just say so and do nothing
    // else. This ensures that previous hard filtered values are held.
    if(resSFilter.Current() > 5.0) {
        writeLcdText("-----", LCD_XCENTER, LCD_YRES, LCD_RES_FONT_SIZE, 
            LCD_TXT_CENTER, LCD_RES_COLOR);
        writeLcdText("Over Range", LCD_XCENTER, LCD_YUNITS, LCD_UNITS_FONT_SIZE, 
            LCD_TXT_CENTER, LCD_UNITS_COLOR);
    } else {

        // Choose to display microOhms/milliOhms/Ohms, select a multiplier for the displayed value,
        // select decimal places, then write the units to the units line on the LCD.
        if(resSFilter.Current() < 0.001) {
            writeLcdText("micro Ohms", LCD_XCENTER, LCD_YUNITS, LCD_UNITS_FONT_SIZE, 
                LCD_TXT_CENTER, LCD_UNITS_COLOR);
        } else if(resSFilter.Current() < 1.0) {
            dec = resSFilter.Current() < 0.01 ? 3 : resSFilter.Current() < 0.1 ? 2 : 1; 
            mul = 1000.0;
            writeLcdText("milli Ohms", LCD_XCENTER, LCD_YUNITS, LCD_UNITS_FONT_SIZE, 
                LCD_TXT_CENTER, LCD_UNITS_COLOR);
        } else {
            dec = 3;
            mul = 1.0;
            writeLcdText("   Ohms   ", LCD_XCENTER, LCD_YUNITS, LCD_UNITS_FONT_SIZE, 
                LCD_TXT_CENTER, LCD_UNITS_COLOR);
        }
        // Write the soft filtered resistance value.
        writeLcdText(dToStringFmt(resSFilter.Current()*mul, 5, dec, buf), LCD_XCENTER, LCD_YRES, 
            LCD_RES_FONT_SIZE, LCD_TXT_CENTER, LCD_RES_COLOR);

        // Write the hard filtered resistance value and the square root of the summ of squares of 
        // the filtered value deviations. The RMS deviation is tested to be less than 10% of the 
        // filtered value. If not then neither the deviation nor the hard filtered values are 
        // displayed at all.
        if (sqrt(devFilter.Current()) > resHFilter.Current() / 10.0) {
            writeLcdText("-----", LCD_RIGHT, LCD_YLINE3, LCD_DATA_FONT_SIZE, 
                LCD_TXT_RIGHT, LCD_DATA_COLOR);
            writeLcdText("-----", LCD_RIGHT, LCD_YLINE4, LCD_DATA_FONT_SIZE, 
                LCD_TXT_RIGHT, LCD_DATA_COLOR);
        } else {
            writeLcdText(dToStringFmt(resHFilter.Current()*mul, 5, dec, buf), LCD_RIGHT,
                LCD_YLINE3, LCD_DATA_FONT_SIZE, LCD_TXT_RIGHT, LCD_DATA_COLOR);
            writeLcdText(dToStringFmt(sqrt(devFilter.Current())*mul, 5, dec, buf), LCD_RIGHT, 
                LCD_YLINE4, LCD_DATA_FONT_SIZE, LCD_TXT_RIGHT, LCD_DATA_COLOR);
        }
        // Just write the fixed text fields (in case they are corrupted for some reason).
        writeLcdText("Filtered :", LCD_LEFT, LCD_YLINE3, LCD_DATA_FONT_SIZE, LCD_TXT_LEFT, 
            LCD_DATA_COLOR);
        writeLcdText("RMS Dev  :", LCD_LEFT, LCD_YLINE4, LCD_DATA_FONT_SIZE, LCD_TXT_LEFT, 
            LCD_DATA_COLOR);
    }
    #ifdef _DEBUG_
    Serial.print(F("Gain: "));
    switch (gn) {
        case GAIN_SIXTEEN   : Serial.print(F("16 ")); break;
        case GAIN_EIGHT     : Serial.print(F("8  ")); break;
        case GAIN_FOUR      : Serial.print(F("4  ")); break; 
        case GAIN_TWO       : Serial.print(F("2  ")); break;   
        case GAIN_ONE       : Serial.print(F("1  ")); break;    
        default             : Serial.print(F("2/3"));
    }
    sprintf_P(buf, PSTR(" Raw: %5d %5d "), rw, rawFilter.Current());
    Serial.print(buf);
    Serial.print(F(" Res: "));
    Serial.print(dtostrf(resSFilter.Current(), 8, 6, buf));
    Serial.print(F("R  Bat: "));
    Serial.print(dtostrf(dblBatV, 5, 3, buf));
    Serial.print(F("V  Status: "));
    if(!digitalRead(CHG_PIN))  Serial.print(F("Chg"));
    if(!digitalRead(STBY_PIN)) Serial.print(F("|Stby"));
    Serial.println();
    #endif // _DEBUG_
}

/**
 *  \brief      Draw the battery charge status indicator.
 *  \remarks    Draws an indicator bar on the LCD showing the amount of charge remaining in the 
 *              battery and overlain by the battery state in text. If the battery is charging
 *              then a charging symbol is shown.
 *  \param [in] uint8_t  charge  The charge level of the battery in 1/2% increments (0..200).
 *  \param [in] BatState status  The charge status of the battery.
 */
void drawBatteryStatus(uint8_t charge, BatState status) {

    char buf[18]; // Keep this updated with code changes.

    // Draw the bar indicator. It shows percentage charge in 1/2 % increments.
     uint8_t midX = (BAT_BAR_LENGTH * (uint16_t)charge) / 200;

    Lcd.fillRect(BAT_BAR_LEFT, BAT_BAR_TOP, midX, BAT_BAR_HEIGHT, status == BAT_EMPTY ? 
        BAT_BAR_COLOR_LOW : BAT_BAR_COLOR_FG);
    Lcd.fillRect(BAT_BAR_LEFT+midX, BAT_BAR_TOP, BAT_BAR_LENGTH-midX, BAT_BAR_HEIGHT, 
        BAT_BAR_COLOR_BKG);
    Lcd.fillRoundRect(BAT_IND_LEFT+BAT_IND_LENGTH-2, BAT_IND_TOP+(BAT_IND_HEIGHT-BAT_IND_TERMH)/2,
        BAT_IND_TERML+2, BAT_IND_TERMH, 2, BAT_BAR_COLOR_BDR);
    Lcd.drawRoundRect(BAT_IND_LEFT, BAT_IND_TOP, BAT_IND_LENGTH, BAT_IND_HEIGHT, 3, 
        BAT_BAR_COLOR_BDR);
    Lcd.drawRoundRect(BAT_IND_LEFT+1, BAT_IND_TOP+1, BAT_IND_LENGTH-2, BAT_IND_HEIGHT-2, 2, 
        BAT_BAR_COLOR_BDR);

    // Write the battery status text information.
    switch (status) {
        case BAT_EMPTY : // Battery is empty.
            writeLcdText("DISCHARGED", BAT_BAR_MIDX, LCD_YLINE_BAR, LCD_BAR_FONT_SIZE, 
                LCD_TXT_CENTER, ILI9341_BLACK, true);
            break;
        case BAT_CHARGING : // Battery is charging so draw a shocko symbol on the bar indicator.
            Lcd.fillTriangle(BAT_BAR_MIDX+SHOCKO_HEIGHT/10, BAT_BAR_MIDY-SHOCKO_HEIGHT/2, 
                             BAT_BAR_MIDX+SHOCKO_HEIGHT/10, BAT_BAR_MIDY+SHOCKO_HEIGHT/10, 
                             BAT_BAR_MIDX-SHOCKO_HEIGHT,    BAT_BAR_MIDY+SHOCKO_HEIGHT/5, 
                             BAT_SHOCKO_COLOR);
            Lcd.fillTriangle(BAT_BAR_MIDX-SHOCKO_HEIGHT/10, BAT_BAR_MIDY+SHOCKO_HEIGHT/2, 
                             BAT_BAR_MIDX-SHOCKO_HEIGHT/10, BAT_BAR_MIDY-SHOCKO_HEIGHT/10, 
                             BAT_BAR_MIDX+SHOCKO_HEIGHT,    BAT_BAR_MIDY-SHOCKO_HEIGHT/5, 
                             BAT_SHOCKO_COLOR);
            break;
        case BAT_FULL : // Battery is full.
            writeLcdText("CHARGED", BAT_BAR_MIDX, LCD_YLINE_BAR, LCD_BAR_FONT_SIZE, 
                LCD_TXT_CENTER, ILI9341_BLACK, true);
            break;        
        case BAT_NONE : // None or disconnected battery.
            writeLcdText("NO BATTERY", BAT_BAR_MIDX, LCD_YLINE_BAR, LCD_BAR_FONT_SIZE, 
                LCD_TXT_CENTER, ILI9341_BLACK, true);
            break;        
        default : // All is normal so just write the battery charge percentage.
            sprintf_P(buf, PSTR("%3u%%"), charge/2); 
            writeLcdText(buf, BAT_BAR_MIDX, LCD_YLINE_BAR, LCD_BAR_FONT_SIZE, 
                LCD_TXT_CENTER, ILI9341_BLACK, true);
    }
    writeLcdText("Battery", LCD_LEFT, LCD_YLINE_BAT, LCD_BAT_FONT_SIZE, 
        LCD_TXT_LEFT, LCD_MSG_COLOR);
}

/**
 *  \brief    Get the resistance value.
 *  \remarks  Reads the A/D converter to get raw data then converts it to a resistance. The 
 *            resistance value is filtered with a digital filter and the difference between the 
 *            computed resistance and the filtered resistance is squared and filtered. The filters
 *            implement simple linear recursive exponential filters. These are essentially first
 *            order FIR filters which are ideal for the removal of white noise typically found on
 *            sensor data. They are not suitable for spectral filtering.
 *  \par      
 *            The meter circuit provides 100mA of source current so the resistance
 *            is equal to Measured Voltage * 10 where Measured Voltage is the raw a/d value
 *            multiplied by a gain factor in V/bit.
 *  \par      
 *            The raw a/d value is compared to the hysteresis limit for the purpose of changing
 *            the gain of the PGA. If the raw value is below the lower limit then the gain is
 *            increased. If the value is above the upper limit then the gain is reduced. The
 *            hysteresis is necessary to prevent the display from flickering between ranges.
 */
void getResistance() {

    // Get the raw integer value of the a/d converter. Valide range is 0..0x7FFF
    int16_t raw = Ads1115.readADC_SingleEnded(0);

    #ifdef _DEBUG_
    rawFilter.Filter(raw);
    rw = raw;
    gn = Ads1115.getGain();
    #endif // _DEBUG_

    // Just throw away negative voltages.
    if(raw < 0) raw = 0;

    // Use this value to update the resistance and deviation filter.
    // adjustGain() returns Ohms/bit.
    double res = adjustGain(DIR_AS) * raw;
    resSFilter.Filter(res);
    resHFilter.Filter(res);
    devFilter.Filter(sqr(res-resHFilter.Current()));   

    // Change the gain settings for next loop based on specified hysteresis limits.
    if (raw < RAW_ADS_SPAN/2-RAW_HYSTERESIS) adjustGain(DIR_UP);
    else if(raw > RAW_ADS_SPAN/2+RAW_HYSTERESIS) adjustGain(DIR_DN);                  
}                                                                           

/**
 *  \brief                  Change the gain of the A/D converter PGA.
 *  \remarks                Changes the gain by one step only, either up, down, or no change.
 *  \param [in] Direction_t dir   Direction to adjust the gain.
 *  \return     double      gain factor (Ohms/bit).
 */
double adjustGain(Direction_t dir) {

    adsGain_t gain = Ads1115.getGain();

    if(dir != DIR_AS)  {
        switch (gain) {
            case GAIN_SIXTEEN   : gain = dir == DIR_UP ? GAIN_SIXTEEN : GAIN_EIGHT;     break;
            case GAIN_EIGHT     : gain = dir == DIR_UP ? GAIN_SIXTEEN : GAIN_FOUR;      break;
            case GAIN_FOUR      : gain = dir == DIR_UP ? GAIN_EIGHT   : GAIN_TWO;       break;
            case GAIN_TWO       : gain = dir == DIR_UP ? GAIN_FOUR    : GAIN_ONE;       break;
            case GAIN_ONE       : gain = dir == DIR_UP ? GAIN_TWO     : GAIN_TWOTHIRDS; break;
            default             : gain = dir == DIR_UP ? GAIN_ONE     : GAIN_TWOTHIRDS;
        }
        Ads1115.setGain(gain);
    }
    // Return the gain in Ohms/bit.
    switch (gain) {
        case GAIN_SIXTEEN   : return VPB_SIXTEEN / RES_TEST_CURRENT / DIFF_AMP_GAIN;
        case GAIN_EIGHT     : return VPB_EIGHT   / RES_TEST_CURRENT / DIFF_AMP_GAIN; 
        case GAIN_FOUR      : return VPB_FOUR    / RES_TEST_CURRENT / DIFF_AMP_GAIN;  
        case GAIN_TWO       : return VPB_TWO     / RES_TEST_CURRENT / DIFF_AMP_GAIN;   
        case GAIN_ONE       : return VPB_ONE     / RES_TEST_CURRENT / DIFF_AMP_GAIN;    
    }
    return VPB_TWOTHIRDS / RES_TEST_CURRENT / DIFF_AMP_GAIN;   // default is GAIN_TWOTHIRDS
}

/**
 *  \brief    Initialize the battery monitoring hardware.
 *  \remarks  This is just 2 pins to monitor the status of the TP4056 status pins. These pins
 *            are open drain and drive 2 on-board LEDs connected to USB power. The CPU can monitor
 *            them through a series diode thereby preventing the CPU pins from being energised
 *            by the charger when the CPU power is removed (switch off).
 */
void initBattery() {

    pinMode(CHG_PIN,  INPUT_PULLUP);
    pinMode(STBY_PIN, INPUT_PULLUP);
}

/**
 *  \brief    Get the battery voltage.
 *  \remarks  The battery is a single LiIon cylindrical cell. It is nominally 3.7V and 4.2V at 
 *            full charge. It is 2.4V fully discharged. The voltage is measured single ended 
 *            (ie. referenced to ground) and the gain is set for a 6.144V range. On completion
 *            the gain is restored to its previous value.
 */
void getBattVolts() {

    // Save the A/D converter gain.
    adsGain_t gain = Ads1115.getGain();
    Ads1115.setGain(GAIN_TWOTHIRDS);

    // Get the battery voltage using the lowest A/D converter gain..
    dblBatV = VPB_TWOTHIRDS * Ads1115.readADC_SingleEnded(2);

    // Restore the A/D converter gain.
    Ads1115.setGain(gain);

    // Convert the battery voltage to a state of charge (0 to 200, 0.5% increments).
    uint8_t chgVal = dblBatV < BAT_VDISCHARGED ? 0 : dblBatV >= BAT_VCHARGED ? 200 :
        (uint8_t)((dblBatV-BAT_VDISCHARGED)/(BAT_VCHARGED-BAT_VDISCHARGED)*200.0);

    // Get the battery operating status from the status pins.
    bool chg  = !digitalRead(CHG_PIN);
    bool stby = !digitalRead(STBY_PIN);

    // Convert the operating status to a battery state.
    BatState chgState;   
    if(chg && !stby) chgState = BAT_CHARGING;
    else if((chgVal < 200) && (chgVal > 0)) chgState = BAT_NORMAL;
    else if (!chg && stby && (chgVal >= 200)) chgState = BAT_FULL;
    else if (!chg && !stby) chgState = BAT_NONE;
    else chgState = BAT_EMPTY;

    // Now draw the battery indicator showing battery state and voltage.
    drawBatteryStatus(chgVal, chgState);   
}

/**
 *  \brief    Initialize the ADS1115 16b A/D converter.
 *  \remarks  Starts the converter and sets the converter to the default gain.
 */
void initAds1115() {
    Ads1115.begin();                                // Initialize ADS1115
    Ads1115.setGain(GAIN_TWOTHIRDS);                // +/- 6.144V 187.5uV/bit
}

/**
 *  \brief                 Writes a text string to the LCD.
 *  \remarks               Generic text string writer. Draws the text clear.
 *  \param [in] char*      buf      Buffer containing the string.
 *  \param [in] int16_t    x        X position of the string reference in pixels.
 *  \param [in] int16_t    y        Y position of the string reference in pixels.
 *  \param [in] uint8_t    size     Text size.
 *  \param [in] LcdTxtJust just     Location of the string reference.
 *  \param [in] uint16_t   fgcolor  Color of the text.
 */
void writeLcdText(char *buf, int16_t x, int16_t y, uint8_t size, 
    LcdTxtJust just, uint16_t fgcolor, boolean clear) {
    switch (just) {
        case LCD_TXT_LEFT:   Lcd.setCursor(x, y); break;
        case LCD_TXT_CENTER: Lcd.setCursor(x - strlen(buf) * 3 * size, y); break;
        case LCD_TXT_RIGHT:  Lcd.setCursor(x - strlen(buf) * 6 * size, y);
    }
    Lcd.setTextSize(size);
    if(clear) Lcd.setTextColor(fgcolor);
    else Lcd.setTextColor(fgcolor, LCD_BK_COLOR);
    Lcd.print(buf);
}

/**
*  \brief    Initialize the LCD.
*  \remarks  Sets up the LCD orientation, background color, text color, text wrap and mode.
*  \li       Initializes the backlight PWM modulator.
*  \li       Writes the splash screen, pauses, then fills the screen with the background color.
*/
void initLcd() {

    // Setup the LCd.
    Lcd.begin();
    Lcd.setRotation(3);
    Lcd.fillScreen(LCD_BK_COLOR);
    Lcd.setTextWrap(false);

    // Setup the backlight. This is PWM and may be under user control in a future version.
    pinMode(LCD_BL_PIN, OUTPUT);
    analogWrite(LCD_BL_PIN, (LCD_DEF_BL_LEVEL * 128) / 100);

    // Write the splash screen.
    Lcd.setTextSize(2);
    Lcd.setTextColor(LCD_MSG_COLOR);
    Lcd.setCursor(LCD_LEFT, LCD_TB_MARGIN + 10);
    writeLcdText(xstr(_DEVICENAME_), LCD_LEFT, LCD_TB_MARGIN + 10, 3, LCD_TXT_LEFT, LCD_RES_COLOR);
    writeLcdText(xstr(_PROGNAME_), LCD_LEFT, LCD_TB_MARGIN + 40, 3, LCD_TXT_LEFT, LCD_RES_COLOR);
    writeLcdText("Version " xstr(_VERSION_MAJOR_) "." xstr(_VERSION_MINOR_) "." xstr(_REVISION_)
         " " xstr(_DATE_), LCD_LEFT, LCD_TB_MARGIN + 90, 2, LCD_TXT_LEFT, LCD_DATA_COLOR);
    writeLcdText("Author : " xstr(_AUTHOR_), LCD_LEFT, LCD_TB_MARGIN + 130, 2, 
         LCD_TXT_LEFT, LCD_UNITS_COLOR);
    writeLcdText("Copyright (c) " xstr(_COPYRIGHT_), LCD_LEFT, LCD_TB_MARGIN + 155, 2,
         LCD_TXT_LEFT, LCD_MSG_COLOR);
    writeLcdText(xstr(_COMPANY_), LCD_LEFT, LCD_TB_MARGIN + 180, 2, LCD_TXT_LEFT, LCD_MSG_COLOR);
    writeLcdText("All rights reserved", LCD_LEFT, LCD_TB_MARGIN + 205, 2,
         LCD_TXT_LEFT, LCD_MSG_COLOR);

    // Wait a while then clear the display.
    delay(LCD_SPLASH_DELAY);
    Lcd.fillScreen(LCD_BK_COLOR);
}

#ifdef _DEBUG_
/**
*  \brief    Start a debugging session.
*  \remarks  Sets the serial monitor baud rate and writes a header to the monitor.
*/
void startDebug() {
    Serial.begin(BAUDRATE);
    Serial.println(F("\r\nMilliOhmMeter Control Program"));
    Serial.println(F("Author: J. F. Fitter"));
    Serial.println(F("Copyright c 2017 All rights reserved.\r\n"));
}

#endif // _DEBUG_


// EOF MILLIOHMMETER.INO
