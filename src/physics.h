#pragma once
#define MAX_BODY_COUNT 256
#define MAX_CONTACT_POINTS 2
#define METER_2_PIXEL 100.0f
#define PIXEL_2_METER (1.0f / METER_2_PIXEL)

#include <raymath.h>

#include "language_layer.h"
#include "memory.h"

namespace physics {

  // Box vertex and edge numbering:
  //
  //        ^ y
  //        |
  //        e1
  //   v2 ------ v1
  //    |        |
  // e2 |        | e4  --> x
  //    |        |
  //   v3 ------ v4
  //        e3
  //

  enum Axis { FACE_A_X, FACE_A_Y, FACE_B_X, FACE_B_Y };

  enum EdgeNumbers { NO_EDGE = 0, EDGE1, EDGE2, EDGE3, EDGE4 };

  union FeaturePair {
    struct Edges {
      u8 in_edge_1;
      u8 out_edge_1;
      u8 in_edge_2;
      u8 out_edge_2;
    } e;
    u32 value;
  };

  struct ClipVertex {
    v2 v;
    FeaturePair fp;
  };

  struct Body {
    v2 position;
    f32 rotation;

    v2 velocity;
    f32 angular_velocity;

    v2 force;
    f32 torque;

    v2 width;

    f32 friction;
    f32 mass, inv_mass;

    f32 inertia, inv_inertia;

    b32 lock_rotation;
    b32 is_grounded;
  };

  struct Contact {
    v2 position;
    v2 normal;
    v2 r1, r2;
    f32 seperation;
    f32 acc_normal_impulse;
    f32 acc_tangent_impulse;
    f32 acc_biased_normal_impulse;
    f32 mass_normal;
    f32 mass_tangent;
    f32 bias;
    FeaturePair feature;
  };

  struct ArbiterKey {
    Body *b1;
    Body *b2;
  };

  struct Arbiter {
    Body *b1;
    Body *b2;
    float combined_friction;

    Contact contacts[MAX_CONTACT_POINTS];
    u32 contacts_count;
  };

  struct World {
    Body bodies[MAX_BODY_COUNT];
    HashTable<Arbiter, MAX_BODY_COUNT> arbiter_table;

    u32 bodies_count;
    Vector2 gravity;
    usize iterations;

#if DEVELOPER
    b32 debug;
#endif
  };
};  // namespace physics
