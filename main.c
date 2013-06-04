/*****************************************************************************
* main.c - main file of the AMDRSSTC Interrupter project. Built from a
* time-slice kenel there are two modes; Manual and Midi. Midi mode uses midi.c
* to read std Midi 1.0 data and update output. Manual mode reads user input
* from two potentiometers and updates the output.
*
* Data from either mode is displayed on a Nokia 5110 LCD, and updates a fiber-
* optic output driven by the TA0 timer.
*
* WWU EET Senior Project - AMDRSSTC Interrupter
* Nikolas Knutson-Bradac
* Date of Last Revision: 06.03.2013
*****************************************************************************/
#include "includes.h"

static void SystemInit(void);
static void GPIOInit(void);
static void UARTInit(void);
static void TimersInit(void);
static void ADCInit(void);
static void SPIInit(void);

static void ManualModeTask(void);
static void ButtonHandlerTask(void);
static void UpdateLCDTask(void);
static void WaitForSlice(void);

static void ModeChange(void);
static void UpdateTimer(INT8U Frequency, INT8U OnTime);
static void SampleADC(INT16U *FrequenyPtr, INT16U *OnTimePtr);
static void ByteToString(INT16U Byte, INT8U* Str);


const INT8U* NoteLookup[128] =
{"OFF ","C0# ","D0 ","D0# ","E0 ","F0 ","F0# ","G0 ","G0#","A0 ","A0#","B0 ",
 "C1 ","C1# ","D1 ","D1# ","E1 ","F1 ","F1# ","G1 ","G1#","A1 ","A1#","B1 ",
 "C2 ","C2# ","D2 ","D2# ","E2 ","F2 ","F2# ","G2 ","G2#","A2 ","A2#","B2 ",
 "C3 ","C3# ","D3 ","D3# ","E3 ","F3 ","F3# ","G3 ","G3#","A3 ","A3#","B3 ",
 "C4 ","C4# ","D4 ","D4# ","E4 ","F4 ","F4# ","G4 ","G4#","A4 ","A4#","B4 ",
 "C5 ","C5# ","D5 ","D5# ","E5 ","F5 ","F5# ","G5 ","G5#","A5 ","A5#","B5 ",
 "C6 ","C6# ","D6 ","D6# ","E6 ","F6 ","F6# ","G6 ","G6#","A6 ","A6#","B6 ",
 "C7 ","C7# ","D7 ","D7# ","E7 ","F7 ","F7# ","G7 ","G7#","A7 ","A7#","B7 ",
 "C8 ","C8# ","D8 ","D8# ","E8 ","F8 ","F8# ","G8 ","G8#","A8 ","A8#","B8 ",
 "C9 ","C9# ","D9 ","D9# ","E9 ","F9 ","F9# ","G9 ","G9#","A9 ","A9#","B9 ",
 "C10","C10#","D10","D10#","E10","F10","F10#","G10"};

const INT16U PeriodLookup[128] =
{0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,
 0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,
 0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,64792,
 61156,57724,54484,51426,48540,45815,43244,40817,38526,36364,34323,32396,
 30578,28862,27242,25713,24270,22908,21622,20408,19263,18182,17161,16198,
 15289,14431,13621,12856,12135,11454,10811,10204,9632 ,9091 ,8581 ,8099 ,
 7645 ,7216 ,6811 ,6428 ,6068 ,5727 ,5406 ,5102 ,4816 ,4546 ,4290 ,4050 ,
 3822 ,3608 ,3405 ,3214 ,3034 ,2864 ,2703 ,2551, 2408 ,2273 ,2145 ,2025 ,
 1911 ,1804 ,1703 ,1607 ,1517 ,1432 ,1351 ,1276 ,1204 ,1136 ,1073 ,1012 ,
 956  ,902  ,851  ,804  ,758  ,716  ,676  ,638  ,602  ,568  ,536  ,506  ,
 478  ,451  ,426  ,402  ,379  ,358  ,338  ,319};

