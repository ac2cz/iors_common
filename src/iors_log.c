/*
 * iors_log.c
 *
 *  Created on: Jun 2, 2024
 *      Author: g0kla
 *
 * This allows the creating of a log file to store events.  The events are in binary to make them
 * as compact as possible.  The file is closed after each write but is stored in a temportary
 * folder.  After a defined period the log is moved to the queue where it will be added to
 * the pacsat directory.
 *
 * This allows only one activity log to be created because the filename is static.
 *
 * There are four log event formats called ALOG_1, ALOG_1F, ALOG_2 and ALOG_2F.
 * ALOG_1 is 9 bytes and contains event, length, date/time, 16 bits of data and the rx channel
 * ALOG_1F is the full format and adds 6 32 bit data fields
 * ALOG_2 is like ALOG_1 but adds a 6 byte callsign field and ssid.
 * ALOG_2F adds 6 32 bit fields to ALOG_2
 *
 * To keep things even shorter, we do not pass the callsign for all FTL0 events if they are logged.  We
 * pass a serial number in the login event and then use that to index the callsign/session in other events
 *
 * This means that log messages can be kept short but it does make it more complex.
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

/* This MUST match the enums for events in iors_log.h */
//static char *event_text[] = {
//	" ",
//	"STARTUP",
//	"SHUTDOWN",
//	"LOGIN",
//	"LOGOUT",
//	"BLOWOFF",
//	"REFUSED",
//	"BCST ON",
//	"BCST OFF",
//	"FREE DISK",
//	"DELETE",
//	"DOWNLOAD",
//	"UPLOAD",
//	"SHUT",
//	"OPEN",
//	"DIR",
//	"SELECT",
//	"ADEL OK",
//	"ADEL FAIL",
//	"DL DONE",
//	"UL DONE",
//	"DIR DONE",
//	"SEL DONE",
//	"IORS_ERR"
//	};

/* Forward declarations */
//void log_process_prev_file(char * log_folder, char *filename, int roll_logs_at_startup);
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
	strlcpy(filename, folder, MAX_FILE_PATH_LEN);
	strlcat(filename,"/",MAX_FILE_PATH_LEN);
	strlcat(filename,prefix,MAX_FILE_PATH_LEN); // put the folder as the first part of the name too

//	/* Process any previous logs left over from a crash or other situation */
//	log_process_prev_file(folder, filename, roll_logs_at_startup);

	//debug_print("Opening log: %s\n",filename);

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

///**
// * Looks for any previous logs in the folder and add them to the pacsat dir queue.
// * We do that by removing the .tmp extension.
// * Note that it is possible that the file was corrupted as we crashed, but that is an
// * issue for the ground station to solve.
// *
// */
//void log_process_prev_file(char * log_folder, char *filename, int roll_logs_at_startup) {
//	char tmp_filename[MAX_FILE_PATH_LEN];
//	log_make_tmp_filename(filename, tmp_filename);
//	DIR * d = opendir(log_folder);
//	if (d == NULL) { error_print("** Could not open dir: %s\n",log_folder); return; }
//	struct dirent *de;
//	char file_name[MAX_FILE_PATH_LEN];
//	char to_filename[MAX_FILE_PATH_LEN];
//	for (de = readdir(d); de != NULL; de = readdir(d)) {
//		if ((strcmp(de->d_name, ".") != 0) && (strcmp(de->d_name, "..") != 0)) {
//			if (str_ends_with(de->d_name, FILE_TMP)) {
//				strlcpy(file_name, log_folder, sizeof(file_name));
//				strlcat(file_name, "/", sizeof(file_name));
//				strlcat(file_name, de->d_name, sizeof(file_name));
//				if (roll_logs_at_startup && strncmp(file_name, tmp_filename, sizeof(file_name)) == 0) {
//					debug_print("leaving current log %s\n",de->d_name);
//				} else {
//					strlcpy(to_filename, file_name, sizeof(to_filename));
//					to_filename[strlen(to_filename) - 4] = '\0'; // We know this has exactly .tmp at the end, so remove it
//					debug_print("Processing old log file: %s to %s\n",de->d_name, to_filename);
//					rename(file_name,to_filename);
//				}
//			}
//		}
//	}
//}

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
	struct ALOG_1 log_event;
	log_event.event = ALOG_IORS_ERR;
	log_event.len = sizeof(log_event);
	log_event.tstamp = time(0);
	log_event.rxchan = 0;
	log_event.serial_no = error_code;
	log_append(filename, (uint8_t *)&log_event, log_event.len);
}

