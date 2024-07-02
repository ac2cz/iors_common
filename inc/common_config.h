/*
 * common_config.h
 *
 *  Created on: Sep 28, 2022
 *      Author: g0kla
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 */

#ifndef COMMON_CONFIG_H_
#define COMMON_CONFIG_H_

#include "debug.h"

#define COMMON_VERSION __DATE__ " iors_common - Version 0.1"
#define DEBUG 1
#define true 1
#define false 0
#define MAX_CALLSIGN_LEN 10
#define MAX_FILE_PATH_LEN 256
#define AX25_MAX_DATA_LEN 2048
#define AGW_PORT2 8002
#define AGW_PORT 8000
#define IORS_PORT 8010
#define MAX_CONFIG_LINE_LENGTH 128

#define CLOCK_2024_01_01 1704085200

extern int g_common_bit_rate; /* the bit rate of the TNC - 1200 4800 9600. Change actual value in DireWolf) */
extern int g_common_max_frames_in_tx_buffer;
extern int g_common_frames_queued;
extern char g_iors_last_command_time_path[MAX_FILE_PATH_LEN];

#endif /* COMMON_CONFIG_H_ */
