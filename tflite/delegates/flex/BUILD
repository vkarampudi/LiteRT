load(
    "@org_tensorflow//tensorflow:tensorflow.bzl",
    "if_android",
    "if_mobile",
    "if_not_mobile",
    "tf_cc_test",
    "tf_features_nolayering_check_if_ios",
    "tf_opts_nortti_if_lite_protos",
    "tf_opts_nortti_if_mobile",
)
load("@org_tensorflow//tensorflow:tensorflow.default.bzl", "get_compatible_with_portable")
load("//tflite:build_def.bzl", "tflite_copts")
load("//tflite:special_rules.bzl", "internal_visibility_allowlist")
load("//tflite/delegates/flex:build_def.bzl", "tflite_flex_cc_library", "tflite_flex_shared_library")

default_visibility = [
    "@org_tensorflow//tensorflow/compiler/mlir/lite:__subpackages__",
    "//tflite/android:__subpackages__",
    "//tflite/toco/tflite:__subpackages__",
]

#
# This is a TF Lite delegate that is powered by TensorFlow's Eager.
#
package(
    # copybara:uncomment default_applicable_licenses = ["@org_tensorflow//tensorflow:license"],
    default_visibility = default_visibility,
    licenses = ["notice"],
)

exports_files([
    "delegate.h",
    "exported_symbols.lds",
    "version_script.lds",
])

cc_library(
    name = "buffer_map",
    srcs = ["buffer_map.cc"],
    hdrs = ["buffer_map.h"],
    compatible_with = get_compatible_with_portable(),
    copts = tf_opts_nortti_if_lite_protos(),
    features = tf_features_nolayering_check_if_ios() + if_android(["-layering_check"]),
    deps = [
        ":buffer_map_util",
        ":util",
        "//tflite:string",
        "//tflite/core/c:common",
        "//tflite/kernels/internal:compatibility",
    ] + if_mobile([
        "@org_tensorflow//tensorflow/core:portable_tensorflow_lib_lite",
    ]) + if_not_mobile([
        "@org_tensorflow//tensorflow/c:c_api_internal",
        "@org_tensorflow//tensorflow/core:framework",
    ]),
)

cc_library(
    name = "buffer_map_util",
    srcs = ["buffer_map_util.cc"],
    hdrs = ["buffer_map_util.h"],
    compatible_with = get_compatible_with_portable(),
    copts = tf_opts_nortti_if_lite_protos(),
    features = tf_features_nolayering_check_if_ios() + if_android(["-layering_check"]),
    deps = [
        ":util",
        "//tflite:string_util",
        "//tflite/c:c_api_types",
        "//tflite/core/c:common",
        "//tflite/experimental/resource",
    ] + if_mobile([
        "@org_tensorflow//tensorflow/core:portable_tensorflow_lib_lite",
    ]) + if_not_mobile([
        "@org_tensorflow//tensorflow/c:c_api_internal",
        "@org_tensorflow//tensorflow/core:framework",
        "@org_tensorflow//tensorflow/c:tf_tensor_internal",
        "@org_tensorflow//tensorflow/core:protos_all_cc",
        "@org_tensorflow//tensorflow/core/platform:status",
    ]),
)

tf_cc_test(
    name = "buffer_map_test",
    size = "small",
    srcs = ["buffer_map_test.cc"],
    deps = [
        ":buffer_map",
        ":buffer_map_util",
        "//tflite:framework",
        "//tflite:string_util",
        "//tflite:util",
        "//tflite/core/c:c_api_types",
        "//tflite/testing:util",
        "@com_google_googletest//:gtest_main",
        "@org_tensorflow//tensorflow/core:framework",
    ],
)

