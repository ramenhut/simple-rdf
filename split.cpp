
#include "split.h"

#include "numeric.h"
#include "random.h"

namespace base {

void SplitFunction::Initialize(int32 max_search_radius) {
  uint32 count = random_integer_range(1, 2);

  params_.clear();

  for (uint32 i = 0; i < count; i++) {
    SplitCoord param_offset;
    param_offset.x =
        random_integer_range(-max_search_radius, max_search_radius);
    param_offset.y =
        random_integer_range(-max_search_radius, max_search_radius);
    params_.push_back(param_offset);
  }
}

SplitCoord ProjectCoord(const Image& data_source, const SplitCoord& source,
                        const SplitCoord& offset) {
  // We do not permit offsets that are greater than half the dimension
  int32 half_width = data_source.width >> 0x1;
  int32 half_height = data_source.height >> 0x1;

  int32 offset_x = clip_range(offset.x, -half_width, half_width);
  int32 offset_y = clip_range(offset.y, -half_height, half_height);

  // Perform a simple reverb if our coordinate is OOB.
  SplitCoord result = {source.x + offset_x, source.y + offset_y};

  if (result.x < 0) result.x = -1 * result.x;
  if (result.y < 0) result.y = -1 * result.y;

  if (result.x >= data_source.width - 1) {
    result.x = ((data_source.width - 1) << 0x1) - result.x;
  }

  if (result.y >= data_source.height - 1) {
    result.y = ((data_source.height - 1) << 0x1) - result.y;
  }

  return result;
}

bool SplitFunction::Split(const SplitCoord& coord, Image* data_source) {
  if (!params_.size()) {
    return false;
  }

  if (2 == params_.size()) {
    SplitCoord param_coord0 = ProjectCoord(*data_source, coord, params_.at(0));
    SplitCoord param_coord1 = ProjectCoord(*data_source, coord, params_.at(1));
    int32 value0 = data_source->GetPixel(param_coord0.x, param_coord0.y);
    int32 value1 = data_source->GetPixel(param_coord1.x, param_coord1.y);

    return value1 > value0;
  } else if (1 == params_.size()) {
    SplitCoord param_coord0 = ProjectCoord(*data_source, coord, params_.at(0));
    int32 value0 = data_source->GetPixel(param_coord0.x, param_coord0.y);
    int32 source = data_source->GetPixel(coord.x, coord.y);

    return source > value0;
  }

  return false;
}

}  // namespace base