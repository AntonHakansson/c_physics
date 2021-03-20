#pragma once

#include <raylib.h>

#include "language_layer.h"

constexpr u32 MAX_DRAW_CALLS = 1024;

enum RENDER_COMMAND_TYPE {
  FILLED_RECT,
  SCALED_TEX_RECT,
  CIRCLE,
  LINE,
  TEXT,

  MAX_RENDER_COMMANDS,
};

struct RenderCommand {
  RENDER_COMMAND_TYPE type;
  u8 flags;
  union {
    // NOTE(anton) : render command RECT
    struct {
      v2 pos;
      v2 size;
      Color c;
      f32 angle;
    } rect;

    // NOTE(anton) : render command RECT
    struct {
      v2 pos;
      v2 size;
      Texture2D texture;
      Color tint;
      f32 angle;
    } texture;

    // NOTE(anton) : render command CIRCLE
    struct {
      v2 pos;
      f32 radius;
      Color c;
    } circle;

    // NOTE(anton) : render command LINE
    struct {
      v2 start_pos;
      v2 end_pos;
      Color c;
    } line;

    // NOTE(anton) : render command TEXT
    struct {
      v2 pos;
      Color c;
      f32 font_scale;
      const char *text;
    } text;
  };
};

struct Renderer {
  RenderCommand render_commands[MAX_DRAW_CALLS];
  u32 render_commands_count;

  Camera2D world_camera;
  u32 left, right, top, bottom;
};
