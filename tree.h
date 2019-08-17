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

#ifndef __DECISION_TREE_H__
#define __DECISION_TREE_H__

#include <fstream>
#include <memory>
#include <utility>

#include "base_types.h"
#include "histogram.h"
#include "image.h"
#include "split.h"

using ::std::ifstream;
using ::std::ofstream;
using ::std::unique_ptr;

namespace base {

typedef struct DecisionTreeParams {
  // maximum depth for any decision tree.
  uint32 max_tree_depth;
  // how many trials to run when configuring a node.
  uint32 node_trial_count;
  // how many different label values are there? set
  // this value to zero to force auto-detection.
  uint32 class_count;
  // how wide a kernel to use during training.
  uint32 visual_search_radius;
  // minimum number of samples required to perform a split.
  uint32 min_sample_count;
} DecisionTreeParams;

typedef struct TrainSet {
  // The underlying image set for the sample.
  ImageSet *data_source;
  // The coordinates within the training set that
  // identify our current sample.
  SplitCoord coord;

  TrainSet(ImageSet *data, uint32 x, uint32 y) {
    data_source = data;
    coord.x = x;
    coord.y = y;
  }
} TrainSet;

// A binary node within a decision tree.
class DecisionNode {
 public:
  // Constructs a decision tree according to the specified parameters.
  // Performs N iterations to determine the best candidate split function and
  // then traverses to populate children. Halts once exit criteria (defined by
  // DecisionTreeParams) is satisfied.
  bool Train(const DecisionTreeParams &params, uint32 depth,
             vector<TrainSet> *samples, const Histogram &sample_histogram,
             string *error = nullptr);
  // Determines the class represented by the sample.
  bool Classify(const SplitCoord &coord, Image *data_source, Histogram *output,
                string *error = nullptr);

 private:
  bool is_leaf_;
  Histogram histogram_;
  SplitFunction function_;
  unique_ptr<DecisionNode> left_child_;
  unique_ptr<DecisionNode> right_child_;
  // Provide access to our serialization API.
  friend bool SaveDecisionTree(ofstream *out_stream, class DecisionTree *input,
                               string *error);
  friend bool LoadDecisionTree(ifstream *in_stream, class DecisionTree *output,
                               string *error);
};

class DecisionTree {
 public:
  // Trains the tree based on the supplied labelled training images.
  bool Train(const DecisionTreeParams &params, vector<ImageSet> *training_data,
             uint32 training_start_index, uint32 training_count,
             string *error = nullptr);
  // Determines the class of object represented by the pixel.
  bool ClassifyPixel(uint32 x, uint32 y, Image *input, Histogram *output,
                     string *error = nullptr);

 private:
  // Binary tree represents our actual decision tree struture.
  unique_ptr<DecisionNode> root_node_;
  // Cached copy of our decision tree params.
  DecisionTreeParams params_;
  // Provide access to our serialization API.
  friend bool SaveDecisionTree(ofstream *out_stream, DecisionTree *input,
                               string *error);
  friend bool LoadDecisionTree(ifstream *in_stream, DecisionTree *output,
                               string *error);
};

}  // namespace base

#endif  // __DECISION_TREE_H__