/******************************************************************************
 * LCD.c - A simple library to interface with the PCD8544 driving logic of the
 * Nokia 5110 LCD.
 *
 * Adapted from code by RobG of 43oh.com
 * WWU EET Senior Project - AMDRSSTC Interrupter
 * Nikolas Knutson-Bradac
 * Date of Last Revision: 06.03.2013
 *****************************************************************************/

#include "PCD8544.h"
#include "includes.h"

/******************************************************************************
 * InitLCD() - Configures the hardware to communicate with the LCD and sends
 * the configuration messages for the LCD to run.
 *****************************************************************************/
void InitLCD(void){
  P2OUT &= ~LCD_RESET_PIN;				      /*Initialize LCD logic*/
	__delay_cycles(2560);						  /*Wait for LCD logic to reset*/
    P2OUT |= LCD_RESET_PIN; 					  /*Enable LCD logic*/
    /*LCD start-up command sequence*/
    WriteToLCD(LCD5110_COMMAND, PCD8544_FUNCTIONSET | PCD8544_EXTENDEDINSTRUCTION);
    WriteToLCD(LCD5110_COMMAND, PCD8544_SETVOP | LCD5110_CONTRAST);	/*Set LCD contrast*/
    WriteToLCD(LCD5110_COMMAND, PCD8544_SETTEMP | 0x02);			/*Set temp co-eff*/
    WriteToLCD(LCD5110_COMMAND, PCD8544_SETBIAS | 0x03);			/*Set bias co-eff*/
    WriteToLCD(LCD5110_COMMAND, PCD8544_FUNCTIONSET);
    WriteToLCD(LCD5110_COMMAND, PCD8544_DISPLAYCONTROL | PCD8544_DISPLAYNORMAL); /*Set display mode*/

    ClearLCD();
}
/******************************************************************************
 * WriteStringToLCD(const INT8U *)
 *****************************************************************************/
void WriteStringToLCD(const INT8U *str) {
    while(*str){
        WriteCharToLCD(*str);
        str++;
    }
}
/******************************************************************************
 * WriteCharToLCD(INT8U)
 *****************************************************************************/
void WriteCharToLCD(INT8U c) {
    INT8U i;
    for(i = 0; i < 5; i++) {
        WriteToLCD(LCD5110_DATA, font[c - 0x20][i]);
    }
    WriteToLCD(LCD5110_DATA, 0);
}
/******************************************************************************
 * WriteBlockToLCD(INT8U, INT8U) - Writes a line to the LCD defined by byte,
 * over the length of length.
 *****************************************************************************/
void WriteBlockToLCD(INT8U byte, INT8U length) {
    INT8U c = 0;
    while(c < length) {
        WriteToLCD(LCD5110_DATA, byte);
        c++;
    }
    while(c < LCD5110_LENGTH) {      /*Clear rest of bank*/
          WriteToLCD(LCD5110_DATA, 0);
          c++;
      }
}

/******************************************************************************
 * WriteToLCD(INT8U, INT8U) - Sends a byte to the LCD, and sets the data/command pin
 * accordingly.
 *****************************************************************************/
void WriteToLCD(INT8U dataCommand, INT8U data) {
    if(dataCommand) {
        LCD5110_SET_DATA;
    } else {
        LCD5110_SET_COMMAND;
    }
    UCB0TXBUF = data;
    while(!(IFG2 & UCB0TXIFG)){}; /*Wait for USCI B0 Tx buffer*/
}

/******************************************************************************
 * ClearLCD()
 *****************************************************************************/
void ClearLCD(void) {
    INT16U c = 0;
    SetAddr(0, 0);
    while(c < PCD8544_MAXBYTES) {
        WriteToLCD(LCD5110_DATA, 0);
        c++;
    }
    SetAddr(0, 0);
}
/******************************************************************************
 * ClearBank(INT8U)
 *****************************************************************************/
void ClearBank(INT8U bank) {
    INT8U c = 0;
    SetAddr(0, bank);
    while(c < PCD8544_HPIXELS) {
        WriteToLCD(LCD5110_DATA, 0);
        c++;
    }
    SetAddr(0, bank);
}
/******************************************************************************
 * SetAddr(INT8U, INT8U) - Sets the current LCD cursor coordinates.
 *****************************************************************************/
void SetAddr(INT8U xAddr, INT8U yAddr) {
    WriteToLCD(LCD5110_COMMAND, PCD8544_SETXADDR | xAddr);
    WriteToLCD(LCD5110_COMMAND, PCD8544_SETYADDR | yAddr);
}


