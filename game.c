#include "tetris.h"

/* 初始化游戏数据 */
void Game_Init(GameData* game) {
    int i;

    Map_Init(game->board);
    game->score = 0;
    game->level = 0;
    game->drop_speed = SPEED_LEVEL_0;
    game->state = STATE_MENU;
    game->mouse_x = 0;
    game->mouse_y = 0;
    game->mouse_down = 0;
    game->stamina = 50;
    game->sound_on = 1;
    game->volume = 50;

    /* 初始化排行榜 */
    for (i = 0; i < 10; i++) {
        game->high_scores[i] = 0;
        _tcscpy_s(game->high_names[i], _countof(game->high_names[0]),
                  _T("---"));
    }

    /* 预生成下一个方块 */
    game->next_type = Block_RandomType();
    game->current.block.type = PIECE_I;
    Block_Init(&game->current.block, PIECE_I);
    game->current.x = 3;
    game->current.y = -2;
    game->current.type = PIECE_I;
}

/* 生成新方块: 将 next 取出, 并生成新的 next */
void Game_NewPiece(GameData* game) {
    game->current.type = game->next_type;
    Block_Init(&game->current.block, game->current.type);
    game->current.x = (BOARD_COLS - BLOCK_SIZE) / 2;
    game->current.y = -2;
    game->next_type = Block_RandomType();
}

/* 方块自动下落一格 */
void Game_Drop(GameData* game) {
    if (game->state != STATE_PLAYING) return;

    if (!Map_CheckCollision(game->board, &game->current.block,
                            game->current.x, game->current.y + 1)) {
        game->current.y++;
    } else {
        /* 锁定方块到地图 */
        Map_LockPiece(game->board, &game->current);
        if (game->sound_on) Render_PlayBeep(200, 50);

        /* 消除行并计分 */
        {
            int points = Map_ClearLines(game->board);
            if (points > 0) {
                game->score += points;
                if (game->sound_on) Render_PlayBeep(600, 100);
            }
        }

        /* 更新速度等级 & 生成新方块 */
        Game_UpdateSpeed(game);
        Game_NewPiece(game);

        /* 检查游戏结束 */
        if (Game_IsOver(game)) {
            game->state = STATE_GAME_OVER;
            if (game->sound_on) Render_PlayBeep(150, 300);
        }
    }
}

/* 左移 */
void Game_MoveLeft(GameData* game) {
    if (game->state != STATE_PLAYING) return;
    if (!Map_CheckCollision(game->board, &game->current.block,
                            game->current.x - 1, game->current.y)) {
        game->current.x--;
    }
}

/* 右移 */
void Game_MoveRight(GameData* game) {
    if (game->state != STATE_PLAYING) return;
    if (!Map_CheckCollision(game->board, &game->current.block,
                            game->current.x + 1, game->current.y)) {
        game->current.x++;
    }
}

/* 顺时针旋转 (带墙踢: 尝试偏移修正) */
void Game_RotateCW(GameData* game) {
    Block temp;
    if (game->state != STATE_PLAYING) return;

    temp = game->current.block;
    Block_RotateCW(&temp);

    /* 尝试: 原位 -> 左移 -> 右移 -> 上移 */
    if (!Map_CheckCollision(game->board, &temp,
                            game->current.x, game->current.y)) {
        game->current.block = temp;
        return;
    }
    if (!Map_CheckCollision(game->board, &temp,
                            game->current.x - 1, game->current.y)) {
        game->current.block = temp;
        game->current.x--;
        return;
    }
    if (!Map_CheckCollision(game->board, &temp,
                            game->current.x + 1, game->current.y)) {
        game->current.block = temp;
        game->current.x++;
        return;
    }
    if (!Map_CheckCollision(game->board, &temp,
                            game->current.x, game->current.y - 1)) {
        game->current.block = temp;
        game->current.y--;
        return;
    }
}

/* 逆时针旋转 (带墙踢) */
void Game_RotateCCW(GameData* game) {
    Block temp;
    if (game->state != STATE_PLAYING) return;

    temp = game->current.block;
    Block_RotateCCW(&temp);

    if (!Map_CheckCollision(game->board, &temp,
                            game->current.x, game->current.y)) {
        game->current.block = temp;
        return;
    }
    if (!Map_CheckCollision(game->board, &temp,
                            game->current.x - 1, game->current.y)) {
        game->current.block = temp;
        game->current.x--;
        return;
    }
    if (!Map_CheckCollision(game->board, &temp,
                            game->current.x + 1, game->current.y)) {
        game->current.block = temp;
        game->current.x++;
        return;
    }
    if (!Map_CheckCollision(game->board, &temp,
                            game->current.x, game->current.y - 1)) {
        game->current.block = temp;
        game->current.y--;
        return;
    }
}

/* 软降: 加速下落一格, 奖励1分 */
void Game_SoftDrop(GameData* game) {
    if (game->state != STATE_PLAYING) return;
    Game_Drop(game);
    if (game->state == STATE_PLAYING) {
        game->score += 1;
    }
}

