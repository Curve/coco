cmake_minimum_required(VERSION 3.25)
project(coco LANGUAGES CXX VERSION 3.2.1)

# --------------------------------------------------------------------------------------------------------
# Library options
# --------------------------------------------------------------------------------------------------------

option(coco_tests "Build tests" OFF)

# --------------------------------------------------------------------------------------------------------
# Create library
# --------------------------------------------------------------------------------------------------------

add_library(${PROJECT_NAME} STATIC "src/latch.cpp" "src/stray.cpp")
add_library(cr::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20 CXX_EXTENSIONS OFF CXX_STANDARD_REQUIRED ON)

# --------------------------------------------------------------------------------------------------------
# Include "include" folder
# --------------------------------------------------------------------------------------------------------

target_include_directories(${PROJECT_NAME} PRIVATE "include/coco")

target_include_directories(
  ${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
                         $<INSTALL_INTERFACE:include/${PROJECT_NAME}-${PROJECT_VERSION}>
)

# --------------------------------------------------------------------------------------------------------
# Build Tests
# --------------------------------------------------------------------------------------------------------
    
if (coco_tests)
  message(STATUS "[coco] Building Tests")
  add_subdirectory(tests)
endif()

# --------------------------------------------------------------------------------------------------------
# Package Config
# --------------------------------------------------------------------------------------------------------

include("cmake/cpm.cmake")

CPMFindPackage(
  NAME           PackageProject
  VERSION        1.13.0
  GIT_REPOSITORY "https://github.com/TheLartians/PackageProject.cmake"
)

packageProject(
  NAMESPACE cr
  NAME ${PROJECT_NAME}
  VERSION ${PROJECT_VERSION}
  BINARY_DIR ${PROJECT_BINARY_DIR}
  INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include
  INCLUDE_DESTINATION include/${PROJECT_NAME}-${PROJECT_VERSION}
)
