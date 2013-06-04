/******************************************************************************
 * Midi.c - Reads MIDI 1.0 input from UART and updates an output timer's
 * frequency and duty cycle according to the pitch and velocity of the MIDI
 * Input.
 *
 * WWU EET Senior Project - AMDRSSTC Interrupter
 * Nikolas Knutson-Bradac
 * Date of Last Revision: 06.03.2013
 *****************************************************************************/
#include "MIDI.h"
#include "includes.h"


INT8U MidiRxFlag = FALSE;
static INT8U MidiByte;
static INT8U Status;
static MIDI_STRUCT DataBytes;
static MIDI_STRUCT NoteBuffer[NOTE_BUFF_LEN];
static INT16U Bend = BEND_CENTER;
static const MIDI_STRUCT EmptyStruct = {0};

extern INT8U Frequency;
extern INT8U OnTime;
extern INT8U Mode;

/******************************************************************************
 * HandleMidiFrameTask() - Build and process MIDI frame.
 *****************************************************************************/
void HandleMidiFrameTask(void){
  static INT8U bytepos = UPPER;
	if(MidiRxFlag){
		MidiRxFlag = FALSE;
		if(Mode == MIDI_MODE){
			if(MidiByte & STATUS_BIT){			/*New status*/
				Status = MidiByte;
				if(Status >= TUNE_REQUEST){		/*Messages with no data bytes*/
					DataBytes.Upper = 0x00;
					DataBytes.Lower = 0x00;
					ProcessMidiData();
				}else{}
			}else if((Status == CHANNEL_PRESSURE)||(Status == PROGRAM_CHANGE)||(Status == SONG_SELECT)||(Status == BUS_SELECT)){	/*Messages with one data byte*/
				DataBytes.Upper = 0x00;
				DataBytes.Lower = MidiByte;
				ProcessMidiData();
			}else{								/*Messages with two data bytes*/
				if(bytepos == UPPER){
					DataBytes.Upper = MidiByte;
					bytepos = LOWER;
				}else{
					DataBytes.Lower = MidiByte;
					bytepos = UPPER;
					ProcessMidiData();
				}
			}
		}else{} /*In Manual Mode*/
	}else{}		/*No data to process*/
}
/******************************************************************************
 * UpdateSynth() - Update the output timer's pitch and duty cycle from the
 * output index of note buffer and other midi objects.
 *****************************************************************************/
void UpdateSynth(void){
	OnTime = NoteBuffer[OUTPUT].VELOCITY;
	Frequency = NoteBuffer[OUTPUT].KEY;
	if(Bend != BEND_CENTER){	   /*Check for Bend*/
		Frequency = Frequency - ((Bend-BEND_CENTER) >> 8);
	}else{}
	if(Frequency > MAX_FREQUENCY){
		Frequency = MAX_FREQUENCY;
	}else{}
	if(OnTime > MAX_ONTIME){
		OnTime = MAX_ONTIME;
	}else{}
	TA0CCR0 = PeriodLookup[Frequency];
	TA0CCR1 = OnTimeLookup[OnTime];
}

/******************************************************************************
 * NoteOff() - Scan the note buffer for the note to be turned off; if it is
 * present turn off note and shift higher indexed objects toward output.
 * Otherwise disregard.
 *****************************************************************************/
void NoteOff(void){
	INT8U i;
	for(i=0;i<NOTE_BUFF_LEN-1;i++){
		if(NoteBuffer[i].KEY == DataBytes.KEY){
			for(i;i<NOTE_BUFF_LEN-1;i++){
				NoteBuffer[i] = NoteBuffer[i+1];
			}
			break;
		}else{}
	}
}
/******************************************************************************
 * NoteOn() - Checks if curent note on data has a velocity of zero; if so turn
 * off that note. Otherwise shift objects in note buffer over and set new note
 * as output.
 *****************************************************************************/