const INT8U OnTimeLookup[128]=
{0  ,1  ,2  ,3  ,5  ,6  ,7  ,8  ,9  ,10 ,11 ,12 ,14 ,15 ,16 ,17 ,18 ,19 ,
 20 ,22 ,23 ,24 ,25 ,26 ,27 ,28 ,29 ,31 ,32 ,33 ,34 ,35 ,36 ,37 ,39 ,40 ,
 41 ,42 ,43 ,44 ,45 ,46 ,48 ,49 ,50 ,51 ,52 ,53 ,54 ,56 ,57 ,58 ,59 ,60 ,
 61 ,62 ,63 ,65 ,66 ,67 ,68 ,69 ,70 ,71 ,73 ,74 ,75 ,76 ,77 ,78 ,79 ,81 ,
 82 ,83 ,84 ,85 ,86 ,87 ,88 ,90 ,91 ,92 ,93 ,94 ,95 ,96 ,98 ,99 ,100,101,
 102,103,104,105,107,108,109,110,111,112,113,115,116,117,118,119,120,121,
 122,124,125,126,127,128,129,130,132,133,134,135,136,137,138,139,141,142,
 143,144};

static const INT8U MidiStr[]      = "Midi Mode";
static const INT8U ManualStr[]    = "Manual Mode";
static const INT8U FrequencyStr[] = "Frequency:---";
static const INT8U OnTimeStr[]    = "On Time:---";

static volatile INT16U ADCDataBuffer[TOP_CHANNEL];

INT8U Mode = MIDI_MODE;
static INT8U SliceCount = 0;
static INT8U Tick;
static INT8U LCDAddrX = 0;
static INT8U LCDAddrY = 0;

INT16U Frequency;
INT16U OnTime;

extern INT8U MidiRxFlag;

void main(void){
  SystemInit();
	FOREVER(){
		HandleMidiFrameTask();
		ManualModeTask();
		ButtonHandlerTask();
		UpdateLCDTask();
		WaitForSlice();
	}
}
/******************************************************************************
 * ModeChange() - Facilitates the transition between MIDI and Manual mode.
 *****************************************************************************/
void ModeChange(void){
	INT8U velstr[3];
	if(Mode == MIDI_MODE){
		ClearBank(0);				/*Init LCD for Manual Mode*/
		LCDAddrX = 0;
		LCDAddrY = 0;
		SetAddr(LCDAddrX, LCDAddrY);
		WriteStringToLCD(ManualStr);
		LCDAddrX = 60;
		LCDAddrY = 2;
		SetAddr(LCDAddrX, LCDAddrY);
		WriteStringToLCD(NoteLookup[Frequency]);
		LCDAddrX = 0;
		LCDAddrY = 3;
		SetAddr(LCDAddrX, LCDAddrY);
		WriteBlockToLCD(BLOCK, SCALE(Frequency, LCD5110_LENGTH, 128));
		LCDAddrX = 47;
		LCDAddrY = 4;
		SetAddr(LCDAddrX, LCDAddrY);
		ByteToString(SCALE(OnTime,NRM_FACTOR,NRM_CLK), velstr);
		WriteStringToLCD(velstr);
		LCDAddrX = 0;
		LCDAddrY = 5;
		SetAddr(LCDAddrX, LCDAddrY);
		WriteBlockToLCD(BLOCK, SCALE(OnTime, LCD5110_LENGTH,128));

		Mode = MANUAL_MODE;
	}else{
		ClearLCD(); 				/*Init LCD for Midi Mode*/
		LCDAddrX = 0;
		LCDAddrY = 0;
		SetAddr(LCDAddrX,LCDAddrY);
		WriteStringToLCD(MidiStr);
		LCDAddrX = 0;
		LCDAddrY = 1;
		SetAddr(LCDAddrX,LCDAddrY);
		WriteBlockToLCD(UNDERLINE,LCD5110_LENGTH);
		LCDAddrX = 0;
		LCDAddrY = 2;
		SetAddr(LCDAddrX,LCDAddrY);
		WriteStringToLCD(FrequencyStr);
		LCDAddrX = 0;
		LCDAddrY = 4;
		SetAddr(LCDAddrX,LCDAddrY);
		WriteStringToLCD(OnTimeStr);
		LCDAddrX = 65;
		LCDAddrY = 4;
		SetAddr(LCDAddrX,LCDAddrY);
		WriteCharToLCD(0x80);
		WriteCharToLCD('s');

		ClearNoteBuffer();			/*Init data and hardware for Midi Mode*/
		TA0CCR0  = 0;
		TA0CCR1  = 0;
	    P2OUT &= ~ENABLE_OUT_PIN;
		Mode = MIDI_MODE;
	}
}
/******************************************************************************
 * ByteToString(INT16U, INT8U*) - Takes in an INT16U word and loads a string
 * representation in to the given pointer address.
 *****************************************************************************/
