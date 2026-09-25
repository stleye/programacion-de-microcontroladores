/*
 * API_cmdparser.c
 *
 *  Created on: Sep 24, 2026
 *      Author: sebastian
 */

#include "API_cmdparser.h"
#include "API_uart.h"

typedef enum {
	CMD_IDLE, CMD_RECEIVING, CMD_PROCESS, CMD_EXEC, CMD_ERROR,
} cmdParserState_t;

#define CMD_MAX_BYTES_PER_POLL 16

static cmdParserState_t currentState;

void cmdParserInit(void) {
	currentState = CMD_IDLE;
}

void cmdPoll(void) {
	uint8_t c;
	for (int i = 0; i < CMD_MAX_BYTES_PER_POLL; i++) {
		if (!uartReceiveStringSize(&c, 1)) {
			break;
		}
		if (c == '\r') {
			uartSendString((uint8_t*) "\r\n");
		} else if (c == 0x7F || c == '\b') {
			uartSendString((uint8_t*) "\b \b");
		} else {
			uartSendStringSize(&c, 1);
		}
	}
}

void cmdPrintHelp(void) {

}

void uartCmdParserFSM_update() {
	switch (currentState) {
	case CMD_IDLE:
		char msg[80];
		uartReceiveStringSize((uint8_t*) msg, 1);
		currentState = CMD_RECEIVING;
		break;
	case CMD_RECEIVING:
		break;
	case CMD_PROCESS:
		break;
	case CMD_EXEC:
		break;
	case CMD_ERROR:
		break;
	default:
		cmdParserInit();
	}
}
