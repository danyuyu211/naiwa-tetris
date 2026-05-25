#include "tetris.h"

/* ================================================================
 * main.c - 俄罗斯方块 程序入口 / 游戏主循环
 *
 * 控制流:
 *   main() -> Render_Init() -> Game_Init() -> 主循环
 *   主循环中根据 GameState 分发: MENU / PLAYING / PAUSED / GAME_OVER
 *
 * 键盘控制 (游戏中):
 *   A / Left   - 左移
 *   D / Right  - 右移
 *   Q          - 逆时针旋转
 *   E          - 顺时针旋转
 *   S / Down   - 软降 (加速下落)
 *   Space      - 硬降 (直接落底)
 *   ESC        - 暂停 / 返回菜单
 * ================================================================ */

/* 处理游戏中的键盘输入 */
static void HandlePlayingInput(GameData* game) {
    int key;
    if (!_kbhit()) return;
    key = _getch();

    /* 方向键是双字节: 第一个字节 224, 第二个字节是方向码 */
    if (key == 224 || key == 0) {
        key = _getch();
        switch (key) {
            case 75:  /* 左箭头 */
                Game_MoveLeft(game);
                break;
            case 77:  /* 右箭头 */
                Game_MoveRight(game);
                break;
            case 80:  /* 下箭头 */
                Game_SoftDrop(game);
                break;
            case 72:  /* 上箭头 - 顺时针旋转 */
                Game_RotateCW(game);
                break;
        }
        return;
    }

    switch (key) {
        case 'a':
        case 'A':
            Game_MoveLeft(game);
            break;
        case 'd':
        case 'D':
            Game_MoveRight(game);
            break;
        case 's':
        case 'S':
            Game_SoftDrop(game);
            break;
        case 'q':
        case 'Q':
            Game_RotateCCW(game);
            break;
        case 'e':
        case 'E':
            Game_RotateCW(game);
            break;
        case ' ':  /* 空格 = 硬降 */
            Game_HardDrop(game);
            break;
        case 27:   /* ESC = 暂停 */
            game->state = STATE_PAUSED;
            break;
    }
}

/* 处理暂停状态下的输入 */
static void HandlePausedInput(GameData* game) {
    int key;
    if (!_kbhit()) return;

    key = _getch();
    if (key == 224 || key == 0) {
        _getch(); /* 吞掉方向键 */
        return;
    }
    if (key == 27) { /* ESC 继续 */
        game->state = STATE_PLAYING;
    }
}

/* 获取鼠标状态 (使用 EasyX 的 ExMessage 或直接检测) */
static void UpdateMouseState(GameData* game) {
    /* EasyX 的鼠标检测 */
    MOUSEMSG msg;
    game->mouse_down = 0;

    while (MouseHit()) {
        msg = GetMouseMsg();
        game->mouse_x = msg.x;
        game->mouse_y = msg.y;
        if (msg.uMsg == WM_LBUTTONDOWN) {
            game->mouse_down = 1;
        }
    }
}

/* 处理菜单状态下的鼠标点击 */
static void HandleMenuInput(GameData* game) {
    UpdateMouseState(game);
    if (game->mouse_down) {
        Game_HandleMenuClick(game, game->mouse_x, game->mouse_y);
    }
}

/* 处理暂停状态下的鼠标点击 */
static void HandlePauseMouse(GameData* game) {
    UpdateMouseState(game);
    if (game->mouse_down) {
        Game_HandlePauseClick(game, game->mouse_x, game->mouse_y);
    }
}

/* 处理结算状态下的鼠标点击 */
static void HandleGameOverInput(GameData* game) {
    UpdateMouseState(game);
    if (game->mouse_down) {
        Game_HandleGameOverClick(game, game->mouse_x, game->mouse_y);
    }
}

/* 重绘整个游戏画面 (根据当前状态) */
static void RenderFrame(const GameData* game) {
    switch (game->state) {
        case STATE_MENU:
            Render_Menu(game);
            break;

        case STATE_PLAYING:
        case STATE_PAUSED:
            cleardevice();
            Render_Board(game->board);
            Render_ActivePiece(&game->current);
            Render_NextPiece(game->next_type);
            Render_Sidebar(game);
            if (game->state == STATE_PAUSED) {
                Render_PauseOverlay();
            }
            break;

        case STATE_GAME_OVER:
            /* 先绘制游戏画面, 再绘制覆盖层 */
            cleardevice();
            Render_Board(game->board);
            Render_NextPiece(game->next_type);
            Render_Sidebar(game);
            Render_GameOver(game);
            break;
    }
}

/* 程序入口 */
int main(void) {
    GameData game;
    DWORD last_drop_time;   /* 上一次自动下落的时间 */
    DWORD current_time;

    /* 初始化图形和随机种子 */
    Render_Init();
    Game_Init(&game);

    /* 设置窗口标题 */
    SetWindowText(GetHWnd(), _T("Tetris - Russian Square"));

    last_drop_time = GetTickCount();

    /* ===== 主循环 ===== */
    while (1) {
        GameState prev_state = game.state;
        current_time = GetTickCount();

        /* 自动下落 (仅在游戏进行中) */
        if (game.state == STATE_PLAYING) {
            /* 刚进入游戏状态时重置计时器 */
            if (prev_state != STATE_PLAYING) {
                last_drop_time = current_time;
            }
            if (current_time - last_drop_time >= (DWORD)game.drop_speed) {
                Game_Drop(&game);
                last_drop_time = current_time;
            }
        }

        /* 处理输入 (根据状态分发) */
        switch (game.state) {
            case STATE_MENU:
                HandleMenuInput(&game);
                break;
            case STATE_PLAYING:
                HandlePlayingInput(&game);
                break;
            case STATE_PAUSED:
                HandlePausedInput(&game);
                HandlePauseMouse(&game);
                break;
            case STATE_GAME_OVER:
                HandleGameOverInput(&game);
                break;
        }

        /* 渲染 */
        BeginBatchDraw();
        RenderFrame(&game);
        FlushBatchDraw();
        EndBatchDraw();

        /* 帧率控制 (~60 FPS) */
        Sleep(16);
    }

    closegraph();
    return 0;
}
