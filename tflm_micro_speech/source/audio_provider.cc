/* Copyright 2018 The TensorFlow Authors. All Rights Reserved.

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

#include "audio_provider.h"

#include "micro_features/micro_model_settings.h"
#include "3ds.h"
#include <stdio.h>
extern u8 *audiobuf;
extern u32 audiobuf_size;
extern u32 audiobuf_pos;

namespace {
static int16_t g_output_audio_data[kMaxAudioSampleSize] = {0};
static int32_t g_latest_audio_timestamp = 0;
}  // namespace

TfLiteStatus GetAudioSamples(tflite::ErrorReporter* error_reporter,
                             int start_ms,
                             int duration_ms,
                             int* audio_samples_size,
                             int16_t** audio_samples)
{
  int start_offset = start_ms * (kAudioSampleFrequency / 1000);
  const int duration_sample_count = duration_ms * (kAudioSampleFrequency / 1000);

  int capture_index;
  // Since the audiobuf declared in main.cc is a u8 array, but the model requires u16 type,
  // thus we convert the pointer and the size accordingly.
  int16_t* audioBuf16 = (int16_t*)audiobuf;
  size_t audioBufSize16 = audiobuf_size * 0.5;
  for (int i = 0; i < duration_sample_count/* typically 480 */; ++i)
  {
    capture_index = (start_offset + i) % audioBufSize16;
    g_output_audio_data[i] = audioBuf16[capture_index];
  }
  *audio_samples_size = kMaxAudioSampleSize/*512*/;
  *audio_samples = g_output_audio_data;
  return kTfLiteOk;
}

int32_t LatestAudioTimestamp() {
  static u32 last_audiobuf_pos = audiobuf_pos;
  u32 diff_pos = 0;
  if (last_audiobuf_pos > audiobuf_pos) // audiobuf rounded back
  {
    diff_pos = audiobuf_size - last_audiobuf_pos + audiobuf_pos;
  }
  else
  {
    diff_pos = audiobuf_pos - last_audiobuf_pos;
  }

  last_audiobuf_pos = audiobuf_pos;
  g_latest_audio_timestamp += diff_pos * 0.5 / (kAudioSampleFrequency / 1000);
  return g_latest_audio_timestamp;
}
