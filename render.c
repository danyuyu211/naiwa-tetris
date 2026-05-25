#include "tetris.h"

/* min/max 辅助 */
static int min_int(int a, int b) { return a < b ? a : b; }

/* 绘制单个格子 (带立体高光/阴影效果) */
static void DrawCell(int px, int py, COLORREF color) {
    /* 主体 */
    setfillcolor(color);
    solidrectangle(px + 1, py + 1, px + CELL_SIZE - 2, py + CELL_SIZE - 2);
    /* 高光边 (左上) */
    setfillcolor(RGB(
        min_int(255, (int)(GetRValue(color) * 1.3)),
        min_int(255, (int)(GetGValue(color) * 1.3)),
        min_int(255, (int)(GetBValue(color) * 1.3))
    ));
    solidrectangle(px + 1, py + 1, px + CELL_SIZE - 2, py + 3);
    solidrectangle(px + 1, py + 1, px + 3, py + CELL_SIZE - 2);
    /* 阴影边 (右下) */
    setfillcolor(RGB(
        (int)(GetRValue(color) * 0.6),
        (int)(GetGValue(color) * 0.6),
        (int)(GetBValue(color) * 0.6)
    ));
    solidrectangle(px + 1, py + CELL_SIZE - 4,
                   px + CELL_SIZE - 2, py + CELL_SIZE - 2);
    solidrectangle(px + CELL_SIZE - 4, py + 1,
                   px + CELL_SIZE - 2, py + CELL_SIZE - 2);
}

/* 绘制居中文本 (使用 TCHAR 以兼容 Unicode/MultiByte) */
static void DrawCenteredText(int x, int y, int width,
                             LPCTSTR text, int font_height, COLORREF color) {
    int text_w;
    settextcolor(color);
    settextstyle(font_height, 0, _T("Consolas"));
    text_w = textwidth(text);
    outtextxy(x + (width - text_w) / 2, y, text);
}

/* 绘制矩形按钮 */
static void DrawButton(int x, int y, int w, int h,
                       LPCTSTR text, int hovered, int font_h) {
    setfillcolor(hovered ? COLOR_BUTTON_HOVER : COLOR_BUTTON);
    setlinecolor(COLOR_BORDER);
    fillrectangle(x, y, x + w, y + h);
    DrawCenteredText(x, y + (h - font_h) / 2, w, text, font_h,
                     hovered ? COLOR_HIGHLIGHT : COLOR_TEXT);
}

/* 初始化图形窗口 */
void Render_Init(void) {
    initgraph(WINDOW_WIDTH, WINDOW_HEIGHT);
    setbkcolor(COLOR_BG);
    cleardevice();
    srand((unsigned int)time(NULL));
}

/* 绘制游戏地图 */
void Render_Board(const int board[BOARD_ROWS][BOARD_COLS]) {
    int i, j;
    /* 地图背景 */
    setfillcolor(COLOR_EMPTY);
    solidrectangle(BOARD_X - 2, BOARD_Y - 2,
                   BOARD_X + BOARD_COLS * CELL_SIZE + 2,
                   BOARD_Y + BOARD_ROWS * CELL_SIZE + 2);
    /* 网格线 */
    setlinecolor(COLOR_GRID);
    for (i = 0; i <= BOARD_ROWS; i++) {
        line(BOARD_X, BOARD_Y + i * CELL_SIZE,
             BOARD_X + BOARD_COLS * CELL_SIZE, BOARD_Y + i * CELL_SIZE);
    }
    for (j = 0; j <= BOARD_COLS; j++) {
        line(BOARD_X + j * CELL_SIZE, BOARD_Y,
             BOARD_X + j * CELL_SIZE, BOARD_Y + BOARD_ROWS * CELL_SIZE);
    }
    /* 已锁定的方块 */
    for (i = 0; i < BOARD_ROWS; i++) {
        for (j = 0; j < BOARD_COLS; j++) {
            if (board[i][j] != 0) {
                DrawCell(BOARD_X + j * CELL_SIZE,
                         BOARD_Y + i * CELL_SIZE, board[i][j]);
            }
        }
    }
    /* 外边框 */
    setlinecolor(COLOR_BORDER);
    rectangle(BOARD_X - 2, BOARD_Y - 2,
              BOARD_X + BOARD_COLS * CELL_SIZE + 2,
              BOARD_Y + BOARD_ROWS * CELL_SIZE + 2);
}

