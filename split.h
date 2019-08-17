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

#ifndef __SPLIT_H__
#define __SPLIT_H__

#include <fstream>
#include <vector>

#include "base_types.h"
#include "image.h"

using ::std::ifstream;
using ::std::ofstream;
using ::std::vector;

namespace base {

typedef struct SplitCoord {
  // The x coordinate of the sample within an image.
  int32 x;
  // The y coordinate of the sample within an image.
  int32 y;
} SplitCoord;

// Our split function (aka weak learner) that is selected out of a
// pool of randomly generated functions.
class SplitFunction {
 public:
  // Initializes the object with random parameters bounded by the radius.
  void Initialize(int32 max_search_radius);
  // Sorts the sample based on internal parameters.
  bool Split(const SplitCoord& coord, Image* data_source);

 private:
  // The 2D offset parameters that define the behavior of this split.
  vector<SplitCoord> params_;
  // Provide access for our serialization API.
  friend bool SaveSplitFunction(ofstream* out_stream,
                                const SplitFunction& input, string* error);
  friend bool LoadSplitFunction(ifstream* in_stream, SplitFunction* output,
                                string* error);
};

}  // namespace base

#endif  // __SPLIT_H__