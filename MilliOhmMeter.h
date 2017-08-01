#ifndef _MILLIOHMMETER_H
#define _MILLIOHMMETER_H
/***********************************************************************************************//**
 *  \par        MilliOhmMeter Control Program - Header File.
 *  \file       MILLIOHMMETER.H
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
 *//***********************************************************************************************/

//#define _DEBUG_
#define __STDC_LIMIT_MACROS
#define __STDC_CONSTANT_MACROS


#define _DEVICENAME_        MilliOhm Meter        
#define _PROGNAME_          Control Firmware      
#define _AUTHOR_            J.F.Fitter  
#define _VERSION_MAJOR_     1
#define _VERSION_MINOR_     0
#define _REVISION_          0
#define _DATE_              25jul2017
#define _COPYRIGHT_         2017      
#define _COMPANY_           Eagle Air Australia p/l

// Usefull macros.
#define str(s)              #s
#define xstr(s)             str(s)
#define sqr(x)              ((x)*(x))

// Communications definitions.
#define BAUDRATE            115200                      /**< Serial comms data bit rate (bps)*/

// Hardware definitions.
#define CHG_PIN             6                           /**< Charging status pin */
#define STBY_PIN            7                           /**< Standby status pin */
#define LCD_BL_PIN          3                           /**< LCD backlight pin */
#define LCD_DC_PIN          2                           /**< SD card select pin ?????? */
#define LCD_RST_PIN         4                           /**< LCD reset pin */
#define LCD_CS_PIN          5                           /**< LCD selsect pin */
#define RES_TEST_CURRENT    0.1                         /**< Resistance test current (A) */
#define DIFF_AMP_GAIN       10.0                        /**< Differential amplifier gain */

// Data filtering definitions.
#define FILTERWT_RESS       80                          /**< Resistance soft filter strength (100 = min) */
#define FILTERWT_RESH       10                          /**< Resistance hard filter strength */
#define FILTERWT_DEV        10                          /**< Deviation filter strength */

// Sample and display rate definitions.
#define RES_SAMPLE_RATE     50                          /**< Resistance sample rate (sps) */
#define BAT_SAMPLE_RATE     1                           /**< Battery voltage sample rate (sps) */
#define LCD_REFRESH_RATE    3                           /**< LCD frame refresh rate (fps) */

// LCD geometry definitions.
#define LCD_WIDTH           320                         /**< LCD width in pixels */
#define LCD_HEIGHT          240                         /**< LCD height in pixels */
#define LCD_LR_MARGIN       10                          /**< LCD Screen left/right margins */
#define LCD_TB_MARGIN       10                          /**< LCD Screen top/bottom margins */

#define LCD_LEFT            LCD_LR_MARGIN
#define LCD_RIGHT           (LCD_WIDTH-LCD_LR_MARGIN)
#define LCD_TOP             LCD_TB_MARGIN
#define LCD_BOTTOM          (LCD_HEIGHT-LCD_TB_MARGIN)
#define LCD_XCENTER         ((LCD_LEFT+LCD_RIGHT)/2)
#define LCD_YCENTER         ((LCD_TOP+LCD_BOTTOM)/2)

// LCD management definitions.
#define LCD_DEF_BL_LEVEL    60                          /**< Backlight intensity % */
#define LCD_SPLASH_DELAY    2000                        /**< Splash screen display time ms */

// Autoranging definitions.
#define RAW_ADS_SPAN        0x8000                      /**< Span of A/D converter single ended */ 
#define RAW_HYSTERESIS      0x1000                      /**< Raw data hysteresis */

// A/D Converter range factors in V/bit.
#define VPB_SIXTEEN        0.0000078125                 /**< A/D gain 16x range factor V/bit */
#define VPB_EIGHT          0.000015625                  /**< A/D gain 8x range factor V/bit */
#define VPB_FOUR           0.00003125                   /**< A/D gain 4x range factor V/bit */
#define VPB_TWO            0.0000625                    /**< A/D gain 2x range factor V/bit */
#define VPB_ONE            0.000125                     /**< A/D gain 1x range factor V/bit */
#define VPB_TWOTHIRDS      0.0001875                    /**< A/D gain 2/3x range factor V/bit */

// LCD color definitions.
#define LCD_BK_COLOR        ILI9341_BLACK               /**< LCD background color */
#define LCD_RES_COLOR       ILI9341_MAGENTA             /**< LCD color of resistance value */
#define LCD_UNITS_COLOR     ILI9341_GREEN               /**< LCD color of resistance units */
#define LCD_MSG_COLOR       ILI9341_YELLOW              /**< LCD color of messages */
#define LCD_DATA_COLOR      ILI9341_CYAN                /**< LCD color of additional data */

// LCD data layout definitions.
#define LCD_RES_FONT_SIZE   10                          /**< Resistance font size */
#define LCD_YRES            (LCD_TOP+10)                /**< Resistance line Y-position */  

#define LCD_UNITS_FONT_SIZE 4                           /**< Units font size */
#define LCD_YUNITS          (LCD_YRES+9*LCD_RES_FONT_SIZE-5) /**< Units line Y-position */    

#define LCD_DATA_FONT_SIZE  3                           /**< Data font size */
#define LCD_YLINE3          (LCD_YUNITS+9*LCD_UNITS_FONT_SIZE+5) /**< Data line 1 Y-position */ 
#define LCD_YLINE4          (LCD_YLINE3+9*LCD_DATA_FONT_SIZE)  /**< Data line 1 Y-position */ 

