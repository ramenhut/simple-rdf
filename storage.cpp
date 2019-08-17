
#include "storage.h"

namespace base {

bool SaveSplitFunction(ofstream *out_stream, const SplitFunction &input,
                       string *error) {
  uint32 param_count = input.params_.size();
  if (!out_stream->write((char *)&param_count, sizeof(uint32))) {
    if (error) {
      *error = "Failed to write split param count to disk.";
    }
    return false;
  }

  for (auto value : input.params_) {
    if (!out_stream->write((char *)&value.x, sizeof(int32)) ||
        !out_stream->write((char *)&value.y, sizeof(int32))) {
      if (error) {
        *error = "Failed to write split params to disk.";
      }
      return false;
    }
  }

  return true;
}

bool LoadSplitFunction(ifstream *in_stream, SplitFunction *output,
                       string *error) {
  uint32 param_count = 0;
  if (!in_stream->read((char *)&param_count, sizeof(uint32))) {
    if (error) {
      *error = "Failed to read split param count from disk.";
    }
    return false;
  }

  output->params_.resize(param_count);
  for (auto &value : output->params_) {
    if (!in_stream->read((char *)&value.x, sizeof(int32)) ||
        !in_stream->read((char *)&value.y, sizeof(int32))) {
      if (error) {
        *error = "Failed to read split params from disk.";
      }
      return false;
    }
  }

  return true;
}

bool SaveHistogram(ofstream *out_stream, const Histogram &input,
                   string *error) {
  if (!out_stream->write((char *)&input.sample_total_, sizeof(uint64))) {
    if (error) {
      *error = "Failed to write histogram total sample count to disk.";
    }
    return false;
  }

  uint32 class_count = input.class_totals_.size();
  if (!out_stream->write((char *)&class_count, sizeof(uint32))) {
    if (error) {
      *error = "Failed to write histogram class count to disk.";
    }
    return false;
  }

  for (auto value : input.class_totals_) {
    if (!out_stream->write((char *)&value, sizeof(uint32))) {
      if (error) {
        *error = "Failed to write histogram sample to disk.";
      }
      return false;
    }
  }

  return true;
}

bool LoadHistogram(ifstream *in_stream, Histogram *output, string *error) {
  if (!in_stream->read((char *)&output->sample_total_, sizeof(uint64))) {
    if (error) {
      *error = "Failed to read histogram total sample count from disk.";
    }
    return false;
  }

  uint32 class_count = 0;
  if (!in_stream->read((char *)&class_count, sizeof(uint32))) {
    if (error) {
      *error = "Failed to read histogram class count from disk.";
    }
    return false;
  }

  output->class_totals_.resize(class_count);
  for (auto &value : output->class_totals_) {
    if (!in_stream->read((char *)&value, sizeof(uint32))) {
      if (error) {
        *error = "Failed to read histogram sample from disk.";
      }
      return false;
    }
  }

  return true;
}

bool SaveDecisionTree(ofstream *out_stream, DecisionTree *input,
                      string *error) {
  // First is our DecisionTreeParams structure
  if (!out_stream->write((char *)&input->params_, sizeof(DecisionTreeParams))) {
    if (error) {
      *error = "Failed to write decision tree params to disk.";
    }
    return false;
  }

  vector<DecisionNode *> tree_stack;
  tree_stack.push_back(input->root_node_.get());

  while (!tree_stack.empty()) {
    // Decision trees are serialized in breadth first order. We pop the first
    // stack entry, push children if available, and serialize the node.
    DecisionNode *node = tree_stack.at(0);

    if (!out_stream->write((char *)&node->is_leaf_, sizeof(bool))) {
      if (error) {
        *error = "Failed to write decision node flag to disk.";
      }
      return false;
    }

    if (!node->is_leaf_) {
      tree_stack.push_back(node->left_child_.get());
      tree_stack.push_back(node->right_child_.get());

      if (!SaveSplitFunction(out_stream, node->function_, error)) {
        return false;
      }
    } else {
      if (!SaveHistogram(out_stream, node->histogram_, error)) {
        return false;
      }
    }

    // Pop our current node off the stack.
    tree_stack.erase(tree_stack.begin());
  }

  return true;
}

bool LoadDecisionTree(ifstream *in_stream, DecisionTree *output,
                      string *error) {
  // First is our DecisionTreeParams structure
  if (!in_stream->read((char *)&output->params_, sizeof(DecisionTreeParams))) {
    if (error) {
      *error = "Failed to read decision tree params from disk.";
    }
    return false;
  }

  vector<DecisionNode *> tree_stack;
  output->root_node_.reset(new DecisionNode);

  if (!output->root_node_) {
    return false;
  }

  tree_stack.push_back(output->root_node_.get());

  while (!tree_stack.empty()) {
    // Decision trees are serialized in breadth first order. We pop the first
    // stack entry, push children if available, and deserialize the node.
    DecisionNode *node = tree_stack.at(0);

    if (!in_stream->read((char *)&node->is_leaf_, sizeof(bool))) {
      if (error) {
        *error = "Failed to read decision node flag from disk.";
      }
      return false;
    }

    if (!node->is_leaf_) {
      node->left_child_.reset(new DecisionNode);
      node->right_child_.reset(new DecisionNode);

      tree_stack.push_back(node->left_child_.get());
      tree_stack.push_back(node->right_child_.get());

      if (!LoadSplitFunction(in_stream, &node->function_, error)) {
        return false;
      }
    } else {
      if (!LoadHistogram(in_stream, &node->histogram_, error)) {
        return false;
      }
    }

    // Pop our current node off the stack.
    tree_stack.erase(tree_stack.begin());
  }

  return true;
}

bool SaveDecisionForest(const string &filename, DecisionForest *input,
                        string *error) {
  ofstream out_stream(filename, ::std::ios::out | ::std::ios::binary);

  if (!out_stream.write((char *)&input->forest_params_,
                        sizeof(DecisionForestParams))) {
    if (error) {
      *error = "Failed to write decision forest params to disk.";
    }
    return false;
  }

  if (!out_stream.write((char *)&input->tree_params_,
                        sizeof(DecisionTreeParams))) {
    if (error) {
      *error = "Failed to write decision tree params to disk.";
    }
    return false;
  }

  for (auto &i : input->decision_forest_) {
    if (!SaveDecisionTree(&out_stream, &i, error)) {
      return false;
    }
  }

  return true;
}

bool LoadDecisionForest(const string &filename, DecisionForest *output,
                        string *error) {
  ifstream in_stream(filename, ::std::ios::in | ::std::ios::binary);

  if (!in_stream.read((char *)&output->forest_params_,
                      sizeof(DecisionForestParams))) {
    if (error) {
      *error = "Failed to read decision forest params from disk.";
    }
    return false;
  }

  if (!in_stream.read((char *)&output->tree_params_,
                      sizeof(DecisionTreeParams))) {
    if (error) {
      *error = "Failed to read decision tree params from disk.";
    }
    return false;
  }

  output->decision_forest_.resize(output->forest_params_.total_tree_count);

  for (auto &i : output->decision_forest_) {
    if (!LoadDecisionTree(&in_stream, &i, error)) {
      return false;
    }
  }

  return true;
}

}  // namespace base