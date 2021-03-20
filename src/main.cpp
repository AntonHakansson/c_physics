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
//
#include "physics.cpp"
#include "player.cpp"

void setup_physics_box2d_debug() {
  physics::AddBody(&game->world, {4.662292f, 2.879695f}, {2.0f, 2.0f}, 10.0f)->rotation = 5.831474f;
  physics::AddBody(&game->world, {6.460000f, 4.460000f}, {4.0f, 1.5f}, 10.0f)->rotation = 0.261799f;
}

void setup_physics_demo_1() {
  v2 mid = {GetScreenWidth() * PIXEL_2_METER * 0.5f, GetScreenHeight() * PIXEL_2_METER * 0.5f};
  physics::AddBody(&game->world, {mid.x, 0.0f}, {mid.x * 4.0f, 3.0f}, F32_Max);
}

void setup_physics_demo_2() {
  v2 mid = {GetScreenWidth() * PIXEL_2_METER * 0.5f, GetScreenHeight() * PIXEL_2_METER * 0.5f};
  physics::AddBody(&game->world, {mid.x, 0.0f}, {mid.x * 4.0f, 3.0f}, F32_Max);
  physics::AddBody(&game->world, {2.30000f, 6.50000f}, {4.000000f, 0.250000f}, F32_Max)->rotation
      = -0.261799f;

  physics::AddBody(&game->world, {1.610000f, 2.620000f}, {4.000000f, 0.250000f}, F32_Max)->rotation
      = PI / 2.0f - PI / 24.0f;

  physics::AddBody(&game->world, {6.460000f, 4.460000f}, {4.000000f, 0.250000f}, F32_Max)->rotation
      = 0.261799f;
}

int main(int argc, char** argv) {
  // Init Raylib
  InitWindow(1024, 768, "BERSERKER");
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
  setup_physics_demo_2();
  // setup_physics_box2d_debug();

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
          {2.5f * (GetRandomValue(1, 100) / 100.0f), 1.5f * (GetRandomValue(10, 100) / 100.0f)},
          25.0f);
      b->rotation = (GetRandomValue(0, 100) / 100.0f) * 2.0f * PI;
    }

    // Camera Update
    {
      v2 camera_target = game->player.body->position;
      camera_target += game->player.body->velocity * 0.18f;

      game->renderer.world_camera.target
          = Vector2Lerp(game->renderer.world_camera.target, camera_target, 10.0f * GetFrameTime());

      game->renderer.world_camera.offset.x = -3.0f;
      game->renderer.world_camera.offset.y = -2.0f;

      if (IsKeyDown(KEY_LEFT_ALT)) {
        game->renderer.world_camera.zoom += 0.2f * GetMouseWheelMove();
        // // if (IsMouseButtonPressed(MOUSE_))
        // Log("mouse scroll %d\n", GetMouseWheelMove());
      }
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

#if DEVELOPER
        physics::DebugDraw(&game->world);
#endif

        PushText(&game->renderer, "HELLO BIATCH", game->player.body->position, 1.0f, BLACK);
      }
      RenderEnd(&game->renderer);

      // Render UI
      //-----------------------------------------------
      SetMatrixProjection(MatrixOrtho(0.0f, GetScreenWidth(), GetScreenHeight(), 0.0f, 0.0f, 1.0f));
      DrawFPS(10, 10);
    }
    EndDrawing();
  }

  return 0;
}
