/******************************************************************************
 * LCD.h - Header for the LCD.c Module
 *
 * Adapted from code by RobG of 43oh.com
 * WWU EET Senior Project - AMDRSSTC Interrupter
 * Nikolas Knutson-Bradac
 * Date of Last Revision: 06.03.2013
 *****************************************************************************/
#define LCD5110_SCLK_PIN BIT5
#define LCD5110_DN_PIN BIT7
#define LCD5110_DC_PIN BIT6
#define LCD5110_SET_COMMAND P1OUT &= ~LCD5110_DC_PIN
#define LCD5110_SET_DATA P1OUT |= LCD5110_DC_PIN
#define LCD5110_COMMAND 0
#define LCD5110_DATA 1
#define LCD5110_CONTRAST 0x23
#define LCD5110_LENGTH 84
#define TOPLINE 0x01
#define UNDERLINE 0x08
#define BLOCK 0xFF
#define MICRO 0x80

#define SPI_MSB_FIRST UCB0CTL0 |= UCMSB
#define SPI_LSB_FIRST UCB0CTL0 &= ~UCMSB s

void WriteStringToLCD(const INT8U *string);
void WriteCharToLCD(INT8U c);
void WriteBlockToLCD(INT8U byte, INT8U length);
void WriteToLCD(INT8U dataCommand, INT8U data);
void ClearLCD(void);
void ClearBank(INT8U bank);
void SetAddr(INT8U xAddr, INT8U yAddr);
void InitLCD(void);