/* 绘制当前活动方块 */
void Render_ActivePiece(const ActivePiece* piece) {
    int i, j;
    for (i = 0; i < BLOCK_SIZE; i++) {
        for (j = 0; j < BLOCK_SIZE; j++) {
            if (piece->block.shape[i][j]) {
                int px = BOARD_X + (piece->x + j) * CELL_SIZE;
                int py = BOARD_Y + (piece->y + i) * CELL_SIZE;
                if (piece->y + i >= 0) {
                    DrawCell(px, py, piece->block.color);
                }
            }
        }
    }
}

/* 绘制下一个方块预览 */
void Render_NextPiece(PieceType type) {
    int preview_x = 540, preview_y = 80;
    int preview_cell = 22;
    int i, j;
    Block preview_block;

    settextcolor(COLOR_TEXT);
    settextstyle(18, 0, _T("Consolas"));
    outtextxy(preview_x, preview_y - 30, _T("NEXT"));

    setfillcolor(COLOR_EMPTY);
    solidrectangle(preview_x, preview_y,
                   preview_x + 5 * preview_cell, preview_y + 5 * preview_cell);

    Block_Init(&preview_block, type);
    for (i = 0; i < BLOCK_SIZE; i++) {
        for (j = 0; j < BLOCK_SIZE; j++) {
            if (preview_block.shape[i][j]) {
                int px = preview_x + j * preview_cell;
                int py = preview_y + i * preview_cell;
                setfillcolor(preview_block.color);
                solidrectangle(px + 1, py + 1,
                               px + preview_cell - 2, py + preview_cell - 2);
            }
        }
    }
    setlinecolor(COLOR_BORDER);
    rectangle(preview_x, preview_y,
              preview_x + 5 * preview_cell, preview_y + 5 * preview_cell);
}

/* 绘制侧边栏 (分数/等级/速度/控制说明) */
void Render_Sidebar(const GameData* game) {
    int sx = 530;
    TCHAR buf[64];

    settextcolor(COLOR_TEXT);
    settextstyle(20, 0, _T("Consolas"));

    /* 分数 */
    _stprintf_s(buf, _countof(buf), _T("SCORE  %d"), game->score);
    outtextxy(sx, 220, buf);

    /* 等级 */
    _stprintf_s(buf, _countof(buf), _T("LEVEL  %d"), game->level + 1);
    outtextxy(sx, 260, buf);

    /* 速度 */
    _stprintf_s(buf, _countof(buf), _T("SPEED  %dms"), game->drop_speed);
    outtextxy(sx, 300, buf);

    /* 分隔线 */
    setlinecolor(COLOR_BORDER);
    line(sx, 340, sx + 200, 340);

    /* 操作说明 */
    settextstyle(14, 0, _T("Consolas"));
    settextcolor(COLOR_BORDER);
    outtextxy(sx, 360, _T("CONTROLS"));
    settextstyle(12, 0, _T("Consolas"));
    settextcolor(COLOR_TEXT);
    outtextxy(sx, 385, _T("A/D  - Move Left/Right"));
    outtextxy(sx, 405, _T("Q/E  - Rotate CCW/CW"));
    outtextxy(sx, 425, _T("S    - Soft Drop"));
    outtextxy(sx, 445, _T("Space- Hard Drop"));
    outtextxy(sx, 465, _T("ESC  - Pause / Menu"));

    /* 状态提示 */
    settextcolor(COLOR_HIGHLIGHT);
    settextstyle(16, 0, _T("Consolas"));
    if (game->state == STATE_PAUSED) {
        outtextxy(sx, 500, _T("[ PAUSED ]"));
    }
}

