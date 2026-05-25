#include "tetris.h"

/* 7种标准方块的4x4矩阵定义 (初始朝向) */

/* I: 长条 - 水平放置
   [0 0 0 0]
   [1 1 1 1]
   [0 0 0 0]
   [0 0 0 0] */
static const int shape_I[BLOCK_SIZE][BLOCK_SIZE] = {
    {0, 0, 0, 0},
    {1, 1, 1, 1},
    {0, 0, 0, 0},
    {0, 0, 0, 0}
};

/* O: 方块
   [0 0 0 0]
   [0 1 1 0]
   [0 1 1 0]
   [0 0 0 0] */
static const int shape_O[BLOCK_SIZE][BLOCK_SIZE] = {
    {0, 0, 0, 0},
    {0, 1, 1, 0},
    {0, 1, 1, 0},
    {0, 0, 0, 0}
};

/* T: T形
   [0 0 0 0]
   [0 1 0 0]
   [1 1 1 0]
   [0 0 0 0] */
static const int shape_T[BLOCK_SIZE][BLOCK_SIZE] = {
    {0, 0, 0, 0},
    {0, 1, 0, 0},
    {1, 1, 1, 0},
    {0, 0, 0, 0}
};

/* S: S形
   [0 0 0 0]
   [0 1 1 0]
   [1 1 0 0]
   [0 0 0 0] */
static const int shape_S[BLOCK_SIZE][BLOCK_SIZE] = {
    {0, 0, 0, 0},
    {0, 1, 1, 0},
    {1, 1, 0, 0},
    {0, 0, 0, 0}
};

/* Z: Z形
   [0 0 0 0]
   [1 1 0 0]
   [0 1 1 0]
   [0 0 0 0] */
static const int shape_Z[BLOCK_SIZE][BLOCK_SIZE] = {
    {0, 0, 0, 0},
    {1, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 0, 0}
};

/* J: J形
   [0 0 0 0]
   [1 0 0 0]
   [1 1 1 0]
   [0 0 0 0] */
static const int shape_J[BLOCK_SIZE][BLOCK_SIZE] = {
    {0, 0, 0, 0},
    {1, 0, 0, 0},
    {1, 1, 1, 0},
    {0, 0, 0, 0}
};

/* L: L形
   [0 0 0 0]
   [0 0 1 0]
   [1 1 1 0]
   [0 0 0 0] */
static const int shape_L[BLOCK_SIZE][BLOCK_SIZE] = {
    {0, 0, 0, 0},
    {0, 0, 1, 0},
    {1, 1, 1, 0},
    {0, 0, 0, 0}
};

/* 各类型方块的形状数据指针数组 */
static const int (*shapes[BLOCK_TYPES])[BLOCK_SIZE] = {
    shape_I, shape_O, shape_T, shape_S, shape_Z, shape_J, shape_L
};

/* 初始化方块: 复制对应类型的形状数据 */
void Block_Init(Block* blk, PieceType type) {
    int i, j;
    blk->type = type;
    blk->color = Block_GetColor(type);
    for (i = 0; i < BLOCK_SIZE; i++) {
        for (j = 0; j < BLOCK_SIZE; j++) {
            blk->shape[i][j] = shapes[type][i][j];
        }
    }
}

/* 顺时针旋转: 将矩阵顺时针旋转90度, O形无需旋转 */
void Block_RotateCW(Block* blk) {
    int i, j;
    int temp[BLOCK_SIZE][BLOCK_SIZE];
    if (blk->type == PIECE_O) return; /* O形旋转后不变 */
    for (i = 0; i < BLOCK_SIZE; i++) {
        for (j = 0; j < BLOCK_SIZE; j++) {
            temp[j][BLOCK_SIZE - 1 - i] = blk->shape[i][j];
        }
    }
    for (i = 0; i < BLOCK_SIZE; i++) {
        for (j = 0; j < BLOCK_SIZE; j++) {
            blk->shape[i][j] = temp[i][j];
        }
    }
}

/* 逆时针旋转: 将矩阵逆时针旋转90度 */
void Block_RotateCCW(Block* blk) {
    int i, j;
    int temp[BLOCK_SIZE][BLOCK_SIZE];
    if (blk->type == PIECE_O) return;
    for (i = 0; i < BLOCK_SIZE; i++) {
        for (j = 0; j < BLOCK_SIZE; j++) {
            temp[BLOCK_SIZE - 1 - j][i] = blk->shape[i][j];
        }
    }
    for (i = 0; i < BLOCK_SIZE; i++) {
        for (j = 0; j < BLOCK_SIZE; j++) {
            blk->shape[i][j] = temp[i][j];
        }
    }
}

/* 随机生成方块类型 */
PieceType Block_RandomType(void) {
    return (PieceType)(rand() % BLOCK_TYPES);
}

/* 获取方块类型对应的颜色 */
COLORREF Block_GetColor(PieceType type) {
    switch (type) {
        case PIECE_I: return COLOR_I;
        case PIECE_O: return COLOR_O;
        case PIECE_T: return COLOR_T;
        case PIECE_S: return COLOR_S;
        case PIECE_Z: return COLOR_Z;
        case PIECE_J: return COLOR_J;
        case PIECE_L: return COLOR_L;
        default:      return COLOR_TEXT;
    }
}
