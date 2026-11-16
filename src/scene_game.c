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

#define CAR_TRACES_LEN 40
#define CAR_SMOKE_LEN 10

typedef struct {
    Texture2D layers[8];
    float rotation;
    Vector2 position;
    Vector2 velocity;

    Vector2 traces[CAR_TRACES_LEN];
    size_t trace_idx;

    Vector2 smoke[CAR_SMOKE_LEN];
    Vector2 smoke_velocity[CAR_SMOKE_LEN];
    size_t smoke_age[CAR_SMOKE_LEN];
    size_t smoke_idx;
} Car;

static struct {
    Texture2D road;
    Camera2D camera;
    Car car;
    size_t counter;
} S;

static void enter(AppState *app) {
    (void)app;
    assets_load_group(ASSET_GROUP_GAME);
    
    S.car.layers[0] = assets_texture(CAR_LAYOUT_0);
    S.car.layers[1] = assets_texture(CAR_LAYOUT_1);
    S.car.layers[2] = assets_texture(CAR_LAYOUT_2);
    S.car.layers[3] = assets_texture(CAR_LAYOUT_3);
    S.car.layers[4] = assets_texture(CAR_LAYOUT_4);
    S.car.layers[5] = assets_texture(CAR_LAYOUT_5);
    S.car.layers[6] = assets_texture(CAR_LAYOUT_6);
    S.car.layers[7] = assets_texture(CAR_LAYOUT_7);

    S.car.position = (Vector2){512.0f, 512.0f};
    S.car.velocity = (Vector2){-1.0f, 0.0f};
    S.car.rotation = 0.0f;
    S.car.trace_idx = 0;

    S.road = assets_texture(ROAD_TESTS);

    S.camera.offset = (Vector2){400.0f, 300.0f};
    S.camera.target = S.car.position;
    S.camera.rotation = 0.0f;
    S.camera.zoom = 1.0f;
}

static void update(AppState *app, float dt) {
    (void)app;
    (void)dt;

    S.counter += 1;

    float acceleration = 200.0f;

    if (IsKeyDown(KEY_LEFT)) S.car.rotation -= 180.0f * dt;
    if (IsKeyDown(KEY_RIGHT)) S.car.rotation += 180.0f * dt;

    Vector2 foward = {0.0f, 0.0f};
    if (IsKeyDown(KEY_UP)) {
        foward = (Vector2){
            -cosf(S.car.rotation * DEG2RAD),
            -sinf(S.car.rotation * DEG2RAD)
        };

        S.car.velocity.x += foward.x * acceleration * dt;
        S.car.velocity.y += foward.y * acceleration * dt;
    }

    S.car.velocity = Vector2Scale(S.car.velocity, 0.99f);
    S.car.velocity = Vector2ClampValue(S.car.velocity, 0.0f, 400.0f);
    S.car.position = Vector2Add(
        S.car.position, 
        Vector2Scale(S.car.velocity, dt)
    );

    S.camera.target = Vector2Lerp(
        S.camera.target,
        Vector2Add(
            S.car.position,
            Vector2Scale(foward, 100.0f)
        ),
        0.1f
    );

    S.camera.target.x = Clamp(S.camera.target.x, 400.0f, 1648.0f);
    S.camera.target.y = Clamp(S.camera.target.y, 300.0f, 1748.0f);
}

static void car_draw(void) {
    Color color = {0, 0, 0, 100};
    Vector2 size = {10.0f, 10.0f};

    Vector2 backwards = (Vector2){
        cosf(S.car.rotation * DEG2RAD),
        sinf(S.car.rotation * DEG2RAD)
    };
    Vector2 par = {-backwards.y, backwards.x};
    Vector2 base = Vector2Add(
        S.car.position, 
        Vector2Scale(backwards, 64)
    );

    if (S.counter % 1 == 0) {
        S.car.smoke[S.car.smoke_idx] = base;
        S.car.smoke_velocity[S.car.smoke_idx] = Vector2Scale(backwards, 4);
        S.car.smoke_age[S.car.smoke_idx] = 0;
        S.car.smoke_idx = (S.car.smoke_idx + 1) % CAR_SMOKE_LEN;
    }

    if (S.counter % 5 == 0) { 
        S.car.traces[S.car.trace_idx] = Vector2Add(
            base, 
            Vector2Scale(par, 16)
        );

        S.car.traces[S.car.trace_idx + 1] = Vector2Subtract(
            base, 
            Vector2Scale(par, 16)
        );
 
        S.car.trace_idx = (S.car.trace_idx + 2) % CAR_TRACES_LEN;
    }

    for (size_t index = 0; index < CAR_TRACES_LEN; ++index) {
        DrawRectangleV(S.car.traces[index], size, color);
    }
    // for (size_t index = 0; index < S.car.trace_idx; index += 2) {
    //     // TODO: Transform this in lines.
    //     DrawLineEx(
    //         S.car.traces[index],
    //         S.car.traces[index + 2],
    //         10.0f,
    //         color
    //     );
    // }
 
    float dt = GetFrameTime();
    for (size_t index = 0; index < CAR_SMOKE_LEN; ++index) {
        S.car.smoke_age[index] += 1;
        S.car.smoke_velocity[index] = Vector2Scale(S.car.smoke_velocity[index], 0.99f);
        S.car.smoke_velocity[index].y -= 100.0f;
        S.car.smoke[index] = Vector2Add(S.car.smoke[index], 
            Vector2Scale(S.car.smoke_velocity[index], dt)
        );
        DrawRectangleV(S.car.smoke[index], size, 
            (Color) {
                255, 255, 255,
                (int)(255.0f - Clamp(S.car.smoke_age[index] * 30, 0.0f, 255.0f))
            }
        );
    }

    for (size_t index = 0; index < 8; ++index) {
        Rectangle src = {0.0f, 0.0f, (float)S.car.layers[index].width, (float)S.car.layers[index].height};
        Rectangle dst = {S.car.position.x, S.car.position.y - index * 4, src.width * 2.0f, src.height * 2.0f};
        Vector2 origin = { dst.width / 2.0f, dst.height / 2.0f };

        DrawTexturePro(S.car.layers[index], src, dst, origin, S.car.rotation, WHITE);
    }

}

static void road_draw(void) {
    Rectangle road_src = {0.0f, 0.0f, (float)S.road.width, (float)S.road.height};
    Rectangle road_dst = {0.0f, 0.0f, (float)road_src.width * 4.0f, (float)road_src.height * 4.0f};
    Vector2 road_origin = { 0.0f, 0.0f };
    DrawTexturePro(S.road, road_src, road_dst, road_origin, 0.0f, WHITE);
}

static void render(AppState *app) {
    (void)app;

    BeginMode2D(S.camera);

    road_draw();
    car_draw();

    EndMode2D();

    DrawText(TextFormat("pos: %d %d", (int)S.car.position.x, (int)S.car.position.y), 0, 0, 16, BLACK);
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
