// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Gustavo Henrique Sales


#ifndef SCENE_H
#define SCENE_H


typedef struct AppState AppState;

typedef struct Scene {
    const char *name;
    void (*enter) (AppState *app);
    void (*update)(AppState *app, float dt);
    void (*render)(AppState *app);
    void (*exit)  (AppState *app);
} Scene;

void scene_switch(AppState *app, const Scene *next);
void scene_apply_pending(AppState *app);
void scene_leave_current(AppState *app);

#endif // SCENE_H
