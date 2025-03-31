cmake_minimum_required(VERSION 3.15)

# Fetch Google Test using FetchContent
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.14.0 # Or any specific tag/commit you want
)
FetchContent_MakeAvailable(googletest)

# Fetch tensorflow
include(FetchContent)
FetchContent_Declare(
    tflite
    GIT_REPOSITORY https://github.com/tensorflow/tensorflow.git 
    GIT_TAG        v2.19.0
)
FetchContent_MakeAvailable(tflite)

FetchContent_Declare(
  abseil-cpp
  GIT_REPOSITORY https://github.com/abseil/abseil-cpp.git
  GIT_TAG        20250127.1 # Or any specific tag/commit you want
)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
FetchContent_MakeAvailable(abseil-cpp)

# Fetch OpenCL-Headers using FetchContent
FetchContent_Declare(
  opencl-headers
  GIT_REPOSITORY https://github.com/KhronosGroup/OpenCL-Headers.git
  GIT_TAG        v2024.10.24 # Or a valid tag from the OpenCL-Headers repository
)
FetchContent_MakeAvailable(opencl-headers)

# Fetch OpenCL-ICD-Loader using FetchContent
FetchContent_Declare(
  opencl-icd-loader
  GIT_REPOSITORY https://github.com/KhronosGroup/OpenCL-ICD-Loader.git
  GIT_TAG        v2024.10.24 # Or a valid tag from the OpenCL-ICD-Loader repository
)
FetchContent_MakeAvailable(opencl-icd-loader)

set(OPENCL_ICD_LOADER_HEADERS_DIR "${opencl-headers_SOURCE_DIR}" CACHE PATH "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "Disable OpenCL ICD Loader tests" FORCE)

# --------

function(lite_rt_friends out_var)
  set(${out_var} "" PARENT_SCOPE) # No-op, returns an empty list.
endfunction()

# CMake equivalent of gles_deps()
function(gles_deps out_var)
  set(${out_var} "" PARENT_SCOPE) # No-op, returns an empty list.
endfunction()

# CMake equivalent of gles_headers()
function(gles_headers out_var)
  set(${out_var} "" PARENT_SCOPE) # No-op, returns an empty list.
endfunction()

# CMake equivalent of gles_linkopts()
function(gles_linkopts out_var)
  set(${out_var} "" PARENT_SCOPE) # No-op, returns an empty list.
endfunction()

# litert_build_defs.bzl functions (simplified for CMake)
function(make_linkopt out_var opt)
  set(<span class="math-inline">\{out\_var\} "\-Wl,</span>{opt}" PARENT_SCOPE)
endfunction()

function(make_rpaths out_var rpaths)
  string(JOIN ":" rpaths_str "<span class="math-inline">\{rpaths\}"\)
make\_linkopt\(</span>{out_var} "-rpath=${rpaths_str}")
endfunction()

function(append_rule_kwargs rule_kwargs append)
  foreach(key IN LISTS append)
    list(GET append ${key} value)
    list(APPEND ${rule_kwargs} "${value}")
  endforeach()
  set(${rule_kwargs} "${${rule_kwargs}}" PARENT_SCOPE)
endfunction()

function(absolute_label out_var label package_name)
  if(NOT package_name)
    get_filename_component(package_name ${CMAKE_CURRENT_LIST_DIR} NAME)
  endif()
  if(NOT "${label}" MATCHES "^//")
    if("${label}" MATCHES "^:")
      set(${out_var} "//${package_name}${label}" PARENT_SCOPE)
    elseif("${label}" MATCHES ":")
      set(${out_var} "//${package_name}/${label}" PARENT_SCOPE)
    else()
      set(${out_var} "//${package_name}:${label}" PARENT_SCOPE)
    endif()
  else()
    if("${label}" MATCHES ":")
      set(${out_var} "${label}" PARENT_SCOPE)
    else()
      string(REPLACE "/" ":" label_colon "${label}")
      string(REPLACE "//" "" label_colon "${label_colon}")
      string(REPLACE ":" "/" label_dir "${label_colon}")
      get_filename_component(label_name "${label_dir}" NAME)
      set(${out_var} "${label}:${label_name}" PARENT_SCOPE)
    endif()
  endif()
endfunction()

function(copy_file name src target)
  add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${target}"
    COMMAND ${CMAKE_COMMAND} -E copy "${src}" "${CMAKE_CURRENT_BINARY_DIR}/${target}"
    DEPENDS "${src}"
    COMMENT "Copying ${src} to ${target}"
  )
  add_custom_target(${name} DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${target}")
endfunction()

function(gtest_main_no_heapcheck_deps out_var)
  set(<span class="math-inline">\{out\_var\} GTest\:\:gtest\_main PARENT\_SCOPE\)
endfunction()

# special\_rule\.bzl functions \(no\-op in this case\)
function\(gles\_deps out\_var\)
set\(</span>{out_var} "" PARENT_SCOPE)
endfunction()

function(gles_headers out_var)
  set(<span class="math-inline">\{out\_var\} "" PARENT\_SCOPE\)
endfunction()

function\(gles\_linkopts out\_var\)
set\(</span>{out_var} "" PARENT_SCOPE)
endfunction()

# litert_common
add_library(litert_common STATIC litert_common.cc litert_common.h)
target_include_directories(litert_common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})


