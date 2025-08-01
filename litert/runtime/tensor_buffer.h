// Copyright 2024 Google LLC.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef ODML_LITERT_LITERT_RUNTIME_TENSOR_BUFFER_H_
#define ODML_LITERT_LITERT_RUNTIME_TENSOR_BUFFER_H_

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#include "absl/container/flat_hash_map.h"  // from @com_google_absl
#include "absl/strings/string_view.h"  // from @com_google_absl
#include "absl/types/span.h"  // from @com_google_absl
#include "litert/c/litert_common.h"
#include "litert/c/litert_gl_types.h"
#include "litert/c/litert_layout.h"
#include "litert/c/litert_model.h"
#include "litert/c/litert_tensor_buffer.h"
#include "litert/c/litert_tensor_buffer_types.h"
#include "litert/cc/litert_expected.h"
#include "litert/runtime/custom_buffer.h"
#include "litert/runtime/event.h"
#include "litert/runtime/gl_buffer.h"
#include "litert/runtime/gl_texture.h"

#if LITERT_HAS_OPENCL_SUPPORT
#include "litert/runtime/open_cl_memory.h"
#include <CL/cl.h>
#endif  // LITERT_HAS_OPENCL_SUPPORT

namespace litert::internal {
class GpuEnvironment;
}  // namespace litert::internal

class LiteRtTensorBufferT {
 public:
  using Ptr = std::unique_ptr<LiteRtTensorBufferT>;

  ~LiteRtTensorBufferT();

  // Make this class non-copiable because it includes raw pointers and resource
  // handles.
  LiteRtTensorBufferT(const LiteRtTensorBufferT&) = delete;
  LiteRtTensorBufferT(LiteRtTensorBufferT&&) = delete;
  LiteRtTensorBufferT& operator=(const LiteRtTensorBufferT&) = delete;
  LiteRtTensorBufferT& operator=(LiteRtTensorBufferT&&) = delete;

  static litert::Expected<Ptr> CreateFromHostMemory(
      const LiteRtRankedTensorType& tensor_type,
      absl::Span<uint8_t> host_memory,
      LiteRtHostMemoryDeallocator deallocator = nullptr);

  static litert::Expected<Ptr> CreateFromAhwb(
      LiteRtEnvironment env, const LiteRtRankedTensorType& tensor_type,
      AHardwareBuffer* ahwb, size_t ahwb_offset,
      LiteRtAhwbDeallocator deallocator = nullptr);

  static litert::Expected<Ptr> CreateFromIonBuffer(
      const LiteRtRankedTensorType& tensor_type, void* ion_buffer_addr,
      int ion_buffer_fd, size_t ion_buffer_size, size_t ion_buffer_offset,
      LiteRtIonDeallocator deallocator = nullptr);

  static litert::Expected<Ptr> CreateFromDmaBufBuffer(
      const LiteRtRankedTensorType& tensor_type, void* dmabuf_buffer_addr,
      int dmabuf_buffer_fd, size_t dmabuf_buffer_size,
      size_t dmabuf_buffer_offset,
      LiteRtDmaBufDeallocator deallocator = nullptr);

  static litert::Expected<Ptr> CreateFromFastRpcBuffer(
      const LiteRtRankedTensorType& tensor_type, void* fastrpc_buffer_addr,
      int fastrpc_buffer_fd, size_t fastrpc_buffer_size,
      size_t fastrpc_buffer_offset,
      LiteRtFastRpcDeallocator deallocator = nullptr);

  static litert::Expected<Ptr> CreateFromGlBuffer(
      LiteRtEnvironment env, const LiteRtRankedTensorType& tensor_type,
      LiteRtGLenum target, LiteRtGLuint id, size_t size_bytes, size_t offset,
      LiteRtGlBufferDeallocator deallocator = nullptr);

  static litert::Expected<Ptr> CreateFromGlTexture(
      LiteRtEnvironment env, const LiteRtRankedTensorType& tensor_type,
      LiteRtGLenum target, LiteRtGLuint id, LiteRtGLenum format,
      size_t size_bytes, LiteRtGLint layer,
      LiteRtGlTextureDeallocator deallocator = nullptr);

  static litert::Expected<Ptr> CreateManaged(
      LiteRtEnvironment env, LiteRtTensorBufferType buffer_type,
      const LiteRtRankedTensorType& tensor_type, size_t buffer_size);

#if LITERT_HAS_OPENCL_SUPPORT
  static litert::Expected<Ptr> CreateFromOpenClMemory(
      LiteRtEnvironment env, const LiteRtRankedTensorType& tensor_type,
      LiteRtTensorBufferType buffer_type, cl_mem buffer,
      size_t opencl_buffer_size, LiteRtOpenClDeallocator deallocator = nullptr);
#endif  // LITERT_HAS_OPENCL_SUPPORT

  LiteRtRankedTensorType tensor_type() const { return tensor_type_; }
  LiteRtTensorBufferType buffer_type() const { return buffer_type_; }

  size_t packed_buffer_size() const { return packed_buffer_size_; }
  size_t buffer_size() const { return buffer_size_; }
  size_t buffer_offset() const { return buffer_offset_; }

  bool is_opencl_memory() const { return IsOpenClMemory(buffer_type_); }

  bool HasEvent() const { return event_ != nullptr; }

  litert::Expected<LiteRtEventT*> GetEvent() const {
    if (!HasEvent()) {
      return litert::Error(kLiteRtStatusErrorRuntimeFailure,
                           "TensorBuffer has no event");
    }
    return event_.get();
  }

