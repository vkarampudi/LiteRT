// Copyright 2025 Google LLC.
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

#include "litert/runtime/tensor_buffer_registry.h"

#include "absl/strings/str_cat.h"  // from @com_google_absl
#include "litert/c/litert_common.h"
#include "litert/c/litert_tensor_buffer_types.h"
#include "litert/cc/litert_expected.h"
#include "litert/cc/litert_tensor_buffer_utils.h"

namespace litert {
namespace internal {

TensorBufferRegistry& TensorBufferRegistry::GetInstance() {
  static TensorBufferRegistry* instance_ = new TensorBufferRegistry();
  return *instance_;
}

litert::Expected<void> TensorBufferRegistry::RegisterHandlers(
    LiteRtTensorBufferType buffer_type,
    const CustomTensorBufferHandlers& handlers) {
  if (auto it = handlers_.find(buffer_type); it != handlers_.end()) {
    return litert::Unexpected(
        kLiteRtStatusErrorAlreadyExists,
        absl::StrCat("Custom tensor buffer handler is already registered for "
                     "buffer type ",
                     BufferTypeToString(buffer_type)));
  }
  handlers_[buffer_type] = handlers;
  return {};
}

litert::Expected<CustomTensorBufferHandlers>
TensorBufferRegistry::GetCustomHandlers(
    const LiteRtTensorBufferType buffer_type) {
  auto it = handlers_.find(buffer_type);
  if (it == handlers_.end()) {
    return litert::Unexpected(
        kLiteRtStatusErrorNotFound,
        absl::StrCat("No custom tensor buffer handler is registered for "
                     "buffer type ",
                     BufferTypeToString(buffer_type)));
  }
  return it->second;
}

}  // namespace internal
}  // namespace litert