# Define the standard flex delegate library, that pulls in the standard set
# of TensorFlow ops and kernels, using tflite_flex_cc_library with no
# models parameter. Custom flex delegate can be defined with
# tflite_flex_cc_library if the parameter models is provided. Tensorflow
# user-provided ops could also be supported by passing to additional_deps.
# Ex:
# tflite_flex_cc_library(
#   name = "sample_delegate",
#   models = ["model1.tflite", "model2.tflite"],
#   additional_deps = ["your_custom_ops_lib"],
# )
tflite_flex_cc_library(
    name = "delegate",
    compatible_with = get_compatible_with_portable(),
    visibility = ["//visibility:public"],
)

# Compared to the library above, this one doesn't define a strong symbol for
# AcquireFlexDelegate(). This is useful if one doesn't want the default flex
# delegate to be automatically applied when building the interpreter.
tflite_flex_cc_library(
    name = "delegate_without_symbol",
    link_symbol = False,
    visibility = ["//visibility:public"],
)

# Shared lib target for convenience, pulls in the standard set of TensorFlow
# ops and kernels. The output library name is platform dependent:
#   - Linux/Android: `libtensorflowlite_flex.so`
#   - Mac: `libtensorflowlite_flex.dylib`
#   - Windows: `tensorflowlite_flex.dll`
tflite_flex_shared_library(
    name = "tensorflowlite_flex",
)

cc_library(
    name = "delegate_symbol",
    srcs = [
        "delegate_symbol.cc",
    ],
    compatible_with = get_compatible_with_portable(),
    copts = tflite_copts(),
    visibility = ["//visibility:public"],
    deps = [
        ":delegate_only_runtime",
        "//tflite/core/c:c_api_types",
    ],
    alwayslink = 1,
)

# Delegate implementation that does *not* pull in the standard set of TensorFlow
# ops and kernels.
cc_library(
    name = "delegate_only_runtime",
    srcs = [
        "delegate.cc",
        "kernel.cc",
        "kernel.h",
    ],
    hdrs = [
        "delegate.h",
    ],
    compatible_with = get_compatible_with_portable(),
    copts = tflite_copts() + tf_opts_nortti_if_mobile(),
    features = tf_features_nolayering_check_if_ios() + if_android(["-layering_check"]),
    visibility = ["//visibility:public"],
    deps = [
        ":buffer_map",
        ":delegate_data",
        ":tflite_subgraph_execute",
        ":util",
        "//tflite:kernel_api",
        "//tflite:macros",
        "//tflite:minimal_logging",
        "//tflite:string",
        "//tflite:string_util",
        "//tflite:util",
        "//tflite/core:subgraph",
        "//tflite/core/api",
        "//tflite/core/c:common",
        "//tflite/delegates/utils:simple_delegate",
        "//tflite/kernels:kernel_util",
        "@com_google_absl//absl/container:inlined_vector",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "@flatbuffers",
        "@org_tensorflow//tensorflow/core:session_options",
        "@org_tensorflow//tensorflow/core/tfrt/fallback:op_kernel_runner",
    ] + if_mobile([
        "@org_tensorflow//tensorflow/core:portable_tensorflow_lib_lite",
    ]) + if_not_mobile([
        "@org_tensorflow//tensorflow/core/common_runtime/eager:context",
        "@org_tensorflow//tensorflow/core:lib",
        "@org_tensorflow//tensorflow/core:protos_all_cc",
        "@org_tensorflow//tensorflow/core:framework",
    ]),
    alwayslink = 1,
)