/*
 * Store Log event
 */
void log_alog1(int level, char *filename, enum LOG_EVENT event_code, uint16_t var) {
	if (level > log_level) return;
	struct ALOG_1 log_event;
	log_event.event = event_code;
	log_event.len = sizeof(log_event);
	log_event.tstamp = time(0);
	log_event.rxchan = 0;
	log_event.serial_no = var;
	log_append(filename, (uint8_t *)&log_event, log_event.len);
}

void log_alog1f(int level, char *filename, enum LOG_EVENT event_code,
		uint32_t var1,uint32_t var2,uint32_t var3,uint32_t var4,uint32_t var5,uint32_t var6) {
	if (level > log_level) return;
	struct ALOG_1F log_event;
	log_event.event = event_code;
	log_event.len = sizeof(log_event);
	log_event.tstamp = time(0);
	log_event.rxchan = 0;
	log_event.var1 = var1;
	log_event.var2 = var2;
	log_event.var3 = var3;
	log_event.var4 = var4;
	log_event.var5 = var5;
	log_event.var6 = var6;
	log_append(filename, (uint8_t *)&log_event, log_event.len);
}

void log_alog2(int level, char *filename, enum LOG_EVENT event_code, char * callsign, uint8_t ssid, uint16_t var) {
	if (level > log_level) return;
	struct ALOG_2 log_event;
	log_event.event = event_code;
	log_event.len = sizeof(log_event);
	log_event.tstamp = time(0);
	log_event.serial_no = var;
	log_event.rxchan = 0;
	memcpy(log_event.call, callsign, sizeof(log_event.call));
	log_event.ssid = ssid;
	log_append(filename, (uint8_t *)&log_event, log_event.len);
}

void log_alog2f(int level, char *filename, enum LOG_EVENT event_code, char * callsign, uint8_t ssid,
		uint32_t var1,uint32_t var2,uint32_t var3,uint32_t var4,uint32_t var5,uint32_t var6) {
	if (level > log_level) return;
	struct ALOG_2F log_event;
	log_event.event = event_code;
	log_event.len = sizeof(log_event);
	log_event.tstamp = time(0);
	log_event.rxchan = 0;
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
 * Add the log file to the pacsat directory by removing the tmp extension and giving it a timestamp.
 */
int log_add_to_directory(char * filename) {
	char log_name[25];
	time_t now = time(0);
	strftime(log_name, sizeof(log_name), "%y%m%d%H%M", gmtime(&now)); // If enabled, roll of log if reboot on different min.  Use mins as that is the resolution of the scheduler

	char tmp_filename[MAX_FILE_PATH_LEN];
	char dir_filename[MAX_FILE_PATH_LEN];
	log_make_tmp_filename(filename, tmp_filename);
	strlcpy(dir_filename, filename,MAX_FILE_PATH_LEN);
	strlcat(dir_filename,log_name,MAX_FILE_PATH_LEN);
	debug_print("Adding %s log to dir as: %s\n",filename, dir_filename);
	int rc = rename(tmp_filename,dir_filename);
	if (rc == -1) {
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

/**
 * Note, this can not read compressed logs
 */
//void log_debug_print(char * filename) {
//	char buffer[256];
//	struct ALOG_1 *alog_err = (struct ALOG_1 *)buffer;
//	FILE * infile=fopen(filename,"rb");
//	if (infile == NULL) {
//		return;
//	}
//	debug_print("IORS Activity Log\n");
//	debug_print("Timestamp            Event    Data\n");
//	while(1) {
//		int count = fread(buffer, 1, 2, infile);
//		if (count != 2) {
//			fclose(infile);
//			return; // we could not read from the infile, probablly at the end
//		}
//		if (alog_err->len != sizeof (struct ALOG_1)) {
//			debug_print("File is damaged 2\n");
//			return;
//		}
//		count = fread(buffer+2, 1, alog_err->len-2, infile);
//		if (count+2 != alog_err->len) {
//			debug_print("File is damaged 3\n");
//			return;
//		}
//		char buf[30];
//		time_t now = alog_err->tstamp;
//		strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", gmtime(&now));
//		if (alog_err->event > 0 && alog_err->event < ALOG_NUMBER_OF_EVENTS)
//			debug_print("%s  %s %d\n", buf, event_text[alog_err->event], alog_err->serial_no);
//		else
//			debug_print("Unknown Event: %d\n",alog_err->event);
//	}
//	fclose(infile);
//}

