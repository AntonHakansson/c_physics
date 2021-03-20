
#include "player.h"

#include <raylib.h>
#include <raymath.h>

#include "language_layer.h"
#include "physics.h"

Player PlayerInit(physics::World *world) {
  Player p = {0};

  p.acc = 25.0f;
  p.jump_acc = 300.0f;
  p.speed_max = 5.0f;

  p.body = physics::AddBody(world, {1.0f, 4.0f}, {1.0f, 1.5f}, 50.0f);
  p.body->friction = 1.0f;
  p.body->lock_rotation = true;

  return p;
}

void PlayerUpdate(Player *p) {
  if (p->body->is_grounded) {
    p->body->friction = 1.0f;
    p->last_grounded_timestamp_s = GetTime();
  } else {
    p->body->friction = 0.0f;
  }

  if (IsKeyPressed(KEY_SPACE)) {
    f64 time_elapsed_since_grounded_s = GetTime() - p->last_grounded_timestamp_s;
    if (time_elapsed_since_grounded_s <= 0.2) {
      p->body->force.y = p->jump_acc * p->body->mass;
    }
  }

  if (IsKeyDown(KEY_D)) {
    if (p->body->velocity.x <= p->speed_max) {
      p->body->force.x = p->acc * p->body->mass;
    }
  }
  if (IsKeyDown(KEY_A)) {
    if (p->body->velocity.x >= -p->speed_max) {
      p->body->force.x = -p->acc * p->body->mass;
    }
  }
}

void PlayerDraw(Player *p) {
  v2 h = p->body->width * 0.5f;
  PushRect(&game->renderer, p->body->position, p->body->width, BLUE);

  v2 eye = p->body->position;
  eye.y += h.y * 0.75;
  eye.x += h.x * 0.75 * (Sign(p->body->velocity.x));

  PushCircle(&game->renderer, eye, 0.05f, RED);
}
