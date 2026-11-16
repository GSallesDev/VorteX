// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Gustavo Henrique Sales


#include "scenes/scene_game.h"
#include "scenes/scene_menu.h"
#include "app.h"
#include "assets.h"

#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#include <stddef.h>
#include <stdio.h>

#include "settings.h"


static struct {
    Texture2D car_layers[8];
    float rotation;
} S;

static void enter(AppState *app) {
    (void)app;
    assets_load_group(ASSET_GROUP_GAME);
    
    S.car_layers[0] = assets_texture(CAR_LAYOUT_0);
    S.car_layers[1] = assets_texture(CAR_LAYOUT_1);
    S.car_layers[2] = assets_texture(CAR_LAYOUT_2);
    S.car_layers[3] = assets_texture(CAR_LAYOUT_3);
    S.car_layers[4] = assets_texture(CAR_LAYOUT_4);
    S.car_layers[5] = assets_texture(CAR_LAYOUT_5);
    S.car_layers[6] = assets_texture(CAR_LAYOUT_6);
    S.car_layers[7] = assets_texture(CAR_LAYOUT_7);

    S.rotation = 0.0f;
}

static void update(AppState *app, float dt) {
    (void)app;
    (void)dt;
    S.rotation += 10.0f * 3.1415f * dt;
}

static void render(AppState *app) {
    (void)app;
    for (size_t index = 0; index < 8; ++index) {
        Rectangle src = {0.0f, 0.0f, (float)S.car_layers[index].width, (float)S.car_layers[index].height};
        Rectangle dst = {400.0f, 300.0f - index * 4, src.width * 2.0f, src.height * 2.0f};
        Vector2 origin = { dst.width / 2.0f, dst.height / 2.0f };

        DrawTexturePro(S.car_layers[index], src, dst, origin, S.rotation, WHITE);
    }
}

static void exit_(AppState *app) {
    (void)app;
    assets_unload_group(ASSET_GROUP_GAME);
}

const Scene scene_game = {
    .name   = "game",
    .enter  = enter,
    .update = update,
    .render = render,
    .exit   = exit_,
};
