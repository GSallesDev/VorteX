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

#define BOX_W 96
#define BOX_H 96
#define LAYER_COUNT 8
#define HEIGHT_SCALE 2.0f
#define BEND 0.0f
#define SPRITE_YAW_OFFSET 0.0f

static const Vector2 PIVOT = { BOX_W/2.0f, BOX_H/2.0f };

typedef struct {
    Vector2 s[4];
    float yawRad, pitch, roll;
} LayerPose;

typedef struct {
    Vector2 position;
    Vector2 velocity;

    float rotation, roll, pitch;
    float roll_vel, pitch_vel;

    Vector2 prev_velocity;
    float prev_yaw;

    Texture2D layers[8];
    Texture2D layers_normal[8];

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

    RenderTexture2D colorRT;
    RenderTexture2D normalRT;

    Shader part_shader; 
    Shader light_shader;

    int part_colX, part_colY, part_colZ;
    int lt_normal, lt_light, lt_carPos;
    int lt_boxSize, lt_pivot, lt_scale;
} S;

static void enter(AppState *app) {
    (void)app;
    assets_load_group(ASSET_GROUP_GAME);

    const AssetId layer_tex_ids[8]  = {
        CAR_LAYOUT_0, CAR_LAYOUT_1,
        CAR_LAYOUT_2, CAR_LAYOUT_3,
        CAR_LAYOUT_4, CAR_LAYOUT_5,
        CAR_LAYOUT_6, CAR_LAYOUT_7
    };

    const AssetId normal_layer_tex_ids[8]  = {
        CAR_LAYOUT_N0, CAR_LAYOUT_N1,
        CAR_LAYOUT_N2, CAR_LAYOUT_N3,
        CAR_LAYOUT_N4, CAR_LAYOUT_N5,
        CAR_LAYOUT_N6, CAR_LAYOUT_N7
    };

    for (size_t index = 0; index < 8; ++index) {
        S.car.layers[index] = assets_texture(layer_tex_ids[layer_tex_ids[index]]);
        S.car.layers_normal[index] = assets_texture(normal_layer_tex_ids[layer_tex_ids[index]]);
    }
    
    S.car.position = (Vector2){512.0f, 512.0f};
    S.car.velocity = (Vector2){-1.0f, 0.0f};
    S.car.rotation = 0.0f;
    S.car.trace_idx = 0;

    S.road = assets_texture(ROAD_TESTS);

    S.camera.offset = (Vector2){400.0f, 300.0f};
    S.camera.target = S.car.position;
    S.camera.rotation = 0.0f;
    S.camera.zoom = 1.0f;

    S.colorRT = LoadRenderTexture(BOX_W, BOX_H);
    S.normalRT = LoadRenderTexture(BOX_W, BOX_H);
    SetTextureFilter(S.colorRT.texture,  TEXTURE_FILTER_POINT);
    SetTextureFilter(S.normalRT.texture,  TEXTURE_FILTER_POINT);

    S.part_shader  = LoadShader(0, "res/shaders/normal_part.fs");
    S.light_shader = LoadShader(0, "res/shaders/light.fs");

    S.part_colX = GetShaderLocation(S.part_shader, "colX");
    S.part_colY = GetShaderLocation(S.part_shader, "colY");
    S.part_colZ = GetShaderLocation(S.part_shader, "colZ");

    S.lt_normal  = GetShaderLocation(S.light_shader, "normalMap");
    S.lt_light   = GetShaderLocation(S.light_shader, "lightPos");
    S.lt_carPos  = GetShaderLocation(S.light_shader, "carPos");
    S.lt_boxSize = GetShaderLocation(S.light_shader, "boxSize");
    S.lt_pivot   = GetShaderLocation(S.light_shader, "pivot");
    S.lt_scale   = GetShaderLocation(S.light_shader, "scale");
}

static void car_update(Vector2 forward, float dt) {
    if (IsKeyDown(KEY_LEFT))  S.car.rotation -= 180.0f * dt;
    if (IsKeyDown(KEY_RIGHT)) S.car.rotation += 180.0f * dt;

    S.car.velocity = Vector2Scale(S.car.velocity, 0.99f);
    S.car.velocity = Vector2ClampValue(S.car.velocity, 0.0f, 400.0f);
    S.car.position = Vector2Add(
        S.car.position,
        Vector2Scale(S.car.velocity, dt)
    );

    if (dt <= 0.0f) return;

    float yawRad = S.car.rotation * DEG2RAD;
    Vector2 fwd   = { -cosf(yawRad), -sinf(yawRad) };

    Vector2 accel = Vector2Scale(Vector2Subtract(S.car.velocity, S.car.prev_velocity), 1.0f / dt);
    float a_long = Vector2DotProduct(accel, fwd);

    float yaw_rate = (S.car.rotation - S.car.prev_yaw) * DEG2RAD / dt;
    float speed    = Vector2Length(S.car.velocity);
    float a_lat    = speed * yaw_rate;

    const float ROLL_GAIN  = -0.0002f;
    const float PITCH_GAIN =  0.0005f;

    float roll_target  = a_lat  * ROLL_GAIN;
    float pitch_target = a_long * PITCH_GAIN;

    const float ks = 60.0f, kd = 8.0f;

    S.car.roll_vel  += (-ks * (S.car.roll  - roll_target)  - kd * S.car.roll_vel)  * dt;
    S.car.pitch_vel += (-ks * (S.car.pitch - pitch_target) - kd * S.car.pitch_vel) * dt;
    S.car.roll  += S.car.roll_vel  * dt;
    S.car.pitch += S.car.pitch_vel * dt;

    S.car.roll  = Clamp(S.car.roll,  -0.35f, 0.35f);
    S.car.pitch = Clamp(S.car.pitch, -0.35f, 0.35f);

    S.car.prev_velocity = S.car.velocity;
    S.car.prev_yaw      = S.car.rotation;
}

