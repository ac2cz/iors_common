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

enum LOG_EVENT {
	/* Initial event numbers are historical to match the PACSAT ALOG events */
     ALOG_FTL0_STARTUP = 1		/* ftl0 startup */
    ,ALOG_FTL0_SHUTDOWN 		/* ftl0 shutdown */
    ,ALOG_START_SESSION 		/* user logon */
    ,ALOG_CLOSE_SESSION 		/* user logout */
    ,ALOG_DISCONNECT = 5		/* user timedout */
    ,ALOG_USER_REFUSED 		/* user refused (max sessions) */
    ,ALOG_BCAST_START  		/* added to list */
    ,ALOG_BCAST_STOP 		/* removed from list */
    ,ALOG_DISKSPACE 		/* free disk space */
    ,ALOG_FILE_DELETE = 10		/* file deleted */
    ,ALOG_FILE_DOWNLOAD 		/* file download */
    ,ALOG_FILE_UPLOAD 		/* file upload */
    ,ALOG_BBS_SHUT 		/* BBS is shut */
    ,ALOG_BBS_OPEN 		/* BBS is open */
    ,ALOG_DIR = 15			/* directory request */
    ,ALOG_SELECT 		/* Select */
    ,ALOG_FILE_REMOVED  /* Autodelete */
    ,ALOG_FILE_NOT_REMOVED  /* Autodelete failed */
    ,ALOG_END_DOWNLOAD 	/* End of download */
    ,ALOG_END_UPLOAD = 20	/* End of download */
    ,ALOG_END_DIR 				/* end of downloading dir file */
    ,ALOG_SELECT_DONE 	/* End of select */

	/* Events after this will not be decoded by historical ALOG program */
	,ALOG_IORS_ERR = 23
	,ALOG_NUMBER_OF_EVENTS
};

/* Every entry has an ALOG Header*/
struct ALOG_err {
	uint8_t event;		/* event code */
	uint8_t len;		    /* length of entry including the header */
	uint32_t tstamp;			/* time stamp */
	uint8_t err_code;		/* error code to be saved in log for this event */
};

/* Historical ALOG structures */
struct ALOG_1 {
	uint8_t event;		/* event code */
	uint8_t len;		/* length of entry */
	uint32_t tstamp;		/* time stamp */
	uint16_t serial_no;		/* serial number */
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

int log_init(char *prefix, char *folder, char *filename);
char * get_log_name_str(enum LOG_NAME name);
void log_set_level(enum LOG_LEVEL level);
void log_make_tmp_filename(char *filename, char *tmp_filename);
void log_err(char *filename, uint8_t error_code);
void log_alog1(char *filename, enum LOG_EVENT event_code);
void log_alog1f(char *filename, enum LOG_EVENT event_code,
		uint32_t var1,uint32_t var2,uint32_t var3,uint32_t var4,uint32_t var5,uint32_t var6);
void log_alog2(char *filename, enum LOG_EVENT event_code, char * callsign, uint8_t ssid);
void log_alog2f(char *filename, enum LOG_EVENT event_code, char * callsign, uint8_t ssid,
		uint32_t var1,uint32_t var2,uint32_t var3,uint32_t var4,uint32_t var5,uint32_t var6);
int log_append(char *filename, uint8_t * data, int len);
int log_add_to_directory(char *filename);

void log_debug_print(char * filename);

#endif /* IORS_LOG_H_ */
