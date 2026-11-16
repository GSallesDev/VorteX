// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Gustavo Henrique Sales


#ifndef APP_H
#define APP_H

#include <stdbool.h>
#include "scene.h"

struct AppState {
    const Scene* scene;
    const Scene* next_scene;

    float dt;
    bool should_close;
};

bool app_init(AppState* app);
bool app_should_close(const AppState* app);
void app_frame(AppState* app);
void app_shutdown(AppState* app);

#endif // APP_H
