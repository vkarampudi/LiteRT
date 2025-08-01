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

#include "tflite/core/c/common.h"

#ifndef TF_LITE_STATIC_MEMORY
#include <cstdlib>
#endif  // TF_LITE_STATIC_MEMORY

#include <cstring>
#include <type_traits>
#include <utility>

#include "tflite/core/c/c_api_types.h"
#ifdef TF_LITE_TENSORFLOW_PROFILER
#include "tflite/tensorflow_profiler_logger.h"
#endif

namespace {

template <class T>
size_t TfLiteVarArrayGetSizeInBytes(const int size) {
  constexpr size_t data_size = sizeof(std::declval<T>().data[0]);
  size_t computed_size = sizeof(T) + data_size * size;
#if defined(_MSC_VER)
  // Context for why this is needed is in http://b/189926408#comment21
  computed_size -= data_size;
#endif
  return computed_size;
}

template <class T, class U>
int TfLiteVarArrayEqualsArray(const T* const a, const int b_size,
                              const U* const b_data) {
  static_assert(std::is_same<decltype(a->data[0]), const U&>::value,
                "TfLiteVarArrayEqualsArray can only compare same type arrays");
  if (a == nullptr) {
    return b_size == 0;
  }
  if (a->size != b_size) {
    return 0;
  }
  return !memcmp(a->data, b_data, a->size * sizeof(a->data[0]));
}

template <class T>
int TfLiteVarArrayEqual(const T* const a, const T* const b) {
  // This goes first because null arrays must compare equal.
  if (a == b) {
    return 1;
  }
  if (a == nullptr || b == nullptr) {
    return 0;
  }
  return TfLiteVarArrayEqualsArray(a, b->size, b->data);
}

#ifndef TF_LITE_STATIC_MEMORY

template <class T>
T* TfLiteVarArrayCreate(const int size) {
  const size_t alloc_size = TfLiteVarArrayGetSizeInBytes<T>(size);
  if (alloc_size <= 0) {
    return nullptr;
  }
  T* ret = (T*)malloc(alloc_size);
  if (!ret) {
    return nullptr;
  }
  ret->size = size;
  return ret;
}

template <class T>
T* TfLiteVarArrayCopy(const T* const src) {
  if (!src) {
    return nullptr;
  }
  T* const ret = TfLiteVarArrayCreate<T>(src->size);
  if (ret) {
    memcpy(ret->data, src->data, src->size * sizeof(src->data[0]));
  }
  return ret;
}

#endif  // TF_LITE_STATIC_MEMORY

template <class T>
void TfLiteVarArrayFree(T* a) {
  free(a);
}

#ifndef TF_LITE_STATIC_MEMORY

TfLiteSparsity TfLiteSparsityClone(const TfLiteSparsity& src) {
  TfLiteSparsity dst = src;
  dst.traversal_order = TfLiteIntArrayCopy(src.traversal_order);
  dst.block_map = TfLiteIntArrayCopy(src.block_map);
  if (src.dim_metadata) {
    dst.dim_metadata = reinterpret_cast<TfLiteDimensionMetadata*>(
        calloc(1, sizeof(TfLiteDimensionMetadata) * src.dim_metadata_size));
    for (int i = 0; i < src.dim_metadata_size; ++i) {
      dst.dim_metadata[i] = src.dim_metadata[i];
      dst.dim_metadata[i].array_segments =
          TfLiteIntArrayCopy(src.dim_metadata[i].array_segments);
      dst.dim_metadata[i].array_indices =
          TfLiteIntArrayCopy(src.dim_metadata[i].array_indices);
    }
  }
  return dst;
}

// Clones the source sparsity to a newly allocated object.
TfLiteSparsity* TfLiteSparsityClone(const TfLiteSparsity* const src) {
  if (!src) {
    return nullptr;
  }
  TfLiteSparsity* dst =
      reinterpret_cast<TfLiteSparsity*>(calloc(1, sizeof(TfLiteSparsity)));
  *dst = TfLiteSparsityClone(*src);
  return dst;
}

#endif  // TF_LITE_STATIC_MEMORY

}  // namespace

