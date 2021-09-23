#include <raylib.h>
#include <raymath.h>

#include "language_layer.h"
#include "memory.h"
#include "physics.h"
#include "player.h"
#include "renderer.h"

struct GameState {
  Renderer renderer;
  physics::World world;
  v2 world_cursor_position;
  Player player;
};

struct Application {
  MemoryArena permanent_arena;
  MemoryArena frame_arena;
};

global Application* app;
global GameState* game;

// UNITY BUILD
#include "language_layer.cpp"
#include "memory.cpp"
#include "renderer.cpp"
#include "physics.cpp"
#include "player.cpp"

void setup_physics_demo() {
  v2 mid = {GetScreenWidth() * PIXEL_2_METER * 0.5f, GetScreenHeight() * PIXEL_2_METER * 0.5f};
  physics::AddBody(&game->world, {mid.x, 0.0f}, {mid.x * 4.0f, 3.0f}, F32_Max);
  physics::AddBody(&game->world, {0.f, 6.f}, {4.000000f, 0.250000f}, F32_Max)->rotation
      = -0.261799f;

  physics::AddBody(&game->world, {1.610000f, 2.620000f}, {4.000000f, 0.250000f}, F32_Max)->rotation
      = PI / 8.f;

  physics::AddBody(&game->world, {6.460000f, 4.460000f}, {4.000000f, 0.250000f}, F32_Max)->rotation
      = 0.261799f;
}

int main(int argc, char** argv) {
  // Init Raylib
  InitWindow(1024, 768, "c_physics");
  Defer(CloseWindow(););

  SetTargetFPS(60);

  Application application = {0};
  application.permanent_arena = MemoryArenaInitialize();
  application.frame_arena = MemoryArenaInitialize();
  Defer(MemoryArenaRelease(&application.permanent_arena));
  Defer(MemoryArenaRelease(&application.frame_arena));
  app = &application;

  game = (GameState*)MemoryArenaPush(&app->permanent_arena, sizeof(GameState));

  game->renderer = RenderInit();
  physics::InitWorld(&game->world, {0.0f, -10.0f});
  v2 mid = {GetScreenWidth() * PIXEL_2_METER * 0.5f, GetScreenHeight() * PIXEL_2_METER * 0.5f};

  game->player = PlayerInit(&game->world);

  // ground
  setup_physics_demo();

  while (!WindowShouldClose()) {
    // Update Game state
    //-----------------------------------------------
    game->world_cursor_position
        = game->renderer.world_camera.target + game->renderer.world_camera.offset;
    game->world_cursor_position.x += GetMouseX() * PIXEL_2_METER;
    game->world_cursor_position.y += (GetScreenHeight() - GetMouseY()) * PIXEL_2_METER;

    if (IsMouseButtonPressed(0)) {
      physics::Body* b = physics::AddBody(
          &game->world, game->world_cursor_position,
          {2.0f * (GetRandomValue(1, 100) / 100.0f), 1.0f * (GetRandomValue(10, 100) / 100.0f)},
          25.0f);
      b->rotation = (GetRandomValue(0, 100) / 100.0f) * 2.0f * PI;
    }

    // Camera Update
    {
      v2 camera_target = game->player.body->position;
      camera_target += game->player.body->velocity * 0.18f;

      game->renderer.world_camera.target
          = Vector2Lerp(game->renderer.world_camera.target, camera_target, 10.0f * GetFrameTime());

      game->renderer.world_camera.offset.x = -4.0f;
      game->renderer.world_camera.offset.y = -3.3f;

      if (IsKeyDown(KEY_R)) {
        game->renderer.world_camera.zoom = 1.0f;
      }
      game->renderer.world_camera.zoom += 0.18f * GetMouseWheelMove();
    }

    PlayerUpdate(&game->player);
    physics::Step(&game->world);

    BeginDrawing();
    {
      ClearBackground(WHITE);

      // Render Game
      //-----------------------------------------------
      RenderBegin(&game->renderer);
      {
        PlayerDraw(&game->player);

        physics::Draw(&game->world);
      }
      RenderEnd(&game->renderer);

      // Render UI
      //-----------------------------------------------
      SetMatrixProjection(MatrixOrtho(0.0f, GetScreenWidth(), GetScreenHeight(), 0.0f, 0.0f, 1.0f));
      DrawText("Controls:", 20, 20, 10, BLACK);
      DrawText("- WASD to move", 40, 40, 10, DARKGRAY);
      DrawText("- Space to jump", 40, 60, 10, DARKGRAY);
      DrawText("- Left click to spawn rigidbodies", 40, 80, 10, DARKGRAY);
      DrawText("- Mouse Wheel to Zoom in-out, R to reset zoom", 40, 100, 10, DARKGRAY);
    }
    EndDrawing();
  }

  return 0;
}
