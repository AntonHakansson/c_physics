#include "physics.h"

#include <raylib.h>
#include <raymath.h>

#include "language_layer.h"

namespace physics {
  void InitWorld(World *world, Vector2 gravity) {
    *world = {};
    world->gravity = gravity;
    world->iterations = 10;
    HashTableInit(&world->arbiter_table);
  }

  Body *AddBody(World *world, v2 position, v2 width, f32 mass) {
    Assert(world->bodies_count <= MAX_BODY_COUNT);
    Body *body = world->bodies + world->bodies_count;

    *body = {0};
    body->position = position;
    body->width = width;
    body->mass = mass;
    body->friction = 0.2f;
    body->inertia = F32_Max;

    if (mass < F32_Max) {
      body->inv_mass = 1.0f / mass;
      body->inertia = mass * (width.x * width.x + width.y * width.y) / 12.0f;
      body->inv_inertia = 1.0f / body->inertia;
    }

    world->bodies_count++;

    return body;
  }

  int ClipSegmentToLine(ClipVertex v_out[2], ClipVertex v_in[2], const v2 &normal, f32 offset,
                        f32 clip_edge) {
    // Start with no output points
    u64 num_out = 0;

    // Calculate the distance of end points to the line
    f32 distance0 = Vector2DotProduct(normal, v_in[0].v) - offset;
    f32 distance1 = Vector2DotProduct(normal, v_in[1].v) - offset;

    // If the points are behind the plane
    if (distance0 <= 0.0f) {
      v_out[num_out] = v_in[0];
      num_out++;
    }
    if (distance1 <= 0.0f) {
      v_out[num_out] = v_in[1];
      num_out++;
    }

    // If the points are on different sides of the plane
    if (distance0 * distance1 < 0.0f) {
      // Find intersection point of edge and plane
      f32 interp = distance0 / (distance0 - distance1);
      v_out[num_out].v = v_in[0].v + (v_in[1].v - v_in[0].v) * interp;
      if (distance0 > 0.0f) {
        v_out[num_out].fp = v_in[0].fp;
        v_out[num_out].fp.e.in_edge_1 = clip_edge;
        v_out[num_out].fp.e.in_edge_2 = NO_EDGE;
      } else {
        v_out[num_out].fp = v_in[1].fp;
        v_out[num_out].fp.e.out_edge_1 = clip_edge;
        v_out[num_out].fp.e.out_edge_2 = NO_EDGE;
      }

      num_out++;
    }

    return num_out;
  }

  void ComputeIncidentEdge(ClipVertex c[2], const v2 &h, const v2 &pos, const Matrix2x2 &rot,
                           const v2 &normal) {
    // The normal is from the reference box. Convert it
    // to the incident boxe's frame and flip sign.
    Matrix2x2 rotT = Matrix2x2Transpose(rot);
    v2 n = (rotT * normal) * (-1.0f);
    v2 n_abs = Vector2Abs(n);

    if (n_abs.x > n_abs.y) {
      if (n.x >= 0.0f) {
        c[0].v = {h.x, -h.y};
        c[0].fp.e.in_edge_2 = EDGE3;
        c[0].fp.e.out_edge_2 = EDGE4;

        c[1].v = {h.x, h.y};
        c[1].fp.e.in_edge_2 = EDGE4;
        c[1].fp.e.out_edge_2 = EDGE1;
      } else {
        c[0].v = {-h.x, h.y};
        c[0].fp.e.in_edge_2 = EDGE1;
        c[0].fp.e.out_edge_2 = EDGE2;

        c[1].v = {-h.x, -h.y};
        c[1].fp.e.in_edge_2 = EDGE2;
        c[1].fp.e.out_edge_2 = EDGE3;
      }
    } else {
      if (n.y >= 0.0f) {
        c[0].v = {h.x, h.y};
        c[0].fp.e.in_edge_2 = EDGE4;
        c[0].fp.e.out_edge_2 = EDGE1;

        c[1].v = {-h.x, h.y};
        c[1].fp.e.in_edge_2 = EDGE1;
        c[1].fp.e.out_edge_2 = EDGE2;
      } else {
        c[0].v = {-h.x, -h.y};
        c[0].fp.e.in_edge_2 = EDGE2;
        c[0].fp.e.out_edge_2 = EDGE3;

        c[1].v = {h.x, -h.y};
        c[1].fp.e.in_edge_2 = EDGE3;
        c[1].fp.e.out_edge_2 = EDGE4;
      }
    }

    c[0].v = pos + rot * c[0].v;
    c[1].v = pos + rot * c[1].v;
  }

