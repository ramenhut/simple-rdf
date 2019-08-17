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

#ifndef __STORAGE_H__
#define __STORAGE_H__

#include <fstream>
#include <string>
#include "base_types.h"
#include "forest.h"
#include "tree.h"

using ::std::ifstream;
using ::std::ofstream;
using ::std::string;

namespace base {
// Saves a split function to an established output file stream.
bool SaveSplitFunction(ofstream* out_stream, const SplitFunction& input,
                       string* error = nullptr);
// Loads a split function from an established input file stream.
bool LoadSplitFunction(ifstream* in_stream, SplitFunction* output,
                       string* error = nullptr);
// Saves a histogram to an established output file stream.
bool SaveHistogram(ofstream* out_stream, const Histogram& input,
                   string* error = nullptr);
// Loads a histogram from an established input file stream.
bool LoadHistogram(ifstream* in_stream, Histogram* output,
                   string* error = nullptr);
// Saves a decision tree to an established output file stream.
bool SaveDecisionTree(ofstream* out_stream, DecisionTree* input,
                      string* error = nullptr);
// Loads a decision tree from an established input file stream.
bool LoadDecisionTree(ifstream* in_stream, DecisionTree* output,
                      string* error = nullptr);
// Saves a decision forest to filename.
bool SaveDecisionForest(const string& filename, DecisionForest* input,
                        string* error = nullptr);
// Loads a decision forest from filename.
bool LoadDecisionForest(const string& filename, DecisionForest* output,
                        string* error = nullptr);

}  // namespace base

#endif  // __STORAGE_H__