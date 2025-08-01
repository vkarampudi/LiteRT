# This package is a shim layer over TF and TFLite op kernels.

load("@org_tensorflow//tensorflow:tensorflow.bzl", "if_android", "if_mobile", "if_not_mobile", "tf_cc_test", "tf_copts")
load("@org_tensorflow//tensorflow:tensorflow.default.bzl", "get_compatible_with_portable")

package(
    # copybara:uncomment default_applicable_licenses = ["@org_tensorflow//tensorflow:license"],
    default_compatible_with = get_compatible_with_portable(),
    # The public targets are marked public individually
    default_visibility = ["@org_tensorflow//tensorflow:__subpackages__"],
    licenses = ["notice"],
)

filegroup(
    name = "tf_headers",
    srcs = [
        "op_kernel.h",
        "shape.h",
        "status_macros.h",
        "tensor_view.h",
        "tf_op_shim.h",
        "tf_tensor_view.h",
    ],
    visibility = ["@org_tensorflow//tensorflow/core:__pkg__"],
)

cc_library(
    name = "tf_kernel_shim",
    deps = [
        ":op_kernel",
        ":shape",
        ":status_macros",
        ":tensor_view",
        ":tf_op_shim",
        ":tf_tensor_view",
    ],
)

cc_library(
    name = "tensor_view",
    hdrs = ["tensor_view.h"],
    visibility = ["//visibility:public"],
    deps = [
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:span",
        "@org_tensorflow//tensorflow/core:tflite_portable_logging",
        "@org_tensorflow//tensorflow/core/platform:tstring",
    ],
)

cc_library(
    name = "shape",
    srcs = ["shape.cc"],
    hdrs = ["shape.h"],
    visibility = ["//visibility:public"],
    deps = [
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/types:span",
    ],
)

cc_test(
    name = "shape_test",
    srcs = ["shape_test.cc"],
    deps = [
        ":shape",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "tf_tensor_view",
    srcs = ["tf_tensor_view.cc"],
    hdrs = ["tf_tensor_view.h"],
    copts = tf_copts(),
    features = if_android(["-layering_check"]),
    deps = [
        ":tensor_view",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
    ] + if_mobile([
        "@org_tensorflow//tensorflow/core:portable_tensorflow_lib_lite",
    ]) + if_not_mobile([
        "@org_tensorflow//tensorflow/core:framework",
        "@org_tensorflow//tensorflow/core:protos_all_cc",
    ]),
)

tf_cc_test(
    name = "tf_tensor_view_test",
    srcs = ["tf_tensor_view_test.cc"],
    deps = [
        ":tf_tensor_view",
        "@com_google_absl//absl/strings",
        "@com_google_googletest//:gtest_main",
    ] + if_mobile([
        "@org_tensorflow//tensorflow/core:portable_tensorflow_lib_lite",
    ]) + if_not_mobile([
        "@org_tensorflow//tensorflow/core:framework_lite",
        "@org_tensorflow//tensorflow/core:protos_all_cc",
        "@org_tensorflow//tensorflow/core/platform:tstring",
    ]),
)

cc_library(
    name = "tflite_tensor_view",
    srcs = ["tflite_tensor_view.cc"],
    hdrs = ["tflite_tensor_view.h"],
    deps = [
        ":tensor_view",
        "//tflite:string_util",
        "//tflite:type_to_tflitetype",
        "//tflite/core/c:common",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:span",
        "@com_google_absl//absl/types:variant",
    ],
)

cc_test(
    name = "tflite_tensor_view_test",
    srcs = ["tflite_tensor_view_test.cc"],
    deps = [
        ":test_util",
        ":tflite_tensor_view",
        "//tflite:framework",
        "//tflite:string_util",
        "//tflite/core:framework_stable",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "op_kernel",
    hdrs = ["op_kernel.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":shape",
        ":status_macros",
        ":tensor_view",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:variant",
    ],
)

cc_library(
    name = "tf_op_shim",
    srcs = ["tf_op_shim.cc"],
    hdrs = ["tf_op_shim.h"],
    copts = tf_copts(),
    features = if_android(["-layering_check"]),
    visibility = ["//visibility:public"],
    deps = [
        ":op_kernel",
        ":shape",
        ":status_macros",
        ":tensor_view",
        ":tf_tensor_view",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@local_tsl//tsl/platform:macros",
    ] + if_mobile([
        "@org_tensorflow//tensorflow/core:portable_tensorflow_lib_lite",
    ]) + if_not_mobile([
        "@org_tensorflow//tensorflow/core:framework",
        "@org_tensorflow//tensorflow/core:protos_all_cc",
        "@org_tensorflow//tensorflow/core/platform:errors",
        "@org_tensorflow//tensorflow/core/platform:status",
    ]),
)

cc_library(
    name = "tflite_op_shim",
    srcs = ["tflite_op_shim.cc"],
    hdrs = ["tflite_op_shim.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":op_kernel",
        ":shape",
        ":status_macros",
        ":tensor_view",
        ":tflite_tensor_view",
        "//tflite:mutable_op_resolver",
        "//tflite:string_util",
        "//tflite/core/c:common",
        "//tflite/kernels:kernel_util",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "@flatbuffers",
    ],
)

cc_library(
    name = "tflite_op_wrapper",
    hdrs = ["tflite_op_wrapper.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":op_kernel",
        ":status_macros",
        "//tflite:type_to_tflitetype",
        "//tflite/c:common",
    ],
)

cc_test(
    name = "tflite_op_wrapper_test",
    srcs = ["tflite_op_wrapper_test.cc"],
    deps = [
        ":op_kernel",
        ":tflite_op_shim",
        ":tflite_op_wrapper",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "@com_google_googletest//:gtest_main",
        "@flatbuffers",
        "@org_tensorflow//tensorflow/core/platform:tstring",
    ],
)

cc_library(
    name = "status_macros",
    hdrs = ["status_macros.h"],
    visibility = ["//visibility:public"],
    deps = [
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
    ],
)

cc_library(
    name = "test_util",
    testonly = True,
    srcs = ["test_util.cc"],
    hdrs = ["test_util.h"],
    deps = [
        "//tflite:string_util",
        "//tflite:type_to_tflitetype",
        "//tflite/core/c:common",
        "//tflite/kernels/internal:compatibility",
        "@com_google_absl//absl/strings",
        "@org_tensorflow//tensorflow/core:tflite_portable_logging",
    ],
)

cc_test(
    name = "test_util_test",
    srcs = ["test_util_test.cc"],
    deps = [
        ":test_util",
        "//tflite:cc_api_experimental",
        "//tflite/core:framework_stable",
        "@com_google_googletest//:gtest_main",
    ],
)
