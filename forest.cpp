
#include "forest.h"
#include "numeric.h"
#include "random.h"

#include <time.h>
#include <thread>

#if _DEBUG
#define ENABLE_MULTITHREADING (0)
#else
#define ENABLE_MULTITHREADING (1)
#endif

namespace base {

uint64 GetTrainingSeed() {
#if defined(BASE_PLATFORM_WINDOWS)
  uint64 result = 0;
  SYSTEMTIME sys_time;
  GetSystemTime(&sys_time);
  result |= sys_time.wHour;
  result <<= 16;
  result |= sys_time.wMinute;
  result <<= 16;
  result |= sys_time.wSecond;
  result <<= 16;
  result |= sys_time.wMilliseconds;
  result <<= 16;
  return result;
#elif defined(BASE_PLATFORM_MACOSX)
  timeval time;
  gettimeofday(&time, NULL);
  return (time.tv_sec * 1000) + (time.tv_usec / 1000);
#endif
}

void TrainTreeFunction(DecisionTree* tree,
                       const DecisionTreeParams& tree_params,
                       vector<ImageSet>* training_data, uint32 train_start,
                       uint32 train_count) {
  set_seed(GetTrainingSeed());

  tree->Train(tree_params, training_data, train_start, train_count);
}

bool DecisionForest::Train(const DecisionForestParams& forest_params,
                           const DecisionTreeParams& tree_params,
                           vector<ImageSet>* training_data, string* error) {
  if (!training_data || !training_data->size()) {
    if (error) {
      *error = "Invalid training data.";
    }
    return false;
  }

  if (!forest_params.total_tree_count ||
      !forest_params.tree_training_percentage) {
    if (error) {
      *error = "Invalid DecisionForest params.";
    }
    return false;
  }

  tree_params_ = tree_params;
  forest_params_ = forest_params;
  decision_forest_.resize(forest_params.total_tree_count);

  uint32 train_range = training_data->size() / forest_params.total_tree_count;
  uint32 train_count =
      (forest_params.tree_training_percentage * training_data->size()) / 100;

#if ENABLE_MULTITHREADING
  uint32 thread_count = ::std::thread::hardware_concurrency();
  uint32 pass_remainder = forest_params.total_tree_count % thread_count;
  uint32 pass_count =
      align(forest_params.total_tree_count, thread_count) / thread_count;

  for (uint32 pass = 0; pass < pass_count; pass++) {
    uint32 tree_count = thread_count;
    vector<::std::thread> thread_list;

    if (pass == pass_count - 1 && pass_remainder) {
      tree_count = pass_remainder;
    }

    for (uint32 i = 0; i < tree_count; i++) {
      DecisionTree* tree = &decision_forest_.at(pass * thread_count + i);
      thread_list.emplace_back(&TrainTreeFunction, tree, tree_params_,
                               training_data, i * train_range, train_count);
    }

    for (auto& thread_ : thread_list) {
      thread_.join();
    }
  }
#else
  set_seed(GetSystemTimeMs());
  for (uint32 i = 0; i < forest_params_.total_tree_count; i++) {
    DecisionTree* tree = &decision_forest_.at(i);
    if (!tree->Train(tree_params_, training_data, i * train_range, train_count,
                     error)) {
      return false;
    }
  }
#endif

  return true;
}

void DecisionForest::ClassifyImage(Image* image_input, Image* label_output,
                                   string* error) {
  if (image_input->width != label_output->width ||
      image_input->height != label_output->height) {
    if (error) {
      *error = "Invalid parameter specified to DecisionForest::ClassifyImage.";
    }
    return;
  }

  if (!decision_forest_.size()) {
    if (error) {
      *error = "Decision forest must be trained before it can classify.";
    }
    return;
  }

  for (uint32 j = 0; j < image_input->height; j++) {
    for (uint32 i = 0; i < image_input->width; i++) {
      Histogram result(tree_params_.class_count);
      for (uint32 k = 0; k < decision_forest_.size(); k++) {
        // Collect votes from each forest and unify them.
        Histogram tree_result(tree_params_.class_count);
        if (!decision_forest_.at(k).ClassifyPixel(i, j, image_input,
                                                  &tree_result, error)) {
          return;
        }
        // We combine all of the votes from our decision trees into a
        // single histogram and then use ots dominant value.
        result += tree_result;
      }
      label_output->SetPixel(i, j, result.GetDominantClass());
    }
  }
}

uint8 DecisionForest::Classify(Image* input, string* error) {
  if (!input) {
    if (error) {
      *error = "Invalid parameter(s) specified to DecisionForest::Classify.";
    }
    return kBackgroundClassLabel;
  }

  if (!decision_forest_.size()) {
    if (error) {
      *error = "Decision forest must be trained before it can classify.";
    }
    return kBackgroundClassLabel;
  }

  // Accumulate histograms for each pixel, and then combine and grab dominant
  // class per pixel. Then count up the totals across the image and take the
  // dominant non-background class.

  Histogram image_result(tree_params_.class_count);
  for (uint32 j = 0; j < input->height; j++) {
    for (uint32 i = 0; i < input->width; i++) {
      Histogram pixel_result(tree_params_.class_count);
      for (uint32 k = 0; k < decision_forest_.size(); k++) {
        // Collect votes from each forest and unify them.
        Histogram tree_result(tree_params_.class_count);
        if (!decision_forest_.at(k).ClassifyPixel(i, j, input, &tree_result,
                                                  error)) {
          return kBackgroundClassLabel;
        }
        // We combine all of the votes from our decision trees into a
        // single histogram and then use its dominant value.
        pixel_result += tree_result;
      }
      image_result.IncrementValue(pixel_result.GetDominantClass());
    }
  }
  // We ignore background samples which will likely be the most
  // frequent value in our class.
  image_result.ClearClass(kBackgroundClassLabel);

  return image_result.GetDominantClass();
}

DecisionForestParams DecisionForest::GetForestParams() const {
  return forest_params_;
}

DecisionTreeParams DecisionForest::GetTreeParams() const {
  return tree_params_;
}

}  // namespace base