cmake_minimum_required(VERSION 3.8)

# Link this 'library' to use the following warnings
add_library(project_warnings INTERFACE)

if(CMAKE_COMPILER_IS_GNUCC)
  option(ENABLE_COVERAGE "Enable coverage reporting for gcc/clang" FALSE)

  if(ENABLE_COVERAGE)
    add_compile_options(--coverage -O0)
  endif()
endif()

if(MSVC)
  target_compile_options(project_warnings INTERFACE /W4)
else()
  target_compile_options(project_warnings
    INTERFACE
    -Wall
    -Og
    -g
    -Wextra # reasonable and standard
    -Wshadow # warn the user if a variable declaration shadows one from a
    # parent context
    -Wnon-virtual-dtor # warn the user if a class with virtual functions has a
    # non-virtual destructor. This helps catch hard to
    # track down memory errors
    -Wold-style-cast # warn for c-style casts
    -Wcast-align # warn for potential performance problem casts
    -Wunused # warn on anything being unused
    -Woverloaded-virtual # warn if you overload (not override) a virtual
    # function
    -Wpedantic # warn if non-standard C++ is used
    -Wconversion # warn on type conversions that may lose data
    -Wsign-conversion # warn on sign conversions
    -Wmisleading-indentation # warn if identation implies blocks where blocks
    # do not exist
    -Wduplicated-cond # warn if if / else chain has duplicated conditions
    -Wduplicated-branches # warn if if / else branches have duplicated code
    -Wlogical-op # warn about logical operations being used where bitwise were
    # probably wanted
    -Wnull-dereference # warn if a null dereference is detected
    -Wuseless-cast # warn if you perform a cast to the same type
    -Wdouble-promotion # warn if float is implicit promoted to double
    -Wformat=2 # warn on security issues around functions that format output
    # (ie printf)
    )
endif()

# Install conan dependencies
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_executable(sokoban src/main.cpp src/sdl2.cpp)
target_compile_features(sokoban PRIVATE cxx_std_14)
target_link_libraries(sokoban PRIVATE ${CONAN_LIBS} project_warnings --coverage)

enable_testing()

add_executable(tester tests/main.cpp)
target_link_libraries(tester PRIVATE project_warnings --coverage)
add_test(Tester tester)