void NoteOn(void){
	INT8U i;
	if(DataBytes.VELOCITY == 0){
		NoteOff();
	}else{
		for(i=NOTE_BUFF_LEN-1;i>0;i--){
			NoteBuffer[i] = NoteBuffer[i-1];
		}
		NoteBuffer[OUTPUT] = DataBytes;
	}
}
/******************************************************************************
 * KeyPressure() - Scans the NoteBuffer and sets the active key velocity.
 *****************************************************************************/
void KeyPressure(void){
	INT8U i;
	for(i=0;i<NOTE_BUFF_LEN;i++){
		if(NoteBuffer[i].KEY == DataBytes.KEY){
			NoteBuffer[i].VELOCITY = DataBytes.VELOCITY;
			break;
		}else{}
	}
}
/******************************************************************************
 * ChannelPressure() - Changes the velocity the current output.
 *****************************************************************************/
void ChannelPressure(void){
	NoteBuffer[OUTPUT].VELOCITY = DataBytes.Upper;
}
/******************************************************************************
 * PitchBend() - Builds and sets the Bend variable from the Midi data bytes.
 *****************************************************************************/
void PitchBend(void){
	Bend = (DataBytes.Lower << 7)| DataBytes.Upper; /*Build 14 bit word*/
}
/******************************************************************************
 * ProcessMidiData() - A switch statement that calls the appropriate function
 * to process the MIDI data bytes based off the current status.
 *****************************************************************************/
void ProcessMidiData(void){
	switch(Status){
	case NOTE_OFF:
		NoteOff();
		UpdateSynth();
		break;
	case NOTE_ON:
		NoteOn();
		UpdateSynth();
		break;
	case KEY_PRESSURE:
		KeyPressure();
		UpdateSynth();
		break;
	case CONTROLLER_CHANGE:
		ControllerChange();
		break;
	case PROGRAM_CHANGE:
		ProgramChange();
		break;
	case CHANNEL_PRESSURE:
		ChannelPressure();
		UpdateSynth();
		break;
	case PITCH_BEND:
		PitchBend();
		UpdateSynth();
		break;
	case SYSTEM_EXCLUSIVE:
		SystemExclusive();
		break;
	case SONG_POSITION:
		SongPosition();
		break;
	case SONG_SELECT:
		SongSelect();
		break;
	case BUS_SELECT:
		BusSelect();
		break;
	case TUNE_REQUEST:
		TuneRequest();
		break;
	case END_OF_SYSTEM_EXCLUSIVE:
		EndOfSystemExclusive();
		break;
	case TIMING_TICK:
		TimingTick();
		break;
	case START_SONG:
		StartSong();
		break;
	case CONTINUE_SONG:
		ContinueSong();
		break;
	case STOP_SONG:
		StopSong();
		break;
	case ACTIVE_SENSING:
		ActiveSensing();
		break;
	case SYSTEM_RESET:
		SystemReset();
		break;
	default:
		break;
	}
}
/******************************************************************************
 * ClearNoteBuffer()
 *****************************************************************************/
void ClearNoteBuffer(void){
	INT8U i;
	for(i=NOTE_BUFF_LEN-1;i>0;i--){
		NoteBuffer[i] = EmptyStruct;
	}
}
/******************************************************************************
 * MIDI_RX() - USCI A0 Interrupt, called when the Rx buffer is full.
 * Relays the received byte over Tx loads byte into global and sets ready flag.
 *****************************************************************************/
#pragma vector=USCIAB0RX_VECTOR
__interrupt void MIDI_RX(void){
	while (!(IFG2&UCA0TXIFG)){};
    UCA0TXBUF  = UCA0RXBUF;
    MidiByte   = UCA0RXBUF;
    MidiRxFlag = TRUE;
}
/*Unused Midi functions, included for portability*/
void ControllerChange(void){}
void ProgramChange(void){}
void SystemExclusive(void){}
void SongPosition(void){}
void SongSelect(void){}
void BusSelect(void){}
void TuneRequest(void){}
void EndOfSystemExclusive(void){}
void TimingTick(void){}
void StartSong(void){}
void ActiveSensing(void){}
void SystemReset(void){}
void StopSong(void){}
void ContinueSong(void){}