  u64 Collide(Contact *contacts, Body *b1, Body *b2) {
    v2 h1 = b1->width * 0.5f;
    v2 h2 = b2->width * 0.5f;

    v2 pos1 = b1->position;
    v2 pos2 = b2->position;

    Matrix2x2 rot1 = Matrix2x2FromAngle(b1->rotation);
    Matrix2x2 rot2 = Matrix2x2FromAngle(b2->rotation);

    Matrix2x2 rot1T = Matrix2x2Transpose(rot1);
    Matrix2x2 rot2T = Matrix2x2Transpose(rot2);

    v2 dp = pos2 - pos1;
    v2 d1 = rot1T * dp;
    v2 d2 = rot2T * dp;

    Matrix2x2 C = rot1T * rot2;
    Matrix2x2 absC = Matrix2x2Abs(C);
    Matrix2x2 absCT = Matrix2x2Transpose(absC);

    // Box 1 faces
    v2 face1 = Vector2Abs(d1) - h1 - (absC * h2);
    if (face1.x > 0.0f || face1.y > 0.0f) {
      return 0;
    }

    // Box 2 faces
    v2 face2 = Vector2Abs(d2) - h2 - (absCT * h1);
    if (face2.x > 0.0f || face2.y > 0.0f) {
      return 0;
    }

    // Find best axis
    Axis axis;
    f32 seperation;
    v2 normal;
    {
      // Box 1 faces
      axis = FACE_A_X;
      seperation = face1.x;
      normal = d1.x > 0.0f ? rot1.col1 : rot1.col1 * (-1.0f);

      const f32 relative_to_l = 0.95f;
      const f32 absolute_to_l = 0.01f;

      if (face1.y > relative_to_l * seperation + absolute_to_l * h1.y) {
        axis = FACE_A_Y;
        seperation = face1.y;
        normal = d1.y > 0.0f ? rot1.col2 : rot1.col2 * (-1.0f);
      }

      // Box 2 faces
      if (face2.x > relative_to_l * seperation + absolute_to_l * h2.x) {
        axis = FACE_B_X;
        seperation = face2.x;
        normal = d2.x > 0.0f ? rot2.col1 : rot2.col1 * (-1.0f);
      }

      if (face2.y > relative_to_l * seperation + absolute_to_l * h2.y) {
        axis = FACE_B_Y;
        seperation = face2.y;
        normal = d2.y > 0.0f ? rot2.col2 : rot2.col2 * (-1.0f);
      }
    }

    // Setup clipping plane data based on the separating axis
    v2 front_normal, side_normal;
    ClipVertex incident_edge[2];
    f32 front, neg_side, pos_side;
    u8 neg_edge, pos_edge;

    // Compute the clipping lines and the line segment to be clipped
    switch (axis) {
      case FACE_A_X: {
        front_normal = normal;
        front = Vector2DotProduct(pos1, front_normal) + h1.x;
        side_normal = rot1.col2;
        f32 side = Vector2DotProduct(pos1, side_normal);
        neg_side = -side + h1.y;
        pos_side = side + h1.y;
        neg_edge = EDGE3;
        pos_edge = EDGE1;
        ComputeIncidentEdge(incident_edge, h2, pos2, rot2, front_normal);
      } break;
      case FACE_A_Y: {
        front_normal = normal;
        front = Vector2DotProduct(pos1, front_normal) + h1.y;
        side_normal = rot1.col1;
        f32 side = Vector2DotProduct(pos1, side_normal);
        neg_side = -side + h1.x;
        pos_side = side + h1.x;
        neg_edge = EDGE2;
        pos_edge = EDGE4;
        ComputeIncidentEdge(incident_edge, h2, pos2, rot2, front_normal);
      } break;
      case FACE_B_X: {
        front_normal = normal * (-1.0f);
        front = Vector2DotProduct(pos2, front_normal) + h2.x;
        side_normal = rot2.col2;
        f32 side = Vector2DotProduct(pos2, side_normal);
        neg_side = -side + h2.y;
        pos_side = side + h2.y;
        neg_edge = EDGE3;
        pos_edge = EDGE1;
        ComputeIncidentEdge(incident_edge, h1, pos1, rot1, front_normal);
      } break;
      case FACE_B_Y: {
        front_normal = normal * (-1.0f);
        front = Vector2DotProduct(pos2, front_normal) + h2.y;
        side_normal = rot2.col1;
        f32 side = Vector2DotProduct(pos2, side_normal);
        neg_side = -side + h2.x;
        pos_side = side + h2.x;
        neg_edge = EDGE2;
        pos_edge = EDGE4;
        ComputeIncidentEdge(incident_edge, h1, pos1, rot1, front_normal);
      } break;
    }

    // clip other face with 5 box planes (1 face plane, 4 edge planes)
    ClipVertex clip_points1[2];
    ClipVertex clip_points2[2];
    int np;

    // Clip to box side 1
    np = ClipSegmentToLine(clip_points1, incident_edge, side_normal * (-1.0f), neg_side, neg_edge);

    if (np < 2) {
      return 0;
    }

    // Clip to negative box side 1
    np = ClipSegmentToLine(clip_points2, clip_points1, side_normal, pos_side, pos_edge);

    if (np < 2) {
      return 0;
    }

    // Now clip_points2 contains the clipping points.
    // Due to roundoff, it is possible that clipping removes all points

    u32 num_contacts = 0;
    for (u32 i = 0; i < 2; i++) {
      f32 seperation = Vector2DotProduct(front_normal, clip_points2[i].v) - front;

      if (seperation <= 0) {
        contacts[num_contacts].seperation = seperation;
        contacts[num_contacts].normal = normal;
        // slide contact point onto reference face (easy to cull)
        contacts[num_contacts].position = clip_points2[i].v - front_normal * seperation;
        contacts[num_contacts].feature = clip_points2[i].fp;

        if (axis == FACE_B_X || axis == FACE_B_Y) {
          Swap(contacts[num_contacts].feature.e.in_edge_1,
               contacts[num_contacts].feature.e.in_edge_2);
          Swap(contacts[num_contacts].feature.e.out_edge_1,
               contacts[num_contacts].feature.e.out_edge_2);
        }

        num_contacts++;
      }
    }

    if (!b1->is_grounded) {
      b1->is_grounded = (normal.y < 0.0f);
    }

    return num_contacts;
  }

