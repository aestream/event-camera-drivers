cmake_minimum_required(VERSION 3.15)
project(event_camera_drivers)

# Warn if the user invokes CMake directly
if (NOT SKBUILD)
  message(WARNING "\
  This CMake file is meant to be executed using 'scikit-build-core'.
  Running it directly will almost certainly not produce the desired
  result. If you are a user trying to install this package, use the
  command below, which will install all necessary build dependencies,
  compile the package in an isolated environment, and then install it.
  =====================================================================
   $ pip install .
  =====================================================================
  If you are a software developer, and this is your own package, then
  it is usually much more efficient to install the build dependencies
  in your environment once and use the following command that avoids
  a costly creation of a new virtual environment at every compilation:
  =====================================================================
   $ pip install nanobind scikit-build-core[pyproject]
   $ pip install --no-build-isolation -ve .
  =====================================================================
  You may optionally add -Ceditable.rebuild=true to auto-rebuild when
  the package is imported. Otherwise, you need to rerun the above
  after editing C++ files.")
endif()

# Find dependencies
#find_package(OpenCV REQUIRED)
#find_package(MetavisionSDK COMPONENTS core stream REQUIRED)
find_package(libcaer REQUIRED)

# Initialize lists for sources and libraries
set(input_libraries "")

# Add Metavision if found
if (${MetavisionSDK_FOUND})    
    message(STATUS "Prophesee dependencies (Metavision) found at ${MetavisionSDK_DIR}")
    add_library(prophesee INTERFACE)
    target_sources(prophesee INTERFACE src/cpp/prophesee.hpp)
    target_link_libraries(prophesee INTERFACE MetavisionSDK::core MetavisionSDK::stream)
    target_include_directories(prophesee INTERFACE ${MetavisionSDK_INCLUDE_DIRS})
    set_property(TARGET prophesee PROPERTY LINKER_LANGUAGE CXX)
    list(APPEND input_libraries prophesee)
endif()

# Add libcaer if found
if (${libcaer_FOUND})    
    message(STATUS "Inivation dependencies (libcaer) found at ${libcaer_DIR}")
    add_library(inivation INTERFACE)
    target_sources(inivation INTERFACE src/cpp/inivation.hpp)
    target_link_libraries(inivation INTERFACE libcaer::caer)
    target_include_directories(inivation INTERFACE ${libcaer_INCLUDE_DIRS})
    set_property(TARGET inivation PROPERTY LINKER_LANGUAGE CXX)
    list(APPEND input_libraries inivation)
endif()

# Find Python and nanobind
find_package(Python COMPONENTS Interpreter Development.Module REQUIRED)
find_package(nanobind CONFIG REQUIRED)

# Create the Python module using nanobind
nanobind_add_module(_event_camera_drivers
    src/cpp/bindings.cpp
)
target_link_libraries(_event_camera_drivers PRIVATE
    ${input_libraries} # link the C++ libraries
)
target_include_directories(_event_camera_drivers PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/event_camera_drivers.cpp)

# Install rules
install(TARGETS _event_camera_drivers LIBRARY DESTINATION .)
