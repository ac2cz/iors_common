/*
 * iors_log.c
 *
 *  Created on: Jun 2, 2024
 *      Author: g0kla
 *
 * This allows the creating of a log file to store events.  The events are in binary and
 * as compact as possible.  They file is closed after each write but is stored in a temportary
 * folder.  After a defined period the log is moved to the queue where it will be added to
 * the pacsat directory.
 *
 * This allows only one activity log to be created because the filename is static.
 *
 * At program start we check if there is a left over activity log and this is added to the
 * pacsat dir queue.  A new one is then started.
 *
 */

#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <dirent.h>

#include "common_config.h"
#include "iors_log.h"
#include "str_util.h"

/* Local static variables */
static int log_level = ERR_LOG;
//static char log_folder[MAX_FILE_PATH_LEN];
//static char tmp_filename[MAX_FILE_PATH_LEN];
//static char filename[MAX_FILE_PATH_LEN];

static char *log_name_str[] = {
	"log"
	,"wod"
	,"err"
};

static char *event_text[] = {
	" ",
	"STARTUP",
	"SHUTDOWN",
	"LOGIN",
	"LOGOUT",
	"BLOWOFF",
	"REFUSED",
	"BCST ON",
	"BCST OFF",
	"FREE DISK",
	"DELETE",
	"DOWNLOAD",
	"UPLOAD",
	"SHUT",
	"OPEN",
	"DIR",
	"SELECT",
	"ADEL OK",
	"ADEL FAIL",
	"DL DONE",
	"UL DONE",
	"DIR DONE",
	"SEL DONE",
	"IORS_ERR"
	};

/* Forward declarations */
void log_process_prev_file(char * log_folder, char *filename);
int log_append(char *filename, uint8_t * data, int len);

/**
 * log_init()
 *
 * Process a previous log if it exists.  Then start a new log.
 *
 * The filepath should be the target filename in the queue folder.  Data will be saved in
 * a .tmp file until it is ready to be added to the directory.
 *
 */
int log_init(char *prefix, char *folder, char *filename) {
	char log_name[25];
	time_t now = time(0);
	strftime(log_name, sizeof(log_name), "%y%m%d%H", gmtime(&now));
	strlcpy(filename, folder, MAX_FILE_PATH_LEN);
	strlcat(filename,"/",MAX_FILE_PATH_LEN);
	strlcat(filename,prefix,MAX_FILE_PATH_LEN); // put the folder as the first part of the name too
	strlcat(filename,log_name,MAX_FILE_PATH_LEN);

	log_process_prev_file(folder, filename);

	debug_print("Opening log: %s\n",filename);

	return EXIT_SUCCESS;
}

char * get_log_name_str(enum LOG_NAME name) {
	if (name < 0 || name >= NumberOfLogNames)
		return log_name_str[0];
	return log_name_str[name];
}

void log_make_tmp_filename(char *filename, char *tmp_filename) {
	strlcpy(tmp_filename, filename, MAX_FILE_PATH_LEN);
	strlcat(tmp_filename,".tmp",MAX_FILE_PATH_LEN);
}

/**
 * Looks for any previous logs in the folder and add them to the pacsat dir queue.
 * We do that by removing the .tmp extension.
 * Note that it is possible that the file was corrupted as we crashed, but that is an
 * issue for the ground station to solve.
 *
 */
void log_process_prev_file(char * log_folder, char *filename) {
	char tmp_filename[MAX_FILE_PATH_LEN];
	log_make_tmp_filename(filename, tmp_filename);
	DIR * d = opendir(log_folder);
	if (d == NULL) { error_print("** Could not open dir: %s\n",log_folder); return; }
	struct dirent *de;
	char file_name[MAX_FILE_PATH_LEN];
	char to_filename[MAX_FILE_PATH_LEN];
	for (de = readdir(d); de != NULL; de = readdir(d)) {
		if ((strcmp(de->d_name, ".") != 0) && (strcmp(de->d_name, "..") != 0)) {
			if (str_ends_with(de->d_name, FILE_TMP)) {
				strlcpy(file_name, log_folder, sizeof(file_name));
				strlcat(file_name, "/", sizeof(file_name));
				strlcat(file_name, de->d_name, sizeof(file_name));
				if (strncmp(file_name, tmp_filename, sizeof(file_name)) == 0) {
					debug_print("leaving todays log %s\n",de->d_name);
				} else {
					strlcpy(to_filename, file_name, sizeof(to_filename));
					to_filename[strlen(to_filename) - 4] = '\0'; // We know this has exactly .tmp at the end, so remove it
					debug_print("Processing old log file: %s to %s\n",de->d_name, to_filename);
					rename(file_name,filename);
				}
			}
		}
	}
}

