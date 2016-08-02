/*
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2011-2014, Willow Garage, Inc.
 *  Copyright (c) 2014-2016, Open Source Robotics Foundation
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of Open Source Robotics Foundation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

/** \author Jia Pan */

#ifndef FCL_RSS_H
#define FCL_RSS_H

#include "fcl/math/constants.h"
#include "fcl/math/geometry.h"
#include "fcl/BV/bv_utility.h"

namespace fcl
{

/// @brief A class for rectangle sphere-swept bounding volume
class RSS
{
public:
  /// @brief Orientation of RSS. axis[i] is the ith column of the orientation matrix for the RSS; it is also the i-th principle direction of the RSS.
  /// We assume that axis[0] corresponds to the axis with the longest length, axis[1] corresponds to the shorter one and axis[2] corresponds to the shortest one.
  Matrix3d axis;

  /// @brief Origin of the rectangle in RSS
  Vector3d Tr;

  /// @brief Side lengths of rectangle
  FCL_REAL l[2];

  /// @brief Radius of sphere summed with rectangle to form RSS
  FCL_REAL r;

  /// Constructor
  RSS();

  /// @brief Check collision between two RSS
  bool overlap(const RSS& other) const;

  /// @brief Check collision between two RSS and return the overlap part.
  /// For RSS, we return nothing, as the overlap part of two RSSs usually is not a RSS.
  bool overlap(const RSS& other, RSS& overlap_part) const;

  /// @brief Check whether the RSS contains a point
  inline bool contain(const Vector3d& p) const;

  /// @brief A simple way to merge the RSS and a point, not compact.
  /// @todo This function may have some bug.
  RSS& operator += (const Vector3d& p);

  /// @brief Merge the RSS and another RSS
  RSS& operator += (const RSS& other);

  /// @brief Return the merged RSS of current RSS and the other one
  RSS operator + (const RSS& other) const;

  /// @brief Width of the RSS
  FCL_REAL width() const;

  /// @brief Height of the RSS
  FCL_REAL height() const;

  /// @brief Depth of the RSS
  FCL_REAL depth() const;

  /// @brief Volume of the RSS
  FCL_REAL volume() const;

  /// @brief Size of the RSS (used in BV_Splitter to order two RSSs)
  FCL_REAL size() const;

  /// @brief The RSS center
  const Vector3d& center() const;

