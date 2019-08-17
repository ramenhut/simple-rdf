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

#ifndef __DECISION_FOREST_H__
#define __DECISION_FOREST_H__

#include <vector>

#include "base_types.h"
#include "tree.h"

using ::std::vector;

namespace base {

typedef struct DecisionForestParams {
  // how many trees to assemble in the forest.
  uint32 total_tree_count;
  // what percent of training data is used for each tree.
  uint32 tree_training_percentage;
} DecisionForestParams;

class DecisionForest {
 public:
  bool Train(const DecisionForestParams& forest_params,
             const DecisionTreeParams& tree_params,
             vector<ImageSet>* training_data, string* error = nullptr);
  // Classifies the input image and produces a label map.
  void ClassifyImage(Image* image_input, Image* label_output,
                     string* error = nullptr);
  // Classifies the input image and returns the dominant class index.
  uint8 Classify(Image* input, string* error = nullptr);
  // Returns the params used to construct the forest.
  DecisionForestParams GetForestParams() const;
  // Returns the params used to construct each tree in the forest.
  DecisionTreeParams GetTreeParams() const;

 private:
  // Our internal forest of decision trees.
  vector<DecisionTree> decision_forest_;
  // Overall forest parameters.
  DecisionForestParams forest_params_;
  // Tree level parameters.
  DecisionTreeParams tree_params_;

  // Provide access to our serialization API.
  friend bool SaveDecisionForest(const string& filename, DecisionForest* input,
                                 string* error);
  friend bool LoadDecisionForest(const string& filename, DecisionForest* output,
                                 string* error);
};

}  // namespace base

#endif  // __DECISION_FOREST_H__