// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Gustavo Henrique Sales


#include <stddef.h>
#include "scene.h"
#include "app.h"


void scene_switch(AppState *app, const Scene *next) {
    app->next_scene = next;
}

void scene_apply_pending(AppState *app) {
    if (!app->next_scene) return;

    if (app->scene) app->scene->exit(app);
    app->scene = app->next_scene;
    app->next_scene = NULL;
    app->scene->enter(app);
}

void scene_leave_current(AppState *app) {
    if (app->scene) {
        app->scene->exit(app);
        app->scene = NULL;
    }
}

