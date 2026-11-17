// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Gustavo Henrique Sales


#ifndef ASSETS_H
#define ASSETS_H

#include "raylib.h"

typedef enum {
    ASSET_GROUP_COMMON,
    ASSET_GROUP_MENU,
    ASSET_GROUP_GAME,
    ASSET_GROUP_COUNT
} AssetGroup;

typedef enum {
    ASSET_TEXTURE,
    ASSET_MODEL,
    ASSET_KIND_COUNT
} AssetKind;

#define ASSET_LIST(X) \
    X(CAR_LAYOUT_0, ASSET_TEXTURE, "res/car/layout/0.png", ASSET_GROUP_GAME) \
    X(CAR_LAYOUT_1, ASSET_TEXTURE, "res/car/layout/1.png", ASSET_GROUP_GAME) \
    X(CAR_LAYOUT_2, ASSET_TEXTURE, "res/car/layout/2.png", ASSET_GROUP_GAME) \
    X(CAR_LAYOUT_3, ASSET_TEXTURE, "res/car/layout/3.png", ASSET_GROUP_GAME) \
    X(CAR_LAYOUT_4, ASSET_TEXTURE, "res/car/layout/4.png", ASSET_GROUP_GAME) \
    X(CAR_LAYOUT_5, ASSET_TEXTURE, "res/car/layout/5.png", ASSET_GROUP_GAME) \
    X(CAR_LAYOUT_6, ASSET_TEXTURE, "res/car/layout/6.png", ASSET_GROUP_GAME) \
    X(CAR_LAYOUT_7, ASSET_TEXTURE, "res/car/layout/7.png", ASSET_GROUP_GAME) \
    X(CAR_LAYOUT_N0, ASSET_TEXTURE, "res/car/layout/n0.png", ASSET_GROUP_GAME) \
    X(CAR_LAYOUT_N1, ASSET_TEXTURE, "res/car/layout/n1.png", ASSET_GROUP_GAME) \
    X(CAR_LAYOUT_N2, ASSET_TEXTURE, "res/car/layout/n2.png", ASSET_GROUP_GAME) \
    X(CAR_LAYOUT_N3, ASSET_TEXTURE, "res/car/layout/n3.png", ASSET_GROUP_GAME) \
    X(CAR_LAYOUT_N4, ASSET_TEXTURE, "res/car/layout/n4.png", ASSET_GROUP_GAME) \
    X(CAR_LAYOUT_N5, ASSET_TEXTURE, "res/car/layout/n5.png", ASSET_GROUP_GAME) \
    X(CAR_LAYOUT_N6, ASSET_TEXTURE, "res/car/layout/n6.png", ASSET_GROUP_GAME) \
    X(CAR_LAYOUT_N7, ASSET_TEXTURE, "res/car/layout/n7.png", ASSET_GROUP_GAME) \
    X(ROAD_TESTS, ASSET_TEXTURE, "res/roads/tests.png", ASSET_GROUP_GAME)


typedef enum {
    #define X(id, kind, path, group) id,
    ASSET_LIST(X)
    #undef X
    ASSET_COUNT
} AssetId;

void      assets_init(void);
void      assets_shutdown(void);
void      assets_load_group(AssetGroup g);
void      assets_unload_group(AssetGroup g);

Texture2D assets_texture(AssetId id);
Model     assets_model(AssetId id);

#endif
