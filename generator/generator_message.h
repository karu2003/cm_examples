// Copyright 2022 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef GENERATOR_MESSAGE_H_
#define GENERATOR_MESSAGE_H_

#include "libs/base/ipc_message_buffer.h"

// [start-sphinx-snippet:ipc-message]
enum class GeneratorMessageType : uint8_t {
  kSetStatus,
  kAck,
};

enum class FreqType : uint32_t { linear = 0, quad, log, sin, triangle };

typedef struct {
  float Samlerate;
  float Duration;
  float F0;
  float F1;
  float amp;
  float phi;
  FreqType TypeF;
  uint8_t AutoRestart;
  uint8_t RunBack;
  uint8_t StartDAC;
} GeneratorSettings;

typedef struct {
  int32_t sample_rate_hz;
  int32_t sample_format;
  int32_t dma_buffer_size_ms;
  int32_t num_dma_buffers;
  int32_t drop_first_samples_ms;
} AudioSettings;

struct GeneratorAppMessage {
  GeneratorMessageType type;
  GeneratorSettings Settings;
} __attribute__((packed));

typedef struct __attribute__((packed)){
  AudioSettings ASettings;
  GeneratorSettings GSettings;
} SocketMessage;

// [end-sphinx-snippet:ipc-message]

static_assert(sizeof(GeneratorAppMessage) <=
              coralmicro::kIpcMessageBufferDataSize);

#endif  // GENERATOR_MESSAGE_H_
