
#include "tree.h"

#include "numeric.h"

namespace base {

float32 ComputeInformationGain(const Histogram& parent, const Histogram& left,
                               const Histogram& right) {
  // Our computations use integer variables but must occur at float precision.
  float32 parent_total = parent.GetSampleTotal();
  return parent.GetEntropy() -
         (((left.GetSampleTotal() / parent_total) * (left.GetEntropy())) +
          ((right.GetSampleTotal() / parent_total) * (right.GetEntropy())));
}

bool DecisionNode::Train(const DecisionTreeParams& params, uint32 depth,
                         vector<TrainSet>* samples,
                         const Histogram& sample_histogram, string* error) {
  if (!samples) {
    if (error) {
      *error = "Invalid parameter(s) specified to DecisionNode::Train.";
    }
    return false;
  }

  // Cache our incoming histogram, which defines the statitics at the
  // current node. This is useful during training, and potentially useful
  // for classification if the current node ends up being a leaf.
  histogram_ = sample_histogram;

  // If we've reached our exit criteria then we early exit, leaving this
  // node as a leaf in the tree.
  if (depth >= params.max_tree_depth || !samples->size() ||
      samples->size() < params.min_sample_count) {
    is_leaf_ = true;
    return true;
  }

  float32 node_entropy = histogram_.GetEntropy();
  // If our incoming entropy is zero then our data set is of uniform
  // type, and we can declare this node a leaf.
  if (0.0f == node_entropy || -0.0f == node_entropy) {
    is_leaf_ = true;
    return true;
  }

  // Perform a maximum of params.node_trial_count trials to find
  // a best candidate split function for this node. We're guaranteed
  // to finish with a candidate, even if it's only a local best.

  float32 best_info_gain = -1.0f;
  Histogram best_left_hist, best_right_hist;
  vector<TrainSet> best_left_samples, best_right_samples;
  SplitFunction best_split_function, trial_split_function;

  for (uint32 i = 0; i < params.node_trial_count; i++) {
    Histogram trial_left_hist, trial_right_hist;
    vector<TrainSet> trial_left_samples, trial_right_samples;

    trial_left_hist.Initialize(params.class_count);
    trial_right_hist.Initialize(params.class_count);
    trial_left_samples.reserve(samples->size());
    trial_right_samples.reserve(samples->size());
    trial_split_function.Initialize(params.visual_search_radius);

    // Iterate over all samples, performing split. True goes right.
    for (uint32 j = 0; j < samples->size(); j++) {
      SplitCoord current_coord = samples->at(j).coord;
      uint8 sample_label = samples->at(j).data_source->label.GetPixel(
          current_coord.x, current_coord.y);
      if (trial_split_function.Split(current_coord,
                                     &samples->at(j).data_source->image)) {
        trial_right_samples.push_back(samples->at(j));
        trial_right_hist.IncrementValue(sample_label);
      } else {
        trial_left_samples.push_back(samples->at(j));
        trial_left_hist.IncrementValue(sample_label);
      }
    }

    float32 current_info_gain =
        ComputeInformationGain(histogram_, trial_left_hist, trial_right_hist);

    if (current_info_gain >= best_info_gain) {
      best_info_gain = current_info_gain;
      best_left_hist = trial_left_hist;
      best_right_hist = trial_right_hist;
      best_left_samples = std::move(trial_left_samples);
      best_right_samples = std::move(trial_right_samples);
      best_split_function = trial_split_function;

      // If our current info gain equals entropy (i.e. both buckets have zero
      // entropy), then we can immediately select this as our best option.
      if (current_info_gain == node_entropy) {
        break;
      }
    }
  }

  // Bind the best split function that we found during our trials.
  function_ = best_split_function;
  is_leaf_ = false;

  // We have our best so we allocate children and attempt to train them.
  left_child_.reset(new DecisionNode);
  right_child_.reset(new DecisionNode);

  if (!left_child_ || !right_child_) {
    if (error) {
      *error = "Failed to allocate child nodes.";
    }
    return false;
  }

  if (!left_child_->Train(params, depth + 1, &best_left_samples, best_left_hist,
                          error) ||
      !right_child_->Train(params, depth + 1, &best_right_samples,
                           best_right_hist, error)) {
    return false;
  }

  return true;
}

bool DecisionNode::Classify(const SplitCoord& coord, Image* data_source,
                            Histogram* output, string* error) {
  if ((!!left_child_) ^ (!!right_child_)) {
    if (error) {
      *error = "Invalid tree structure.";
    }
    return false;
  }

  if (!output || !data_source) {
    if (error) {
      *error = "Invalid parameter specified to DecisionNode::Classify.";
    }
    return false;
  }

  if (is_leaf_) {
    *output = histogram_;
    return true;
  }

  if (function_.Split(coord, data_source)) {
    return right_child_->Classify(coord, data_source, output);
  }
  return left_child_->Classify(coord, data_source, output);
}

bool DecisionTree::Train(const DecisionTreeParams& params,
                         vector<ImageSet>* training_data,
                         uint32 training_start_index, uint32 training_count,
                         string* error) {
  if (training_data->empty() || training_count > training_data->size()) {
    if (error) {
      *error = "Invalid parameter specified to DecisionTree::Train.";
    }
    return false;
  }

  Histogram initial_histogram(params.class_count);
  vector<TrainSet> tree_training_set;

  // Cache a copy of our tree params for later use during classification.
  params_ = params;

  // This is one of the most expensive operations in our system, so we
  // estimate the required size and reserve memory for it.
  uint64 required_size = training_count * training_data->at(0).image.width *
                         training_data->at(0).image.height;
  tree_training_set.reserve(required_size);

  for (uint32 i = 0; i < training_count; i++) {
    uint32 index = (training_start_index + i) % training_data->size();
    uint32 width = training_data->at(index).image.width;
    uint32 height = training_data->at(index).image.height;

    for (uint32 y = 0; y < height; y++)
      for (uint32 x = 0; x < width; x++) {
        uint8 label_value = training_data->at(index).label.GetPixel(x, y);
        tree_training_set.emplace_back(&training_data->at(index), x, y);
        initial_histogram.IncrementValue(label_value);
      }
  }

  root_node_.reset(new DecisionNode);

  if (!root_node_) {
    if (error) {
      *error = "Failed allocation of decision tree root node.";
    }
    return false;
  }

  return root_node_->Train(params, 0, &tree_training_set, initial_histogram);
}

bool DecisionTree::ClassifyPixel(uint32 x, uint32 y, Image* input,
                                 Histogram* output, string* error) {
  if (!root_node_) {
    if (error) {
      *error = "Invalid root node detected.";
    }
    return false;
  }

  if (!input || !output) {
    if (error) {
      *error = "Invalid parameter specified to DecisionTree::ClassifyPixel.";
    }
    return false;
  }

  SplitCoord coord = {x, y};

  return root_node_->Classify(coord, input, output, error);
}

}  // namespace base