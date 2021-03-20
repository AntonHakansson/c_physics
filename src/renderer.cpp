#include "renderer.h"

#include <raylib.h>

#include "physics.h"

Renderer RenderInit() {
  Renderer r = {};

  r.world_camera.zoom = 1.0f;

  return r;
}

Vector2 RenderVector2Remap(Renderer *r, Vector2 a) {
  v2 screen_space = a * METER_2_PIXEL;
  screen_space.y = GetScreenHeight() - screen_space.y;
  return screen_space;
}

void PushRect(Renderer *r, v2 pos, v2 size, f32 angle, Color c) {
  pos = RenderVector2Remap(r, pos);
  size = size * METER_2_PIXEL;

  auto *command = r->render_commands + r->render_commands_count;
  command->type = RENDER_COMMAND_TYPE::FILLED_RECT;
  command->rect.pos = pos;
  command->rect.size = size;
  command->rect.c = c;
  command->rect.angle = angle;

  r->render_commands_count++;
}

void PushTexture(Renderer *r, v2 pos, v2 size, Texture texture, f32 angle) {
  pos = RenderVector2Remap(r, pos);
  size = size * METER_2_PIXEL;

  auto *command = r->render_commands + r->render_commands_count;
  command->type = RENDER_COMMAND_TYPE::SCALED_TEX_RECT;
  command->texture.pos = pos;
  command->texture.size = size;
  command->texture.texture = texture;
  command->texture.angle = angle;
  command->texture.tint = WHITE;

  r->render_commands_count++;
}

void PushRect(Renderer *r, v2 pos, v2 size, Color c) { PushRect(r, pos, size, 0.0f, c); }

void PushCircle(Renderer *r, v2 pos, f32 radius, Color c) {
  pos = RenderVector2Remap(r, pos);

  auto *command = r->render_commands + r->render_commands_count;
  command->type = RENDER_COMMAND_TYPE::CIRCLE;
  command->circle.pos = pos;
  command->circle.radius = radius * METER_2_PIXEL;
  command->circle.c = c;

  r->render_commands_count++;
}

void PushLine(Renderer *r, v2 start_pos, v2 end_pos, Color c) {
  start_pos = RenderVector2Remap(r, start_pos);
  end_pos = RenderVector2Remap(r, end_pos);

  auto *command = r->render_commands + r->render_commands_count;
  command->type = RENDER_COMMAND_TYPE::LINE;
  command->line.start_pos = start_pos;
  command->line.end_pos = end_pos;
  command->line.c = c;

  r->render_commands_count++;
}

void PushText(Renderer *r, char *text, v2 pos, f32 font_scale, Color c) {
  pos = RenderVector2Remap(r, pos);
  auto *command = r->render_commands + r->render_commands_count;

  command->type = RENDER_COMMAND_TYPE::TEXT;
  command->text.text = text;
  command->text.pos = pos;
  command->text.font_scale = font_scale;
  command->text.c = c;

  r->render_commands_count++;
}

void Render(Renderer *r) {
  for (u32 i = 0; i < r->render_commands_count; i++) {
    auto *command = r->render_commands + i;
    switch (command->type) {
      case RENDER_COMMAND_TYPE::FILLED_RECT: {
        Rectangle rect;
        rect.x = command->rect.pos.x;
        rect.y = command->rect.pos.y;

        rect.width = command->rect.size.x;
        rect.height = command->rect.size.y;

        DrawRectanglePro(rect, command->rect.size * 0.5f, command->rect.angle, command->rect.c);
      } break;

      case RENDER_COMMAND_TYPE::SCALED_TEX_RECT: {
        Rectangle rect;
        rect.x = command->texture.pos.x;
        rect.y = command->texture.pos.y;
        rect.width = command->texture.size.x;
        rect.height = command->texture.size.y;

        DrawTexturePro(
            command->texture.texture,
            {0, 0, (f32)command->texture.texture.width, (f32)command->texture.texture.height}, rect,
            command->texture.size * 0.5f, -command->texture.angle * RAD2DEG, command->texture.tint);
      } break;

      case RENDER_COMMAND_TYPE::CIRCLE: {
        DrawCircleV(command->circle.pos, command->circle.radius, command->circle.c);
      } break;

      case RENDER_COMMAND_TYPE::LINE: {
        DrawLineV(command->line.start_pos, command->line.end_pos, command->line.c);
      } break;

      case RENDER_COMMAND_TYPE::TEXT: {
        DrawText(command->text.text, command->text.pos.x, command->text.pos.y,
                 command->text.font_scale, command->text.c);
      } break;
    }
  }
}

void RenderBegin(Renderer *r) {
  r->render_commands_count = 0;

  Camera2D camera = r->world_camera;
  camera.target = RenderVector2Remap(r, r->world_camera.target);
  camera.offset = r->world_camera.offset * METER_2_PIXEL * -1.0f;
  camera.offset.y *= -1.0f;
  camera.offset.y += GetScreenHeight();

  BeginMode2D(camera);
}

void RenderEnd(Renderer *r) {
  Render(r);
  EndMode2D();
}
