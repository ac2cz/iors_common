/*
 * iors_command.c
 *
 *  Created on: Nov 27, 2023
 *      Author: g0kla
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "common_config.h"
#include "str_util.h"
#include "iors_command.h"
#include "hmac_sha256.h"
#include "keyfile.h"

/* Forwards */
int load_last_command_time();
int store_last_command_time();
int CommandTimeOK(uint32_t dateTime);

char last_command_time_path[MAX_FILE_PATH_LEN] = "pacsat_last_command_time.dat";
static uint32_t last_command_time = 0x0; /* Keep track of the time that the last command was received */
static SWCmdUplink last_command;

/* This defines the folder names that can be referenced in commands using the ids in FolderIds
 * IMPORTANT - Must also change the enum in iors_command.h that corresponds to this */
char *FolderIdStrings[] = {
		"sstv_q1"
		,"sstv_q2"
		,"sstv_q3"
		,"sstv_q4"
		,"sstv_q5"
		,"sstv_q6"
		,"sstv_q7"
		,"sstv_q8"
		,"sstv_q9"
		,"orbital_positions"
		,"bin" //4
		,"lib"
		,"cfg"
		,"pacsat/dir"
		,"pacsat/upload"
		,"pacsat/wod"
		,"pacsat/log"
		,"pacsat/txt"
		,"data1"
		,"data2"
		,"data3"
		,"data4"
		,"data5"
		,"data6"
		,"data7"
		,"data8"
		,"data9"
};

/* This gives user understandable names for the name spaces.  It must match the enum in iors_command.h */
char *NameSpaceStrings[] = {
		"-"
		,"ops"
		,"telem"
		,"fs"
};

/* This gives user understandable names for the OPS Commands.  It must match the enum in iors_command.h */
char *OpsCommandStrings[] = {
		"reserved"
		,"pm1"
		,"xband"
		,"aprs"
		,"crew" // 4
		,"safe"
		,"fs"
		,"telem"
		,"ant-sel"
		,"time"
		,"reset-cpu" //10
		,"reset-radio"
		,"enable-telem"
		,"enable-time"
		,"enable-status"
		,"reset-iors"
		,"pkt-rate" //16
		,"roll-log"
		,"enable-wod"
		,"enable-err"
		,"sstv-set-mode" //20
		,"sstv-loop"
		,"sstv-stop"
		,"sstv-pre"
		,"nop"
		,"cmd-key"
		,"hdmi"
};
char *PacsatCommandStrings[] = {
		"reserved"
		,"pb"
		,"uplink"
		,"install"
		,"del" // 4
		,"del-folder" // 5
		,"default-file-exp-period"
		,"file-exp-period"
		,"dir-maint-period"
		,"up-maint-period"
		,"que-chk-period" // 10
		,"max-file-size"
		,"max-upload-age"
		,"exec-file" //13
};

int SymbolRates[] = {
		1200
		,9600
};

SWCmdUplink *get_last_command() {
	return &last_command;
}

void init_commanding() {
	load_last_command_time();
}

char * get_folder_str(FolderIds i) {
	if (i < 0 || i > NumberOfFolderIds) return NULL;
	return FolderIdStrings[i];
}

int get_symbol_rates(SymbolRateIds i) {
	if (i < 0 || i > NumberOfSymbolRateIds) return -1;
	return SymbolRates[i];
}

int get_namespace_from_str(char *name_space) {
	int i = 0;
	for (i = 0; i < SWCmdNumberOfNamespaces; i++) {
		if (strcasecmp(name_space, NameSpaceStrings[i]) == 0) {
			return i;
		}
	}
	return SWCmdNSReserved;
}

int get_command_from_str(SWCommandNameSpace name_space, char * cmd) {
	if (name_space == SWCmdNSOps) {
		int i = 0;
		for (i = 0; i < SWCmdOpsNumberOfCommands; i++) {
			if (strcasecmp(cmd, OpsCommandStrings[i]) == 0) {
				return i;
			}
		}
	}
	if (name_space == SWCmdNSPacsat) {
		int i = 0;
		for (i = 0; i < SwCmdPacsatNumberOfCommands; i++) {
			if (strcasecmp(cmd, PacsatCommandStrings[i]) == 0) {
				return i;
			}
		}
	}
	return SWCmdOpsReserved;
}

int load_last_command_time() {
	FILE * fd = fopen(last_command_time_path, "r");
	if (fd == NULL) {
		// no command time file, make a new one
		store_last_command_time();
	} else {
		char line [ MAX_CONFIG_LINE_LENGTH ]; /* or other suitable maximum line size */
		fgets ( line, sizeof line, fd ); /* read a line */
		line[strcspn(line,"\n")] = 0; // Move the nul termination to get rid of the new line
		last_command_time = atol(line);
		debug_print("Last Command Time was: %d\n",last_command_time);
		fclose(fd);
	}

	return EXIT_SUCCESS;
}

