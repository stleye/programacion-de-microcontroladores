/*
 * API_cmdparser.c
 *
 *  Created on: Sep 24, 2026
 *      Author: sebastian
 */

#include "API_cmdparser.h"
#include "API_uart.h"
#include "stm32f4xx_hal.h"
#include <strings.h>

#define CMD_MAX_BYTES_PER_POLL 16

#define CHAR_DEL 0x7F

#define LD2_Pin GPIO_PIN_5
#define LD2_GPIO_Port GPIOA

typedef enum {
	CMD_IDLE, CMD_RECEIVING, CMD_PROCESS, CMD_EXEC, CMD_ERROR,
} cmdParserState_t;

typedef enum {
	CMD_ID_NONE, CMD_ID_HELP, CMD_ID_LED_ON, CMD_ID_LED_OFF, CMD_ID_LED_TOGGLE, CMD_ID_STATUS,
} cmdId_t;

static cmdParserState_t currentState;
static char line[CMD_MAX_LINE];
static uint8_t lineIndex;
static cmd_status_t lastError;
static bool_t discardingLine;
static cmdId_t pendingCommand;

static bool_t isTerminator(uint8_t c);
static bool_t isDelete(uint8_t c);
static bool_t isComment(const char *pline);
static bool_t isBlank(char c);
static uint8_t cmdTokenize(char *tokens[]);
static cmd_status_t cmdProcessLine(void);
static void cmdPrintError(cmd_status_t status);
static void cmdExecute(cmdId_t command);

static bool_t isTerminator(uint8_t c) {
	return c == '\r' || c == '\n';
}

static bool_t isDelete(uint8_t c) {
	return c == CHAR_DEL || c == '\b';
}

static bool_t isComment(const char *pline) {
	return pline[0] == '#' || (pline[0] == '/' && pline[1] == '/');
}

static bool_t isBlank(char c) {
	return c == ' ' || c == '\t';
}

static uint8_t cmdTokenize(char *tokens[]) {
	uint8_t count = 0;
	char *p = line;

	while (*p != '\0') {
		while (isBlank(*p)) {
			*p++ = '\0';
		}
		if (*p == '\0') {
			break;
		}
		if (count == CMD_MAX_TOKENS) {
			return CMD_MAX_TOKENS + 1;
		}
		tokens[count++] = p;
		while (*p != '\0' && !isBlank(*p)) {
			p++;
		}
	}
	return count;
}

static cmd_status_t cmdProcessLine(void) {
	char *tokens[CMD_MAX_TOKENS];
	uint8_t count = cmdTokenize(tokens);

	pendingCommand = CMD_ID_NONE;

	if (count == 0) {
		return CMD_OK;
	}
	if (count > CMD_MAX_TOKENS) {
		return CMD_ERR_ARG;
	}

	if (strcasecmp(tokens[0], "HELP") == 0) {
		if (count != 1) {
			return CMD_ERR_ARG;
		}
		pendingCommand = CMD_ID_HELP;
		return CMD_OK;
	}

	if (strcasecmp(tokens[0], "STATUS") == 0) {
		if (count != 1) {
			return CMD_ERR_ARG;
		}
		pendingCommand = CMD_ID_STATUS;
		return CMD_OK;
	}

	if (strcasecmp(tokens[0], "LED") == 0) {
		if (count != 2) {
			return CMD_ERR_ARG;
		}
		if (strcasecmp(tokens[1], "ON") == 0) {
			pendingCommand = CMD_ID_LED_ON;
		} else if (strcasecmp(tokens[1], "OFF") == 0) {
			pendingCommand = CMD_ID_LED_OFF;
		} else if (strcasecmp(tokens[1], "TOGGLE") == 0) {
			pendingCommand = CMD_ID_LED_TOGGLE;
		} else {
			return CMD_ERR_ARG;
		}
		return CMD_OK;
	}

	return CMD_ERR_UNKNOWN;
}

