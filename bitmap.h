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

#ifndef __BITMAP_H__
#define __BITMAP_H__

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

#include "base_types.h"
#include "image.h"

using ::std::ifstream;
using ::std::ofstream;
using ::std::string;
using ::std::vector;

namespace base {

#ifndef BI_RGB
#define BI_RGB (0)
#endif

#pragma pack(push)
#pragma pack(2)

typedef struct PTCX_BITMAP_FILE_HEADER {
  uint16 type;
  uint32 size;
  uint16 reserved[2];
  uint32 off_bits;

} PTCX_BITMAP_FILE_HEADER;

typedef struct PTCX_BITMAP_INFO_HEADER {
  uint32 size;
  int32 width;
  int32 height;
  uint16 planes;
  uint16 bit_count;
  uint32 compression;
  uint32 size_image;
  int32 x_pels_per_meter;
  int32 y_pels_per_meter;
  uint32 clr_used;
  uint32 clr_important;

} PTCX_BITMAP_INFO_HEADER;

#pragma pack(pop)

inline uint32 greater_multiple(uint32 value, uint32 multiple) {
  uint32 mod = value % multiple;

  if (0 != mod) {
    value += multiple - mod;
  }

  return value;
}

/* Loads a 24 bit RGB bitmap file into an 8 bit image vector. */
bool LoadBitmapImage8(const string &filename, Image *output,
                      string *error = nullptr) {
  if (filename.empty() || !output) {
    if (error) {
      *error = "Invalid inputs to LoadBitmapImage.";
    }
    return false;
  }

  uint32 bytes_read = 0;
  PTCX_BITMAP_INFO_HEADER bih;
  PTCX_BITMAP_FILE_HEADER bmf_header;

  ifstream input_file(filename, ::std::ios::in | ::std::ios::binary);

  if (!input_file.read((char *)&bmf_header, sizeof(PTCX_BITMAP_FILE_HEADER))) {
    if (error) {
      *error = "Failed to read bitmap file header";
    }
    return false;
  }

  if (!input_file.read((char *)&bih, sizeof(PTCX_BITMAP_INFO_HEADER))) {
    if (error) {
      *error = "Failed to read bitmap info header.\n";
    }
    return false;
  }

  if (bih.bit_count != 24) {
    if (error) {
      *error = "Unsupported bitmap data format.\n";
    }
    return false;
  }

  uint32 image_row_pitch = bih.width * 3;
  uint32 image_size = image_row_pitch * bih.height;

  vector<uint8> bitmap_image24(image_size), bitmap_image8;
  bitmap_image8.reserve(bih.width * bih.height);

  /* The BMP format requires each scanline to be 32 bit aligned, so we insert
     padding if necessary. */
  uint32 scanline_padding =
      greater_multiple(bih.width * 3, 4) - (bih.width * 3);

  uint32 row_stride = bih.width * 3;

  for (uint32 i = 0; i < bih.height; i++) {
    uint32 y_offset = (bih.height - i - 1) * row_stride;
    uint8 *dest_row = &bitmap_image24.at(y_offset);

    if (!input_file.read((char *)dest_row, row_stride)) {
      if (error) {
        *error = "Abrupt error reading file.\n";
      }
      return false;
    }

    uint32 dummy = 0; /* Padding will always be < 4 bytes. */
    if (!input_file.read((char *)&dummy, scanline_padding)) {
      if (error) {
        *error = "Abrupt error reading file.\n";
      }
      return false;
    }

    /* Condense our RGB bitmap data down to single channel R. */
    for (uint32 j = 0; j < bih.width; j++) {
      uint32 x_offset = y_offset + j * 3;
      uint8 bitmap24_r = bitmap_image24.at(x_offset);
      bitmap_image8.push_back(bitmap24_r);
    }
  }

  /* Copy our temp into our output image. */
  output->data = bitmap_image8;
  output->width = bih.width;
  output->height = bih.height;

  return true;
}

bool SaveBitmapImage8(const string &filename, Image *input,
                      string *error = nullptr) {
  if (filename.empty() || !input || !input->width || !input->height) {
    if (error) {
      *error = "Invalid inputs to SaveBitmapImage.";
    }
    return false;
  }

  uint32 bytes_written = 0;
  uint32 total_image_bytes = (3 * input->width) * input->height;
  uint32 header_size =
      sizeof(PTCX_BITMAP_FILE_HEADER) + sizeof(PTCX_BITMAP_INFO_HEADER);

  PTCX_BITMAP_FILE_HEADER bmf_header = {0x4D42,  // BM
                                        header_size + total_image_bytes, 0, 0,
                                        header_size};

  PTCX_BITMAP_INFO_HEADER bih = {sizeof(PTCX_BITMAP_INFO_HEADER),
                                 input->width,
                                 input->height,
                                 1,
                                 24,
                                 BI_RGB,
                                 total_image_bytes,
                                 0,
                                 0,
                                 0,
                                 0};

  ofstream output_file(filename, ::std::ios::out | ::std::ios::binary);

  if (!output_file.write((char *)&bmf_header,
                         sizeof(PTCX_BITMAP_FILE_HEADER))) {
    if (error) {
      *error = "Failed to write bitmap file header";
    }
    return false;
  }

  if (!output_file.write((char *)&bih, sizeof(PTCX_BITMAP_INFO_HEADER))) {
    if (error) {
      *error = "Failed to write bitmap info header.\n";
    }
    return false;
  }

  /* The BMP format requires each scanline to be 32 bit aligned, so we insert
     padding if necessary. */
  uint32 scanline_padding =
      greater_multiple(bih.width * 3, 4) - (bih.width * 3);

  ::std::vector<uint8> one_texel_row(bih.width * 3);

  for (uint32 i = 0; i < bih.height; i++) {
    /* Expand our single channel input image into grayscale RGB24. */
    for (uint32 j = 0; j < bih.width; j++) {
      uint8 input_r = input->GetPixel(j, bih.height - i - 1);
      one_texel_row.at(j * 3 + 0) = input_r;
      one_texel_row.at(j * 3 + 1) = input_r;
      one_texel_row.at(j * 3 + 2) = input_r;
    }

    if (!output_file.write((char *)&one_texel_row.at(0), bih.width * 3)) {
      if (error) {
        *error = "Abrupt error writing file.\n";
      }
      return false;
    }

    uint32 dummy = 0; /* Padding will always be < 4 bytes. */
    if (!output_file.write((char *)&dummy, scanline_padding)) {
      if (error) {
        *error = "Abrupt error writing file.\n";
      }
      return false;
    }
  }

  return true;
}

}  // namespace base

#endif  // __BITMAP_H__