void log_set_level(enum LOG_LEVEL level) {
	log_level = level;
}

/**
 * log_err()
 * Log an error.  The code is stored in the log.
 * If log level is set below ERR_LOG then nothing is logged.
 *
 */
void log_err(char *filename, uint8_t error_code) {
	if (log_level < ERR_LOG) return;
	struct ALOG_err log_err;
	log_err.event = ALOG_IORS_ERR;
	log_err.len = sizeof(log_err);
	log_err.tstamp = time(0);
	log_err.err_code = error_code;
	log_append(filename, (uint8_t *)&log_err, log_err.len);
}

void log_alog1(char *filename, enum LOG_EVENT event_code) {
	if (log_level < ERR_LOG) return;
	struct ALOG_1 log_event;
	log_event.event = event_code;
	log_event.len = sizeof(log_event);
	log_event.tstamp = time(0);
	log_event.rxchan = 1;
	log_append(filename, (uint8_t *)&log_event, log_event.len);
}

void log_alog1f(char *filename, enum LOG_EVENT event_code,
		uint32_t var1,uint32_t var2,uint32_t var3,uint32_t var4,uint32_t var5,uint32_t var6) {
	if (log_level < ERR_LOG) return;
	struct ALOG_1F log_event;
	log_event.event = event_code;
	log_event.len = sizeof(log_event);
	log_event.tstamp = time(0);
	log_event.rxchan = 1;
	log_event.var1 = var1;
	log_event.var2 = var2;
	log_event.var3 = var3;
	log_event.var4 = var4;
	log_event.var5 = var5;
	log_event.var6 = var6;
	log_append(filename, (uint8_t *)&log_event, log_event.len);
}

void log_alog2(char *filename, enum LOG_EVENT event_code, char * callsign, uint8_t ssid) {
	if (log_level < ERR_LOG) return;
	struct ALOG_2 log_event;
	log_event.event = event_code;
	log_event.len = sizeof(log_event);
	log_event.tstamp = time(0);
	log_event.rxchan = 1;
	memcpy(log_event.call, callsign, sizeof(log_event.call));
	log_event.ssid = ssid;
	log_append(filename, (uint8_t *)&log_event, log_event.len);
}

void log_alog2f(char *filename, enum LOG_EVENT event_code, char * callsign, uint8_t ssid,
		uint32_t var1,uint32_t var2,uint32_t var3,uint32_t var4,uint32_t var5,uint32_t var6) {
	if (log_level < ERR_LOG) return;
	struct ALOG_2F log_event;
	log_event.event = event_code;
	log_event.len = sizeof(log_event);
	log_event.tstamp = time(0);
	log_event.rxchan = 1;
	log_event.var1 = var1;
	log_event.var2 = var2;
	log_event.var3 = var3;
	log_event.var4 = var4;
	log_event.var5 = var5;
	log_event.var6 = var6;
	memcpy(log_event.call, callsign, sizeof(log_event.call));
	log_event.ssid = ssid;
	log_append(filename, (uint8_t *)&log_event, log_event.len);
}
/**
 * log_append()
 * Append a log event to the binary log file
 * TODO:
 * Note that if we crash while writing to the log then the data could be corrupt. We
 * could fix this by writing to a tmp file but we would have to copy the file first
 * and then rename it over the top of the original.  This would make it not thread safe
 * unless we block a second process that tries to write.  In that case we could then crash
 * and leave the tmp file which would block all future writes.  So some design thought is
 * needed.  For now we keep it simple and leave a corrupted log as an issue for the
 * ground station to deal with.
 *
 * One other solutiuon would be to frame each event, e.g. like a kiss frame.  Then if
 * an event is partially written we will not corrupt the rest of the log.
 *
 */
