#include "tetris.h"

/* 初始化游戏地图: 所有格子设为空 */
void Map_Init(int board[BOARD_ROWS][BOARD_COLS]) {
    int i, j;
    for (i = 0; i < BOARD_ROWS; i++) {
        for (j = 0; j < BOARD_COLS; j++) {
            board[i][j] = 0;
        }
    }
}

/*
 * 碰撞检测: 检查方块在指定位置是否与地图边界或已锁定方块重叠
 * 返回: 0=无碰撞, 1=有碰撞
 */
int Map_CheckCollision(const int board[BOARD_ROWS][BOARD_COLS],
                       const Block* block, int pos_x, int pos_y) {
    int i, j;
    for (i = 0; i < BLOCK_SIZE; i++) {
        for (j = 0; j < BLOCK_SIZE; j++) {
            if (block->shape[i][j]) {
                int bx = pos_x + j;
                int by = pos_y + i;

                /* 检查边界 */
                if (bx < 0 || bx >= BOARD_COLS || by >= BOARD_ROWS) {
                    return 1;
                }
                /* 允许方块在顶部以上的位置 (by < 0) */
                if (by < 0) continue;

                /* 检查与已锁定方块的重叠 */
                if (board[by][bx] != 0) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

/* 将当前活动方块锁定到地图上 */
void Map_LockPiece(int board[BOARD_ROWS][BOARD_COLS],
                   const ActivePiece* piece) {
    int i, j;
    for (i = 0; i < BLOCK_SIZE; i++) {
        for (j = 0; j < BLOCK_SIZE; j++) {
            if (piece->block.shape[i][j]) {
                int bx = piece->x + j;
                int by = piece->y + i;
                if (by >= 0 && by < BOARD_ROWS && bx >= 0 && bx < BOARD_COLS) {
                    board[by][bx] = piece->block.color;
                }
            }
        }
    }
}

/*
 * 消除已满的行: 从下往上扫描, 满行则消除
 * 积分规则: 1行=10, 2行=30, 3行=50, 4行=80
 * 返回本次消除获得的分数
 */
int Map_ClearLines(int board[BOARD_ROWS][BOARD_COLS]) {
    int lines_cleared = 0;
    int row, i, j;

    for (row = BOARD_ROWS - 1; row >= 0; row--) {
        int full = 1;
        /* 检查当前行是否已满 */
        for (j = 0; j < BOARD_COLS; j++) {
            if (board[row][j] == 0) {
                full = 0;
                break;
            }
        }

        if (full) {
            lines_cleared++;
            /* 将上方所有行向下移动一行 */
            for (i = row; i > 0; i--) {
                for (j = 0; j < BOARD_COLS; j++) {
                    board[i][j] = board[i - 1][j];
                }
            }
            /* 顶部新增空行 */
            for (j = 0; j < BOARD_COLS; j++) {
                board[0][j] = 0;
            }
            row++; /* 重新检查当前行 (因为上方行下移了) */
        }
    }

    /* 根据消除行数计算分数 */
    switch (lines_cleared) {
        case 1: return 10;
        case 2: return 30;
        case 3: return 50;
        case 4: return 80;
        default: return 0;
    }
}
