#pragma once

#include "physics.h"

struct Player {
  physics::Body *body;

  f32 acc;
  f32 speed_max;

  f32 jump_acc;
  f64 last_grounded_timestamp_s;
};