void ByteToString(INT16U Byte, INT8U* Str){
		Str[0] = (Byte / 100) + 0x30;
		Str[1] = ((Byte % 100) / 10) + 0x30;
		Str[2] = (Byte % 10) + 0x30;
		Str[3] = 0x00;
}
/******************************************************************************
 * ButtonHandlerTask() - Polls and processes user button input.
 *****************************************************************************/
void ButtonHandlerTask(void){
	static INT8U ModeIntegrator = 0;
	static INT8U BackLightIntegrator = 0;
	static INT8U ModeButtonState = FALSE;
	static INT8U BackLightButtonState = FALSE;
	static INT8U ModeButton;
	static INT8U BackLightButton;

	if(!MODE_BUTTON()){						    /*Mode button integrator*/
		if(ModeIntegrator > 0){
			ModeIntegrator--;
		}else{}
	}else if(ModeIntegrator < MAXIMUM){
		ModeIntegrator++;
	}else{}

	if(ModeIntegrator == 0){					/*Mode button triggering*/
		ModeButton = FALSE;
	}else if(ModeIntegrator >= MAXIMUM){
		ModeButton = TRUE;
	}else{}

	if(ModeButtonState != ModeButton){		    /*Mode button Edge detection*/
		ModeButtonState = ModeButton;
		if(ModeButton == FALSE){ 				/*Falling Edge*/
			ModeChange();
		}else{}
	}else{}

	if(!BACKLIGHT_BUTTON()){					/*Backlight button integrator*/
		if(BackLightIntegrator > 0){
			BackLightIntegrator--;
		}else{}
	}else if(BackLightIntegrator < MAXIMUM){
		BackLightIntegrator++;
	}else{}

	if(BackLightIntegrator == 0){				/*Backlight button triggering*/
		BackLightButton = FALSE;
	}else if(BackLightIntegrator >= MAXIMUM){
		BackLightButton = TRUE;
	}else{}

	if(BackLightButtonState != BackLightButton){/*Backlight button edge detection*/
		BackLightButtonState = BackLightButton;
		if(ModeButton == FALSE){
		}else{}
	}else{
		if(BackLightButton){
			TA1CCR1 += 128;
		}else{}
	}

}

/******************************************************************************
 * ManualModeTask() - Sample the analog user inputs and update the timers
 * accordingly.
 * Period = 30 ms
 *****************************************************************************/
void ManualModeTask(void){
	static INT8U PassCount = 0;
	if(Mode == MANUAL_MODE){
		PassCount++;
		if(PassCount >= 3){
			PassCount = 0;
			SampleADC(&Frequency,&OnTime);
			UpdateTimer(Frequency, OnTime);
		}else{}
	}else{}		/*In Midi mode*/
}
/******************************************************************************
 * WaitForSlice()
 * -MIDI mode waits for either the system tick or a received Midi byte.
 * -Manual mode waits for system tick.
 * Period = 10 ms to 256 us
 *****************************************************************************/
void WaitForSlice(void){
	if(Mode == MIDI_MODE){
		while(!MidiRxFlag && !Tick){}
		Tick = FALSE;
	}else{
		while(!Tick){}
		Tick = FALSE;
	}
}
/******************************************************************************
 * UpdateLCDTask() - Update the display with the current value of the output
 * time and the mode of the system.
 * Period = 40 ms
 *****************************************************************************/
