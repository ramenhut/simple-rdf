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

#ifndef __HISTOGRAM_H__
#define __HISTOGRAM_H__

#include <fstream>
#include <vector>
#include "base_types.h"

using ::std::ifstream;
using ::std::ofstream;
using ::std::vector;

namespace base {

class Histogram {
 public:
  Histogram();
  // Initializes with a specific class count.
  Histogram(uint32 class_count);
  // Initializes with a specific class count.
  void Initialize(uint32 class_count);
  // Increments a specific class total. Does not recompute entropy.
  bool IncrementValue(uint32 class_index);
  // Queries the percentage coverage of a specific class.
  float32 GetPercentage(uint32 class_index) const;
  // Computes entropy for the current sample set.
  float32 GetEntropy() const;
  // Removes a class from the histogram. This is used to ignore
  // certain features (e.g. background classes).
  void ClearClass(uint32 class_index);
  // Queries the total number of samples contained in the histogram.
  uint64 GetSampleTotal() const;
  // Queries the number of classes covered by the histogram.
  uint32 GetClassCount() const;
  // Queries the population of a specific class.
  uint32 GetClassTotal(uint32 class_index) const;
  // Returns the class index with the highest representation.
  uint32 GetDominantClass() const;
  // Support the ability to combine histograms.
  Histogram& operator+=(const Histogram& rhs);

 private:
  // The total number of samples tracked in class_totals_.
  uint64 sample_total_;
  // The per-class totals.
  vector<uint32> class_totals_;
  // Provide access for our serialization API.
  friend bool SaveHistogram(ofstream* out_stream, const Histogram& input,
                            string* error);
  friend bool LoadHistogram(ifstream* in_stream, Histogram* output,
                            string* error);
};

}  // namespace base

#endif  // __HISTOGRAM_H__