#ifndef TF_LITE_STATIC_MEMORY

TfLiteQuantization TfLiteQuantizationClone(const TfLiteQuantization& src) {
  TfLiteQuantization dst;
  dst.type = src.type;
  switch (src.type) {
    case kTfLiteNoQuantization:
      break;
    case kTfLiteAffineQuantization: {
      dst.params = calloc(1, sizeof(TfLiteAffineQuantization));
      const TfLiteAffineQuantization* const src_params =
          reinterpret_cast<TfLiteAffineQuantization*>(src.params);
      TfLiteAffineQuantization* const dst_params =
          reinterpret_cast<TfLiteAffineQuantization*>(dst.params);
      dst_params->quantized_dimension = src_params->quantized_dimension;
      dst_params->scale = TfLiteFloatArrayCopy(src_params->scale);
      dst_params->zero_point = TfLiteIntArrayCopy(src_params->zero_point);
      break;
    }
    case kTfLiteBlockwiseQuantization: {
      dst.params = calloc(1, sizeof(TfLiteBlockwiseQuantization));
      const TfLiteBlockwiseQuantization* const src_params =
          (TfLiteBlockwiseQuantization*)(src.params);
      TfLiteBlockwiseQuantization* const dst_params =
          (TfLiteBlockwiseQuantization*)(dst.params);
      dst_params->blocksize = src_params->blocksize;
      dst_params->scale = src_params->scale;
      dst_params->zero_point = src_params->zero_point;
      break;
    }
  }
  return dst;
}

#endif  // TF_LITE_STATIC_MEMORY