# litert_common_test
add_executable(litert_common_test litert_common_test.cc)
target_link_libraries(litert_common_test PRIVATE litert_common GTest::gtest_main)

# litert_any
# add_library(litert_any litert_any.h)
add_library(litert_any INTERFACE)
target_sources(litert_any INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/litert_any.h)
set_target_properties(litert_any PROPERTIES LINKER_LANGUAGE C)

# litert_environment
add_library(litert_environment litert_environment.cc litert_environment.h)
target_link_libraries(litert_environment PRIVATE
  litert_common
  litert_environment_options
  absl::span
  litert_macros
  litert_core_environment
  litert_runtime_gpu_environment
  litert_runtime_accelerators_auto_registration
)

# litert_environment_options
add_library(litert_environment_options litert_environment_options.cc litert_environment_options.h)
target_link_libraries(litert_environment_options PRIVATE
  litert_any
  litert_common
  litert_macros
  litert_core_environment_options
)

# litert_environment_options_header
add_library(litert_environment_options_header litert_environment_options.h)
target_link_libraries(litert_environment_options_header PRIVATE litert_any litert_common)
set_target_properties(litert_environment_options_header PROPERTIES EXCLUDE_FROM_ALL TRUE)

# litert_environment_options_test
add_executable(litert_environment_options_test litert_environment_options_test.cc)
target_link_libraries(litert_environment_options_test PRIVATE
  litert_any
  litert_common
  litert_environment_options
  GTest::gtest_main
  litert_core_environment_options
  litert_test_matchers
)

# litert_logging
add_library(litert_logging litert_logging.cc litert_logging.h)
target_link_libraries(litert_logging PRIVATE litert_common tensorflow_lite_minimal_logging)

# litert_logging_test
add_executable(litert_logging_test litert_logging_test.cc)
target_link_libraries(litert_logging_test PRIVATE litert_common litert_logging GTest::gtest_main)

# litert_op_code
add_library(litert_op_code INTERFACE)
target_sources(litert_op_code INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/litert_op_code.h)
target_include_directories(litert_op_code INTERFACE ${CMAKE_SOURCE_DIR} ${tflite_SOURCE_DIR}/tensorflow/lite)
target_link_libraries(litert_op_code INTERFACE builtin_ops)

# litert_layout
add_library(litert_layout litert_layout.h)
target_link_libraries(litert_layout PRIVATE
  litert_common
  litert_op_code
  absl::strings
  litert_buffer_ref
  litert_macros
  tensorflow_lite_c_api_types
)

# litert_model
add_library(litert_model litert_model.cc litert_model.h)
target_link_libraries(litert_model PRIVATE
  litert_common
  litert_layout
  litert_op_code
  absl::strings
  litert_buffer_ref
  litert_core_model
  litert_core_model_load
  litert_core_model_serialize
  tensorflow_lite_c_api_types
)

# litert_model_test
add_executable(litert_model_test litert_model_test.cc)
target_link_libraries(litert_model_test PRIVATE
  litert_common
  litert_model
  litert_op_code
  GTest::gtest_main
  absl::strings
  absl::span
  litert_buffer_ref
  litert_core_model
  litert_core_util_flatbuffer_tools
  litert_test_matchers
)