/* 绘制主菜单界面 */
void Render_Menu(const GameData* game) {
    int cx = WINDOW_WIDTH / 2;
    int start_y = 200;
    int btn_w = 220, btn_h = 45, btn_gap = 12;
    int font_h = 22;
    int mx = game->mouse_x, my = game->mouse_y;
    int i;

    /* 按钮标签 */
    LPCTSTR labels[] = {
        _T("START GAME"),
        _T("LEADERBOARD"),
        _T("GAME SETTINGS"),
        _T("TEAM"),
        _T("EXIT")
    };
    int n_btns = 5;

    cleardevice();

    /* 标题 */
    settextcolor(COLOR_TEXT);
    settextstyle(48, 0, _T("Consolas"));
    {
        LPCTSTR title = _T("TETRIS");
        int tw = textwidth(title);
        outtextxy(cx - tw / 2, 80, title);
    }

    /* 副标题 */
    settextstyle(14, 0, _T("Consolas"));
    settextcolor(COLOR_BORDER);
    {
        LPCTSTR sub = _T("Russian Square - EasyX Edition");
        int sw = textwidth(sub);
        outtextxy(cx - sw / 2, 140, sub);
    }

    /* 按钮 */
    for (i = 0; i < n_btns; i++) {
        int by = start_y + i * (btn_h + btn_gap);
        int bx = cx - btn_w / 2;
        int hovered = (mx >= bx && mx <= bx + btn_w &&
                       my >= by && my <= by + btn_h);
        DrawButton(bx, by, btn_w, btn_h, labels[i], hovered, font_h);
    }
}

/* 绘制暂停覆盖层 */
void Render_PauseOverlay(void) {
    setfillcolor(RGB(20, 20, 40));
    setlinecolor(RGB(20, 20, 40));
    solidrectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    settextcolor(COLOR_TEXT);
    settextstyle(48, 0, _T("Consolas"));
    {
        LPCTSTR paused = _T("PAUSED");
        int pw = textwidth(paused);
        outtextxy(WINDOW_WIDTH / 2 - pw / 2, 180, paused);
    }

    settextstyle(16, 0, _T("Consolas"));
    settextcolor(COLOR_BORDER);
    {
        LPCTSTR hint = _T("Press ESC to resume, or click to continue");
        int hw = textwidth(hint);
        outtextxy(WINDOW_WIDTH / 2 - hw / 2, 250, hint);
    }
}

/* 绘制游戏结束界面 */
void Render_GameOver(const GameData* game) {
    TCHAR buf[64];

    setfillcolor(RGB(10, 10, 30));
    solidrectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    settextcolor(RGB(255, 60, 60));
    settextstyle(52, 0, _T("Consolas"));
    {
        LPCTSTR go = _T("GAME OVER");
        int gw = textwidth(go);
        outtextxy(WINDOW_WIDTH / 2 - gw / 2, 160, go);
    }

    settextcolor(COLOR_HIGHLIGHT);
    settextstyle(28, 0, _T("Consolas"));
    _stprintf_s(buf, _countof(buf), _T("FINAL SCORE: %d"), game->score);
    {
        int sw = textwidth(buf);
        outtextxy(WINDOW_WIDTH / 2 - sw / 2, 260, buf);
    }

    settextcolor(COLOR_BORDER);
    settextstyle(16, 0, _T("Consolas"));
    {
        LPCTSTR hint = _T("Click anywhere to return to menu");
        int hw = textwidth(hint);
        outtextxy(WINDOW_WIDTH / 2 - hw / 2, 350, hint);
    }
}

/* 播放提示音效 (使用系统蜂鸣, 不依赖外部文件) */
void Render_PlayBeep(int freq, int duration) {
    Beep(freq, duration);
}