  void SetEvent(LiteRtEventT* e) {
    // Take ownership of the event.
    event_ = std::unique_ptr<LiteRtEventT>(e);
  }
  void ClearEvent() { event_ = nullptr; }

  litert::Expected<void*> GetHostBuffer();
  litert::Expected<AHardwareBuffer*> GetAhwbBuffer();
  litert::Expected<std::pair<void*, int>> GetIonBuffer();
  litert::Expected<std::pair<void*, int>> GetDmaBufBuffer();
  litert::Expected<std::pair<void*, int>> GetFastRpcBuffer();
  litert::Expected<litert::internal::GlBuffer*> GetGlBuffer();
  litert::Expected<litert::internal::GlTexture*> GetGlTexture();
#if LITERT_HAS_OPENCL_SUPPORT
  litert::Expected<litert::internal::OpenClMemory*> GetOpenClMemory();
#endif  // LITERT_HAS_OPENCL_SUPPORT
  litert::Expected<litert::internal::CustomBuffer*> GetCustomBuffer();

  litert::Expected<void*> Lock(LiteRtTensorBufferLockMode mode);
  litert::Expected<void> Unlock();

  // Used to duplicate the current tensor buffer. Internally it increases
  // reference count to the underlying buffer.
  void Duplicate() const { Ref(); }

  // Increments reference count by one.
  void Ref() const { ref_.fetch_add(1, std::memory_order_relaxed); }

  // Decrements reference count by one.  If the count remains
  // positive, returns false.  When the count reaches zero, returns
  // true.
  bool Unref() const {
    if (ref_.fetch_sub(1, std::memory_order_acq_rel) == 1) {
      return true;
    }
    return false;
  }

  // Gets the current reference count.
  int RefCount() const { return ref_.load(std::memory_order_relaxed); }

 private:
  struct HostBuffer {
    void* addr;
    LiteRtHostMemoryDeallocator deallocator;
  };

  struct AhwbBuffer {
    AHardwareBuffer* ahwb;
    LiteRtAhwbDeallocator deallocator;
  };

  struct IonBuffer {
    void* addr;
    int fd;
    LiteRtIonDeallocator deallocator;
  };

  struct DmaBufBuffer {
    void* addr;
    int fd;
    LiteRtDmaBufDeallocator deallocator;
  };

  struct FastRpcBuffer {
    void* addr;
    int fd;
    LiteRtFastRpcDeallocator deallocator;
  };

  using BufferVariant =
      std::variant<HostBuffer, AhwbBuffer, IonBuffer, DmaBufBuffer,
                   FastRpcBuffer,
#if LITERT_HAS_OPENCL_SUPPORT
                   litert::internal::OpenClMemory,
#endif  // LITERT_HAS_OPENCL_SUPPORT
                   litert::internal::CustomBuffer, litert::internal::GlBuffer,
                   litert::internal::GlTexture>;

  LiteRtTensorBufferT(LiteRtEnvironment env,
                      const LiteRtRankedTensorType& tensor_type,
                      LiteRtTensorBufferType buffer_type, size_t buffer_size,
                      size_t buffer_offset = 0);

  static litert::Expected<Ptr> CreateManagedOnHostMemory(
      const LiteRtRankedTensorType& tensor_type, size_t buffer_size);

  static litert::Expected<Ptr> CreateManagedAhwbBuffer(
      LiteRtEnvironment env, const LiteRtRankedTensorType& tensor_type,
      size_t buffer_size);

  static litert::Expected<Ptr> CreateManagedIonBuffer(
      const LiteRtRankedTensorType& tensor_type, size_t buffer_size);

  static litert::Expected<Ptr> CreateManagedDmaBufBuffer(
      const LiteRtRankedTensorType& tensor_type, size_t buffer_size);

  static litert::Expected<Ptr> CreateManagedFastRpcBuffer(
      const LiteRtRankedTensorType& tensor_type, size_t buffer_size);

  static litert::Expected<Ptr> CreateManagedOpenClMemory(
      LiteRtEnvironment env, const LiteRtRankedTensorType& tensor_type,
      LiteRtTensorBufferType buffer_type, size_t buffer_size);

  static litert::Expected<Ptr> CreateManagedGlBuffer(
      LiteRtEnvironment env, const LiteRtRankedTensorType& tensor_type,
      size_t buffer_size);

  static litert::Expected<Ptr> CreateManagedWebGpuBuffer(
      LiteRtEnvironment env, const LiteRtRankedTensorType& tensor_type,
      LiteRtTensorBufferType buffer_type, size_t buffer_size);

  litert::Expected<void> IsValid();

  LiteRtEnvironment env_;
  LiteRtRankedTensorType tensor_type_;
  std::vector<std::decay_t<decltype(LiteRtLayout::dimensions[0])>> dimensions_;
  std::vector<std::decay_t<decltype(LiteRtLayout::strides[0])>> strides_;
  LiteRtTensorBufferType buffer_type_;
  size_t buffer_size_;
  size_t buffer_offset_;
  size_t packed_buffer_size_;
  BufferVariant buffer_;
  std::unique_ptr<LiteRtEventT> event_;
  mutable std::atomic_int_fast32_t ref_;
  // A map of memory backed buffers. Memory backed buffers are backed by the
  // memory of buffer_. For example, a GL buffer can be backed by the memory of
  // an AHWB buffer.
  absl::flat_hash_map<LiteRtTensorBufferType, BufferVariant>
      memory_backed_buffers_;
  bool is_locked_ = false;
};

#endif  // ODML_LITERT_LITERT_RUNTIME_TENSOR_BUFFER_H_
