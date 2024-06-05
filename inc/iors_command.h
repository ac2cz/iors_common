/*
 * iors_command.h
 *
 *  Created on: Nov 27, 2023
 *      Author: g0kla
 */

#ifndef IORS_COMMAND_H_
#define IORS_COMMAND_H_

#include <stdint.h>

#define AUTH_VECTOR_SIZE 32
#define SW_COMMAND_SIZE 18
#define OUR_ADDRESS 0x1A // Initial test address - redundant as we have a callsign??
#define COMMAND_TIME_TOLLERANCE 30 // An identical command received within this many seconds is ACK but ignored
#define MAX_COMMAND_TIME 2082690000 //Dec 31 2035
#define MIN_COMMAND_TIME 1672462800 //Dec 31 2022

#define EXIT_DUPLICATE 2

/*
 * Following is the data structure representing software uplink commands
 */
typedef struct  __attribute__((__packed__)) {
	uint16_t command;
	uint16_t arguments[4];
} CommandAndArgs;

typedef struct  __attribute__((__packed__)){
	uint32_t dateTime;
	uint8_t reserved;
	uint8_t address;
	uint8_t special;
	uint8_t namespaceNumber;
	CommandAndArgs comArg;
    uint8_t AuthenticationVector[32];
} SWCmdUplink;

/*
 * Here are the definitions of the name spaces and commands
 */

typedef enum {
	 SWCmdNSReserved=0
	,SWCmdNSOps
	,SWCmdNSSSTV
	,SWCmdNSPacsat
	,SWCmdNumberOfNamespaces
}SWCommandNameSpace;

typedef enum {
     SWCmdIntReserved=0
    ,SWCmdIntAutosafeMode
}SWInternalCommands;

typedef enum {
	 SWCmdOpsReserved=0
	,SWCmdOpsPM1 = 1
	,SWCmdOpsXBandRepeaterMode = 2
	,SWCmdOpsAPRSMode = 3
	,SWCmdOpsSafeMode = 5
	,SWCmdOpsPacsatMode = 6
	,SWCmdOpsTelemMode = 7
	,SWCmdOpsAntSelect = 8
	,SWCmdOpsTime = 9
	,SWCmdOpsResetComputer = 10
	,SWCmdOpsResetRadio = 11
	,SWCmdOpsEnableTelemetry = 12
	,SWCmdOpsEnableTimePkts = 13
	,SWCmdOpsEnableStatusPkts = 14
	,SWCmdOpsResetIors = 15
	,SWCmdOpsSymbolRate = 16
	,SWCmdOpsRollLogs = 17
	,SWCmdOpsSSTVSend = 20
	,SWCmdOpsSSTVLoop = 21
	,SWCmdOpsSSTVStop = 22
	,SWCmdOpsSSTVPreProcess = 23
    ,SWCmdOpsNumberOfCommands
}SWOpsCommands;

//const static uint8_t SWCmdOpsArgSize[SWCmdOpsNumberOfCommands]={0,0,0,1,0,0,0};

typedef enum {
    SWCmdTlmReserved=0
   ,SWCmdTlmNumberOfCommands
}SWTlmCommands;

//const static uint8_t SWCmdTlmArgSize[SWCmdTlmNumberOfCommands]={0,1,1};

typedef enum {
	 SwCmdPacsatReserved=0
	,SWCmdPacsatEnablePB			// Args Enable/Disable, Status Period(s), Station Timeout(s)
	,SWCmdPacsatEnableUplink		// Args Enable/Disable, Status Period(s), Station Timeout(s)
	,SWCmdPacsatInstallFile			// Args 32-bit File-Id, Folder-Id  -- TODO - it might be better to move this to Ops
	,SWCmdPacsatDeleteFile
	,SWCmdPacsatDeleteFolder = 5
	,SWCmdPacsatDefaultFileExpiryPeriod // in days
	,SWCmdPacsatFileExpiryPeriod        // in days
	,SWCmdPacsatDirMaintPeriod          // in seconds - one node processed per period
	,SWCmdPacsatFtl0MaintPeriod         // in seconds - Whole upload queue processed per period
	,SWCmdPacsatFileQueueCheckPeriod = 10         // in seconds - Check all the queues for files to add to the dir
	,SWCmdPacsatMaxFileSize				// Set the max upload file size in kb
	,SWCmdPacsatMaxUploadAge			// Set the max age for a file being uploaded.  After this time it is removed from upload queue
	,SwCmdPacsatNumberOfCommands
}SWPacsatCommands;

/* These are the default writable folders on the USB Stick.  The strings are defined in
 * iors_command.c in a static array FolderIdStrings */
typedef enum {
	FolderSSTVQueue1 = 0
	,FolderSSTVQueue2
	,FolderSSTVQueue3
	,FolderOrbitalPositions
	,FolderBin
	,FolderLib
	,FolderCfg
	,FolderDir
	,FolderUpload
	,FolderWod
	,FolderLog
	,NumberOfFolderIds
} FolderIds;

/* These ids are a lookup for the symbolrate strings.  If this is changed then edit the array in iors_control.c */
typedef enum {
	Rate1200 = 0
	,Rate9600
	,NumberOfSymbolRateIds
} SymbolRateIds;

typedef enum {
	MartinM1 = 0
	,MartinM2
	,ScottieS1
	,ScottieS2
	,Robot36
	,PasokonP3 = 5
	,PasokonP5
	,PasokonP7
	,PD90
	,PD120 = 9
	,PD160
	,PD180
	,PD240
	,PD290
	,Robot8BW
	,Robot24BW
} SSTVModes;

void init_commanding(char * last_command_time_file);
char * get_folder_str(FolderIds i);
int get_symbol_rates(SymbolRateIds i);
int get_namespace_from_str(char *name_space);
int get_command_from_str(SWCommandNameSpace name_space, char * cmd);
SWCmdUplink *get_last_command();
int AuthenticateSoftwareCommand(SWCmdUplink *uplink);

#endif /* IORS_COMMAND_H_ */
