
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "bitmap.h"
#include "forest.h"
#include "image.h"
#include "storage.h"
#include "time.h"
#include "tree.h"

using namespace base;
using ::std::cout;
using ::std::endl;
using ::std::make_unique;
using ::std::string;
using ::std::vector;

const string mnist_training_images = "train-images.idx3-ubyte";
const string mnist_training_labels = "train-labels.idx1-ubyte";
const string mnist_classify_images = "t10k-images.idx3-ubyte";
const string mnist_classify_labels = "t10k-labels.idx1-ubyte";

namespace base {

uint64 GetSystemTime() {
#if defined(BASE_PLATFORM_WINDOWS)
  return uint64(double(clock()) / CLOCKS_PER_SEC * 1000);
#elif defined(BASE_PLATFORM_MACOSX)
  timeval time;
  gettimeofday(&time, NULL);
  return (time.tv_sec * 1000) + (time.tv_usec / 1000);
#endif
}

uint64 GetElapsedTimeMs(uint64 from_time) {
  return (GetSystemTime() - from_time);
}

}  // namespace base

void PrintUsage(const char* programName) {
  cout << "Usage: " << programName << " [options]" << endl;
  cout << "  --train  [output forest filename]\t\t"
       << "Generates a forest based on the MNIST dataset." << endl;
  cout << "  --verify [input forest filename] \t\tTests the accuracy of a "
          "forest against the "
       << "MNIST test set." << endl;
}

void PrintForestParams(const DecisionForestParams& params) {
  cout << "  Tree count: " << params.total_tree_count << endl;
  cout << "  Tree train percentage: " << params.tree_training_percentage
       << endl;
}

void PrintTreeParams(const DecisionTreeParams& params) {
  cout << "  Class count: " << params.class_count << endl;
  cout << "  Max tree depth: " << params.max_tree_depth << endl;
  cout << "  Min node sample count: " << params.min_sample_count << endl;
  cout << "  Max node trial count: " << params.node_trial_count << endl;
  cout << "  Max visual search radius: " << params.visual_search_radius << endl;
}

void ExecuteTraining(const string& output_filename) {
  string error;
  uint32 label_count = 0;
  vector<ImageSet> training_data;

  if (output_filename.empty()) {
    cout << "You must specify a valid forest filename to save the forest."
         << endl;
    return;
  }

  if (!LoadImageSet(mnist_training_images, mnist_training_labels,
                    &training_data, &label_count, &error)) {
    cout << "Error detected during data load: " << error << endl;
    return;
  }

  cout << "Loaded " << training_data.size() << " training samples." << endl;

  DecisionForest forest;
  DecisionForestParams forest_params = {18, 80};
  DecisionTreeParams tree_params = {20, 1200, label_count, 20, 2};

  uint64 start_time = GetSystemTime();

  cout << "Initiating training sequence." << endl;

  if (!forest.Train(forest_params, tree_params, &training_data, &error)) {
    cout << "Error detected during training: " << error << endl;
    return;
  }

  uint32 elapsed_time = GetElapsedTimeMs(start_time);

  cout << "Training took " << elapsed_time / 1000.0f << " seconds." << endl;

  if (!SaveDecisionForest(output_filename, &forest, &error)) {
    cout << "Error detected while saving forest to disk: " << error << endl;
    return;
  }
}

void ExecuteVerification(const string& input_filename) {
  string error;
  uint32 label_count = 0;
  DecisionForest forest;
  vector<ImageSet> classify_data;

  if (input_filename.empty()) {
    cout << "You must specify a valid forest file to load for verification."
         << endl;
    return;
  }

  if (!LoadImageSet(mnist_classify_images, mnist_classify_labels,
                    &classify_data, &label_count, &error)) {
    cout << "Error detected during data load: " << error << endl;
    return;
  }

  cout << "Loaded " << classify_data.size() << " test samples." << endl;

  if (!LoadDecisionForest(input_filename, &forest, &error)) {
    cout << "Error detected while loading forest from disk: " << error << endl;
    return;
  }

  cout << "Loaded foreste with the following parameters:" << endl;
  PrintForestParams(forest.GetForestParams());
  PrintTreeParams(forest.GetTreeParams());

  float32 total_correct = 0.0f;

  for (auto& data : classify_data) {
    uint8 forest_result = forest.Classify(&data.image, &error);
    uint8 ground_truth = data.codex;

    if (error.length()) {
      cout << "Error detected during classification: " << error << endl;
      return;
    }

    total_correct += (forest_result == ground_truth);
  }

  cout << "Current forest accuracy level: "
       << 100.0f * total_correct / classify_data.size() << "." << endl;
}

int main(int argc, char** argv) {
  if (argc < 3) {
    PrintUsage(argv[0]);
    return 0;
  }

  for (int i = 1; i < argc; i++) {
    char* optBegin = argv[i];
    for (int j = 0; j < 2; j++) (optBegin[0] == '-') ? optBegin++ : optBegin;

    switch (optBegin[0]) {
      case 't':
        ExecuteTraining(argv[++i]);
        break;
      case 'v':
        ExecuteVerification(argv[++i]);
        break;
    }
  }

  return 0;
}