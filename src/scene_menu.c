// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Gustavo Henrique Sales


#include "scenes/scene_menu.h"
#include "scenes/scene_game.h"
#include "app.h"
#include "assets.h"
#include "raylib.h"
#include "raygui.h"
#include <math.h>


static struct {
    float t;
    bool showMessageBox;
} S;

static void enter(AppState *app) {
    (void)app;
    S.t = 0;
    EnableCursor();
}

static void update(AppState *app, float dt) {
    S.t += dt;
    if (IsKeyPressed(KEY_ENTER)) scene_switch(app, &scene_game);
    if (IsKeyPressed(KEY_ESCAPE)) app->should_close = true;
}

static void render(AppState *app) {
    (void)app;

    BeginDrawing();
    ClearBackground(RAYWHITE);

    DrawText("GAME MOV", 0, 0, 24, BLACK);

    if (GuiButton((Rectangle){ 24, 24, 120, 30 }, "#191#TEST")) S.showMessageBox = true;

    if (S.showMessageBox) {
        int btnActive = -1;
        GuiMessageBox((Rectangle){ 85, 70, 250, 100 },
            "#191#Mesagem de texto", "A", "B;C", &btnActive);

        if (btnActive >= 0) S.showMessageBox = false;
    }

    EndDrawing();
}

static void exit_(AppState *app) {
    (void)app;
}

const Scene scene_menu = {
    .name   = "menu",
    .enter  = enter,
    .update = update,
    .render = render,
    .exit   = exit_,
};
