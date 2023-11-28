/*
 * ax25_tools.h
 *
 *  Created on: Oct 17, 2022
 *      Author: g0kla
 */

#ifndef AX25_TOOLS_H_
#define AX25_TOOLS_H_

struct t_ax25_header {
	unsigned char flag;
	unsigned char to_callsign[7];
	unsigned char from_callsign[7];
	unsigned char control_byte;
	unsigned char pid;
} __attribute__ ((__packed__));
typedef struct t_ax25_header AX25_HEADER;

int encode_call(char *name, unsigned char *buf, int final_call, char command);

#endif /* AX25_TOOLS_H_ */
