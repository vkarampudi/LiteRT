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

# copybara:uncomment load(INTERNAL_PHYSICAL_MOBILE_TESTING_INFRA, "guitar", "guitar_workflow_test")
# copybara:uncomment load("//litert/integration_test:litert_device.bzl", "device_rlocation", "litert_cpu_mh_guitar_test", "litert_device_test", "litert_integration_test", "litert_mediatek_mh_guitar_test", "litert_pixel_9_mh_guitar_test", "litert_qualcomm_mh_guitar_test")
# copybara:comment_begin(oss-only)
load("//litert/integration_test:litert_device.bzl", "device_rlocation", "litert_device_test", "litert_integration_test")
# copybara:comment_end

package(
    # copybara:uncomment default_applicable_licenses = ["@org_tensorflow//tensorflow:license"],
    default_visibility = ["//litert:litert_public"],
)

sh_binary(
    name = "run_on_device_driver",
    srcs = ["run_on_device_driver.sh"],
)

# Internal test to validate the macros and underlying shell scripts.
litert_device_test(
    name = "tools_test",
    srcs = ["smoketests/tools_test.cc"],
    backend_id = "cpu",
    copts = ["-DGOOGLE_COMMANDLINEFLAGS_FULL_API=1"],
    data = ["//litert/integration_test/models:single_op"],
    exec_args = ["--models_dir={}".format(device_rlocation("//litert/integration_test/models:single_op"))],
    deps = [
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_googletest//:gtest_main",
    ],
)

# C++ TEST SCAFFOLD ################################################################################

cc_test(
    name = "gen_device_test",
    srcs = ["gen_device_test.cc"],
    copts = ["-DGOOGLE_COMMANDLINEFLAGS_FULL_API=1"],
    data = ["//litert/integration_test/models:single_op"],
    linkopts = select({
        "@org_tensorflow//tensorflow:android": ["-landroid"],
        "//conditions:default": [],
    }),
    tags = ["manual"],
    deps = [
        ":gen_device_test_lib",
        "//litert/c:litert_logging",
        "//litert/cc:litert_environment",
        "//litert/cc:litert_model",
        "//litert/test:common",
        "//litert/test:matchers",
        "//litert/tools:dump",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/strings:string_view",
        "@com_google_absl//absl/types:span",
        "@com_google_googletest//:gtest",
    ],
)

cc_library(
    name = "gen_device_test_lib",
    testonly = True,
    hdrs = ["gen_device_test_lib.h"],
    deps = [
        "//litert/c:litert_common",
        "//litert/cc:litert_compiled_model",
        "//litert/cc:litert_environment",
        "//litert/cc:litert_model",
        "//litert/cc:litert_tensor_buffer",
        "//litert/core/model",
        "//litert/test:matchers",
        "@com_google_absl//absl/strings:string_view",
        "@com_google_googletest//:gtest",
    ],
)

# ON DEVICE INTEGRATION TESTS ######################################################################

# NOTE: Everything here should be built with -c opt --config=android_arm64.

# CPU ----------------------------------------------------------------------------------------------

litert_integration_test(
    name = "single_op_device_tests_cpu",
    backend_id = "cpu",
    models = "//litert/integration_test/models:single_op",
)

litert_integration_test(
    name = "classic_ml_device_tests_cpu",
    backend_id = "cpu",
    models = "//litert/integration_test/models:classic_ml",
)

# GOOGLE TENSOR ------------------------------------------------------------------------------------

litert_device_test(
    name = "google_tensor_smoketest",
    srcs = ["smoketests/google_tensor_smoketest.cc"],
    backend_id = "google_tensor",
    deps = [
        "//litert/cc:litert_shared_library",
        "@com_google_googletest//:gtest_main",
    ],
)

# QUALCOMM -----------------------------------------------------------------------------------------

litert_device_test(
    name = "qnn_smoketest",
    srcs = ["smoketests/qnn_smoketest.cc"],
    backend_id = "qualcomm",
    deps = [
        "//litert/cc:litert_shared_library",
        "//litert/vendors/qualcomm:common",
        "//litert/vendors/qualcomm:qnn_manager",
        "//litert/vendors/qualcomm/core:common",
        "@com_google_absl//absl/strings:string_view",
        "@com_google_googletest//:gtest_main",
    ],
)

litert_integration_test(
    name = "single_op_device_tests_qualcomm_JIT",
    backend_id = "qualcomm",
    models = "//litert/integration_test/models:single_op",
    skips = [
        "greater_f32",  # TODO: lukeboyer - Investigate (segfault).
        "less_f32",  # TODO: lukeboyer - Investigate (segfault).
    ],
)

litert_integration_test(
    name = "classic_ml_device_tests_qualcomm_JIT",
    backend_id = "qualcomm",
    models = "//litert/integration_test/models:classic_ml",
)