# litert_options
add_library(litert_options litert_options.cc litert_options.h)
target_link_libraries(litert_options PRIVATE
  litert_common
  litert_op_code
  litert_core_model
  tensorflow_compiler_mlir_lite_core_model_builder_base
  tensorflow_lite_c_api_types
)

# litert_options_test
add_executable(litert_options_test litert_options_test.cc)
target_link_libraries(litert_options_test PRIVATE
  litert_common
  litert_options
  GTest::gtest_main
  litert_test_common
  litert_test_matchers
)

# litert_event_type
add_library(litert_event_type INTERFACE)
target_sources(litert_event_type INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/litert_event_type.h)
set_target_properties(litert_event_type PROPERTIES LINKER_LANGUAGE C)

# litert_event
add_library(litert_event litert_event.cc litert_event.h)
target_link_libraries(litert_event PRIVATE
  litert_common
  litert_event_type
  litert_logging
  litert_macros
  litert_runtime_event
)

# litert_tensor_buffer_types
add_library(litert_tensor_buffer_types INTERFACE)
target_sources(litert_tensor_buffer_types INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/litert_tensor_buffer_types.h)
set_target_properties(litert_tensor_buffer_types PROPERTIES LINKER_LANGUAGE C)

# litert_gl_types
add_library(litert_gl_types INTERFACE)
target_sources(litert_gl_types INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/litert_gl_types.h)
set_target_properties(litert_gl_types PROPERTIES LINKER_LANGUAGE C)


# litert_tensor_buffer
add_library(litert_tensor_buffer litert_tensor_buffer.cc litert_tensor_buffer_requirements.cc litert_tensor_buffer.h litert_tensor_buffer_requirements.h)

if(CMAKE_SYSTEM_NAME MATCHES "Android")
  target_link_options(litert_tensor_buffer PRIVATE "-landroid")
endif()
gles_linkopts(GLES_LINKOPTS)
target_link_options(litert_tensor_buffer PRIVATE ${GLES_LINKOPTS})
target_link_libraries(litert_tensor_buffer PRIVATE
  litert_common
  litert_event
  litert_gl_types
  litert_logging
  litert_model
  litert_tensor_buffer_types
  absl::span
  litert_expected
  litert_runtime_tensor_buffer
  OpenCL::OpenCL
)
gles_deps(GLES_DEPS)
target_link_libraries(litert_tensor_buffer PRIVATE ${GLES_DEPS})

# litert_tensor_buffer_test
add_executable(litert_tensor_buffer_test litert_tensor_buffer_test.cc)
target_link_libraries(litert_tensor_buffer_test PRIVATE
  litert_common
  litert_model
  litert_tensor_buffer
  litert_tensor_buffer_types
  GTest::gtest_main
  litert_layout
  litert_runtime_tensor_buffer
)

# litert_tensor_buffer_requirements_test
add_executable(litert_tensor_buffer_requirements_test litert_tensor_buffer_requirements_test.cc)
if(CMAKE_SYSTEM_NAME MATCHES "Android")
  target_link_options(litert_tensor_buffer_requirements_test PRIVATE "-landroid")
endif()
target_link_libraries(litert_tensor_buffer_requirements_test PRIVATE
  litert_common
  litert_tensor_buffer
  GTest::gtest_main
)

# litert_dispatch_delegate
add_library(litert_dispatch_delegate litert_dispatch_delegate.h)
target_link_libraries(litert_dispatch_delegate PRIVATE
  litert_environment_options
  litert_runtime_dispatch_dispatch_delegate
  litert_vendors_c_litert_dispatch_c_api
  tensorflow_lite_c_api
  tensorflow_lite_c_api_opaque
  tensorflow_lite_c_api_types
  tensorflow_lite_c_common
  tensorflow_lite_delegates_utils_simple_opaque_delegate
)

# litert_compilation_options
add_library(litert_compilation_options litert_compilation_options.cc litert_compilation_options.h)
target_link_libraries(litert_compilation_options PRIVATE
  litert_accelerator_compilation_options
  litert_common
  litert_logging
  litert_macros
  litert_runtime_compilation_options
)

# litert_compilation_options_test
add_executable(litert_compilation_options_test litert_compilation_options_test.cc)
target_link_libraries(litert_compilation_options_test PRIVATE
  litert_accelerator_compilation_options
  litert_common
  litert_compilation_options
  GTest::gtest_main
  litert_expected
  litert_macros
)