// Battery management definitions.
#define BAT_VMINOP          3.0                         /**< Battery min operating voltage (V) */
#define BAT_VCHARGED        4.2                         /**< Battery fully charged voltage (V) */
#define BAT_VDISCHARGED     2.4                         /**< Battery discharged voltage (V) */

#define LCD_BAT_FONT_SIZE   3                           /**< Battery data font size */
#define LCD_BAR_FONT_SIZE   2                           /**< Battery status indicator text font size */
#define BAT_IND_TERML       5                           /**< Battery status indicator terminal length */
#define BAT_IND_TERMH       10                          /**< Battery status indicator terminal height */

#define BAT_IND_HEIGHT      24                          /**< Battery status indicator height */
#define BAT_IND_LENGTH      (LCD_RIGHT-LCD_LEFT-8*6*LCD_BAT_FONT_SIZE-BAT_IND_TERML) /**< Battery status indicator length */
#define BAT_IND_LEFT        (LCD_RIGHT-BAT_IND_LENGTH-BAT_IND_TERML)  /**< Battery status indicator left */
#define BAT_IND_TOP         (LCD_BOTTOM-BAT_IND_HEIGHT) /**< Battery status indicator top */

#define BAT_BAR_HEIGHT      (BAT_IND_HEIGHT-4)
#define BAT_BAR_LENGTH      (BAT_IND_LENGTH-4)
#define BAT_BAR_LEFT        (BAT_IND_LEFT+2)
#define BAT_BAR_TOP         (BAT_IND_TOP+2)

#define BAT_BAR_MIDX        (BAT_BAR_LEFT+BAT_BAR_LENGTH/2)
#define BAT_BAR_MIDY        (BAT_BAR_TOP+BAT_BAR_HEIGHT/2)

#define BAT_BAR_COLOR_BKG   ILI9341_YELLOW              /**< Battery status indicator background color */
#define BAT_BAR_COLOR_FG    ILI9341_GREEN               /**< Battery status indicator bar color */
#define BAT_BAR_COLOR_LOW   ILI9341_RED                 /**< Battery status indicator danger color */
#define BAT_BAR_COLOR_BDR   0xFFFF                      /**< Battery status indicator border color */
#define BAT_SHOCKO_COLOR    ILI9341_BLACK               /**< Battery external power indicator color */
#define SHOCKO_HEIGHT       (BAT_IND_HEIGHT-6) 

#define LCD_YLINE_BAT       (LCD_BOTTOM-(BAT_IND_HEIGHT+LCD_BAT_FONT_SIZE*7)/2)  /**< Battery data line Y-position */
#define LCD_YLINE_BAR       (LCD_BOTTOM-(BAT_IND_HEIGHT+LCD_BAR_FONT_SIZE*7)/2)  /**< Battery data line Y-position */

// Enumerations
enum Direction_t {DIR_AS,                               /**< Direction unchanged */
                  DIR_UP,                               /**< Direction up/increase */
                  DIR_DN                                /**< Direction down/decrease */
};

enum LcdTxtJust  {LCD_TXT_LEFT,                         /**< Text left justified */
                  LCD_TXT_CENTER,                       /**< Text center justified */
                  LCD_TXT_RIGHT                         /**< Text right justified */
};

enum BatState    {BAT_NORMAL,                           /**< 2.4V < Battery voltage < 4.0V */
                  BAT_EMPTY,                            /**< 0V < Battery voltage < 2.4V */
                  BAT_CHARGING,                         /**< Sensed from charger */
                  BAT_FULL,                             /**< Battery voltage > 4.0V */
                  BAT_NONE                              /**< No battery - voltage = 0V */
};

// Global object instantiations.
Adafruit_ADS1115 Ads1115;                               /**< A/D converter instance */
Adafruit_ILI9341 Lcd =  
Adafruit_ILI9341(LCD_CS_PIN, LCD_DC_PIN, LCD_RST_PIN);  /**< LCD display instance */

// Timers to manage task scheduling.
TimerUp procTimer[] = {
    TimerUp(1000/LCD_REFRESH_RATE),                     /**< LCD refresh timer */
    TimerUp(1000/RES_SAMPLE_RATE),                      /**< Resistance sampling timer */
    TimerUp(1000/BAT_SAMPLE_RATE)                       /**< Battery sampling timer */
};                                                      

// Data digital filters.
ExponentialFilter<double> resSFilter(FILTERWT_RESS, 0); /**< Resistance soft digital filter */
ExponentialFilter<double> resHFilter(FILTERWT_RESH, 0); /**< Resistance hard digital filter */
ExponentialFilter<double> devFilter(FILTERWT_DEV, 0);   /**< Resistance deviation digital filter*/

// Global variables.
double dblBatV;                                         /**< Battery voltage global */
BatState batteryStatus = BAT_NORMAL;                    /**< Battery charge status global */

#ifdef _DEBUG_
int16_t raw, rw;
adsGain_t gn;
ExponentialFilter<int32_t> rawFilter(FILTERWT_RESS, 0);
#endif // _DEBUG_

void writeLcdText(char *, int16_t, int16_t, uint8_t, LcdTxtJust, uint16_t, boolean = false);

#endif // _MILLIOHMMETER_H

// EOF MILLIOHMMETER.H
