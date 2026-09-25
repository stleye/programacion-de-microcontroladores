/*
 * API_cmdparser.h
 *
 *  Created on: Sep 24, 2026
 *      Author: sebastian
 */

#ifndef API_INC_API_CMDPARSER_H_
#define API_INC_API_CMDPARSER_H_

#define CMD_MAX_LINE    64
#define CMD_MAX_TOKENS  3

typedef enum {
	CMD_OK = 0,
	CMD_ERR_OFERFLOW,
	CMD_ERR_SYNTAX,
	CMD_ERR_UNKNOWN,
	CMD_ERR_ARG
} cmd_status_t;

/**
 * @brief Inicializa el módulo parser de comandos
 */
void cmdParserInit(void);

/**
 * @brief Máquina de estados del parser. Debe ser llamada periódicamente desde el bucle
 * Procesa hasta 16 bytes por invocación (no bloqueante).
 */
void cmdPoll(void);

/**
 * @brief Imprime por UART la lista de comandos disponibles
 */
void cmdPrintHelp(void);

void uartCmdParserFSM_update();

#endif /* API_INC_API_CMDPARSER_H_ */