int log_append(char *filename, uint8_t * data, int len) {
	char tmp_filename[MAX_FILE_PATH_LEN];
	log_make_tmp_filename(filename, tmp_filename);
	FILE * outfile = fopen(tmp_filename, "ab");
	if (outfile == NULL) return EXIT_FAILURE;

	/* Save the header bytes */
	for (int i=0; i<len; i++) {
		int c = fputc(data[i],outfile);
		if (c == EOF) {
			fclose(outfile);
			return EXIT_FAILURE; // we could not write to the file
		}
	}
	fclose(outfile);

	return EXIT_SUCCESS;
}

/**
 * log_add_to_directory()
 * Add the log file to the pacsat directory and start a new log.  Specifically
 * this is added to the log queue.
 */
int log_add_to_directory(char * filename) {
	char tmp_filename[MAX_FILE_PATH_LEN];
	log_make_tmp_filename(filename, tmp_filename);
	debug_print("Finishing log: %s\n",filename);
	int rc = rename(tmp_filename,filename);
	if (rc == -1) {
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

/**
 * Note, this can not read compressed logs
 */
void log_debug_print(char * filename) {
	char buffer[256];
	struct ALOG_err *alog_err = (struct ALOG_err *)buffer;
	FILE * infile=fopen(filename,"rb");
	if (infile == NULL) {
		return;
	}
	debug_print("IORS Activity Log\n");
	debug_print("Timestamp            Event    Data\n");
	while(1) {
		int count = fread(buffer, 1, 2, infile);
		if (count != 2) {
			fclose(infile);
			return; // we could not read from the infile, probablly at the end
		}
		if (alog_err->len != sizeof (struct ALOG_err)) {
			debug_print("File is damaged 2\n");
			return;
		}
		count = fread(buffer+2, 1, alog_err->len-2, infile);
		if (count+2 != alog_err->len) {
			debug_print("File is damaged 3\n");
			return;
		}
		char buf[30];
		time_t now = alog_err->tstamp;
		strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", gmtime(&now));
		if (alog_err->event > 0 && alog_err->event < ALOG_NUMBER_OF_EVENTS)
			debug_print("%s  %s %d\n", buf, event_text[alog_err->event], alog_err->err_code);
		else
			debug_print("Unknown Event: %d\n",alog_err->event);
	}
	fclose(infile);
}

/*------------------------------------------------------------------------
	Checks the type of record in the global alog structure and returns
	an appropriate integer. Generally, this tells if the record type is
	alog1 or alog2, though some other divisions have been introduced.

	START_SESSION MUST RETURN 2
	CONNECTED MODE OPERATIONS MUST RETURN 1
	ADMINISTRATIVE OPERATIONS MUST RETURN 0

	0 means data is stored in ALOG1F and is system generated??
	1 means data is stored in ALOG1F
	2 means data is stored in ALOG2F
------------------------------------------------------------------------*/
int alog_struct_type(enum LOG_EVENT event){

	int type;

	switch(event){

		/* All of these have callsigns in them */
		case ALOG_START_SESSION:
		case ALOG_BCAST_START:
		case ALOG_BCAST_STOP:
		case ALOG_FILE_DELETE:
		case ALOG_BBS_SHUT:
		case ALOG_BBS_OPEN:
			type = 2;
		break;

		/* These don't have or refer to callsigns. This is the */
		/* group containing all automatic s/c generated events.*/
		case ALOG_FTL0_STARTUP:
		case ALOG_FTL0_SHUTDOWN:
		case ALOG_DISKSPACE:
		case ALOG_FILE_REMOVED:
		case ALOG_FILE_NOT_REMOVED:
			type = 0;
		break;

		/* All connected mode events */
		case ALOG_CLOSE_SESSION:
		case ALOG_DISCONNECT:
		case ALOG_FILE_DOWNLOAD:
		case ALOG_FILE_UPLOAD:
		case ALOG_DIR:
		case ALOG_SELECT:
		case ALOG_END_DOWNLOAD:
		case ALOG_END_UPLOAD:
		case ALOG_END_DIR:
		case ALOG_SELECT_DONE:
			type = 1;
		break;

		/* Put refusals in a class by themselves, because they are anoying */
		case ALOG_USER_REFUSED:
			type = 4;
		break;

		default:
			type = 3;
		break;

	}
	return type;
}

