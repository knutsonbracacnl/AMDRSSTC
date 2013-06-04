/******************************************************************************
 * MIDI.h - Header for the MIDI.c Module
 *
 * WWU EET Senior Project - AMDRSSTC Interrupter
 * Nikolas Knutson-Bradac
 * Date of Last Revision: 06.03.2013
 *****************************************************************************/
/******************************************************************************
 * Private Functions
 *****************************************************************************/
/*Midi Status Handler Function*/
static void NoteOff(void);
static void NoteOn(void);
static void KeyPressure(void);
static void ControllerChange(void);  	//Not Implemented
static void ProgramChange(void);		//Not Implemented
static void ChannelPressure(void);
static void PitchBend(void);
static void SystemExclusive(void);		//Not Implemented
static void SongPosition(void);			//Not Implemented
static void SongSelect(void);			//Not Implemented
static void BusSelect(void);			//Not Implemented
static void TuneRequest(void);			//Not Implemented
static void EndOfSystemExclusive(void);	//Not Implemented
static void TimingTick(void);			//Not Implemented
static void StartSong(void);			//Not Implemented
static void ActiveSensing(void);		//Not Implemented
static void SystemReset(void);			//Not Implemented
static void StopSong(void);				//Not Implemented
static void ContinueSong(void);			//Not Implemented

/*Midi Processing Functions*/
static void UpdateSynth(void);
static void ProcessMidiData(void);
/******************************************************************************
 * Public Functions
 *****************************************************************************/
void ClearNoteBuffer(void);
void HandleMidiFrameTask(void);
/******************************************************************************
 * Defines
 *****************************************************************************/
/*MIDI Constants*/
#define CONTROLLER_CHANNEL 		0x00 /*Change from 0x0 through 0xF to set MIDI channel*/
#define NOTE_OFF				(0x80 | CONTROLLER_CHANNEL)
#define NOTE_ON					(0x90 | CONTROLLER_CHANNEL)
#define KEY_PRESSURE			(0xA0 | CONTROLLER_CHANNEL)
#define CONTROLLER_CHANGE		(0xB0 | CONTROLLER_CHANNEL)
#define PROGRAM_CHANGE			(0xC0 | CONTROLLER_CHANNEL)
#define CHANNEL_PRESSURE		(0xD0 | CONTROLLER_CHANNEL)
#define PITCH_BEND				(0xE0 | CONTROLLER_CHANNEL)
#define SYSTEM_EXCLUSIVE		0xF0
#define SONG_POSITION			0xF2
#define SONG_SELECT				0xF3
#define BUS_SELECT				0xF5
#define TUNE_REQUEST			0xF6
#define END_OF_SYSTEM_EXCLUSIVE 0xF7
#define TIMING_TICK				0xF8
#define START_SONG				0xFA
#define CONTINUE_SONG			0xFB
#define STOP_SONG				0xFC
#define ACTIVE_SENSING			0xFE
#define SYSTEM_RESET			0xFF
#define BEND_CENTER		   		0x2000

/*MIDI Module Defines*/
#define NOTE_BUFF_LEN  12
#define UPPER 		   1
#define LOWER          0
#define KEY 	       Upper	/*DataByte assignments*/
#define VELOCITY       Lower
#define OUTPUT 		   0		/*Index 0*/
#define STATUS_BIT	   0x80
#define MAX_FREQUENCY  127
#define MAX_ONTIME     127



