/*
 * checkers.c
 *
 *  Created on: Jun 11, 2025
 *      Author: mishcat
 */

#include "main.h"
#include "checkers.h"

volatile uint8_t checkersState = CHECKERS_STATE_START;
volatile uint8_t checkersBoard[8][8];
volatile uint8_t checkersMoveCount;
volatile uint8_t checkersMoveList[24][5];
volatile uint8_t checkersCaptureCount = 0;
volatile uint8_t checkersPoints[2] = {0, 0};

void checkersCheckPossibleMoves(uint8_t x, uint8_t y, uint8_t enemy) {
	int8_t directions[4][2] = { { -1, -1 }, { -1, 1 }, { 1, -1 }, { 1, 1 } };

	for (int8_t d = 0; d < 4; d++) {
		int8_t dx = directions[d][0], dy = directions[d][1];
		int8_t nx = x + dx, ny = y + dy;

		if (nx < 0 || ny < 0 || nx > 7 || ny > 7)
			continue;

		if (checkersBoard[nx][ny] == CHECKERS_BOARD_EMPTY) {
			// Normal move
			if ((checkersBoard[x][y] & CHECKERS_QUEEN_FLAG) == 0) {
				if ((enemy == CHECKERS_BOARD_BLACK) && (d == 0 || d == 2)) continue;
				if ((enemy == CHECKERS_BOARD_WHITE) && (d == 1 || d == 3)) continue;
			}
			checkersMoveList[checkersMoveCount][0] = x;
			checkersMoveList[checkersMoveCount][1] = y;
			checkersMoveList[checkersMoveCount][2] = nx;
			checkersMoveList[checkersMoveCount][3] = ny;
			checkersMoveList[checkersMoveCount][4] = 0;
			(checkersMoveCount)++;
		} else if ((checkersBoard[nx][ny] & 0b1111) == enemy) {
			int jumpX = nx + dx, jumpY = ny + dy;
			if (jumpX >= 0 && jumpY >= 0 && jumpX < 8 && jumpY < 8 && checkersBoard[jumpX][jumpY] == CHECKERS_BOARD_EMPTY) {
				// Capture move
				checkersMoveList[checkersMoveCount][0] = x;
				checkersMoveList[checkersMoveCount][1] = y;
				checkersMoveList[checkersMoveCount][2] = jumpX;
				checkersMoveList[checkersMoveCount][3] = jumpY;
				checkersMoveList[checkersMoveCount][4] = 1;
				checkersCaptureCount++;
				(checkersMoveCount)++;

				// Optional: Recursively chain multi-jumps
			}
		}
	}
}