static void cmdExecute(cmdId_t command) {
	switch (command) {
	case CMD_ID_HELP:
		cmdPrintHelp();
		break;
	case CMD_ID_LED_ON:
		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
		uartSendString((uint8_t*) "OK\r\n");
		break;
	case CMD_ID_LED_OFF:
		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
		uartSendString((uint8_t*) "OK\r\n");
		break;
	case CMD_ID_LED_TOGGLE:
		HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
		uartSendString((uint8_t*) "OK\r\n");
		break;
	case CMD_ID_STATUS:
		if (HAL_GPIO_ReadPin(LD2_GPIO_Port, LD2_Pin) == GPIO_PIN_SET) {
			uartSendString((uint8_t*) "LED is ON\r\n");
		} else {
			uartSendString((uint8_t*) "LED is OFF\r\n");
		}
		break;
	case CMD_ID_NONE:
	default:
		break;
	}
}

static void cmdPrintError(cmd_status_t status) {
	switch (status) {
	case CMD_ERR_OVERFLOW:
		uartSendString((uint8_t*) "ERROR: line too long\r\n");
		break;
	case CMD_ERR_UNKNOWN:
		uartSendString((uint8_t*) "ERROR: unknown command\r\n");
		break;
	case CMD_ERR_ARG:
		uartSendString((uint8_t*) "ERROR: bad arguments\r\n");
		break;
	case CMD_ERR_SYNTAX:
	default:
		uartSendString((uint8_t*) "ERROR: syntax error\r\n");
		break;
	}
}

void cmdParserInit(void) {
	currentState = CMD_IDLE;
	lineIndex = 0;
	line[0] = '\0';
	lastError = CMD_OK;
	discardingLine = false;
	pendingCommand = CMD_ID_NONE;
}

void cmdPoll(void) {
	uint8_t c;

	for (int i = 0; i < CMD_MAX_BYTES_PER_POLL; i++) {
		switch (currentState) {
		case CMD_IDLE:
			if (!uartReceiveStringSize(&c, 1)) {
				return;
			}
			if (discardingLine) {
				if (isTerminator(c)) {
					discardingLine = false;
				}
				break;
			}
			if (isTerminator(c)) {
				uartSendString((uint8_t*) "\r\n");
				break;
			}
			if (isDelete(c)) {
				break;
			}
			lineIndex = 0;
			line[lineIndex++] = (char) c;
			uartSendStringSize(&c, 1);
			currentState = CMD_RECEIVING;
			break;

		case CMD_RECEIVING:
			if (!uartReceiveStringSize(&c, 1)) {
				return;
			}
			if (isTerminator(c)) {
				line[lineIndex] = '\0';
				uartSendString((uint8_t*) "\r\n");
				currentState = (lineIndex == 0) ? CMD_IDLE : CMD_PROCESS;
				break;
			}
			if (isDelete(c)) {
				if (lineIndex > 0) {
					lineIndex--;
					uartSendString((uint8_t*) "\b \b");
				}
				break;
			}
			if (lineIndex >= CMD_MAX_LINE - 1) {
				uartSendString((uint8_t*) "\r\n");
				lastError = CMD_ERR_OVERFLOW;
				discardingLine = true;
				currentState = CMD_ERROR;
				break;
			}
			line[lineIndex++] = (char) c;
			uartSendStringSize(&c, 1);
			break;

		case CMD_PROCESS:
			if (isComment(line)) {
				currentState = CMD_IDLE;
				break;
			}
			lastError = cmdProcessLine();
			currentState = (lastError == CMD_OK) ? CMD_EXEC : CMD_ERROR;
			break;

		case CMD_EXEC:
			cmdExecute(pendingCommand);
			pendingCommand = CMD_ID_NONE;
			currentState = CMD_IDLE;
			break;

		case CMD_ERROR:
			cmdPrintError(lastError);
			lineIndex = 0;
			currentState = CMD_IDLE;
			break;

		default:
			cmdParserInit();
			break;
		}
	}
}

void cmdPrintHelp(void) {
	uartSendString((uint8_t*) "Comandos disponibles:\r\n");
	uartSendString((uint8_t*) "  HELP        muestra esta ayuda\r\n");
	uartSendString((uint8_t*) "  LED ON      enciende el LED\r\n");
	uartSendString((uint8_t*) "  LED OFF     apaga el LED\r\n");
	uartSendString((uint8_t*) "  LED TOGGLE  conmuta el LED\r\n");
	uartSendString((uint8_t*) "  STATUS      informa el estado del LED\r\n");
}