  Arbiter Collide(Body *b1, Body *b2) {
    Assert(b1 < b2);

    Arbiter result = {0};

    result.b1 = b1;
    result.b2 = b2;

    result.combined_friction = SquareRoot(b1->friction * b2->friction);
    result.contacts_count = Collide(result.contacts, b1, b2);

    return result;
  }

  void ArbiterMergeContacts(Arbiter *a, Arbiter to_merge) {
    Contact merged_contacts[2];

    for (u32 i = 0; i < to_merge.contacts_count; i++) {
      Contact *c_new = to_merge.contacts + i;
      i32 k = -1;
      for (u32 j = 0; j < a->contacts_count; j++) {
        Contact *c_old = a->contacts + j;
        if (c_new->feature.value == c_old->feature.value) {
          k = j;
          break;
        }
      }

      if (k > -1) {
        Contact *c = merged_contacts + i;
        Contact *c_old = a->contacts + k;
        *c = *c_new;

        // Warm starting
        c->acc_normal_impulse = c_old->acc_normal_impulse;
        c->acc_tangent_impulse = c_old->acc_tangent_impulse;
        c->acc_biased_normal_impulse = c_old->acc_biased_normal_impulse;
      } else {
        merged_contacts[i] = to_merge.contacts[i];
      }
    }

    for (u32 i = 0; i < to_merge.contacts_count; i++) {
      a->contacts[i] = merged_contacts[i];
    }

    a->contacts_count = to_merge.contacts_count;
  }

