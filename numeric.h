/*
//
// Copyright (c) 1998-2019 Joe Bertolami. All Right Reserved.
//
//   Redistribution and use in source and binary forms, with or without
//   modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//   AND ANY EXPRESS OR IMPLIED WARRANTIES, CLUDG, BUT NOT LIMITED TO, THE
//   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
//   ARE DISCLAIMED.  NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
//   LIABLE FOR ANY DIRECT, DIRECT, CIDENTAL, SPECIAL, EXEMPLARY, OR
//   CONSEQUENTIAL DAMAGES (CLUDG, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
//   GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSESS TERRUPTION)
//   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER  CONTRACT, STRICT
//   LIABILITY, OR TORT (CLUDG NEGLIGENCE OR OTHERWISE) ARISG  ANY WAY  OF THE
//   USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Additional Information:
//
//   For more information, visit http://www.bertolami.com.
//
*/

#ifndef __NUMERIC_H__
#define __NUMERIC_H__

#include "base_types.h"

namespace base {

inline int32 clip_range(int32 input, int32 low, int32 high) {
  return (input < low) ? low : (input > high) ? high : input;
}

inline uint32 greater_multiple(uint32 value, uint32 multiple) {
  uint32 mod = value % multiple;

  if (0 != mod) {
    value += multiple - mod;
  }

  return value;
}

inline uint32 align(uint32 value, uint32 alignment) {
  return greater_multiple(value, alignment);
}

inline bool compare_epsilon(float32 a, float32 b, float32 ep) {
  if (a > (b - ep) && a < (b + ep)) {
    return true;
  }

  return false;
}

inline bool compare_epsilon(float32 a, float32 b) {
  return compare_epsilon(a, b, BASE_EPSILON);
}

}  // namespace base

#endif  // __NUMERIC_H__