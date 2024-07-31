/*
 * iors_log.h
 *
 *  Created on: Jun 2, 2024
 *      Author: g0kla
 *
 * Binary log format based on the historical pacsat Activity Logs
 */

#ifndef IORS_LOG_H_
#define IORS_LOG_H_

#include <stdint.h>

#define FILE_TMP ".tmp"

enum LOG_LEVEL {
	NO_LOG,
	ERR_LOG,
	WARN_LOG,
	INFO_LOG
};

enum LOG_NAME {
	LOG_NAME
	,WOD_NAME
	,ERR_NAME
	,NumberOfLogNames
};

/* If this is updated then LOG TEXT should be updated in iors_log.c */
enum LOG_EVENT {
	/* Initial event numbers are historical to match the PACSAT ALOG events */
     ALOG_IORS_STARTUP = 1		/* startup */
	,ALOG_IORS_ERR              /* 1 - Error is passed in serial_no */
	,ALOG_PROGRAM_EXIT       /* 1 - Exit code is in serial no */
	,ALOG_COMMAND            /* 2F - command sent from the ground with ground station call and params */
	,ALOG_DISKSPACE 		/* 1F  free disk space */
	,ALOG_FS_STARTUP
	,ALOG_FS_SHUTDOWN
	,ALOG_IORS_LOG_LEVEL  // 1 serial no is the level
//    ,ALOG_FTL0_LOGIN 		/* 2 - user logon */
//    ,ALOG_FTL0_LOGOUT 		/* 1F - user logout */
//    ,ALOG_DISCONNECT = 5		/* 1F - FTL0 user timedout */
//    ,ALOG_USER_REFUSED 		/* 1 - user refused (max sessions) */
//    ,ALOG_BCAST_START  		/* 2F - added to list */
//    ,ALOG_BCAST_STOP 		/* 2F -removed from list */
//    ,ALOG_FILE_DELETE = 10		/* 1F - file deleted */
//    ,ALOG_FILE_DOWNLOAD 		/* 1F - FTL0 file download - obsolete */
//    ,ALOG_FILE_UPLOAD 		/* 1F - FTL0 file upload */
//    ,ALOG_BBS_SHUT 		/* 2 - BBS is shut */
//    ,ALOG_BBS_OPEN 		/* 2 - BBS is open */
//    ,ALOG_DIR = 15			/* 1F - FTL0 directory request - obsolete */
//    ,ALOG_SELECT 		/* 1F - FTL0 Select - obsolete*/
//    ,ALOG_FILE_REMOVED  /* 1F - Autodelete */
//    ,ALOG_FILE_NOT_REMOVED  /* Autodelete failed */
//    ,ALOG_END_DOWNLOAD 	/* 1F - FTL0 End of download - obsolete*/
//    ,ALOG_END_UPLOAD = 20	/* 1F - FTL0 End of upload */
//    ,ALOG_END_DIR 				/* 1F - FTL0 end of downloading dir file - obsolete*/
//    ,ALOG_SELECT_DONE 	/* 1F - End of select - obsolete*/

	,ALOG_NUMBER_OF_EVENTS
};

/* Errors that we want to report in the log.  We should avoid repeating errors that could fill the log.
 * Those should be counted and downlinked in the error telemetry.  e.g. number of I2C failures.*/
enum IORS_LOG_ERRORS {
	IORS_ERR_UNKNOWN
	,IORS_ERR_REMOVING_PID_FILE
	,IORS_ERR_COULD_NOT_STORE_WOD
	,IORS_ERR_SENDING_PKT
	,IORS_ERR_SETTING_TIME
	,IORS_ERR_MAX_RADIO_RETRIES
	,IORS_ERR_MAX_TNC_RETRIES
	,IORS_ERR_SSTV_FAILURE
	,IORS_ERR_PACSAT_FAILURE
	,IORS_ERR_DIREWOLF_FAILURE
	,IORS_ERR_PTT_FAILURE = 10
	,IORS_ERR_TNC_FAILURE
	,IORS_ERR_CREW_INT_FAILURE
	,IORS_ERR_SETTING_RADIO_MODE
	,IORS_ERR_CHECKING_DISK_SPACE
	,IORS_ERR_NUMBER_OF_ERRORS

	// FS ERRORS
	,IORS_ERR_FS_DIR_LOAD_FAILURE = 15
	,IORS_ERR_FS_TNC_FAILURE
};


/* Every entry has an ALOG Header*/
struct ALOG_1 {
	uint8_t event;		/* event code */
	uint8_t len;		/* length of entry */
	uint32_t tstamp;		/* time stamp */
	uint16_t serial_no;		/* serial number or other data for this event */
	uint8_t rxchan;		/* rx channel */
} __attribute__ ((__packed__));


struct ALOG_1F {
	uint8_t event;		/* event code */
	uint8_t len;		/* length of entry */
	uint32_t tstamp;		/* time stamp */
	uint16_t serial_no;		/* serial number */
	uint8_t rxchan;		/* rx channel */
	uint32_t var1;
	uint32_t var2;
	uint32_t var3;
	uint32_t var4;
	uint32_t var5;
	uint32_t var6;

} __attribute__ ((__packed__));

struct ALOG_2 {
	uint8_t event;		/* event code */
	uint8_t len;		/* length of entry */
	uint32_t tstamp;		/* time stamp */
	uint16_t serial_no;		/* serial number */
	uint8_t rxchan;		/* rx channel */
	uint8_t call[6];		/* callsign */
	uint8_t ssid;		/* ssid */
} __attribute__ ((__packed__));

struct ALOG_2F {
	uint8_t event;		/* event code */
	uint8_t len;		/* length of entry */
	uint32_t tstamp;		/* time stamp */
	uint16_t serial_no;		/* serial number */
	uint8_t rxchan;		/* rx channel */
	uint8_t call[6];		/* callsign */
	uint8_t ssid;		/* ssid */
	uint32_t var1;
	uint32_t var2;
	uint32_t var3;
	uint32_t var4;
	uint32_t var5;
	uint32_t var6;
} __attribute__ ((__packed__));

int log_init(char *prefix, char *folder, char *filename, int roll_logs_at_startup);
char * get_log_name_str(enum LOG_NAME name);
void log_set_level(enum LOG_LEVEL level);
void log_make_tmp_filename(char *filename, char *tmp_filename);
void log_err(char *filename, uint8_t error_code);
void log_alog1(int level, char *filename, enum LOG_EVENT event_code, uint16_t var);
void log_alog1f(int level, char *filename, enum LOG_EVENT event_code,
		uint32_t var1,uint32_t var2,uint32_t var3,uint32_t var4,uint32_t var5,uint32_t var6);
void log_alog2(int level, char *filename, enum LOG_EVENT event_code, char * callsign, uint8_t ssid, uint16_t var);
void log_alog2f(int level, char *filename, enum LOG_EVENT event_code, char * callsign, uint8_t ssid,
		uint32_t var1,uint32_t var2,uint32_t var3,uint32_t var4,uint32_t var5,uint32_t var6);
int log_append(char *filename, uint8_t * data, int len);
int log_add_to_directory(char *filename);

void log_debug_print(char * filename);

#endif /* IORS_LOG_H_ */