tf_cc_test(
    name = "delegate_test",
    size = "small",
    srcs = ["delegate_test.cc"],
    tags = [
        "no_gpu",  # GPU + flex is not officially supported.
    ],
    deps = [
        ":delegate",
        ":test_util",
        "//tflite:shared_library",
        "//tflite/kernels:test_util",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "delegate_data",
    srcs = ["delegate_data.cc"],
    hdrs = ["delegate_data.h"],
    compatible_with = get_compatible_with_portable(),
    copts = tf_opts_nortti_if_mobile(),
    features = tf_features_nolayering_check_if_ios() + if_android(["-layering_check"]),
    visibility = ["//visibility:public"],
    deps = [
        ":buffer_map",
        ":subgraph_resource",
        ":util",
        "//tflite:cc_api_experimental",
        "//tflite:util",
        "//tflite/core/c:common",
        "//tflite/schema:schema_fbs",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings",
        "@flatbuffers",
    ] + if_mobile([
        "@org_tensorflow//tensorflow/core:portable_tensorflow_lib_lite",
    ]) + if_not_mobile([
        "@org_tensorflow//tensorflow/core/common_runtime/eager:context",
        "@org_tensorflow//tensorflow/core/common_runtime/eager:core_no_xla",
        "@org_tensorflow//tensorflow/core:core_cpu",
        "@org_tensorflow//tensorflow/core:framework",
        "@org_tensorflow//tensorflow/core:lib",
        "@org_tensorflow//tensorflow/core:protos_all_cc",
    ]),
)

tf_cc_test(
    name = "delegate_data_test",
    size = "small",
    srcs = ["delegate_data_test.cc"],
    deps = [
        ":delegate_data",
        "//tflite:framework",
        "//tflite/core/api:error_reporter",
        "//tflite/core/c:common",
        "//tflite/kernels:subgraph_test_util",
        "//tflite/testing:util",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings",
        "@com_google_googletest//:gtest_main",
        "@org_tensorflow//tensorflow/core:test",
        "@org_tensorflow//tensorflow/core/common_runtime/eager:context",
        "@org_tensorflow//tensorflow/core/platform:mutex",
        "@org_tensorflow//tensorflow/core/platform:protobuf",
        "@org_tensorflow//tensorflow/core/platform:status",
    ],
)

cc_library(
    name = "subgraph_resource",
    hdrs = ["subgraph_resource.h"],
    compatible_with = get_compatible_with_portable(),
    features = tf_features_nolayering_check_if_ios(),
    deps = [
        "//tflite:cc_api_experimental",
        "//tflite/core/c:common",
        "@org_tensorflow//tensorflow/core/platform:mutex",
        "@org_tensorflow//tensorflow/core/platform:thread_annotations",
    ] + if_mobile([
        "@org_tensorflow//tensorflow/core:portable_tensorflow_lib_lite",
    ]) + if_not_mobile([
        "@org_tensorflow//tensorflow/core:framework",
        "@org_tensorflow//tensorflow/core:lib",
    ]),
)

tf_cc_test(
    name = "kernel_test",
    size = "small",
    srcs = [
        "kernel.h",
        "kernel_test.cc",
    ],
    tags = ["no_gpu"],  # GPU + flex is not officially supported.
    deps = [
        ":delegate",
        ":delegate_data",
        ":test_util",
        "//tflite/core/c:c_api_types",
        "//tflite/core/c:common",
        "//tflite/delegates/utils:simple_delegate",
        "//tflite/kernels:kernel_util",
        "@com_google_googletest//:gtest_main",
        "@org_tensorflow//tensorflow/core/platform:status",
        "@org_tensorflow//tensorflow/core/tfrt/fallback:op_kernel_runner",
    ],
)

cc_library(
    name = "test_util",
    testonly = True,
    srcs = ["test_util.cc"],
    hdrs = ["test_util.h"],
    visibility = internal_visibility_allowlist(),
    deps = [
        "//tflite:string",
        "//tflite/kernels:test_util",
        "@com_google_absl//absl/memory",
        "@flatbuffers",
        "@org_tensorflow//tensorflow/c:c_api_internal",
    ],
)

cc_library(
    name = "util",
    srcs = ["util.cc"],
    hdrs = ["util.h"],
    compatible_with = get_compatible_with_portable(),
    features = tf_features_nolayering_check_if_ios() + if_android(["-layering_check"]),
    #TODO(b/206038955): Consider restrict the visibility to '//third_party/fcp/client:__subpackages__'.
    visibility = ["//visibility:public"],
    deps = [
        "//tflite:kernel_api",
        "//tflite:string_util",
        "//tflite:util",
        "//tflite/c:common",
        "//tflite/core/c:c_api_types",
        "//tflite/core/c:common",
        "//tflite/kernels/internal:tensor",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@org_tensorflow//tensorflow/core:protos_all_cc",
    ] + if_mobile([
        "@org_tensorflow//tensorflow/core:portable_tensorflow_lib_lite",
    ]) + if_not_mobile([
        "@org_tensorflow//tensorflow/c:c_api_internal",
        "@org_tensorflow//tensorflow/core:lib",
        "@org_tensorflow//tensorflow/core:framework",
        "@org_tensorflow//tensorflow/core/protobuf:error_codes_proto_impl_cc",
    ]),
)

tf_cc_test(
    name = "util_test",
    size = "small",
    srcs = ["util_test.cc"],
    deps = [
        ":util",
        "//tflite:string",
        "//tflite:string_util",
        "//tflite:util",
        "//tflite/core/c:c_api_types",
        "//tflite/core/c:common",
        "@com_google_absl//absl/status",
        "@com_google_googletest//:gtest_main",
        "@org_tensorflow//tensorflow/c:tf_datatype",
        "@org_tensorflow//tensorflow/core:framework",
        "@org_tensorflow//tensorflow/core:portable_gif_internal",
        "@org_tensorflow//tensorflow/core:protos_all_cc",
        "@org_tensorflow//tensorflow/core/platform:errors",
        "@org_tensorflow//tensorflow/core/platform:status",
        "@org_tensorflow//tensorflow/core/protobuf:error_codes_proto_impl_cc",
    ],
)

tf_cc_test(
    name = "allowlisted_flex_ops_test",
    size = "small",
    srcs = [
        "allowlisted_flex_ops_test.cc",
    ],
    features = tf_features_nolayering_check_if_ios(),
    deps = [
        ":delegate",
        "@com_google_googletest//:gtest_main",
        "@org_tensorflow//tensorflow/compiler/mlir/lite/delegates/flex:allowlisted_flex_ops_lib",
    ] + if_mobile([
        "@org_tensorflow//tensorflow/core:portable_tensorflow_lib_lite",
    ]) + if_not_mobile([
        "@org_tensorflow//tensorflow/core:framework",
    ]),
)

# Alias to support selective build of image ops.
# TODO(b/163285312): Remove after tensorflow/core refactoring completed.
cc_library(
    name = "portable_images_lib",
    visibility = ["//visibility:public"],
    deps = [
        "@org_tensorflow//tensorflow/core:portable_gif_internal",
        "@org_tensorflow//tensorflow/core:portable_jpeg_internal",
        "@org_tensorflow//tensorflow/core/lib/png:png_io",
        "@org_tensorflow//tensorflow/core/lib/webp:webp_io",
    ],
)

cc_library(
    name = "tflite_subgraph_execute",
    srcs = ["tflite_subgraph_execute.cc"],
    compatible_with = get_compatible_with_portable(),
    copts = tf_opts_nortti_if_mobile(),
    features = tf_features_nolayering_check_if_ios() + if_android(["-layering_check"]),
    deps = [
        ":buffer_map_util",
        ":subgraph_resource",
        ":util",
        "//tflite:cc_api_experimental",
        "//tflite:string_util",
        "//tflite/core/c:c_api_types",
        "//tflite/core/c:common",
        "//tflite/kernels:builtin_ops",
        "//tflite/kernels:kernel_util",
        "//tflite/kernels/internal:tensor",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
    ] + if_mobile([
        "@org_tensorflow//tensorflow/core:portable_tensorflow_lib_lite",
    ]) + if_not_mobile([
        "@org_tensorflow//tensorflow/core:framework",
        "@org_tensorflow//tensorflow/core:lib",
        "@org_tensorflow//tensorflow/c:tf_tensor_internal",
    ]),
    alwayslink = 1,
)
