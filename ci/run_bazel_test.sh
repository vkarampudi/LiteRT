#!/usr/bin/env bash
# Copyright 2024 The AI Edge LiteRT Authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ==============================================================================
set -ex

# Run this script under the root directory.
# TODO: b/398924022  remove these once litert is migrated to tflite/
EXPERIMENTAL_TARGETS_ONLY="${EXPERIMENTAL_TARGETS_ONLY:-false}"
LITERT_TARGETS_ONLY="${LITERT_TARGETS_ONLY:-false}"
TEST_LANG_FILTERS="${TEST_LANG_FILTERS:-cc,py}"

BUILD_FLAGS=(
    "--config=bulk_test_cpu"
    "--config=use_local_tf"
    "--test_lang_filters=${TEST_LANG_FILTERS}"
    "--keep_going"
    "--repo_env=USE_PYWRAP_RULES=True"
  )

# Add Bazel --config flags based on kokoro injected env ie. --config=public_cache
BUILD_FLAGS+=(${BAZEL_CONFIG_FLAGS})

# TODO: (b/381310257) - Investigate failing test not included in cpu_full
# TODO: (b/381110338) - Clang errors
# TODO: (b/381124292) - Ambiguous operator errors
# TODO: (b/380870133) - Duplicate op error due to tf_gen_op_wrapper_py
# TODO: (b/382122737) - Module 'keras.src.backend' has no attribute 'convert_to_numpy'
# TODO: (b/382123188) - No member named 'ConvertGenerator' in namespace 'testing'
# TODO: (b/382123664) - Undefined reference due to --no-allow-shlib-undefined: google::protobuf::internal
# TODO(b/385356261): no matching constructor for initialization of 'litert::Tensor::TensorUse'
# TODO(b/385360853): Qualcomm related tests do not build in LiteRT
# TODO(b/385361335): sb_api.h file not found
EXCLUDED_TARGETS=(
        "-//tflite/delegates/flex:buffer_map_test"
        "-//tflite/delegates/gpu/cl/kernels:convolution_transposed_3x3_test"
        "-//tflite/delegates/xnnpack:reduce_test"
        "-//tflite/experimental/acceleration/mini_benchmark:blocking_validator_runner_test"
        "-//tflite/experimental/microfrontend:audio_microfrontend_op_test"
        "-//tflite/kernels/variants/py:end_to_end_test"
        "-//tflite/profiling:memory_info_test"
        "-//tflite/profiling:profile_summarizer_test"
        "-//tflite/profiling:profile_summary_formatter_test"
        "-//tflite/python/authoring:authoring_test"
        "-//tflite/python/metrics:metrics_wrapper_test"
        "-//tflite/python:lite_flex_test"
        "-//tflite/python:lite_test"
        "-//tflite/python:lite_v2_test"
        "-//tflite/python:util_test"
        "-//tflite/testing:zip_test_fully_connected_4bit_hybrid_forward-compat_xnnpack"
        "-//tflite/testing:zip_test_fully_connected_4bit_hybrid_mlir-quant_xnnpack"
        "-//tflite/testing:zip_test_fully_connected_4bit_hybrid_with-flex_xnnpack"
        "-//tflite/testing:zip_test_fully_connected_4bit_hybrid_xnnpack"
        "-//tflite/testing:zip_test_depthwiseconv_with-flex"
        "-//tflite/testing:zip_test_depthwiseconv_forward-compat"
        "-//tflite/testing:zip_test_depthwiseconv_mlir-quant"
        "-//tflite/testing:zip_test_depthwiseconv"
        "-//tflite/tools/optimize/debugging/python:debugger_test"
        "-//tflite/tools:convert_image_to_csv_test"
        "-//tflite/testing:zip_test_depthwiseconv"
        "-//tflite/testing:zip_test_depthwiseconv_forward-compat"
        "-//tflite/testing:zip_test_depthwiseconv_mlir-quant"
        "-//tflite/testing:zip_test_depthwiseconv_with-flex"
        "-//tflite/experimental/acceleration/mini_benchmark:blocking_validator_runner_test"
        # Exclude dir which shouldnt run
        "-//tflite/java/..."
        "-//tflite/tools/benchmark/experimental/..."
        "-//tflite/delegates/gpu/..."
        # TODO: (b/410925271) - Targets not migrated to pywrap_rules yet
)

LITERT_EXCLUDED_TARGETS=(
        "-//litert/c:litert_compiled_model_shared_lib_test"
        "-//litert/c:litert_compiled_model_test"
        "-//litert/cc:litert_compiled_model_test"
        # Requires mGPU environment.
        "-//litert/cc:litert_environment_test"
        "-//litert/runtime:compiled_model_test"
        # Requires c++20.
        "-//litert/tools:tool_display_test"
        # Requires c++20.
        "-//litert/tools:dump_test"
        # Requires c++20.
        "-//litert/tools:apply_plugin_test"
        # Enable once openvino
        "-//litert/vendors/intel_openvino/..."
)


if [ "$LITERT_TARGETS_ONLY" == "true" ]; then
    bazel test "${BUILD_FLAGS[@]}" -- //litert/... "${LITERT_EXCLUDED_TARGETS[@]}"
else
    bazel test "${BUILD_FLAGS[@]}" -- //tflite/... "${EXCLUDED_TARGETS[@]}"
fi
