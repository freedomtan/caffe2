/**
 * Copyright (c) 2016-present, Facebook, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include "GLTexture.h"
#include "DataTransfer.h"
#include "GLPBO.h"

#include "caffe2/core/logging.h"
#include "caffe2/core/timer.h"

const GLTexture::Type GLTexture::FP16 = {GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT};
const GLTexture::Type GLTexture::UI8 = {GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE};
const GLTexture::Type GLTexture::FP16_COMPAT = {GL_RG32UI, GL_RG_INTEGER, GL_UNSIGNED_INT};

void GLTexture::map_read(std::function<void(const void* buffer,
                                            size_t width,
                                            size_t height,
                                            size_t stride,
                                            size_t channels,
                                            const Type& type)> process) const {
  GLPBO* pbo = GLPBO::getContext();
  pbo->mapTextureData(_textureId, _width, _height, _stride, _channels, _type, process);
}

void GLTexture::map_load(std::function<void(void* buffer,
                                            size_t width,
                                            size_t height,
                                            size_t stride,
                                            size_t channels,
                                            const Type& type)> process) const {
  const int alignment = 32; // 4 * _type.dataSize();
  void* buffer = nullptr;
  size_t buffer_size = _width * _height * _channels * _type.dataSize();

#ifdef __ANDROID__
  buffer = (void*)memalign(alignment, buffer_size);
#else
  posix_memalign((void**)&buffer, alignment, buffer_size);
#endif
  CAFFE_ENFORCE(buffer);

  process(buffer, _width, _height, _width, _channels, _type);
  loadData(buffer);
  free(buffer);
}

void GLTexture::loadData(const void* pixels) const {
  glBindTexture(GL_TEXTURE_2D, _textureId);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, _width, _height, _type.format, _type.type, pixels);
  glBindTexture(GL_TEXTURE_2D, 0);
}