/* 硬降: 直接落底, 奖励2分/格 */
void Game_HardDrop(GameData* game) {
    int drop_distance = 0;
    if (game->state != STATE_PLAYING) return;

    while (!Map_CheckCollision(game->board, &game->current.block,
                               game->current.x, game->current.y + 1)) {
        game->current.y++;
        drop_distance++;
    }
    game->score += drop_distance * 2;

    /* 锁定并处理 */
    Map_LockPiece(game->board, &game->current);
    if (game->sound_on) Render_PlayBeep(300, 50);
    {
        int points = Map_ClearLines(game->board);
        if (points > 0) {
            game->score += points;
            if (game->sound_on) Render_PlayBeep(600, 100);
        }
    }
    Game_UpdateSpeed(game);
    Game_NewPiece(game);
    if (Game_IsOver(game)) {
        game->state = STATE_GAME_OVER;
        if (game->sound_on) Render_PlayBeep(150, 300);
    }
}

/* 判断游戏结束: 新方块生成时即碰撞 */
int Game_IsOver(const GameData* game) {
    return Map_CheckCollision(game->board, &game->current.block,
                              game->current.x, game->current.y);
}

/* 根据分数更新下落速度等级 */
void Game_UpdateSpeed(GameData* game) {
    if (game->score >= SCORE_THRESHOLD_3) {
        game->level = 3;
        game->drop_speed = SPEED_LEVEL_3;
    } else if (game->score >= SCORE_THRESHOLD_2) {
        game->level = 2;
        game->drop_speed = SPEED_LEVEL_2;
    } else if (game->score >= SCORE_THRESHOLD_1) {
        game->level = 1;
        game->drop_speed = SPEED_LEVEL_1;
    } else {
        game->level = 0;
        game->drop_speed = SPEED_LEVEL_0;
    }
}

/* 处理主菜单的鼠标点击 */
void Game_HandleMenuClick(GameData* game, int mx, int my) {
    int cx = WINDOW_WIDTH / 2;
    int start_y = 200;
    int btn_w = 220, btn_h = 45, btn_gap = 12;
    int i;

    for (i = 0; i < 5; i++) {
        int by = start_y + i * (btn_h + btn_gap);
        int bx = cx - btn_w / 2;
        if (mx >= bx && mx <= bx + btn_w &&
            my >= by && my <= by + btn_h) {
            switch (i) {
                case 0: /* 开始游戏 */
                    Map_Init(game->board);
                    game->score = 0;
                    game->level = 0;
                    game->drop_speed = SPEED_LEVEL_0;
                    game->next_type = Block_RandomType();
                    Game_NewPiece(game);
                    game->state = STATE_PLAYING;
                    break;

                case 1: /* 排行榜 */
                    {
                        TCHAR msg[512] = _T("TOP 10:\n");
                        TCHAR line[64];
                        int j;
                        for (j = 0; j < 10; j++) {
                            _stprintf_s(line, _countof(line),
                                        _T("%d. %s - %d\n"),
                                        j + 1, game->high_names[j],
                                        game->high_scores[j]);
                            _tcscat_s(msg, _countof(msg), line);
                        }
                        MessageBox(GetHWnd(), msg,
                                   _T("Leaderboard"), MB_OK);
                    }
                    break;

                case 2: /* 设置 - 音效开关 */
                    game->sound_on = !game->sound_on;
                    MessageBox(GetHWnd(),
                               game->sound_on ?
                               _T("Sound: ON") : _T("Sound: OFF"),
                               _T("Settings"), MB_OK);
                    break;

                case 3: /* 团队介绍 */
                    MessageBox(GetHWnd(),
                               _T("Tetris - Russian Square\n\n")
                               _T("Developed with C + EasyX\n")
                               _T("VS2022 Compatible\n\n")
                               _T("Team: Tetris Dev Team"),
                               _T("Team"), MB_OK);
                    break;

                case 4: /* 退出 */
                    exit(0);
                    break;
            }
        }
    }
}

/* 处理暂停界面的鼠标点击 */
void Game_HandlePauseClick(GameData* game, int mx, int my) {
    (void)mx; (void)my;
    game->state = STATE_PLAYING;
}

/* 处理结算界面的鼠标点击 */
void Game_HandleGameOverClick(GameData* game, int mx, int my) {
    int i, pos = -1;
    (void)mx; (void)my;

    /* 检查是否进入排行榜 */
    for (i = 9; i >= 0; i--) {
        if (game->score > game->high_scores[i]) {
            pos = i;
        }
    }
    if (pos >= 0) {
        /* 插入排行榜 */
        int j;
        for (j = 9; j > pos; j--) {
            game->high_scores[j] = game->high_scores[j - 1];
            _tcscpy_s(game->high_names[j],
                      _countof(game->high_names[0]),
                      game->high_names[j - 1]);
        }
        game->high_scores[pos] = game->score;

        /* 输入昵称 */
        InputBox(game->high_names[pos], 20,
                 _T("High Score"),
                 _T("New High Score! Enter your name:"),
                 NULL, 0, 0, 0, 0, FALSE);
        if (game->high_names[pos][0] == _T('\0')) {
            _tcscpy_s(game->high_names[pos],
                      _countof(game->high_names[0]),
                      _T("Player"));
        }
    }

    game->state = STATE_MENU;
}
