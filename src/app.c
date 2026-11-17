// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Gustavo Henrique Sales


#include "app.h"
#include "assets.h"
#include "scenes/scene_menu.h"
#include "raylib.h"


bool app_init(AppState* app) {
    const int screenWidth = 800;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "...");
    SetExitKey(KEY_NULL);
    SetTargetFPS(60);
    
    assets_init();
    assets_load_group(ASSET_GROUP_COMMON);

    scene_switch(app, &scene_menu);
    scene_apply_pending(app);
    return true;
}

bool app_should_close(const AppState* app) {
    return app->should_close || WindowShouldClose();
}

void app_frame(AppState* app) {
    app->dt = GetFrameTime();
    app->scene->update(app, app->dt);
    app->scene->render(app); 
    scene_apply_pending(app);
}

void app_shutdown(AppState* app) {
    scene_leave_current(app);
    assets_shutdown();
    CloseWindow();
}