  /// @brief the distance between two RSS; P and Q, if not NULL, return the nearest points
  FCL_REAL distance(const RSS& other, Vector3d* P = NULL, Vector3d* Q = NULL) const;

};

/// @brief Clip value between a and b
void clipToRange(FCL_REAL& val, FCL_REAL a, FCL_REAL b);

/// @brief Finds the parameters t & u corresponding to the two closest points on a pair of line segments.
/// The first segment is defined as Pa + A*t, 0 <= t <= a,  where "Pa" is one endpoint of the segment, "A" is a unit vector
/// pointing to the other endpoint, and t is a scalar that produces all the points between the two endpoints. Since "A" is a unit
/// vector, "a" is the segment's length.
/// The second segment is defined as Pb + B*u, 0 <= u <= b.
/// Many of the terms needed by the algorithm are already computed for other purposes,so we just pass these terms into the function
/// instead of complete specifications of each segment. "T" in the dot products is the vector betweeen Pa and Pb.
/// Reference: "On fast computation of distance between line segments." Vladimir J. Lumelsky, in Information Processing Letters, no. 21, pages 55-61, 1985.
void segCoords(FCL_REAL& t, FCL_REAL& u, FCL_REAL a, FCL_REAL b, FCL_REAL A_dot_B, FCL_REAL A_dot_T, FCL_REAL B_dot_T);

/// @brief Returns whether the nearest point on rectangle edge
/// Pb + B*u, 0 <= u <= b, to the rectangle edge,
/// Pa + A*t, 0 <= t <= a, is within the half space
/// determined by the point Pa and the direction Anorm.
/// A,B, and Anorm are unit vectors. T is the vector between Pa and Pb.
bool inVoronoi(FCL_REAL a, FCL_REAL b, FCL_REAL Anorm_dot_B, FCL_REAL Anorm_dot_T, FCL_REAL A_dot_B, FCL_REAL A_dot_T, FCL_REAL B_dot_T);

/// @brief Distance between two oriented rectangles; P and Q (optional return values) are the closest points in the rectangles, both are in the local frame of the first rectangle.
FCL_REAL rectDistance(const Matrix3d& Rab, Vector3d const& Tab, const FCL_REAL a[2], const FCL_REAL b[2], Vector3d* P = NULL, Vector3d* Q = NULL);

/// @brief distance between two RSS bounding volumes
/// P and Q (optional return values) are the closest points in the rectangles, not the RSS. But the direction P - Q is the correct direction for cloest points
/// Notice that P and Q are both in the local frame of the first RSS (not global frame and not even the local frame of object 1)
FCL_REAL distance(const Matrix3d& R0, const Vector3d& T0, const RSS& b1, const RSS& b2, Vector3d* P = NULL, Vector3d* Q = NULL);

/// @brief Check collision between two RSSs, b1 is in configuration (R0, T0) and b2 is in identity.
bool overlap(const Matrix3d& R0, const Vector3d& T0, const RSS& b1, const RSS& b2);

/// @brief Translate the RSS bv
RSS translate(const RSS& bv, const Vector3d& t);

//============================================================================//
//                                                                            //
//                              Implementations                               //
//                                                                            //
//============================================================================//

//==============================================================================
RSS::RSS()
  : axis(Matrix3d::Identity()), Tr(Vector3d::Zero())
{
  // Do nothing
}

bool RSS::overlap(const RSS& other) const
{
  /// compute what transform [R,T] that takes us from cs1 to cs2.
  /// [R,T] = [R1,T1]'[R2,T2] = [R1',-R1'T][R2,T2] = [R1'R2, R1'(T2-T1)]
  /// First compute the rotation part, then translation part

  /// First compute T2 - T1
  Vector3d t = other.Tr - Tr;

  /// Then compute R1'(T2 - T1)
  Vector3d T = t.transpose() * axis;

  /// Now compute R1'R2
  Matrix3d R = axis.transpose() * other.axis;

  FCL_REAL dist = rectDistance(R, T, l, other.l);
  return (dist <= (r + other.r));
}

bool RSS::overlap(const RSS& other, RSS& overlap_part) const
{
  return overlap(other);
}

bool RSS::contain(const Vector3d& p) const
{
  Vector3d local_p = p - Tr;
  Vector3d proj = local_p.transpose() * axis;
  FCL_REAL abs_proj2 = fabs(proj[2]);

  /// projection is within the rectangle
  if((proj[0] < l[0]) && (proj[0] > 0) && (proj[1] < l[1]) && (proj[1] > 0))
  {
    return (abs_proj2 < r);
  }
  else if((proj[0] < l[0]) && (proj[0] > 0) && ((proj[1] < 0) || (proj[1] > l[1])))
  {
    FCL_REAL y = (proj[1] > 0) ? l[1] : 0;
    Vector3d v(proj[0], y, 0);
    return ((proj - v).squaredNorm() < r * r);
  }
  else if((proj[1] < l[1]) && (proj[1] > 0) && ((proj[0] < 0) || (proj[0] > l[0])))
  {
    FCL_REAL x = (proj[0] > 0) ? l[0] : 0;
    Vector3d v(x, proj[1], 0);
    return ((proj - v).squaredNorm() < r * r);
  }
  else
  {
    FCL_REAL x = (proj[0] > 0) ? l[0] : 0;
    FCL_REAL y = (proj[1] > 0) ? l[1] : 0;
    Vector3d v(x, y, 0);
    return ((proj - v).squaredNorm() < r * r);
  }
}

RSS&RSS::operator +=(const Vector3d& p)

{
  Vector3d local_p = p - Tr;
  Vector3d proj = local_p.transpose() * axis;
  FCL_REAL abs_proj2 = fabs(proj[2]);

  // projection is within the rectangle
  if((proj[0] < l[0]) && (proj[0] > 0) && (proj[1] < l[1]) && (proj[1] > 0))
  {
    if(abs_proj2 < r)
      ; // do nothing
    else
    {
      r = 0.5 * (r + abs_proj2); // enlarge the r
      // change RSS origin position
      if(proj[2] > 0)
        Tr[2] += 0.5 * (abs_proj2 - r);
      else
        Tr[2] -= 0.5 * (abs_proj2 - r);
    }
  }
  else if((proj[0] < l[0]) && (proj[0] > 0) && ((proj[1] < 0) || (proj[1] > l[1])))
  {
    FCL_REAL y = (proj[1] > 0) ? l[1] : 0;
    Vector3d v(proj[0], y, 0);
    FCL_REAL new_r_sqr = (proj - v).squaredNorm();
    if(new_r_sqr < r * r)
      ; // do nothing
    else
    {
      if(abs_proj2 < r)
      {
        FCL_REAL delta_y = - std::sqrt(r * r - proj[2] * proj[2]) + fabs(proj[1] - y);
        l[1] += delta_y;
        if(proj[1] < 0)
          Tr[1] -= delta_y;
      }
      else
      {
        FCL_REAL delta_y = fabs(proj[1] - y);
        l[1] += delta_y;
        if(proj[1] < 0)
          Tr[1] -= delta_y;

        if(proj[2] > 0)
          Tr[2] += 0.5 * (abs_proj2 - r);
        else
          Tr[2] -= 0.5 * (abs_proj2 - r);
      }
    }
  }
  else if((proj[1] < l[1]) && (proj[1] > 0) && ((proj[0] < 0) || (proj[0] > l[0])))
  {
    FCL_REAL x = (proj[0] > 0) ? l[0] : 0;
    Vector3d v(x, proj[1], 0);
    FCL_REAL new_r_sqr = (proj - v).squaredNorm();
    if(new_r_sqr < r * r)
      ; // do nothing
    else
    {
      if(abs_proj2 < r)
      {
        FCL_REAL delta_x = - std::sqrt(r * r - proj[2] * proj[2]) + fabs(proj[0] - x);
        l[0] += delta_x;
        if(proj[0] < 0)
          Tr[0] -= delta_x;
      }
      else
      {
        FCL_REAL delta_x = fabs(proj[0] - x);
        l[0] += delta_x;
        if(proj[0] < 0)
          Tr[0] -= delta_x;

        if(proj[2] > 0)
          Tr[2] += 0.5 * (abs_proj2 - r);
        else
          Tr[2] -= 0.5 * (abs_proj2 - r);
      }
    }
  }
  else
  {
    FCL_REAL x = (proj[0] > 0) ? l[0] : 0;
    FCL_REAL y = (proj[1] > 0) ? l[1] : 0;
    Vector3d v(x, y, 0);
    FCL_REAL new_r_sqr = (proj - v).squaredNorm();
    if(new_r_sqr < r * r)
      ; // do nothing
    else
    {
      if(abs_proj2 < r)
      {
        FCL_REAL diag = std::sqrt(new_r_sqr - proj[2] * proj[2]);
        FCL_REAL delta_diag = - std::sqrt(r * r - proj[2] * proj[2]) + diag;

        FCL_REAL delta_x = delta_diag / diag * fabs(proj[0] - x);
        FCL_REAL delta_y = delta_diag / diag * fabs(proj[1] - y);
        l[0] += delta_x;
        l[1] += delta_y;

        if(proj[0] < 0 && proj[1] < 0)
        {
          Tr[0] -= delta_x;
          Tr[1] -= delta_y;
        }
      }
      else
      {
        FCL_REAL delta_x = fabs(proj[0] - x);
        FCL_REAL delta_y = fabs(proj[1] - y);

        l[0] += delta_x;
        l[1] += delta_y;

        if(proj[0] < 0 && proj[1] < 0)
        {
          Tr[0] -= delta_x;
          Tr[1] -= delta_y;
        }

        if(proj[2] > 0)
          Tr[2] += 0.5 * (abs_proj2 - r);
        else
          Tr[2] -= 0.5 * (abs_proj2 - r);
      }
    }
  }

  return *this;
}

RSS&RSS::operator +=(const RSS& other)
{
  *this = *this + other;
  return *this;
}

RSS RSS::operator +(const RSS& other) const
{
  RSS bv;

  Vector3d v[16];

  Vector3d d0_pos = other.axis.col(0) * (other.l[0] + other.r);
  Vector3d d1_pos = other.axis.col(1) * (other.l[1] + other.r);

  Vector3d d0_neg = other.axis.col(0) * (-other.r);
  Vector3d d1_neg = other.axis.col(1) * (-other.r);

  Vector3d d2_pos = other.axis.col(2) * other.r;
  Vector3d d2_neg = other.axis.col(2) * (-other.r);

  v[0] = other.Tr + d0_pos + d1_pos + d2_pos;
  v[1] = other.Tr + d0_pos + d1_pos + d2_neg;
  v[2] = other.Tr + d0_pos + d1_neg + d2_pos;
  v[3] = other.Tr + d0_pos + d1_neg + d2_neg;
  v[4] = other.Tr + d0_neg + d1_pos + d2_pos;
  v[5] = other.Tr + d0_neg + d1_pos + d2_neg;
  v[6] = other.Tr + d0_neg + d1_neg + d2_pos;
  v[7] = other.Tr + d0_neg + d1_neg + d2_neg;

  d0_pos = axis.col(0) * (l[0] + r);
  d1_pos = axis.col(1) * (l[1] + r);
  d0_neg = axis.col(0) * (-r);
  d1_neg = axis.col(1) * (-r);
  d2_pos = axis.col(2) * r;
  d2_neg = axis.col(2) * (-r);

  v[8] = Tr + d0_pos + d1_pos + d2_pos;
  v[9] = Tr + d0_pos + d1_pos + d2_neg;
  v[10] = Tr + d0_pos + d1_neg + d2_pos;
  v[11] = Tr + d0_pos + d1_neg + d2_neg;
  v[12] = Tr + d0_neg + d1_pos + d2_pos;
  v[13] = Tr + d0_neg + d1_pos + d2_neg;
  v[14] = Tr + d0_neg + d1_neg + d2_pos;
  v[15] = Tr + d0_neg + d1_neg + d2_neg;


  Matrix3d M; // row first matrix
  Matrix3d E; // row first eigen-vectors
  Vector3d s(0, 0, 0);

  getCovariance<double>(v, NULL, NULL, NULL, 16, M);
  eigen(M, s, E);

  int min, mid, max;
  if(s[0] > s[1]) { max = 0; min = 1; }
  else { min = 0; max = 1; }
  if(s[2] < s[min]) { mid = min; min = 2; }
  else if(s[2] > s[max]) { mid = max; max = 2; }
  else { mid = 2; }

  // column first matrix, as the axis in RSS
  bv.axis.col(0) = E.col(max);
  bv.axis.col(1) = E.col(mid);
  bv.axis.col(2) = axis.col(0).cross(axis.col(1));

  // set rss origin, rectangle size and radius
  getRadiusAndOriginAndRectangleSize<double>(v, NULL, NULL, NULL, 16, bv.axis, bv.Tr, bv.l, bv.r);

  return bv;
}

FCL_REAL RSS::width() const
{
  return l[0] + 2 * r;
}

FCL_REAL RSS::height() const
{
  return l[1] + 2 * r;
}

FCL_REAL RSS::depth() const
{
  return 2 * r;
}

FCL_REAL RSS::volume() const
{
  return (l[0] * l[1] * 2 * r + 4 * constants::pi * r * r * r);
}

FCL_REAL RSS::size() const
{
  return (std::sqrt(l[0] * l[0] + l[1] * l[1]) + 2 * r);
}

const Vector3d&RSS::center() const
{
  return Tr;
}

FCL_REAL RSS::distance(const RSS& other, Vector3d* P, Vector3d* Q) const
{
  // compute what transform [R,T] that takes us from cs1 to cs2.
  // [R,T] = [R1,T1]'[R2,T2] = [R1',-R1'T][R2,T2] = [R1'R2, R1'(T2-T1)]
  // First compute the rotation part, then translation part
  Vector3d t = other.Tr - Tr; // T2 - T1
  Vector3d T = t.transpose() * axis; // R1'(T2-T1)
  Matrix3d R = axis.transpose() * other.axis;

  FCL_REAL dist = rectDistance(R, T, l, other.l, P, Q);
  dist -= (r + other.r);
  return (dist < (FCL_REAL)0.0) ? (FCL_REAL)0.0 : dist;
}

//==============================================================================
void clipToRange(FCL_REAL& val, FCL_REAL a, FCL_REAL b)
{
  if(val < a) val = a;
  else if(val > b) val = b;
}

//==============================================================================
void segCoords(FCL_REAL& t, FCL_REAL& u, FCL_REAL a, FCL_REAL b, FCL_REAL A_dot_B, FCL_REAL A_dot_T, FCL_REAL B_dot_T)
{
  FCL_REAL denom = 1 - A_dot_B * A_dot_B;

  if(denom == 0) t = 0;
  else
  {
    t = (A_dot_T - B_dot_T * A_dot_B) / denom;
    clipToRange(t, 0, a);
  }

  u = t * A_dot_B - B_dot_T;
  if(u < 0)
  {
    u = 0;
    t = A_dot_T;
    clipToRange(t, 0, a);
  }
  else if(u > b)
  {
    u = b;
    t = u * A_dot_B + A_dot_T;
    clipToRange(t, 0, a);
  }
}

//==============================================================================
bool inVoronoi(FCL_REAL a, FCL_REAL b, FCL_REAL Anorm_dot_B, FCL_REAL Anorm_dot_T, FCL_REAL A_dot_B, FCL_REAL A_dot_T, FCL_REAL B_dot_T)
{
  if(fabs(Anorm_dot_B) < 1e-7) return false;

  FCL_REAL t, u, v;

  u = -Anorm_dot_T / Anorm_dot_B;
  clipToRange(u, 0, b);

  t = u * A_dot_B + A_dot_T;
  clipToRange(t, 0, a);

  v = t * A_dot_B - B_dot_T;

  if(Anorm_dot_B > 0)
  {
    if(v > (u + 1e-7)) return true;
  }
  else
  {
    if(v < (u - 1e-7)) return true;
  }
  return false;
}


//==============================================================================
FCL_REAL rectDistance(const Matrix3d& Rab, Vector3d const& Tab, const FCL_REAL a[2], const FCL_REAL b[2], Vector3d* P, Vector3d* Q)
{
  FCL_REAL A0_dot_B0, A0_dot_B1, A1_dot_B0, A1_dot_B1;

  A0_dot_B0 = Rab(0, 0);
  A0_dot_B1 = Rab(0, 1);
  A1_dot_B0 = Rab(1, 0);
  A1_dot_B1 = Rab(1, 1);

  FCL_REAL aA0_dot_B0, aA0_dot_B1, aA1_dot_B0, aA1_dot_B1;
  FCL_REAL bA0_dot_B0, bA0_dot_B1, bA1_dot_B0, bA1_dot_B1;

  aA0_dot_B0 = a[0] * A0_dot_B0;
  aA0_dot_B1 = a[0] * A0_dot_B1;
  aA1_dot_B0 = a[1] * A1_dot_B0;
  aA1_dot_B1 = a[1] * A1_dot_B1;
  bA0_dot_B0 = b[0] * A0_dot_B0;
  bA1_dot_B0 = b[0] * A1_dot_B0;
  bA0_dot_B1 = b[1] * A0_dot_B1;
  bA1_dot_B1 = b[1] * A1_dot_B1;

  Vector3d Tba = Rab.transpose() * Tab;

  Vector3d S;
  FCL_REAL t, u;

  // determine if any edge pair contains the closest points

  FCL_REAL ALL_x, ALU_x, AUL_x, AUU_x;
  FCL_REAL BLL_x, BLU_x, BUL_x, BUU_x;
  FCL_REAL LA1_lx, LA1_ux, UA1_lx, UA1_ux, LB1_lx, LB1_ux, UB1_lx, UB1_ux;

  ALL_x = -Tba[0];
  ALU_x = ALL_x + aA1_dot_B0;
  AUL_x = ALL_x + aA0_dot_B0;
  AUU_x = ALU_x + aA0_dot_B0;

  if(ALL_x < ALU_x)
  {
    LA1_lx = ALL_x;
    LA1_ux = ALU_x;
    UA1_lx = AUL_x;
    UA1_ux = AUU_x;
  }
  else
  {
    LA1_lx = ALU_x;
    LA1_ux = ALL_x;
    UA1_lx = AUU_x;
    UA1_ux = AUL_x;
  }

  BLL_x = Tab[0];
  BLU_x = BLL_x + bA0_dot_B1;
  BUL_x = BLL_x + bA0_dot_B0;
  BUU_x = BLU_x + bA0_dot_B0;

  if(BLL_x < BLU_x)
  {
    LB1_lx = BLL_x;
    LB1_ux = BLU_x;
    UB1_lx = BUL_x;
    UB1_ux = BUU_x;
  }
  else
  {
    LB1_lx = BLU_x;
    LB1_ux = BLL_x;
    UB1_lx = BUU_x;
    UB1_ux = BUL_x;
  }

  // UA1, UB1

  if((UA1_ux > b[0]) && (UB1_ux > a[0]))
  {
    if(((UA1_lx > b[0]) ||
        inVoronoi(b[1], a[1], A1_dot_B0, aA0_dot_B0 - b[0] - Tba[0],
                  A1_dot_B1, aA0_dot_B1 - Tba[1],
                  -Tab[1] - bA1_dot_B0))
       &&
       ((UB1_lx > a[0]) ||
        inVoronoi(a[1], b[1], A0_dot_B1, Tab[0] + bA0_dot_B0 - a[0],
                  A1_dot_B1, Tab[1] + bA1_dot_B0, Tba[1] - aA0_dot_B1)))
    {
      segCoords(t, u, a[1], b[1], A1_dot_B1, Tab[1] + bA1_dot_B0,
                Tba[1] - aA0_dot_B1);

      S[0] = Tab[0] + Rab(0, 0) * b[0] + Rab(0, 1) * u - a[0] ;
      S[1] = Tab[1] + Rab(1, 0) * b[0] + Rab(1, 1) * u - t;
      S[2] = Tab[2] + Rab(2, 0) * b[0] + Rab(2, 1) * u;

      if(P && Q)
      {
        *P << a[0], t, 0;
        *Q = S + (*P);
      }

      return S.norm();
    }
  }


  // UA1, LB1

  if((UA1_lx < 0) && (LB1_ux > a[0]))
  {
    if(((UA1_ux < 0) ||
        inVoronoi(b[1], a[1], -A1_dot_B0, Tba[0] - aA0_dot_B0,
                  A1_dot_B1, aA0_dot_B1 - Tba[1], -Tab[1]))
       &&
       ((LB1_lx > a[0]) ||
        inVoronoi(a[1], b[1], A0_dot_B1, Tab[0] - a[0],
                  A1_dot_B1, Tab[1], Tba[1] - aA0_dot_B1)))
    {
      segCoords(t, u, a[1], b[1], A1_dot_B1, Tab[1], Tba[1] - aA0_dot_B1);

      S[0] = Tab[0] + Rab(0, 1) * u - a[0];
      S[1] = Tab[1] + Rab(1, 1) * u - t;
      S[2] = Tab[2] + Rab(2, 1) * u;

      if(P && Q)
      {
        *P << a[0], t, 0;
        *Q = S + (*P);
      }

      return S.norm();
    }
  }

  // LA1, UB1

  if((LA1_ux > b[0]) && (UB1_lx < 0))
  {
    if(((LA1_lx > b[0]) ||
        inVoronoi(b[1], a[1], A1_dot_B0, -Tba[0] - b[0],
                  A1_dot_B1, -Tba[1], -Tab[1] - bA1_dot_B0))
       &&
       ((UB1_ux < 0) ||
        inVoronoi(a[1], b[1], -A0_dot_B1, -Tab[0] - bA0_dot_B0,
                  A1_dot_B1, Tab[1] + bA1_dot_B0, Tba[1])))
    {
      segCoords(t, u, a[1], b[1], A1_dot_B1, Tab[1] + bA1_dot_B0, Tba[1]);

      S[0] = Tab[0] + Rab(0, 0) * b[0] + Rab(0, 1) * u;
      S[1] = Tab[1] + Rab(1, 0) * b[0] + Rab(1, 1) * u - t;
      S[2] = Tab[2] + Rab(2, 0) * b[0] + Rab(2, 1) * u;

      if(P && Q)
      {
        *P << 0, t, 0;
        *Q = S + (*P);
      }

      return S.norm();
    }
  }

  // LA1, LB1

  if((LA1_lx < 0) && (LB1_lx < 0))
  {
    if (((LA1_ux < 0) ||
         inVoronoi(b[1], a[1], -A1_dot_B0, Tba[0], A1_dot_B1,
                   -Tba[1], -Tab[1]))
        &&
        ((LB1_ux < 0) ||
         inVoronoi(a[1], b[1], -A0_dot_B1, -Tab[0], A1_dot_B1,
                   Tab[1], Tba[1])))
    {
      segCoords(t, u, a[1], b[1], A1_dot_B1, Tab[1], Tba[1]);

      S[0] = Tab[0] + Rab(0, 1) * u;
      S[1] = Tab[1] + Rab(1, 1) * u - t;
      S[2] = Tab[2] + Rab(2, 1) * u;

      if(P && Q)
      {
        *P << 0, t, 0;
        *Q = S + (*P);
      }

      return S.norm();
    }
  }

  FCL_REAL ALL_y, ALU_y, AUL_y, AUU_y;

  ALL_y = -Tba[1];
  ALU_y = ALL_y + aA1_dot_B1;
  AUL_y = ALL_y + aA0_dot_B1;
  AUU_y = ALU_y + aA0_dot_B1;

  FCL_REAL LA1_ly, LA1_uy, UA1_ly, UA1_uy, LB0_lx, LB0_ux, UB0_lx, UB0_ux;

  if(ALL_y < ALU_y)
  {
    LA1_ly = ALL_y;
    LA1_uy = ALU_y;
    UA1_ly = AUL_y;
    UA1_uy = AUU_y;
  }
  else
  {
    LA1_ly = ALU_y;
    LA1_uy = ALL_y;
    UA1_ly = AUU_y;
    UA1_uy = AUL_y;
  }

  if(BLL_x < BUL_x)
  {
    LB0_lx = BLL_x;
    LB0_ux = BUL_x;
    UB0_lx = BLU_x;
    UB0_ux = BUU_x;
  }
  else
  {
    LB0_lx = BUL_x;
    LB0_ux = BLL_x;
    UB0_lx = BUU_x;
    UB0_ux = BLU_x;
  }

  // UA1, UB0

  if((UA1_uy > b[1]) && (UB0_ux > a[0]))
  {
    if(((UA1_ly > b[1]) ||
        inVoronoi(b[0], a[1], A1_dot_B1, aA0_dot_B1 - Tba[1] - b[1],
                  A1_dot_B0, aA0_dot_B0 - Tba[0], -Tab[1] - bA1_dot_B1))
       &&
       ((UB0_lx > a[0]) ||
        inVoronoi(a[1], b[0], A0_dot_B0, Tab[0] - a[0] + bA0_dot_B1,
                  A1_dot_B0, Tab[1] + bA1_dot_B1, Tba[0] - aA0_dot_B0)))
    {
      segCoords(t, u, a[1], b[0], A1_dot_B0, Tab[1] + bA1_dot_B1,
                Tba[0] - aA0_dot_B0);

      S[0] = Tab[0] + Rab(0, 1) * b[1] + Rab(0, 0) * u - a[0] ;
      S[1] = Tab[1] + Rab(1, 1) * b[1] + Rab(1, 0) * u - t;
      S[2] = Tab[2] + Rab(2, 1) * b[1] + Rab(2, 0) * u;

      if(P && Q)
      {
        *P << a[0], t, 0;
        *Q = S + (*P);
      }

      return S.norm();
    }
  }

  // UA1, LB0

  if((UA1_ly < 0) && (LB0_ux > a[0]))
  {
    if(((UA1_uy < 0) ||
        inVoronoi(b[0], a[1], -A1_dot_B1, Tba[1] - aA0_dot_B1, A1_dot_B0,
                  aA0_dot_B0 - Tba[0], -Tab[1]))
       &&
       ((LB0_lx > a[0]) ||
        inVoronoi(a[1], b[0], A0_dot_B0, Tab[0] - a[0],
                  A1_dot_B0, Tab[1], Tba[0] - aA0_dot_B0)))
    {
      segCoords(t, u, a[1], b[0], A1_dot_B0, Tab[1], Tba[0] - aA0_dot_B0);

      S[0] = Tab[0] + Rab(0, 0) * u - a[0];
      S[1] = Tab[1] + Rab(1, 0) * u - t;
      S[2] = Tab[2] + Rab(2, 0) * u;

      if(P && Q)
      {
        *P << a[0], t, 0;
        *Q = S + (*P);
      }

      return S.norm();
    }
  }

  // LA1, UB0

  if((LA1_uy > b[1]) && (UB0_lx < 0))
  {
    if(((LA1_ly > b[1]) ||
        inVoronoi(b[0], a[1], A1_dot_B1, -Tba[1] - b[1],
                  A1_dot_B0, -Tba[0], -Tab[1] - bA1_dot_B1))
       &&

       ((UB0_ux < 0) ||
        inVoronoi(a[1], b[0], -A0_dot_B0, -Tab[0] - bA0_dot_B1, A1_dot_B0,
                  Tab[1] + bA1_dot_B1, Tba[0])))
    {
      segCoords(t, u, a[1], b[0], A1_dot_B0, Tab[1] + bA1_dot_B1, Tba[0]);

      S[0] = Tab[0] + Rab(0, 1) * b[1] + Rab(0, 0) * u;
      S[1] = Tab[1] + Rab(1, 1) * b[1] + Rab(1, 0) * u - t;
      S[2] = Tab[2] + Rab(2, 1) * b[1] + Rab(2, 0) * u;

      if(P && Q)
      {
        *P << 0, t, 0;
        *Q = S + (*P);
      }


      return S.norm();
    }
  }

  // LA1, LB0

  if((LA1_ly < 0) && (LB0_lx < 0))
  {
    if(((LA1_uy < 0) ||
        inVoronoi(b[0], a[1], -A1_dot_B1, Tba[1], A1_dot_B0,
                  -Tba[0], -Tab[1]))
       &&
       ((LB0_ux < 0) ||
        inVoronoi(a[1], b[0], -A0_dot_B0, -Tab[0], A1_dot_B0,
                  Tab[1], Tba[0])))
    {
      segCoords(t, u, a[1], b[0], A1_dot_B0, Tab[1], Tba[0]);

      S[0] = Tab[0] + Rab(0, 0) * u;
      S[1] = Tab[1] + Rab(1, 0) * u - t;
      S[2] = Tab[2] + Rab(2, 0) * u;

      if(P&& Q)
      {
        *P << 0, t, 0;
        *Q = S + (*P);
      }

      return S.norm();
    }
  }

  FCL_REAL BLL_y, BLU_y, BUL_y, BUU_y;

  BLL_y = Tab[1];
  BLU_y = BLL_y + bA1_dot_B1;
  BUL_y = BLL_y + bA1_dot_B0;
  BUU_y = BLU_y + bA1_dot_B0;

  FCL_REAL LA0_lx, LA0_ux, UA0_lx, UA0_ux, LB1_ly, LB1_uy, UB1_ly, UB1_uy;

  if(ALL_x < AUL_x)
  {
    LA0_lx = ALL_x;
    LA0_ux = AUL_x;
    UA0_lx = ALU_x;
    UA0_ux = AUU_x;
  }
  else
  {
    LA0_lx = AUL_x;
    LA0_ux = ALL_x;
    UA0_lx = AUU_x;
    UA0_ux = ALU_x;
  }

  if(BLL_y < BLU_y)
  {
    LB1_ly = BLL_y;
    LB1_uy = BLU_y;
    UB1_ly = BUL_y;
    UB1_uy = BUU_y;
  }
  else
  {
    LB1_ly = BLU_y;
    LB1_uy = BLL_y;
    UB1_ly = BUU_y;
    UB1_uy = BUL_y;
  }

  // UA0, UB1

  if((UA0_ux > b[0]) && (UB1_uy > a[1]))
  {
    if(((UA0_lx > b[0]) ||
        inVoronoi(b[1], a[0], A0_dot_B0, aA1_dot_B0 - Tba[0] - b[0],
                  A0_dot_B1, aA1_dot_B1 - Tba[1], -Tab[0] - bA0_dot_B0))
       &&
       ((UB1_ly > a[1]) ||
        inVoronoi(a[0], b[1], A1_dot_B1, Tab[1] - a[1] + bA1_dot_B0,
                  A0_dot_B1, Tab[0] + bA0_dot_B0, Tba[1] - aA1_dot_B1)))
    {
      segCoords(t, u, a[0], b[1], A0_dot_B1, Tab[0] + bA0_dot_B0,
                Tba[1] - aA1_dot_B1);

      S[0] = Tab[0] + Rab(0, 0) * b[0] + Rab(0, 1) * u - t;
      S[1] = Tab[1] + Rab(1, 0) * b[0] + Rab(1, 1) * u - a[1];
      S[2] = Tab[2] + Rab(2, 0) * b[0] + Rab(2, 1) * u;

      if(P && Q)
      {
        *P << t, a[1], 0;
        *Q = S + (*P);
      }

      return S.norm();
    }
  }

  // UA0, LB1

  if((UA0_lx < 0) && (LB1_uy > a[1]))
  {
    if(((UA0_ux < 0) ||
        inVoronoi(b[1], a[0], -A0_dot_B0, Tba[0] - aA1_dot_B0, A0_dot_B1,
                  aA1_dot_B1 - Tba[1], -Tab[0]))
       &&
       ((LB1_ly > a[1]) ||
        inVoronoi(a[0], b[1], A1_dot_B1, Tab[1] - a[1], A0_dot_B1, Tab[0],
                  Tba[1] - aA1_dot_B1)))
    {
      segCoords(t, u, a[0], b[1], A0_dot_B1, Tab[0], Tba[1] - aA1_dot_B1);

      S[0] = Tab[0] + Rab(0, 1) * u - t;
      S[1] = Tab[1] + Rab(1, 1) * u - a[1];
      S[2] = Tab[2] + Rab(2, 1) * u;

      if(P && Q)
      {
        *P << t, a[1], 0;
        *Q = S + (*P);
      }

      return S.norm();
    }
  }

  // LA0, UB1

  if((LA0_ux > b[0]) && (UB1_ly < 0))
  {
    if(((LA0_lx > b[0]) ||
        inVoronoi(b[1], a[0], A0_dot_B0, -b[0] - Tba[0], A0_dot_B1, -Tba[1],
                  -bA0_dot_B0 - Tab[0]))
       &&
       ((UB1_uy < 0) ||
        inVoronoi(a[0], b[1], -A1_dot_B1, -Tab[1] - bA1_dot_B0, A0_dot_B1,
                  Tab[0] + bA0_dot_B0, Tba[1])))
    {
      segCoords(t, u, a[0], b[1], A0_dot_B1, Tab[0] + bA0_dot_B0, Tba[1]);

      S[0] = Tab[0] + Rab(0, 0) * b[0] + Rab(0, 1) * u - t;
      S[1] = Tab[1] + Rab(1, 0) * b[0] + Rab(1, 1) * u;
      S[2] = Tab[2] + Rab(2, 0) * b[0] + Rab(2, 1) * u;

      if(P && Q)
      {
        *P << t, 0, 0;
        *Q = S + (*P);
      }

      return S.norm();
    }
  }

  // LA0, LB1

  if((LA0_lx < 0) && (LB1_ly < 0))
  {
    if(((LA0_ux < 0) ||
        inVoronoi(b[1], a[0], -A0_dot_B0, Tba[0], A0_dot_B1, -Tba[1],
                  -Tab[0]))
       &&
       ((LB1_uy < 0) ||
        inVoronoi(a[0], b[1], -A1_dot_B1, -Tab[1], A0_dot_B1,
                  Tab[0], Tba[1])))
    {
      segCoords(t, u, a[0], b[1], A0_dot_B1, Tab[0], Tba[1]);

      S[0] = Tab[0] + Rab(0, 1) * u - t;
      S[1] = Tab[1] + Rab(1, 1) * u;
      S[2] = Tab[2] + Rab(2, 1) * u;

      if(P && Q)
      {
        *P << t, 0, 0;
        *Q = S + (*P);
      }

      return S.norm();
    }
  }

  FCL_REAL LA0_ly, LA0_uy, UA0_ly, UA0_uy, LB0_ly, LB0_uy, UB0_ly, UB0_uy;

  if(ALL_y < AUL_y)
  {
    LA0_ly = ALL_y;
    LA0_uy = AUL_y;
    UA0_ly = ALU_y;
    UA0_uy = AUU_y;
  }
  else
  {
    LA0_ly = AUL_y;
    LA0_uy = ALL_y;
    UA0_ly = AUU_y;
    UA0_uy = ALU_y;
  }

  if(BLL_y < BUL_y)
  {
    LB0_ly = BLL_y;
    LB0_uy = BUL_y;
    UB0_ly = BLU_y;
    UB0_uy = BUU_y;
  }
  else
  {
    LB0_ly = BUL_y;
    LB0_uy = BLL_y;
    UB0_ly = BUU_y;
    UB0_uy = BLU_y;
  }

  // UA0, UB0

  if((UA0_uy > b[1]) && (UB0_uy > a[1]))
  {
    if(((UA0_ly > b[1]) ||
        inVoronoi(b[0], a[0], A0_dot_B1, aA1_dot_B1 - Tba[1] - b[1],
                  A0_dot_B0, aA1_dot_B0 - Tba[0], -Tab[0] - bA0_dot_B1))
       &&
       ((UB0_ly > a[1]) ||
        inVoronoi(a[0], b[0], A1_dot_B0, Tab[1] - a[1] + bA1_dot_B1, A0_dot_B0,
                  Tab[0] + bA0_dot_B1, Tba[0] - aA1_dot_B0)))
    {
      segCoords(t, u, a[0], b[0], A0_dot_B0, Tab[0] + bA0_dot_B1,
                Tba[0] - aA1_dot_B0);

      S[0] = Tab[0] + Rab(0, 1) * b[1] + Rab(0, 0) * u - t;
      S[1] = Tab[1] + Rab(1, 1) * b[1] + Rab(1, 0) * u - a[1];
      S[2] = Tab[2] + Rab(2, 1) * b[1] + Rab(2, 0) * u;

      if(P && Q)
      {
        *P << t, a[1], 0;
        *Q = S + (*P);
      }

      return S.norm();
    }
  }

  // UA0, LB0

  if((UA0_ly < 0) && (LB0_uy > a[1]))
  {
    if(((UA0_uy < 0) ||
        inVoronoi(b[0], a[0], -A0_dot_B1, Tba[1] - aA1_dot_B1, A0_dot_B0,
                  aA1_dot_B0 - Tba[0], -Tab[0]))
       &&
       ((LB0_ly > a[1]) ||
        inVoronoi(a[0], b[0], A1_dot_B0, Tab[1] - a[1],
                  A0_dot_B0, Tab[0], Tba[0] - aA1_dot_B0)))
    {
      segCoords(t, u, a[0], b[0], A0_dot_B0, Tab[0], Tba[0] - aA1_dot_B0);

      S[0] = Tab[0] + Rab(0, 0) * u - t;
      S[1] = Tab[1] + Rab(1, 0) * u - a[1];
      S[2] = Tab[2] + Rab(2, 0) * u;

      if(P && Q)
      {
        *P << t, a[1], 0;
        *Q = S + (*P);
      }

      return S.norm();
    }
  }

  // LA0, UB0

  if((LA0_uy > b[1]) && (UB0_ly < 0))
  {
    if(((LA0_ly > b[1]) ||
        inVoronoi(b[0], a[0], A0_dot_B1, -Tba[1] - b[1], A0_dot_B0, -Tba[0],
                  -Tab[0] - bA0_dot_B1))
       &&

       ((UB0_uy < 0) ||
        inVoronoi(a[0], b[0], -A1_dot_B0, -Tab[1] - bA1_dot_B1, A0_dot_B0,
                  Tab[0] + bA0_dot_B1, Tba[0])))
    {
      segCoords(t, u, a[0], b[0], A0_dot_B0, Tab[0] + bA0_dot_B1, Tba[0]);

      S[0] = Tab[0] + Rab(0, 1) * b[1] + Rab(0, 0) * u - t;
      S[1] = Tab[1] + Rab(1, 1) * b[1] + Rab(1, 0) * u;
      S[2] = Tab[2] + Rab(2, 1) * b[1] + Rab(2, 0) * u;

      if(P && Q)
      {
        *P << t, 0, 0;
        *Q = S + (*P);
      }

      return S.norm();
    }
  }

  // LA0, LB0

  if((LA0_ly < 0) && (LB0_ly < 0))
  {
    if(((LA0_uy < 0) ||
        inVoronoi(b[0], a[0], -A0_dot_B1, Tba[1], A0_dot_B0,
                  -Tba[0], -Tab[0]))
       &&
       ((LB0_uy < 0) ||
        inVoronoi(a[0], b[0], -A1_dot_B0, -Tab[1], A0_dot_B0,
                  Tab[0], Tba[0])))
    {
      segCoords(t, u, a[0], b[0], A0_dot_B0, Tab[0], Tba[0]);

      S[0] = Tab[0] + Rab(0, 0) * u - t;
      S[1] = Tab[1] + Rab(1, 0) * u;
      S[2] = Tab[2] + Rab(2, 0) * u;

      if(P && Q)
      {
        *P << t, 0, 0;
        *Q = S + (*P);
      }

      return S.norm();
    }
  }

  // no edges passed, take max separation along face normals

  FCL_REAL sep1, sep2;

  if(Tab[2] > 0.0)
  {
    sep1 = Tab[2];
    if (Rab(2, 0) < 0.0) sep1 += b[0] * Rab(2, 0);
    if (Rab(2, 1) < 0.0) sep1 += b[1] * Rab(2, 1);
  }
  else
  {
    sep1 = -Tab[2];
    if (Rab(2, 0) > 0.0) sep1 -= b[0] * Rab(2, 0);
    if (Rab(2, 1) > 0.0) sep1 -= b[1] * Rab(2, 1);
  }

  if(Tba[2] < 0)
  {
    sep2 = -Tba[2];
    if (Rab(0, 2) < 0.0) sep2 += a[0] * Rab(0, 2);
    if (Rab(1, 2) < 0.0) sep2 += a[1] * Rab(1, 2);
  }
  else
  {
    sep2 = Tba[2];
    if (Rab(0, 2) > 0.0) sep2 -= a[0] * Rab(0, 2);
    if (Rab(1, 2) > 0.0) sep2 -= a[1] * Rab(1, 2);
  }

  if(sep1 >= sep2 && sep1 >= 0)
  {
    if(Tab[2] > 0)
      S << 0, 0, sep1;
    else
      S << 0, 0, -sep1;

    if(P && Q)
    {
      *Q = S;
      P->setZero();
    }
  }

  if(sep2 >= sep1 && sep2 >= 0)
  {
    Vector3d Q_(Tab[0], Tab[1], Tab[2]);
    Vector3d P_;
    if(Tba[2] < 0)
    {
      P_[0] = Rab(0, 2) * sep2 + Tab[0];
      P_[1] = Rab(1, 2) * sep2 + Tab[1];
      P_[2] = Rab(2, 2) * sep2 + Tab[2];
    }
    else
    {
      P_[0] = -Rab(0, 2) * sep2 + Tab[0];
      P_[1] = -Rab(1, 2) * sep2 + Tab[1];
      P_[2] = -Rab(2, 2) * sep2 + Tab[2];
    }

    S = Q_ - P_;

    if(P && Q)
    {
      *P = P_;
      *Q = Q_;
    }
  }

  FCL_REAL sep = (sep1 > sep2 ? sep1 : sep2);
  return (sep > 0 ? sep : 0);
}

//==============================================================================
bool overlap(const Matrix3d& R0, const Vector3d& T0, const RSS& b1, const RSS& b2)
{
  Matrix3d R0b2 = R0 * b2.axis;
  Matrix3d R = b1.axis.transpose() * R0b2;

  Vector3d Ttemp = R0 * b2.Tr + T0 - b1.Tr;
  Vector3d T = Ttemp.transpose() * b1.axis;

  FCL_REAL dist = rectDistance(R, T, b1.l, b2.l);
  return (dist <= (b1.r + b2.r));
}

//==============================================================================
FCL_REAL distance(const Matrix3d& R0, const Vector3d& T0, const RSS& b1, const RSS& b2, Vector3d* P, Vector3d* Q)
{
  Matrix3d R0b2 = R0 * b2.axis;
  Matrix3d R = b1.axis.transpose() * R0b2;

  Vector3d Ttemp = R0 * b2.Tr + T0 - b1.Tr;
  Vector3d T = Ttemp.transpose() * b1.axis;

  FCL_REAL dist = rectDistance(R, T, b1.l, b2.l, P, Q);
  dist -= (b1.r + b2.r);
  return (dist < (FCL_REAL)0.0) ? (FCL_REAL)0.0 : dist;
}

//==============================================================================
RSS translate(const RSS& bv, const Vector3d& t)
{
  RSS res(bv);
  res.Tr += t;
  return res;
}

} // namespace fcl

#endif