extern "C" {

size_t TfLiteIntArrayGetSizeInBytes(int size) {
  return TfLiteVarArrayGetSizeInBytes<TfLiteIntArray>(size);
}

int TfLiteIntArrayEqual(const TfLiteIntArray* a, const TfLiteIntArray* b) {
  return TfLiteVarArrayEqual(a, b);
}

int TfLiteIntArrayEqualsArray(const TfLiteIntArray* a, int b_size,
                              const int b_data[]) {
  return TfLiteVarArrayEqualsArray(a, b_size, b_data);
}

#ifndef TF_LITE_STATIC_MEMORY

TfLiteIntArray* TfLiteIntArrayCreate(int size) {
  return TfLiteVarArrayCreate<TfLiteIntArray>(size);
}

TfLiteIntArray* TfLiteIntArrayCopy(const TfLiteIntArray* src) {
  return TfLiteVarArrayCopy(src);
}

void TfLiteIntArrayFree(TfLiteIntArray* a) { TfLiteVarArrayFree(a); }

#endif  // TF_LITE_STATIC_MEMORY

int TfLiteFloatArrayGetSizeInBytes(int size) {
  return TfLiteVarArrayGetSizeInBytes<TfLiteFloatArray>(size);
}

#ifndef TF_LITE_STATIC_MEMORY

TfLiteFloatArray* TfLiteFloatArrayCreate(int size) {
  return TfLiteVarArrayCreate<TfLiteFloatArray>(size);
}

TfLiteFloatArray* TfLiteFloatArrayCopy(const TfLiteFloatArray* src) {
  return TfLiteVarArrayCopy(src);
}

void TfLiteFloatArrayFree(TfLiteFloatArray* a) { TfLiteVarArrayFree(a); }

void TfLiteTensorDataFree(TfLiteTensor* t) {
  if (t->allocation_type == kTfLiteVariantObject && t->data.data) {
    delete static_cast<VariantData*>(t->data.data);
  } else if (t->allocation_type == kTfLiteDynamic ||
             t->allocation_type == kTfLitePersistentRo) {
    if (t->data.raw) {
#ifdef TF_LITE_TENSORFLOW_PROFILER
      tflite::PauseHeapMonitoring(/*pause=*/true);
      tflite::OnTfLiteTensorDealloc(t);
#endif
      free(t->data.raw);
#ifdef TF_LITE_TENSORFLOW_PROFILER
      tflite::PauseHeapMonitoring(/*pause=*/false);
#endif
    }
  }
  t->data.raw = nullptr;
}

void TfLiteQuantizationFree(TfLiteQuantization* quantization) {
  if (quantization->type == kTfLiteAffineQuantization) {
    TfLiteAffineQuantization* q_params =
        reinterpret_cast<TfLiteAffineQuantization*>(quantization->params);
    if (q_params->scale) {
      TfLiteFloatArrayFree(q_params->scale);
      q_params->scale = nullptr;
    }
    if (q_params->zero_point) {
      TfLiteIntArrayFree(q_params->zero_point);
      q_params->zero_point = nullptr;
    }
    free(q_params);
  }
  if (quantization->type == kTfLiteBlockwiseQuantization) {
    TfLiteBlockwiseQuantization* q_params =
        reinterpret_cast<TfLiteBlockwiseQuantization*>(quantization->params);
    free(q_params);
  }
  quantization->params = nullptr;
  quantization->type = kTfLiteNoQuantization;
}

void TfLiteSparsityFree(TfLiteSparsity* sparsity) {
  if (sparsity == nullptr) {
    return;
  }

  if (sparsity->traversal_order) {
    TfLiteIntArrayFree(sparsity->traversal_order);
    sparsity->traversal_order = nullptr;
  }

  if (sparsity->block_map) {
    TfLiteIntArrayFree(sparsity->block_map);
    sparsity->block_map = nullptr;
  }

  if (sparsity->dim_metadata) {
    int i = 0;
    for (; i < sparsity->dim_metadata_size; i++) {
      TfLiteDimensionMetadata metadata = sparsity->dim_metadata[i];
      if (metadata.format == kTfLiteDimSparseCSR) {
        TfLiteIntArrayFree(metadata.array_segments);
        metadata.array_segments = nullptr;
        TfLiteIntArrayFree(metadata.array_indices);
        metadata.array_indices = nullptr;
      }
    }
    free(sparsity->dim_metadata);
    sparsity->dim_metadata = nullptr;
  }

  free(sparsity);
}

void TfLiteTensorFree(TfLiteTensor* t) {
  TfLiteTensorDataFree(t);
  if (t->dims) TfLiteIntArrayFree(t->dims);
  t->dims = nullptr;

  if (t->dims_signature) {
    TfLiteIntArrayFree((TfLiteIntArray*)t->dims_signature);
  }
  t->dims_signature = nullptr;

  TfLiteQuantizationFree(&t->quantization);
  TfLiteSparsityFree(t->sparsity);
  t->sparsity = nullptr;
}

TfLiteTensor TfLiteTensorClone(const TfLiteTensor src) {
  // We copy all of the source data first, then we clone the fields that can't
  // be shared between two tensor instances.
  TfLiteTensor dst = src;
  // Data that is owned by the original tensor mut be cloned. Check
  // TfLiteTensorFree to find out which members are owned.
  if (src.data.data) {
    const TfLiteAllocationStrategy allocation_strategy =
        TfLiteTensorGetAllocationStrategy(&src);
    switch (allocation_strategy) {
      case kTfLiteAllocationStrategyUnknown:
        // We don't know the allocation strategy, which means that the tensor
        // doesn't own its data: we keep the copied pointer to the data.
        break;
      case kTfLiteAllocationStrategyNone:
        break;
      case kTfLiteAllocationStrategyMMap:
        // Mmapped data is read-only and external to the interpreter. We keep
        // the copied pointer to the data.
        break;
      case kTfLiteAllocationStrategyArena:
        // Arena tensors are allocated when the graph is prepared. There is no
        // data associated to such a tensor between runs so we don't care about
        // the value of `data`.
        break;
      case kTfLiteAllocationStrategyMalloc:
        dst.data.data = malloc(src.bytes);
        std::memcpy(dst.data.data, src.data.data, src.bytes);
        break;
      case kTfLiteAllocationStrategyNew:
        // Special case for variant objects. They are allocated using new/delete
        // but require using the `CloneTo` function.
        if (src.allocation_type == kTfLiteVariantObject) {
          dst.data.data = reinterpret_cast<const VariantData*>(src.data.data)
                              ->CloneTo(nullptr);
        } else {
          dst.data.data = new char[src.bytes];
          std::memcpy(dst.data.data, src.data.data, src.bytes);
        }
        break;
    }
  }
  dst.dims = TfLiteIntArrayCopy(src.dims);
  dst.dims_signature = TfLiteIntArrayCopy(src.dims_signature);
  dst.quantization = TfLiteQuantizationClone(src.quantization);
  dst.sparsity = TfLiteSparsityClone(src.sparsity);
  return dst;
}

void TfLiteTensorReset(TfLiteType type, const char* name, TfLiteIntArray* dims,
                       TfLiteQuantizationParams quantization, char* buffer,
                       size_t size, TfLiteAllocationType allocation_type,
                       const void* allocation, bool is_variable,
                       TfLiteTensor* tensor) {
  TfLiteTensorFree(tensor);
  tensor->type = type;
  tensor->name = name;
  tensor->dims = dims;
  tensor->params = quantization;
  tensor->data.raw = buffer;
  tensor->bytes = size;
  tensor->allocation_type = allocation_type;
  tensor->allocation = allocation;
  tensor->is_variable = is_variable;

  tensor->quantization.type = kTfLiteNoQuantization;
  tensor->quantization.params = nullptr;
}

TfLiteStatus TfLiteTensorCopy(const TfLiteTensor* src, TfLiteTensor* dst) {
  if (!src || !dst) return kTfLiteOk;
  if (src->bytes != dst->bytes) return kTfLiteError;
  if (src == dst) return kTfLiteOk;
  dst->type = src->type;
  if (dst->dims) TfLiteIntArrayFree(dst->dims);
  dst->dims = TfLiteIntArrayCopy(src->dims);
  if (src->allocation_type == kTfLiteVariantObject) {
    // An edge case exists in control flow ops when they copy inputs to outputs
    // before invoking any body, in this case the `dst` will not have its
    // `allocation_type` set properly, so we handle here for now.
    if (dst->allocation_type != kTfLiteVariantObject) {
      TfLiteTensorDataFree(dst);
      dst->allocation_type = kTfLiteVariantObject;
    }
    auto* dst_vd = static_cast<VariantData*>(dst->data.data);
    auto* src_vd = static_cast<VariantData*>(src->data.data);

    // `CloneTo` will handle the case when `dst_vd` is nullptr, so it is safe
    // to `CloneTo` something which was "freed". Also, returning from `CloneTo`
    // will implicitly cast to `VariantData`; don't need static cast here.
    dst->data.data = src_vd->CloneTo(dst_vd);
  } else {
    memcpy(dst->data.raw, src->data.raw, src->bytes);
  }
  dst->buffer_handle = src->buffer_handle;
  dst->data_is_stale = src->data_is_stale;
  dst->delegate = src->delegate;

  return kTfLiteOk;
}

TfLiteStatus TfLiteTensorResizeMaybeCopy(size_t num_bytes, TfLiteTensor* tensor,
                                         bool preserve_data) {
  if (tensor->allocation_type != kTfLiteDynamic &&
      tensor->allocation_type != kTfLitePersistentRo) {
    return kTfLiteOk;
  }
#ifdef TF_LITE_TENSORFLOW_PROFILER
  tflite::PauseHeapMonitoring(/*pause=*/true);
#endif
  // This buffer may be consumed by XNNPack.
  size_t alloc_bytes = num_bytes + /*XNN_EXTRA_BYTES=*/16;
  // TODO(b/145340303): Tensor data should be aligned.
  if (!tensor->data.data) {
    tensor->data.data = (char*)malloc(alloc_bytes);
#ifdef TF_LITE_TENSORFLOW_PROFILER
    tflite::OnTfLiteTensorAlloc(tensor, alloc_bytes);
#endif
  } else if (num_bytes > tensor->bytes) {
#ifdef TF_LITE_TENSORFLOW_PROFILER
    tflite::OnTfLiteTensorDealloc(tensor);
#endif
    if (preserve_data) {
      tensor->data.data = (char*)realloc(tensor->data.data, alloc_bytes);
    } else {
      // Calling free and malloc can be more efficient as it avoids needlessly
      // copying the data when it is not required.
      free(tensor->data.data);
      tensor->data.data = (char*)malloc(alloc_bytes);
    }
#ifdef TF_LITE_TENSORFLOW_PROFILER
    tflite::OnTfLiteTensorAlloc(tensor, alloc_bytes);
#endif
  }
#ifdef TF_LITE_TENSORFLOW_PROFILER
  tflite::PauseHeapMonitoring(/*pause=*/false);
#endif
  tensor->bytes = num_bytes;
  if (tensor->data.data == nullptr && num_bytes != 0) {
    // We are done allocating but tensor is pointing to null and a valid size
    // was requested, so we error.
    return kTfLiteError;
  }
  return kTfLiteOk;
}

TfLiteStatus TfLiteTensorRealloc(size_t num_bytes, TfLiteTensor* tensor) {
  return TfLiteTensorResizeMaybeCopy(num_bytes, tensor, true);
}

const TfLiteIntArray* TfLiteTensorGetDimsSignature(const TfLiteTensor* t) {
  if (t->dims_signature != nullptr && t->dims_signature->size != 0) {
    return t->dims_signature;
  } else {
    return t->dims;
  }
}
#endif  // TF_LITE_STATIC_MEMORY

const char* TfLiteTypeGetName(TfLiteType type) {
  switch (type) {
    case kTfLiteNoType:
      return "NOTYPE";
    case kTfLiteFloat32:
      return "FLOAT32";
    case kTfLiteUInt16:
      return "UINT16";
    case kTfLiteInt16:
      return "INT16";
    case kTfLiteInt32:
      return "INT32";
    case kTfLiteUInt32:
      return "UINT32";
    case kTfLiteUInt8:
      return "UINT8";
    case kTfLiteInt8:
      return "INT8";
    case kTfLiteInt64:
      return "INT64";
    case kTfLiteUInt64:
      return "UINT64";
    case kTfLiteBool:
      return "BOOL";
    case kTfLiteComplex64:
      return "COMPLEX64";
    case kTfLiteComplex128:
      return "COMPLEX128";
    case kTfLiteString:
      return "STRING";
    case kTfLiteFloat16:
      return "FLOAT16";
    case kTfLiteBFloat16:
      return "BFLOAT16";
    case kTfLiteFloat64:
      return "FLOAT64";
    case kTfLiteResource:
      return "RESOURCE";
    case kTfLiteVariant:
      return "VARIANT";
    case kTfLiteInt4:
      return "INT4";
  }
  return "Unknown type";
}

TfLiteDelegate TfLiteDelegateCreate() { return TfLiteDelegate{}; }

// Returns a tensor data allocation strategy.
TfLiteAllocationStrategy TfLiteTensorGetAllocationStrategy(
    const TfLiteTensor* const t) {
  switch (t->allocation_type) {
    case kTfLiteMemNone:
      return kTfLiteAllocationStrategyNone;
    case kTfLiteMmapRo:
      return kTfLiteAllocationStrategyMMap;
    case kTfLiteArenaRw:
      return kTfLiteAllocationStrategyArena;
    case kTfLiteArenaRwPersistent:
      return kTfLiteAllocationStrategyArena;
    case kTfLiteDynamic:
      return kTfLiteAllocationStrategyMalloc;
    case kTfLitePersistentRo:
      return kTfLiteAllocationStrategyMalloc;
    case kTfLiteCustom:
      return kTfLiteAllocationStrategyUnknown;
    case kTfLiteVariantObject:
      return kTfLiteAllocationStrategyNew;
    case kTfLiteNonCpu:
      return kTfLiteAllocationStrategyUnknown;
  }
  return kTfLiteAllocationStrategyUnknown;
}

// Returns how stable a tensor data buffer address is across runs.
TfLiteRunStability TfLiteTensorGetBufferAddressStability(
    const TfLiteTensor* const t) {
  switch (t->allocation_type) {
    case kTfLiteMemNone:
      return kTfLiteRunStabilityAcrossRuns;
    case kTfLiteMmapRo:
      return kTfLiteRunStabilityAcrossRuns;
    case kTfLiteArenaRw:
      return kTfLiteRunStabilityUnstable;
    case kTfLiteArenaRwPersistent:
      return kTfLiteRunStabilityUnstable;
    case kTfLiteDynamic:
      return kTfLiteRunStabilitySingleRun;
    case kTfLitePersistentRo:
      return kTfLiteRunStabilitySingleRun;
    case kTfLiteCustom:
      return kTfLiteRunStabilityUnknown;
    case kTfLiteVariantObject:
      return kTfLiteRunStabilityAcrossRuns;
    case kTfLiteNonCpu:
      return kTfLiteRunStabilityUnknown;
  }
  return kTfLiteRunStabilityUnknown;
}

// Returns how stable a tensor data values are across runs.
TfLiteRunStability TfLiteTensorGetDataStability(const TfLiteTensor* const t) {
  switch (t->allocation_type) {
    case kTfLiteMemNone:
      return kTfLiteRunStabilityAcrossRuns;
    case kTfLiteMmapRo:
      return kTfLiteRunStabilityAcrossRuns;
    case kTfLiteArenaRw:
      return kTfLiteRunStabilitySingleRun;
    case kTfLiteArenaRwPersistent:
      return kTfLiteRunStabilityAcrossRuns;
    case kTfLiteDynamic:
      return kTfLiteRunStabilitySingleRun;
    case kTfLitePersistentRo:
      return kTfLiteRunStabilitySingleRun;
    case kTfLiteCustom:
      return kTfLiteRunStabilityUnknown;
    case kTfLiteVariantObject:
      return kTfLiteRunStabilitySingleRun;
    case kTfLiteNonCpu:
      return kTfLiteRunStabilityUnknown;
  }
  return kTfLiteRunStabilityUnknown;
}

// Returns the operation step when the data of a tensor is populated.
//
// Some operations can precompute their results before the evaluation step. This
// makes the data available earlier for subsequent operations.
TfLiteRunStep TfLiteTensorGetDataKnownStep(const TfLiteTensor* t) {
  switch (t->allocation_type) {
    case kTfLiteMemNone:
      return kTfLiteRunStepInit;
    case kTfLiteMmapRo:
      return kTfLiteRunStepInit;
    case kTfLiteArenaRw:
      return kTfLiteRunStepEval;
    case kTfLiteArenaRwPersistent:
      return kTfLiteRunStepEval;
    case kTfLiteDynamic:
      return kTfLiteRunStepEval;
    case kTfLitePersistentRo:
      return kTfLiteRunStepPrepare;
    case kTfLiteCustom:
      return kTfLiteRunStepUnknown;
    case kTfLiteVariantObject:
      return kTfLiteRunStepEval;
    case kTfLiteNonCpu:
      return kTfLiteRunStepUnknown;
  }
  return kTfLiteRunStepUnknown;
}

// Returns the operation step when the shape of a tensor is computed.
//
// Some operations can precompute the shape of their results before the
// evaluation step. This makes the shape available earlier for subsequent
// operations.
TfLiteRunStep TfLiteTensorGetShapeKnownStep(const TfLiteTensor* t) {
  switch (t->allocation_type) {
    case kTfLiteMemNone:
      return kTfLiteRunStepInit;
    case kTfLiteMmapRo:
      return kTfLiteRunStepInit;
    case kTfLiteArenaRw:
      return kTfLiteRunStepPrepare;
    case kTfLiteArenaRwPersistent:
      return kTfLiteRunStepPrepare;
    case kTfLiteDynamic:
      return kTfLiteRunStepEval;
    case kTfLitePersistentRo:
      return kTfLiteRunStepPrepare;
    case kTfLiteCustom:
      return kTfLiteRunStepUnknown;
    case kTfLiteVariantObject:
      return kTfLiteRunStepEval;
    case kTfLiteNonCpu:
      return kTfLiteRunStepUnknown;
  }
  return kTfLiteRunStepUnknown;
}

// Returns a sentinel value to be used as the user_data field of a TfLiteNode
// when the kernel initialization fails.
void* TfLiteKernelInitFailed() { return reinterpret_cast<void*>(-1); }

}  // extern "C"