void UpdateLCDTask(void){
	static INT8U PassCount = 0;
	static INT8U CurrentFrequency = 0;
	static INT8U CurrentOnTime = 0;
	INT8U VelString[4];

	PassCount++;
	if(PassCount >= 4){
		PassCount = 0;
		if(CurrentFrequency != Frequency){
			CurrentFrequency = Frequency;
			LCDAddrX = 60;
			LCDAddrY = 2;
			SetAddr(LCDAddrX, LCDAddrY);
			WriteStringToLCD(NoteLookup[CurrentFrequency]);
			LCDAddrX = 0;
			LCDAddrY = 3;
			SetAddr(LCDAddrX, LCDAddrY);
			WriteBlockToLCD(BLOCK, SCALE(CurrentFrequency, LCD5110_LENGTH, 128));
		}else{}
		if(CurrentOnTime != OnTime){
			CurrentOnTime = OnTime;
			LCDAddrX = 47;
			LCDAddrY = 4;
			SetAddr(LCDAddrX, LCDAddrY);
			ByteToString(SCALE(CurrentOnTime,NRM_FACTOR,NRM_CLK), VelString);
			WriteStringToLCD(VelString);
			LCDAddrX = 0;
			LCDAddrY = 5;
			SetAddr(LCDAddrX, LCDAddrY);
			WriteBlockToLCD(BLOCK, SCALE(CurrentOnTime, LCD5110_LENGTH,128));
		}else{}
	}
}
/******************************************************************************
 * SampleADC(INT16U *,INT16U *) - Configures hardware module outputs, and user
 * input buttons.
 *****************************************************************************/
void SampleADC(INT16U *FrequencyPtr,INT16U *OnTimePtr){
	ADC10CTL0 &= ~ENC;					/*Start ADC sampling*/
	while(ADC10CTL1 & BUSY){};
	ADC10SA = (INT16U)ADCDataBuffer;	/*Set DMA destination address*/
	ADC10CTL0 |= (ENC | ADC10SC);		/*Disable ADC*/
	*FrequencyPtr = (INT8U)(0x007F & (ADCDataBuffer[FREQUENCY_POT] >> 3));  /*Capture and downsample to 7 bits*/
	*OnTimePtr = (INT8U)(0x007F & (ADCDataBuffer[ON_TIME_POT] >> 3));		/*Capture and downsample to 7 bits*/
}
/******************************************************************************
 * UpdateTimer(INT8U, INT8U) - Update the enable out timer according to
 * current Frequency and On Time configured by the user.
 *****************************************************************************/
void UpdateTimer(INT8U Frequency, INT8U OnTime){
	TA0CCR0 = PeriodLookup[Frequency];
	TA0CCR1 = OnTimeLookup[OnTime];
}
/******************************************************************************
 * SystemInit() - Initialize system.
 *****************************************************************************/
void SystemInit(void){
	WDTCTL  = (WDTPW | WDTHOLD);		/*Stop watchdog*/
	BCSCTL1 = CALBC1_16MHZ;	/*Calibrate system clock*/
	DCOCTL  = CALDCO_16MHZ;

	GPIOInit();				/*Configure peripheral registers and support code*/
	UARTInit();
	ADCInit();
	TimersInit();
	SPIInit();
	InitLCD();

	__bis_SR_register(GIE);	/*Enable global interrupts*/
	LCDAddrX = 0;		  	/*Draw initial LCD*/
	LCDAddrY = 1;
	SetAddr(LCDAddrX,LCDAddrY);
	WriteBlockToLCD(UNDERLINE,LCD5110_LENGTH);
	LCDAddrX = 0;
	LCDAddrY = 0;
	SetAddr(LCDAddrX,LCDAddrY);
	WriteStringToLCD(MidiStr);
	LCDAddrX = 0;
	LCDAddrY = 2;
	SetAddr(LCDAddrX,LCDAddrY);
	WriteStringToLCD(FrequencyStr);
	LCDAddrX = 0;
	LCDAddrY = 4;
	SetAddr(LCDAddrX,LCDAddrY);
	WriteStringToLCD(OnTimeStr);
	LCDAddrX = 65;
	LCDAddrY = 4;
	SetAddr(LCDAddrX,LCDAddrY);
	WriteCharToLCD(0x80);
	WriteCharToLCD('s');
}
/******************************************************************************
 * TimersInit() - Configures Timers;
 * TA0 is used to synthesize the enable output signal
 * TA1 is used to control the intensity of the LCD back light
 * WDT is configured in interval mode to generate the time slice tick.
 *****************************************************************************/
