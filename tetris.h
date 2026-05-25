#ifndef TETRIS_H
#define TETRIS_H

#define NOMINMAX   /* 避免 Windows.h 的 min/max 宏冲突 */
#include <graphics.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <tchar.h>
#include <windows.h>

/* ========== 窗口常量 ========== */
#define WINDOW_WIDTH   800
#define WINDOW_HEIGHT  600

/* ========== 游戏区域常量 ========== */
#define BOARD_COLS     10       /* 地图列数 */
#define BOARD_ROWS     20       /* 地图行数 */
#define CELL_SIZE      28       /* 每个格子的像素大小 */
#define BOARD_X        220      /* 地图左上角X坐标 */
#define BOARD_Y        30       /* 地图左上角Y坐标 */

/* ========== 方块常量 ========== */
#define BLOCK_SIZE     4        /* 方块矩阵大小 (4x4) */
#define BLOCK_TYPES    7        /* 7种标准方块 */

/* ========== 速度等级常量 ========== */
#define SPEED_LEVEL_0  1000     /* 初始速度: 1秒/格 */
#define SPEED_LEVEL_1  800      /* 300分: 0.8秒/格 */
#define SPEED_LEVEL_2  500      /* 500分: 0.5秒/格 */
#define SPEED_LEVEL_3  300      /* 800分: 0.3秒/格 */

#define SCORE_THRESHOLD_1  300
#define SCORE_THRESHOLD_2  500
#define SCORE_THRESHOLD_3  800

/* ========== 游戏状态枚举 ========== */
typedef enum {
    STATE_MENU,
    STATE_PLAYING,
    STATE_PAUSED,
    STATE_GAME_OVER
} GameState;

/* ========== 方块类型枚举 ========== */
typedef enum {
    PIECE_I = 0,   /* I形: 青色 */
    PIECE_O = 1,   /* O形: 黄色 */
    PIECE_T = 2,   /* T形: 紫色 */
    PIECE_S = 3,   /* S形: 绿色 */
    PIECE_Z = 4,   /* Z形: 红色 */
    PIECE_J = 5,   /* J形: 蓝色 */
    PIECE_L = 6    /* L形: 橙色 */
} PieceType;

/* ========== 颜色定义 ========== */
#define COLOR_I     RGB(0, 240, 240)    /* 青色 */
#define COLOR_O     RGB(240, 240, 0)    /* 黄色 */
#define COLOR_T     RGB(160, 0, 240)    /* 紫色 */
#define COLOR_S     RGB(0, 240, 0)      /* 绿色 */
#define COLOR_Z     RGB(240, 0, 0)      /* 红色 */
#define COLOR_J     RGB(0, 0, 240)      /* 蓝色 */
#define COLOR_L     RGB(240, 160, 0)    /* 橙色 */

#define COLOR_BG        RGB(30, 30, 30)     /* 背景色 */
#define COLOR_GRID      RGB(60, 60, 60)     /* 网格线颜色 */
#define COLOR_BORDER    RGB(100, 100, 100)  /* 边框颜色 */
#define COLOR_TEXT      RGB(255, 255, 255)  /* 文字颜色 */
#define COLOR_HIGHLIGHT RGB(255, 255, 0)    /* 高亮颜色 */
#define COLOR_BUTTON    RGB(60, 60, 80)     /* 按钮颜色 */
#define COLOR_BUTTON_HOVER RGB(100, 100, 140) /* 按钮悬停颜色 */
#define COLOR_EMPTY     RGB(20, 20, 20)     /* 空格颜色 */

/* ========== 方块结构体 ========== */
typedef struct {
    int shape[BLOCK_SIZE][BLOCK_SIZE];  /* 4x4矩阵, 1=有方块, 0=空 */
    PieceType type;                      /* 方块类型 */
    int color;                           /* 颜色值 */
} Block;

/* ========== 当前活动方块 ========== */
typedef struct {
    Block block;            /* 方块形状 */
    int x, y;               /* 方块在地图上的位置 (左上角) */
    PieceType type;         /* 类型 */
} ActivePiece;

/* ========== 游戏数据结构 ========== */
typedef struct {
    int board[BOARD_ROWS][BOARD_COLS];  /* 游戏地图, 存储颜色值, 0=空 */
    ActivePiece current;                /* 当前活动方块 */
    PieceType next_type;                /* 下一个方块类型 */
    int score;                          /* 当前分数 */
    int level;                          /* 当前等级 (0-3) */
    int drop_speed;                     /* 当前下落速度(ms) */
    GameState state;                    /* 游戏状态 */
    int mouse_x, mouse_y;               /* 鼠标位置 */
    int mouse_down;                     /* 鼠标是否按下 */
    int high_scores[10];                /* 排行榜 */
    TCHAR high_names[10][20];          /* 排行榜昵称 */
    int sound_on;                       /* 音效开关 */
    int volume;                         /* 音量(0-100) */
    int stamina;                        /* 体力值 */
} GameData;

/* ========== 函数声明 ========== */

/* block.c - 方块模块 */
void Block_Init(Block* blk, PieceType type);
void Block_RotateCW(Block* blk);
void Block_RotateCCW(Block* blk);
PieceType Block_RandomType(void);
COLORREF Block_GetColor(PieceType type);

/* map.c - 地图模块 */
void Map_Init(int board[BOARD_ROWS][BOARD_COLS]);
int Map_CheckCollision(const int board[BOARD_ROWS][BOARD_COLS],
                       const Block* block, int pos_x, int pos_y);
void Map_LockPiece(int board[BOARD_ROWS][BOARD_COLS],
                   const ActivePiece* piece);
int Map_ClearLines(int board[BOARD_ROWS][BOARD_COLS]);

/* render.c - 渲染模块 */
void Render_Init(void);
void Render_Board(const int board[BOARD_ROWS][BOARD_COLS]);
void Render_ActivePiece(const ActivePiece* piece);
void Render_NextPiece(PieceType type);
void Render_Sidebar(const GameData* game);
void Render_Menu(const GameData* game);
void Render_PauseOverlay(void);
void Render_GameOver(const GameData* game);
void Render_PlayBeep(int freq, int duration);

/* game.c - 游戏逻辑模块 */
void Game_Init(GameData* game);
void Game_NewPiece(GameData* game);
void Game_Drop(GameData* game);
void Game_MoveLeft(GameData* game);
void Game_MoveRight(GameData* game);
void Game_RotateCW(GameData* game);
void Game_RotateCCW(GameData* game);
void Game_SoftDrop(GameData* game);
void Game_HardDrop(GameData* game);
int Game_IsOver(const GameData* game);
void Game_UpdateSpeed(GameData* game);
void Game_HandleMenuClick(GameData* game, int mx, int my);
void Game_HandlePauseClick(GameData* game, int mx, int my);
void Game_HandleGameOverClick(GameData* game, int mx, int my);

#endif /* TETRIS_H */