static void camera_update(Vector2 foward) {
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

static void update(AppState *app, float dt) {
    (void)app;
    (void)dt;
    S.counter += 1;

    float acceleration = 200.0f;
    Vector2 foward = {0.0f, 0.0f};
    if (IsKeyDown(KEY_UP)) {
        foward = (Vector2){
            -cosf(S.car.rotation * DEG2RAD),
            -sinf(S.car.rotation * DEG2RAD)
        };

        S.car.velocity.x += foward.x * acceleration * dt;
        S.car.velocity.y += foward.y * acceleration * dt;
    }

    car_update(foward, dt);
    camera_update(foward);
}

static Vector3 Rotate3D(Vector3 p, float yaw, float roll, float pitch) {
    float cr = cosf(roll), sr = sinf(roll);
    p = (Vector3){ p.x*cr + p.z*sr, p.y, -p.x*sr + p.z*cr };
    float cp = cosf(pitch), sp = sinf(pitch);
    p = (Vector3){ p.x, p.y*cp - p.z*sp, p.y*sp + p.z*cp };
    float cy = cosf(yaw), sy = sinf(yaw);
    return (Vector3){ p.x*cy - p.y*sy, p.x*sy + p.y*cy, p.z };
}

static Vector2 Project(Vector3 p, Vector2 origin, float heightScale) {
    return (Vector2){ origin.x + p.x, origin.y + p.y - p.z*heightScale };
}

static LayerPose layer_pose(size_t i) {
    LayerPose p;
    float z = (float)i;
    float zMax = (float)(LAYER_COUNT - 1);
    float k = (1.0f - BEND) + BEND * (z / zMax);

    p.yawRad = (S.car.rotation + SPRITE_YAW_OFFSET) * DEG2RAD;
    p.pitch  = S.car.pitch * k;
    p.roll   = S.car.roll  * k;

    float hw = S.car.layers[i].width  * 0.5f;
    float hh = S.car.layers[i].height * 0.5f;
    Vector3 c[4] = {
        { -hw, -hh, z }, { -hw, hh, z }, { hw, hh, z }, { hw, -hh, z }
    };
    for (int j = 0; j < 4; j++)
        p.s[j] = Project(Rotate3D(c[j], p.yawRad, p.pitch, p.roll), PIVOT, HEIGHT_SCALE);
    return p;
}

static void emit_quad(Texture2D tex, const Vector2 s[4], Color c) {
    rlSetTexture(tex.id);
    rlBegin(RL_QUADS);
        rlColor4ub(c.r, c.g, c.b, c.a);
        rlTexCoord2f(0, 0); rlVertex2f(s[0].x, s[0].y);
        rlTexCoord2f(0, 1); rlVertex2f(s[1].x, s[1].y);
        rlTexCoord2f(1, 1); rlVertex2f(s[2].x, s[2].y);
        rlTexCoord2f(1, 0); rlVertex2f(s[3].x, s[3].y);
    rlEnd();
    rlSetTexture(0);
}

static void car_shadow(Vector3 light, float scale) {
    rlDisableBackfaceCulling();

    for (size_t i = 0; i < LAYER_COUNT; ++i) {
        float z = (float)i;
        float zMax = (float)(LAYER_COUNT - 1);
        float k = (1.0f - BEND) + BEND * (z / zMax);

        float yawRad = (S.car.rotation + SPRITE_YAW_OFFSET) * DEG2RAD;
        float pitch = S.car.pitch * k, roll = S.car.roll * k;

        float hw = S.car.layers[i].width  * 0.5f;
        float hh = S.car.layers[i].height * 0.5f;
        Vector3 c[4] = {
            { -hw, -hh, z }, { -hw, hh, z }, { hw, hh, z }, { hw, -hh, z }
        };

        Vector2 g[4];
        for (int j = 0; j < 4; j++) {
            Vector3 r = Rotate3D(c[j], yawRad, pitch, roll);

            Vector2 w = { S.car.position.x + r.x * scale,
                          S.car.position.y + r.y * scale };
            float h = r.z * HEIGHT_SCALE * scale;

            float denom = fmaxf(light.z - h, 1.0f);
            float t = h / denom;

            g[j] = (Vector2){ w.x + (w.x - light.x) * t,
                              w.y + (w.y - light.y) * t };
        }
        emit_quad(S.car.layers[i], g, (Color){0, 0, 0, 155});
    }
    rlDrawRenderBatchActive();
    rlEnableBackfaceCulling();
}

static void car_compose(void) {
    rlDisableBackfaceCulling();

    BeginTextureMode(S.colorRT);
        ClearBackground(BLANK);
        for (size_t i = 0; i < LAYER_COUNT; ++i) {
            LayerPose p = layer_pose(i);
            emit_quad(S.car.layers[i], p.s, WHITE);
        }
        rlDrawRenderBatchActive();
    EndTextureMode();

    BeginTextureMode(S.normalRT);
        ClearBackground((Color){128, 128, 255, 255});
        BeginShaderMode(S.part_shader);
        for (size_t i = 0; i < LAYER_COUNT; ++i) {
            LayerPose p = layer_pose(i);

            Vector3 cx = Rotate3D((Vector3){1, 0, 0}, p.yawRad, p.pitch, p.roll);
            Vector3 cy = Rotate3D((Vector3){0, 1, 0}, p.yawRad, p.pitch, p.roll);
            Vector3 cz = Rotate3D((Vector3){0, 0, 1}, p.yawRad, p.pitch, p.roll);

            rlDrawRenderBatchActive();
            SetShaderValue(S.part_shader, S.part_colX, &cx, SHADER_UNIFORM_VEC3);
            SetShaderValue(S.part_shader, S.part_colY, &cy, SHADER_UNIFORM_VEC3);
            SetShaderValue(S.part_shader, S.part_colZ, &cz, SHADER_UNIFORM_VEC3);

            emit_quad(S.car.layers_normal[i], p.s, WHITE);
        }
        EndShaderMode();
    EndTextureMode();

    rlEnableBackfaceCulling();
}

static void car_effects(void) {
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
}
static void car_draw(void) {
    float scale = 2.0f;

    Vector2 m = GetScreenToWorld2D(GetMousePosition(), S.camera);
    Vector3 light = { m.x, m.y, 120.0f };

    Vector2 carPos  = S.car.position;
    Vector2 boxSize = { BOX_W, BOX_H };

    Rectangle src = { 0, 0, BOX_W, -BOX_H };
    Rectangle dst = { carPos.x, carPos.y, BOX_W * scale, BOX_H * scale };
    Vector2 origin = { PIVOT.x * scale, PIVOT.y * scale };

    BeginShaderMode(S.light_shader);
        SetShaderValueTexture(S.light_shader, S.lt_normal, S.normalRT.texture);
        SetShaderValue(S.light_shader, S.lt_light,   &light,   SHADER_UNIFORM_VEC3);
        SetShaderValue(S.light_shader, S.lt_carPos,  &carPos,  SHADER_UNIFORM_VEC2);
        SetShaderValue(S.light_shader, S.lt_boxSize, &boxSize, SHADER_UNIFORM_VEC2);
        SetShaderValue(S.light_shader, S.lt_pivot,   &PIVOT,   SHADER_UNIFORM_VEC2);
        SetShaderValue(S.light_shader, S.lt_scale,   &scale,   SHADER_UNIFORM_FLOAT);
        DrawTexturePro(S.colorRT.texture, src, dst, origin, 0.0f, WHITE);
    EndShaderMode();
}

static void road_draw(void) {
    Rectangle road_src = {0.0f, 0.0f, (float)S.road.width, (float)S.road.height};
    Rectangle road_dst = {0.0f, 0.0f, (float)road_src.width * 4.0f, (float)road_src.height * 4.0f};
    Vector2 road_origin = { 0.0f, 0.0f };
    DrawTexturePro(S.road, road_src, road_dst, road_origin, 0.0f, WHITE);
}

static void render(AppState *app) {
    (void)app;

    float scale = 2.0f;

    Vector2 m = GetScreenToWorld2D(GetMousePosition(), S.camera);
    Vector3 light = { m.x, m.y, 120.0f };

    car_compose();

    BeginDrawing();
        ClearBackground(RAYWHITE);
        BeginMode2D(S.camera);
            road_draw();
            car_effects();
            car_shadow(light, scale);
            car_draw();
        EndMode2D();
        DrawText(TextFormat("pos: %d %d", (int)S.car.position.x, (int)S.car.position.y), 0, 0, 16, BLACK);
    EndDrawing();
}

static void exit_(AppState *app) {
    (void)app;
    UnloadRenderTexture(S.colorRT);
    UnloadRenderTexture(S.normalRT);
    assets_unload_group(ASSET_GROUP_GAME);
}

const Scene scene_game = {
    .name   = "game",
    .enter  = enter,
    .update = update,
    .render = render,
    .exit   = exit_,
};
