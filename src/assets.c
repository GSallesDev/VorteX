// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 Gustavo Henrique Sales


#include "assets.h"
#include <assert.h>
#include <string.h>


typedef struct {
    AssetKind   kind;
    const char *path;
    AssetGroup  group;
} AssetMeta;

static const AssetMeta META[ASSET_COUNT] = {
    #define X(id, kind, path, group) [id] = { kind, path, group },
    ASSET_LIST(X)
    #undef X
};

typedef struct {
    bool loaded;
    union {
        Texture2D texture;
        Model     model;
    } as;
} Slot;

static struct { Slot slots[ASSET_COUNT]; } S;


static Texture2D load_texture(AssetId id) {
    if (FileExists(META[id].path)) return LoadTexture(META[id].path);

    Color a = { (unsigned char)(60 + id * 50), 120, 200, 255 };
    Color b = { 30, 30, 40, 255 };
    Image img = GenImageChecked(64, 64, 8, 8, a, b);
    Texture2D t = LoadTextureFromImage(img);
    UnloadImage(img);
    return t;
}

static Model load_model(AssetId id) {
    if (FileExists(META[id].path)) return LoadModel(META[id].path);
    return LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f));
}


void assets_init(void) {
    memset(&S, 0, sizeof S);
}

void assets_shutdown(void) {
    for (int g = 0; g < ASSET_GROUP_COUNT; g++) assets_unload_group((AssetGroup)g);
}

void assets_load_group(AssetGroup g) {
    for (int i = 0; i < ASSET_COUNT; i++) {
        Slot *s = &S.slots[i];
        if (META[i].group != g || s->loaded) continue;

        switch (META[i].kind) {
            case ASSET_TEXTURE: s->as.texture = load_texture((AssetId)i); break;
            case ASSET_MODEL:   s->as.model   = load_model((AssetId)i);   break;
            default:            assert(0 && "ERROR: AssetKind without loader");      break;
        }
        s->loaded = true;
    }
}

void assets_unload_group(AssetGroup g) {
    for (int i = 0; i < ASSET_COUNT; i++) {
        Slot *s = &S.slots[i];
        if (META[i].group != g || !s->loaded) continue;

        switch (META[i].kind) {
            case ASSET_TEXTURE: UnloadTexture(s->as.texture); break;
            case ASSET_MODEL:   UnloadModel(s->as.model);     break;
            default:            break;
        }
        s->loaded = false;
    }
}

Texture2D assets_texture(AssetId id) {
    assert(S.slots[id].loaded && "ERROR");
    assert(META[id].kind == ASSET_TEXTURE && "ERROR:");
    return S.slots[id].as.texture;
}

Model assets_model(AssetId id) {
    assert(S.slots[id].loaded && "ERROR");
    assert(META[id].kind == ASSET_MODEL && "ERROR");
    return S.slots[id].as.model;
}
