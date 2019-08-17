
#include "histogram.h"

namespace base {

Histogram::Histogram() { sample_total_ = 0; }

Histogram::Histogram(uint32 class_count) { Initialize(class_count); }

void Histogram::Initialize(uint32 class_count) {
  sample_total_ = 0;
  class_totals_.resize(class_count);
  memset(&class_totals_.at(0), 0, sizeof(uint32) * class_count);
}

bool Histogram::IncrementValue(uint32 class_index) {
  if (class_totals_.size() <= class_index) {
    return false;
  }

  sample_total_++;
  class_totals_.at(class_index)++;
  return true;
}

uint64 Histogram::GetSampleTotal() const { return sample_total_; }

void Histogram::ClearClass(uint32 class_index) {
  if (class_totals_.size() <= class_index) {
    return;
  }

  sample_total_ -= class_totals_.at(class_index);
  class_totals_.at(class_index) = 0;
}

float32 Histogram::GetPercentage(uint32 class_index) const {
  if (class_totals_.size() <= class_index) {
    return 0.0f;
  }

  if (0 == sample_total_) {
    return 0.0f;
  }

  return static_cast<float32>(class_totals_.at(class_index)) / sample_total_;
}

uint32 Histogram::GetDominantClass() const {
  uint32 highest_total = 0;
  uint32 highest_index = 0;

  // This will return the first class if there is no conclusive winner.
  // We're OK with this since it's probably as good a guess as any.
  for (uint32 index = 0; index < class_totals_.size(); index++) {
    if (class_totals_.at(index) > highest_total) {
      highest_total = class_totals_.at(index);
      highest_index = index;
    }
  }
  return highest_index;
}

float32 Histogram::GetEntropy() const {
  float32 total = 0.0f;

  for (uint32 i = 0; i < class_totals_.size(); i++) {
    float32 class_probability = GetPercentage(i);

    if (class_probability > 0) {
      total = total + class_probability * log2(class_probability);
    }
  }

  return -1.0f * total;
}

uint32 Histogram::GetClassCount() const { return class_totals_.size(); }

uint32 Histogram::GetClassTotal(uint32 class_index) const {
  if (class_totals_.size() <= class_index) {
    return 0;
  }

  return class_totals_.at(class_index);
}

Histogram& Histogram::operator+=(const Histogram& rhs) {
  if (class_totals_.size() != rhs.class_totals_.size()) {
    return (*this);
  }

  sample_total_ += rhs.sample_total_;
  for (uint32 i = 0; i < class_totals_.size(); i++) {
    class_totals_.at(i) += rhs.class_totals_.at(i);
  }
  return (*this);
}

}  // namespace base
