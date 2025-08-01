# Copyright 2025 Google LLC.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

package(
    # copybara:uncomment default_applicable_licenses = ["@org_tensorflow//tensorflow:license"],
    default_visibility = [
        "//visibility:public",  # copybara:comment_replace "//litert:litert_public",
    ],
)

# LiteRt's compliance test suite (CTS).

cc_test(
    name = "cts",
    srcs = ["cts.cc"],
    # TODO: Figure out data seeding and test re-running.
    args = [
        # copybara:uncomment "--gunit_random_seed=42",
        # copybara:uncomment "--gunit_color=yes",
    ],
    copts = ["-DGOOGLE_COMMANDLINEFLAGS_FULL_API=1"],
    deps = [
        ":compiled_model_executor",
        ":cts_configure",
        ":register",
        "//litert/c:litert_common",
        "//litert/c:litert_logging",
        "//litert/cc:litert_c_types_printing",
        "//litert/cc:litert_detail",
        "//litert/cc:litert_expected",
        "//litert/cc:litert_macros",
        "//litert/core/model",
        "//litert/core/util:flatbuffer_tools_testonly",
        "//litert/test:matchers",
        "//litert/test:rng_fixture",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/strings:string_view",
        "@com_google_googletest//:gtest",
    ],
)

cc_library(
    name = "cts_configure",
    testonly = True,
    srcs = ["cts_configure.cc"],
    hdrs = ["cts_configure.h"],
    deps = [
        "//litert/c:litert_common",
        "//litert/c:litert_logging",
        "//litert/cc:litert_expected",
        "//litert/cc:litert_macros",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/strings:string_view",
    ],
)

cc_library(
    name = "compiled_model_executor",
    hdrs = ["compiled_model_executor.h"],
    deps = [
        "//litert/c:litert_common",
        "//litert/cc:litert_expected",
        "//litert/cc:litert_macros",
        "//litert/cc:litert_model",
        "//litert/cc/internal:litert_compiled_model",
        "//litert/cc/internal:litert_environment",
        "//litert/cc/internal:litert_options",
        "//litert/cc/internal:litert_tensor_buffer",
        "//litert/test:simple_buffer",
        "@com_google_absl//absl/strings:string_view",
    ],
)

cc_library(
    name = "register",
    testonly = True,
    hdrs = ["register.h"],
    deps = [
        ":compiled_model_executor",
        ":cts_configure",
        "//litert/c:litert_logging",
        "//litert/c:litert_op_code",
        "//litert/cc:litert_detail",
        "//litert/cc:litert_rng",
        "//litert/test/generators",
        "//tflite/schema:schema_fbs",
        "@com_google_absl//absl/log:absl_check",
    ],
)

cc_test(
    name = "compiled_model_executor_test",
    srcs = ["compiled_model_executor_test.cc"],
    deps = [
        ":compiled_model_executor",
        "//litert/c:litert_op_code",
        "//litert/cc:litert_buffer_ref",
        "//litert/core/model",
        "//litert/test:matchers",
        "//litert/test:simple_buffer",
        "//litert/test/generators:graph_helpers",
        "//tflite/schema:schema_fbs",
        "@com_google_googletest//:gtest_main",
    ],
)
