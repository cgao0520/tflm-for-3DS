/* Copyright 2019 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "image_provider.h"
#include "model_settings.h"

#include <3ds.h>

// preprocessed gray image
static uint8_t img_gray[kNumCols * kNumRows];

static int start_x = (WIDTH - kNumCols) / 2;
static int start_y = (HEIGHT - kNumRows) / 2;

uint8_t* getGrayImg()
{
  return img_gray;
}

// This function convert RGB image to gray image and get centralized 96 by 96 image
void preprocess(u8* cam_buf)
{
  int x, y;
  int k = 0;
  int pos;
  u16* img_16 = (u16*)cam_buf;
  u16 data; // pixel data
  for (y = start_y; y < start_y + kNumRows; y++)
  {
    for (x = start_x; x < start_x + kNumCols; x++)
    {
      pos = y * WIDTH + x;
      data = img_16[pos];
			uint8_t b = ((data >> 11) & 0x1F) << 3;
			uint8_t g = ((data >> 5) & 0x3F) << 2;
			uint8_t r = (data & 0x1F) << 3;
      int gray = (r * 0.3 + g * 0.59 + b * 0.11);
      if (gray > 255)
      {
        img_gray[k] = 255;
      }
      else
      {
        img_gray[k] = (uint8_t)gray;
      }
      k++;
    }
  }
}

TfLiteStatus GetImage(tflite::ErrorReporter* error_reporter, int image_width,
                      int image_height, int channels, int8_t* image_data)
{
  //preprocess();
  int8_t* img_int8 = (int8_t*)img_gray;
  for (int i = 0; i < image_width * image_height * channels; ++i)
  {
    image_data[i] = img_int8[i];
  }
  return kTfLiteOk;
}
