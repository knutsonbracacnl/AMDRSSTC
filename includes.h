/******************************************************************************
 * includes.h - Main includes file of the AMDRSSTC Interrupter project.
 *
 * WWU EET Senior Project - AMDRSSTC Interrupter
 * Nikolas Knutson-Bradac
 * Date of Last Revision: 06.03.2013
 *****************************************************************************/
/*MCU Specific Includes*/
#include "msp430g2553.h"

/*Type Defines*/
typedef unsigned char   INT8U;
typedef signed char     INT8S;
typedef unsigned short  INT16U;
typedef signed short    INT16S;
typedef unsigned long   INT32U;
typedef signed long     INT32S;

typedef struct{
  INT8U Upper;
	INT8U Lower;
}MIDI_STRUCT;

/*General Defines*/
#define TRUE  1
#define FALSE 0
#define ON 1
#define OFF 0
#define FOREVER() while(1)

/*Project Specific Defines*/
#define MANUAL_MODE    1
#define MIDI_MODE	   0

#define SLICE_DIVIDER 5
#define NRM_CLK 40
#define NRM_FACTOR 100
#define SCALE(IN,FACTOR,BOUND) (IN*FACTOR)/BOUND

/*Pin Defines*/
#define MIDI_RX_PIN			BIT1	/*P1.1*/
#define MIDI_TX_PIN			BIT2	/*P1.2*/

#define LCD_CMD_PIN			BIT6	/*P1.6*/
#define MOSI_PIN			BIT7	/*P1.7*/
#define SCLK_PIN			BIT5	/*P1.5*/

#define FREQUENCY_POT_PIN	BIT3	/*P1.3*/
#define ON_TIME_POT_PIN		BIT4	/*P1.4*/

#define ENABLE_OUT_PIN		BIT6	/*P2.6*/
#define LCD_RESET_PIN		BIT5	/*P2.5*/
#define BACK_LIGHT_PIN		BIT1	/*P2.1*/
#define MODE_SW_PIN			BIT0	/*P1.0*/
#define BCKLIGHT_SW_PIN		BIT0	/*P2.0*/

/*Timer Defines*/
#define MIN_CCR0	    319 		/*360 us*/
#define MAX_CCR0	    0xFFFF
#define BACKLIGHT_PWM	0xFFF

/*ADC Index Defines*/
#define TOP_CHANNEL     0x02
#define FREQUENCY_POT	0x00
#define ON_TIME_POT	    0x01

/*Button Defines*/
#define BACKLIGHT_BUTTON()   !(P2IN & BIT0)
#define MODE_BUTTON()		 !(P1IN & BIT0)

/*Button Task Defines*/
#define DEBOUNCE_TIME       0.2
#define SAMPLE_FREQUENCY    100
#define MAXIMUM             (DEBOUNCE_TIME * SAMPLE_FREQUENCY)


/*Module Includes*/
#include "MIDI.h"
#include "LCD.h"

extern const INT8U *NoteLookup[128];
extern const INT16U PeriodLookup[128];
extern const INT8U OnTimeLookup[128];