int store_last_command_time() {
	char tmp_filename[MAX_FILE_PATH_LEN];
	strlcpy(tmp_filename, last_command_time_path, sizeof(tmp_filename));
	strlcat(tmp_filename, ".tmp", sizeof(tmp_filename));
	FILE * fd = fopen(tmp_filename, "w");
	if (fd == NULL) {
		// Not fatal but we will forget the time when we restart
		error_print("Could not open the time command file\n");
		return EXIT_FAILURE;
	}
	int rc = fprintf(fd, "%d\n", last_command_time);
	if (rc <= 0) {
		error_print("Could not write to the time command file: error %d\n",rc);
		fclose(fd);
		return EXIT_FAILURE;
	}

	fclose(fd);
	/* Use rename as the last step so that we get the whole new file or stay with the old.  If
	 * we crash here then we are safer with the last previous command time than with no previous command time. */
	rename(tmp_filename, last_command_time_path);
	return EXIT_SUCCESS;
}

/**
 * Authenticate a packet that contains a 32 bit date time, an arbitrary number of bytes
 * and a 32 byte authentication vector.
 * The vector is calculated with the 32 byte authentication key
 *
 */
int AuthenticatePacket(uint32_t date_time_in_packet, uint8_t * uplink, int pkt_len, uint8_t *auth_vector) {
	uint8_t localSecureHash[32];
	int shaOK;

	hmac_sha256(hmac_sha_key, AUTH_KEY_SIZE,
			uplink, pkt_len,
			localSecureHash, sizeof(localSecureHash));
	shaOK = (memcmp(localSecureHash, auth_vector, 32) == 0);
	    if (0) {
	        debug_print("Local:  ");
	        int i;
	        for (i=0; i<sizeof(localSecureHash);i++)
	        	debug_print("%x ", localSecureHash[i]);
	        debug_print("\nUplink: ");
	        for (i=0; i<sizeof(localSecureHash);i++)
	        	debug_print("%x ", auth_vector[i]);
	        debug_print("\n");
	    }
	if(shaOK){
		return CommandTimeOK(date_time_in_packet);
	} else {
		return EXIT_FAILURE;
	}
}

/**
 * RETURNs EXIT_SUCCESS = 0, EXIT_FAILURE = 1, DUPLICATE = 2
 */
int AuthenticateSoftwareCommand(SWCmdUplink *uplink) {
	return AuthenticatePacket(uplink->dateTime, (uint8_t *)uplink, SW_COMMAND_SIZE,uplink->AuthenticationVector);

//    uint8_t localSecureHash[32];
//    int shaOK;
//
//    hmac_sha256(hmac_sha_key, AUTH_KEY_SIZE,
//                (uint8_t *) uplink, SW_COMMAND_SIZE,
//                localSecureHash, sizeof(localSecureHash));
//    shaOK = (memcmp(localSecureHash, uplink->AuthenticationVector, 32) == 0);
////    if (0) {
////        debug_print("Local: ");
////        int i;
////        for (i=0; i<sizeof(localSecureHash);i++)
////        	debug_print("%x ", localSecureHash[i]);
////        debug_print("\nUplink: ");
////        for (i=0; i<sizeof(uplink->AuthenticationVector);i++)
////        	debug_print("%x ", uplink->AuthenticationVector[i]);
////        debug_print("\n");
////    }
//    if(shaOK){
//        uplink->comArg.command = (uplink->comArg.command); // We might have to look to determine if authenticated
//        return CommandTimeOK(uplink->dateTime);
//    } else {
//       // localErrorCollection.DCTCmdFailAuthenticateCnt++;
//        return EXIT_FAILURE;
//    }

}

/**
 * CommandTimeOK()
 * Here we attempt to prevent a replay attack.  Firstly we keep track of the time that the last
 * command was received.  Our clock on the space station may be wrong but we assume the time
 * in the receieved packet is correct.  A command can not have a time that is the same or
 * before a previous command.
 *
 * If a command was not received but was intercepted by another station then they could replay the
 * command.  This is a small risk because we probablly wanted to send the command anyway.  This risk
 * is also ended once a new command is received as the time of the later packet will have been saved.
 *
 * If our clock is working on the station then we could check that the time on a received packet is
 * close to the actual time.  This mitigates replay attacks for packets that are not received but
 * we may not be able to send commands if the time on the station is wrong or corrupted.
 *
 * One other nuance.  It is common for the ground station to send a command, which is accepted, but
 * to miss the ACK.  It then sends the command again.  These commands have the same dateTime.  A
 * duplicate command received within a tolerance period based should receive an ACK but should
 * not make further changes.  This means that commands should check if they have just executed and
 * do nothing if called again.  e.g. don't change channels if already on that channel.
 *
 * RETURNs EXIT_SUCCESS = 0, EXIT_FAILURE = 1, DUPLICATE = 2
 */
int CommandTimeOK(uint32_t dateTime) {

	if (last_command_time > MAX_COMMAND_TIME || last_command_time < MIN_COMMAND_TIME) {
		// Then it is likely corrupt, set to zero for this check and take the time from the command
		last_command_time = 0;
	}
	if (last_command_time == dateTime) {
		// Duplicate command, ignore
		debug_print("Duplicate Command: %d Last Command %d\n",dateTime, last_command_time);
		return EXIT_DUPLICATE; // DUPLICATE
	}
    if ((dateTime + COMMAND_TIME_TOLLERANCE) <= last_command_time) {
    	debug_print("Command: Bad time on command!\n");
    	debug_print("Command: %d Last Command %d\n",dateTime, last_command_time);
    	return EXIT_FAILURE;
    } else {
    	last_command_time = dateTime;
    	store_last_command_time();
    }
    return EXIT_SUCCESS;
}