  void ArbiterPreStep(Arbiter *a, f32 inv_dt) {
    const f32 k_allowed_penetration = 0.01f;
    f32 k_bias_factor = 0.2f;

    for (usize i = 0; i < a->contacts_count; i++) {
      Contact *c = a->contacts + i;

      v2 r1 = c->position - a->b1->position;
      v2 r2 = c->position - a->b2->position;

      // Precompute normal mass, tangent mass, and bias
      f32 rn1 = Vector2DotProduct(r1, c->normal);
      f32 rn2 = Vector2DotProduct(r2, c->normal);
      f32 k_normal = a->b1->inv_mass + a->b2->inv_mass;
      k_normal += a->b1->inv_inertia * (Vector2DotProduct(r1, r1) - rn1 * rn1)
                  + a->b2->inv_inertia * (Vector2DotProduct(r2, r2) - rn2 * rn2);
      c->mass_normal = 1.0f / k_normal;

      v2 tangent = Vector2Cross(c->normal, 1.0f);
      f32 rt1 = Vector2DotProduct(r1, tangent);
      f32 rt2 = Vector2DotProduct(r2, tangent);
      float k_tangent = a->b1->inv_mass + a->b2->inv_mass;
      k_tangent += a->b1->inv_inertia * (Vector2DotProduct(r1, r1) - rt1 * rt1)
                   + a->b2->inv_inertia * (Vector2DotProduct(r2, r2) - rt2 * rt2);
      c->mass_tangent = 1.0f / k_tangent;

      c->bias = -k_bias_factor * inv_dt * Min(0.0f, c->seperation + k_allowed_penetration);

      // accumulate impulses
      {
        v2 P = c->normal * c->acc_normal_impulse + tangent * c->acc_tangent_impulse;

        a->b1->velocity -= P * a->b1->inv_mass;
        if (!a->b1->lock_rotation) {
          a->b1->angular_velocity -= a->b1->inv_inertia * Vector2Cross(r1, P);
        }

        a->b2->velocity += P * a->b2->inv_mass;
        if (!a->b1->lock_rotation) {
          a->b2->angular_velocity += a->b2->inv_inertia * Vector2Cross(r2, P);
        }
      }
    }
  }

  void ArbiterApplyImpulse(Arbiter *a) {
    Body *b1 = a->b1;
    Body *b2 = a->b2;

    for (usize i = 0; i < a->contacts_count; i++) {
      Contact *c = a->contacts + i;
      c->r1 = c->position - b1->position;
      c->r2 = c->position - b2->position;

      // Relative velocity at contact
      v2 dv = b2->velocity + Vector2Cross(b2->angular_velocity, c->r2) - b1->velocity
              - Vector2Cross(b1->angular_velocity, c->r1);

      // Compute normal impulse
      f32 vn = Vector2DotProduct(dv, c->normal);

      f32 dPn = c->mass_normal * (-vn + c->bias);

      // Clamp the accumulated impulse
      f32 Pn0 = c->acc_normal_impulse;
      c->acc_normal_impulse = Max(Pn0 + dPn, 0.0f);
      dPn = c->acc_normal_impulse - Pn0;

      // Apply contact impulse
      v2 Pn = c->normal * dPn;

      b1->velocity -= Pn * b1->inv_mass;
      if (!b1->lock_rotation) {
        b1->angular_velocity -= b1->inv_inertia * Vector2Cross(c->r1, Pn);
      }

      b2->velocity += Pn * b2->inv_mass;
      if (!b2->lock_rotation) {
        b2->angular_velocity += b2->inv_inertia * Vector2Cross(c->r2, Pn);
      }

      // Relative velocity at contact
      dv = b2->velocity + Vector2Cross(b2->angular_velocity, c->r2) - b1->velocity
           - Vector2Cross(b1->angular_velocity, c->r1);

      v2 tangent = Vector2Cross(c->normal, 1.0f);
      f32 vt = Vector2DotProduct(dv, tangent);
      f32 dPt = vt * c->mass_tangent * (-1.0f);

      // accumulate impulses
      {
        // Compute frictional impulse
        float maxPt = a->combined_friction * c->acc_normal_impulse;
        // Clamp friction
        f32 old_tangent_impulse = c->acc_tangent_impulse;
        c->acc_tangent_impulse = Clamp(old_tangent_impulse + dPt, -maxPt, maxPt);
        dPt = c->acc_tangent_impulse - old_tangent_impulse;
      }

      // Apply contact impulse
      v2 Pt = tangent * dPt;

      b1->velocity -= Pt * b1->inv_mass;
      if (!b1->lock_rotation) {
        b1->angular_velocity -= b1->inv_inertia * Vector2Cross(c->r1, Pt);
      }

      b2->velocity += Pt * b2->inv_mass;
      if (!b2->lock_rotation) {
        b2->angular_velocity += b2->inv_inertia * Vector2Cross(c->r2, Pt);
      }
    }
  }