litert_integration_test(
    name = "pre_compiled_device_tests_qualcomm",
    backend_id = "qualcomm",
    models = "//litert/integration_test/models:pre_compiled",
)

# MEDIATEK -----------------------------------------------------------------------------------------

litert_device_test(
    name = "mediatek_smoketest",
    srcs = ["smoketests/mediatek_smoketest.cc"],
    backend_id = "mediatek",
    deps = [
        "//litert/cc:litert_shared_library",
        "//litert/vendors/mediatek:neuron_adapter_api",
        "@com_google_googletest//:gtest_main",
    ],
)

# copybara:uncomment_begin(google-only)
# # GOOGLE-ONLY ######################################################################################
#
# # (doc) Running with MobileHarness -----------------------------------------------------------------
#
# # FLAGS
# #
# # Executing any of the above with mobile harness requires the following flags:
# # ```
# # --config=android_arm64 \
# # --copt="-DGOOGLE_COMMANDLINEFLAGS_FULL_API=1" \
# # --cache_test_results=no \
# # --test_output=streamed \
# # --notest_loasd \
# # --test_arg=--allocation_exit_strategy=FAIL_FAST_NO_MATCH --test_arg=--run_as=mobileiq-user \
# # --android_ndk_min_sdk_version=26
# # ```
# #
# # The following can be copied into your bazelrc to set these with --config=litert_mh
# # ```
# # build:litert_mh --config=android_arm64
# # build:litert_mh --notest_loasd
# # build:litert_mh --copt="-DGOOGLE_COMMANDLINEFLAGS_FULL_API=1"
# # build:litert_mh --cache_test_results=no
# # build:litert_mh --test_output=streamed
# # build:litert_mh --test_arg=--allocation_exit_strategy=FAIL_FAST_NO_MATCH --test_arg=--run_as=mobileiq-user
# # build:litert_mh --compilation_mode=opt`
# # build:litert_mh --android_ndk_min_sdk_version=26
# #```
#
# # MH PERMISSIONS
# #
# # By default, mh tests will run as the user `odml-device-lab`
# # INTERNAL_PHYSICAL_MOBILE_TESTING_LINK?filter=owner%3Aodml-device-lab
# #
# # If you are not included in this group, or it does not have access to a device you need, please
# # reach out to the litert team.
#
# # Guitar GOOGLE TENSOR -----------------------------------------------------------------------------
#
# GOOGLE_TENSOR_GUITAR_TARGS_SMALL = [
#     ":google_tensor_smoketest",
#     "//litert/runtime/dispatch:dispatch_delegate_google_tensor_test",
#     "//litert/runtime/dispatch:dispatch_delegate_cascade_google_tensor_test",
#     "//litert/runtime/dispatch:dispatch_delegate_mixed_cascade_google_tensor_test",
#     "//litert/cc:litert_compiled_model_gpu_device_test",
#     "//litert/cc:litert_compiled_model_npu_test",
#     "//litert/vendors/google_tensor/dispatch:dispatch_api_google_tensor_test",
#     "//litert/vendors/google_tensor/dispatch:dispatch_api_async_google_tensor_test",
#     "//litert/runtime:gl_buffer_device_test",
#     "//litert/cc:litert_tensor_buffer_device_test",
# ]
#
# guitar_workflow_test(
#     name = "litert_guitar_device_tests_small_google_tensor",
#     auto_deps = GOOGLE_TENSOR_GUITAR_TARGS_SMALL,
#     integration_test = guitar.IntegrationTest(
#         tests = [
#             litert_pixel_9_mh_guitar_test(
#                 targets = GOOGLE_TENSOR_GUITAR_TARGS_SMALL,
#             ),
#         ],
#     ),
#     tags = [
#         "external",
#         "guitar",
#         "manual",
#         "notap",
#     ],
# )
#
# # Guitar QUALCOMM ----------------------------------------------------------------------------------
#
# QUALCOMMM_GUITAR_TARGS_SMALL = [
#     ":qnn_smoketest",
#     "//litert/vendors/qualcomm:qnn_manager_device_test",
#     "//litert/vendors/qualcomm/dispatch:dispatch_api_qualcomm_test",
#     "//litert/vendors/qualcomm/compiler:qnn_compiler_plugin_device_test",
#     "//litert/vendors/qualcomm/core/utils:utils_device_test",
#     "//litert/vendors/qualcomm/core:tensor_pool_device_test",
#     "//litert/vendors/qualcomm/core/wrappers/tests:op_wrapper_device_test",
#     "//litert/vendors/qualcomm/core/wrappers/tests:tensor_wrapper_device_test",
#     "//litert/vendors/qualcomm/core/wrappers/tests:param_wrapper_device_test",
#     "//litert/vendors/qualcomm/core/wrappers/tests:quantize_params_wrapper_device_test",
#     "//litert/runtime/dispatch:dispatch_delegate_qualcomm_test",
#     "//litert/runtime/dispatch:dispatch_delegate_cascade_qualcomm_test",
#     "//litert/runtime/dispatch:dispatch_delegate_mixed_cascade_qualcomm_test",
#     "//litert/runtime/compiler:jit_compilation_qualcomm_test",
#     "//litert/runtime:gl_buffer_device_test",
#     "//litert/cc:litert_compiled_model_qualcomm_test",
#     "//litert/cc:litert_tensor_buffer_device_test",
# ]
#
# guitar_workflow_test(
#     name = "litert_guitar_device_tests_small_qualcomm",
#     auto_deps = QUALCOMMM_GUITAR_TARGS_SMALL,
#     integration_test = guitar.IntegrationTest(
#         tests = [
#             litert_qualcomm_mh_guitar_test(
#                 targets = QUALCOMMM_GUITAR_TARGS_SMALL,
#             ),
#         ],
#     ),
#     tags = [
#         "external",
#         "guitar",
#         "manual",
#         "notap",
#     ],
# )
#
# QUALCOMMM_GUITAR_TARGS_INTEGRATION = [
#     ":single_op_device_tests_qualcomm_JIT",
#     ":classic_ml_device_tests_qualcomm_JIT",
#     ":pre_compiled_device_tests_qualcomm",
# ]
#
# guitar_workflow_test(
#     name = "litert_guitar_device_integration_tests_qualcomm",
#     auto_deps = QUALCOMMM_GUITAR_TARGS_INTEGRATION,
#     integration_test = guitar.IntegrationTest(
#         tests = [
#             litert_qualcomm_mh_guitar_test(
#                 targets = QUALCOMMM_GUITAR_TARGS_INTEGRATION,
#             ),
#         ],
#     ),
#     tags = [
#         "external",
#         "guitar",
#         "manual",
#         "notap",
#     ],
# )
#
# # Guitar MEDIATEK ----------------------------------------------------------------------------------
#
# MEDIATEK_GUITAR_TARGS_SMALL = [
#     ":mediatek_smoketest",
#     "//litert/runtime/dispatch:dispatch_delegate_mediatek_test",
#     "//litert/runtime/dispatch:dispatch_delegate_cascade_mediatek_test",
#     "//litert/runtime/dispatch:dispatch_delegate_mixed_cascade_mediatek_test",
# ]
#
# guitar_workflow_test(
#     name = "litert_guitar_device_tests_small_mediatek",
#     auto_deps = MEDIATEK_GUITAR_TARGS_SMALL,
#     integration_test = guitar.IntegrationTest(
#         tests = [
#             litert_mediatek_mh_guitar_test(
#                 targets = MEDIATEK_GUITAR_TARGS_SMALL,
#             ),
#         ],
#     ),
#     tags = [
#         "external",
#         "guitar",
#         "manual",
#         "notap",
#     ],
# )
#
# # Guitar CPU ---------------------------------------------------------------------------------------
#
# CPU_GUITAR_TARGS_INTEGRATION = [
#     ":single_op_device_tests_cpu",
#     ":classic_ml_device_tests_cpu",
# ]
#
# guitar_workflow_test(
#     name = "litert_guitar_device_integration_tests_cpu",
#     auto_deps = CPU_GUITAR_TARGS_INTEGRATION,
#     integration_test = guitar.IntegrationTest(
#         tests = [
#             litert_cpu_mh_guitar_test(
#                 targets = CPU_GUITAR_TARGS_INTEGRATION,
#             ),
#         ],
#     ),
#     tags = [
#         "external",
#         "guitar",
#         "manual",
#         "notap",
#     ],
# )
#
# guitar_workflow_test(
#     name = "litert_guitar_device_integration_tests_kotlin",
#     auto_deps = [
#         "//litert/kotlin/src/tests/google:litert_kotlin_api_mh_test",
#         "//litert/kotlin/src/tests/google:litert_kotlin_api_npu_mh_test",
#     ],
#     integration_test = guitar.IntegrationTest(
#         tests = [
#             litert_cpu_mh_guitar_test(
#                 targets = ["//litert/kotlin/src/tests/google:litert_kotlin_api_mh_test"],
#             ),
#             litert_qualcomm_mh_guitar_test(
#                 targets = ["//litert/kotlin/src/tests/google:litert_kotlin_api_npu_mh_test"],
#             ),
#         ],
#     ),
#     tags = [
#         "external",
#         "guitar",
#         "manual",
#         "notap",
#     ],
# )
#
# exports_files(srcs = ["run_on_device_driver.sh"])
#
# copybara:uncomment_end