void TimersInit(void){
    TA0CTL   = (TASSEL_2 | ID_2 | MC_1);
    TA0CCTL0 = (CM_0 | CCIS_0 | OUTMOD_4);
    TA0CCTL1 = (CM_0 | CCIS_0 | SCS | CAP | OUTMOD_7);
	TA0CCR0  = 0;
	TA0CCR1  = 0;
    P2OUT &= ~ENABLE_OUT_PIN;
    TA1CTL   = (TASSEL_2 | MC_1);
    TA1CCTL0 = (CM_0 | CCIS_0 | OUTMOD_4);
    TA1CCTL1 = (CM_0 | CCIS_0 | SCS | CAP | OUTMOD_3);
	TA1CCR0  = 65535;
	TA1CCR1  = 30000;
	WDTCTL   = (0x5A00 | WDTTMSEL);
	IE1 |= WDTIE;
}
/******************************************************************************
 * GPIOInit() - Configures hardware module outputs, and user input buttons.
 *****************************************************************************/
void GPIOInit(void){
	/*Configure Module Outputs*/
    P1SEL  |= (MIDI_RX_PIN | MIDI_TX_PIN | SCLK_PIN | MOSI_PIN);
    P1SEL2 |= (MIDI_RX_PIN | MIDI_TX_PIN | SCLK_PIN | MOSI_PIN);
    P1OUT  |= LCD_CMD_PIN;
    P1DIR  |= (MIDI_TX_PIN | MOSI_PIN | LCD_CMD_PIN);
    P2SEL  |= (ENABLE_OUT_PIN | BACK_LIGHT_PIN);
    P2SEL  &= ~BIT7;
    P2DIR  |= (ENABLE_OUT_PIN | BACK_LIGHT_PIN | LCD_RESET_PIN);
    /*Configure Button Inputs*/
    P1REN |= MODE_SW_PIN;
    P1OUT |= MODE_SW_PIN;
    P2REN |= BCKLIGHT_SW_PIN;
    P2OUT |= BCKLIGHT_SW_PIN;
}
/******************************************************************************
 * ADCInit() - Configures the ADC10 module to read from the ADC input pins in
 * single sequence one-shot mode.
 *****************************************************************************/
void ADCInit(void){
	ADC10CTL1  = (INCH_4 | CONSEQ_1);
	ADC10CTL0  = (ADC10ON | MSC | ADC10SHT_3 | ADC10SR);
	ADC10AE0  |= (FREQUENCY_POT_PIN | ON_TIME_POT_PIN);
	ADC10DTC1 |=  TOP_CHANNEL;
}
/******************************************************************************
 * UARTInit() - Configures the USCI A0 module in UART mode with baud rate of
 * 31250; Rx interrupt enabled.
 *****************************************************************************/
void UARTInit(void){
	UCA0CTL1 |= UCSSEL_2;						  /*SMCLK*/
	UCA0BR0   = 0x00;							  /*31250 baud*/
	UCA0BR1   = 0x02;
	UCA0CTL1 &= ~UCSWRST;						  /*Start USCI A0 module*/
	IE2      |= UCA0RXIE;
}
/******************************************************************************
 * SPIInit() - Configures the USCI B0 module in SPI mode with a bit rate of
 * 16 Mbps; MSB first; Master; Three Wire.
 *****************************************************************************/
void SPIInit(void){
	UCB0CTL0 |= UCCKPH + UCMSB + UCMST + UCSYNC;  /*3-pin, 8-bit SPI master*/
    UCB0CTL1 |= UCSSEL_2;                         /*SMCLK*/
    UCB0BR0 = 0x01;                               /*16 Mbps*/
    UCB0BR1 = 0x00;
    UCB0CTL1 &= ~UCSWRST;                     	  /*Start USCI B0 module*/
}
/******************************************************************************
 * OS_Tick() - Serves as the OS tick for the time slice kernel.
 * The ISR is called by the WatchDogTimer ISR configured as a 2 ms interval
 * timer. This 2 ms period is software divided to 10 ms that serves as the
 * slice period.
 *****************************************************************************/
#pragma vector=WDT_VECTOR
__interrupt void OS_Tick(void){
	SliceCount++;
	if(SliceCount >= SLICE_DIVIDER){
		Tick = TRUE;
		SliceCount = 0;
	}else{}
}