  void BroadPhase(World *world) {
    // TODO(anton): slow O^2 broad collision detection, use some spatial lookup function...
    for (u32 i = 0; i < world->bodies_count; i++) {
      Body *bi = world->bodies + i;
      for (u32 j = i + 1; j < world->bodies_count; j++) {
        Body *bj = world->bodies + j;

        if (bi->inv_mass == 0.0f && bj->inv_mass == 0.0f) {
          continue;
        }

        {
          Body *b1;
          Body *b2;
          if (bi < bj) {
            b1 = bi;
            b2 = bj;
          } else {
            b1 = bj;
            b2 = bi;
          }

          Arbiter arbiter = Collide(b1, b2);
          ArbiterKey arbiter_key = {b1, b2};
          u64 hash_table_key = murmur64((void *)&arbiter_key, sizeof(ArbiterKey));
          Arbiter *iter = HashTableGet(&world->arbiter_table, hash_table_key);

          if (arbiter.contacts_count > 0) {
            if (iter == nullptr) {
              HashTableSet(&world->arbiter_table, hash_table_key, arbiter);
            } else {
              ArbiterMergeContacts(iter, arbiter);
              Log("MERGE \n");
            }
          }
        }
      }
    }
  }

  void Step(World *world) {
    f32 dt = GetFrameTime();
    f32 inv_dt = dt > 0.0f ? 1.0f / dt : 0.0f;

    //
    HashTableClear(&world->arbiter_table);
    for (usize i = 0; i < world->bodies_count; i++) {
      Body *b = world->bodies + i;
      b->is_grounded = false;
    }

    //
    BroadPhase(world);

    // Integrate forces
    for (usize i = 0; i < world->bodies_count; i++) {
      Body *b = world->bodies + i;

      if (b->inv_mass == 0.0f) {
        continue;
      }

      b->velocity += (world->gravity + b->force * b->inv_mass) * dt;
      b->angular_velocity += (b->torque * b->inv_inertia) * dt;
    }

    // Perform pre-steps
    for (usize i = 0; i < world->arbiter_table.entries_count; i++) {
      Arbiter *a = &world->arbiter_table.entries[i].value;
      ArbiterPreStep(a, inv_dt);
    }

    // Perform iterations
    for (usize i = 0; i < world->iterations; i++) {
      for (usize ai = 0; ai < world->arbiter_table.entries_count; ai++) {
        Arbiter *a = &world->arbiter_table.entries[ai].value;
        ArbiterApplyImpulse(a);
      }
    }

    // Integrate velocities
    for (usize i = 0; i < world->bodies_count; i++) {
      Body *b = world->bodies + i;

      b->position += b->velocity * dt;
      b->rotation += b->angular_velocity * dt;

      b->torque = 0.0f;
      b->force = Vector2Zero();
    }
  }

  void Draw(World *world) {
    for (u32 i = 0; i < world->bodies_count; i++) {
      Body *b = world->bodies + i;

      Matrix2x2 rot = Matrix2x2FromAngle(b->rotation);
      v2 h = b->width * 0.5f;

      v2 p1 = rot * (v2){-h.x, -h.y};
      v2 p2 = rot * (v2){-h.x, +h.y};
      v2 p3 = rot * (v2){+h.x, +h.y};
      v2 p4 = rot * (v2){+h.x, -h.y};

      p1 = p1 + b->position;
      p2 = p2 + b->position;
      p3 = p3 + b->position;
      p4 = p4 + b->position;

      Color c = LIME;
      PushLine(&game->renderer, p1, p2, c);
      PushLine(&game->renderer, p2, p3, c);
      PushLine(&game->renderer, p3, p4, c);
      PushLine(&game->renderer, p4, p1, c);
      PushLine(&game->renderer, b->position, b->position + rot * v2{0.10f, 0.0f}, c);
    }

    for (usize i = 0; i < world->arbiter_table.entries_count; i++) {
      for (usize j = 0; j < world->arbiter_table.entries[i].value.contacts_count; j++) {
        PushCircle(&game->renderer, world->arbiter_table.entries[i].value.contacts[j].position,
                   0.03f, MAGENTA);
      }
    }
  }

  void PrintArbiterTable(World *world) {
    Log("Arbiters (%ld) {\n", world->arbiter_table.entries_count);
    // for (u32 i = 0; i < MAX_BODY_COUNT; i++) {
    //   Log("\t %d:\t %d{\n", world->arbiter_table.hashes[i], world->arbiter_table.);
    // }
    Log("}\n");
  }
};  // namespace physics
