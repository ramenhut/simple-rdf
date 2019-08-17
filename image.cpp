
#include "image.h"

#include <fstream>
#include <set>
#include <utility>

using ::std::ifstream;
using ::std::set;

namespace base {

void Image::Initialize(uint32 new_width, uint32 new_height) {
  width = new_width;
  height = new_height;
  data.resize(width * height);
}

uint8 Image::GetPixel(uint32 x, uint32 y) {
  if (data.empty()) {
    return 0;
  }

  return data.at(y * width + x);
}

void Image::SetPixel(uint32 x, uint32 y, uint8 value) {
  if (data.empty() || x >= width || y >= height) {
    return;
  }

  data.at(y * width + x) = value;
}

bool LoadImageSet(const string& images_filename, const string& labels_filename,
                  vector<ImageSet>* output, uint32* label_count,
                  string* error) {
  if (images_filename.empty() || labels_filename.empty() || !output ||
      !label_count) {
    if (error) {
      *error = "Invalid inputs to LoadImageSet.";
    }
    return false;
  }

  MNIST_IMAGE_FILE_HEADER image_header;
  MNIST_LABEL_FILE_HEADER label_header;

  ifstream image_source(images_filename, ::std::ios::in | ::std::ios::binary);
  ifstream label_source(labels_filename, ::std::ios::in | ::std::ios::binary);

  if (!image_source || !label_source) {
    if (error) {
      *error = "Failed to open data files.";
    }
    return false;
  }

  // Read in both headers. Data counts must match.
  if (!image_source.read((char*)&image_header,
                         sizeof(MNIST_IMAGE_FILE_HEADER)) ||
      !label_source.read((char*)&label_header,
                         sizeof(MNIST_LABEL_FILE_HEADER))) {
    if (error) {
      *error = "Failed to read MNIST image and/or label file headers.";
    }
    return false;
  }

  // Parse the image header, swapping the dwords into little endian.
  image_header.magic = EndianSwap8in32(image_header.magic);
  image_header.image_count = EndianSwap8in32(image_header.image_count);
  image_header.width = EndianSwap8in32(image_header.width);
  image_header.height = EndianSwap8in32(image_header.height);

  // Parse the label header, swapping the dwords into little endian.
  label_header.magic = EndianSwap8in32(label_header.magic);
  label_header.label_count = EndianSwap8in32(label_header.label_count);

  if (image_header.magic != 2051 || label_header.magic != 2049) {
    if (error) {
      *error = "Invalid MNIST data file(s) detected.";
    }
    return false;
  }

  if (image_header.image_count != label_header.label_count) {
    if (error) {
      *error = "Image and label count mismatch.";
    }
    return false;
  }

  // File reads have a relatively high fixed cost, so we load the entire
  //   data set into memory and then scatter afterwards.
  vector<uint8> image_file_buffer(image_header.image_count *
                                  image_header.width * image_header.height);

  vector<uint8> label_file_buffer(label_header.label_count);

  // Read in both sets of data.
  if (!image_source.read((char*)&image_file_buffer.at(0),
                         image_header.image_count * image_header.width *
                             image_header.height)) {
    if (error) {
      *error = "Failed to read image data from disk.";
    }
    return false;
  }

  if (!label_source.read((char*)&label_file_buffer.at(0),
                         label_header.label_count)) {
    if (error) {
      *error = "Failed to read label data from disk.";
    }
    return false;
  }

  set<uint32> label_set;
  output->resize(image_header.image_count);

  for (uint32 i = 0; i < image_header.image_count; i++) {
    ImageSet* training_set = &output->at(i);

    // Allocate space for our image data.
    training_set->image.Initialize(image_header.width, image_header.height);

    // Allocate space for our label data. Labels are defined on
    // a per-pixel basis in order to support images with multiple
    // objects (even though our MNIST data set doesn't support this.
    training_set->label.Initialize(image_header.width, image_header.height);

    // Populate our image data.
    memcpy(&training_set->image.data.at(0),
           &image_file_buffer.at(image_header.width * image_header.height * i),
           image_header.width * image_header.height);

    // Set our codex equal to the label for the entire file.
    training_set->codex = label_file_buffer.at(i);

    // Populate our label data. We reserve a value of 0xFF to indicate
    // background, and will incorporate this label into training.
    for (uint32 y = 0; y < image_header.height; y++) {
      for (uint32 x = 0; x < image_header.width; x++) {
        Image* data = &training_set->image;
        Image* labels = &training_set->label;
        uint8 image_value = data->GetPixel(x, y);

        if (image_value) {
          labels->SetPixel(x, y, label_file_buffer.at(i));
        } else {
          labels->SetPixel(x, y, kBackgroundClassLabel);
        }

        // Catalog the set of labels in our training set.
        label_set.insert(labels->GetPixel(x, y));
      }
    }
  }

  *label_count = label_set.size();

  return true;
}

}  // namespace base