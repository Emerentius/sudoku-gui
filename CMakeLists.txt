# File originally from a rust_qt_binding_generator demo

project (sudoku-gui)

cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
cmake_policy(SET CMP0046 NEW)
cmake_policy(SET CMP0063 NEW)
cmake_policy(SET CMP0071 NEW)
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

string(TOUPPER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_UPPER)
if(CMAKE_BUILD_TYPE_UPPER STREQUAL DEBUG)
    set(RUST_TARGET_DIR target/debug)
    set(RUST_BUILD_FLAG)
else()
    set(RUST_TARGET_DIR target/release)
    set(RUST_BUILD_FLAG --release)
endif()

include(ExternalProject)

### find dependencies ###

include(FeatureSummary)
find_package(Cargo REQUIRED)
find_package(Rust REQUIRED)
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
find_package(Threads REQUIRED)

set(QT_MIN_VERSION "5.9.5")
find_package(Qt5 "${QT_MIN_VERSION}" CONFIG
    REQUIRED COMPONENTS
        Widgets
)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

### fetch and build rust bindings ###

ExternalProject_Add(
    sudoku_ffi
    PREFIX sudoku_ffi
    GIT_REPOSITORY https://github.com/emerentius/sudoku-ffi
    GIT_TAG f076bfd676ea648a38122e5c57887f60c43397a3
    BUILD_IN_SOURCE 1
    BUILD_COMMAND "${Cargo_EXECUTABLE}" build "${RUST_BUILD_FLAG}"
    CONFIGURE_COMMAND ""
    INSTALL_COMMAND ""
)

# This looks like it should work, but cmake can apparently see through the
# the variables to where the directory is coming from
# and determines that the static library built by the external project is
# actually not a static library, because external projects are hardcoded as UTILITY
# Therefore, it forbids linking.
#set(RUST_DIR ExternalProject_Get_Property(sudoku_ffi SOURCE_DIR))

# so instead, we hack it with the wrong variable
# so it's at least sorta position independent
SET(RUST_DIR "${CMAKE_CURRENT_BINARY_DIR}/sudoku_ffi/src/sudoku_ffi")
SET(RUST_LIB "${RUST_DIR}/${RUST_TARGET_DIR}/libsudoku_ffi.a")
list(APPEND Libs ${RUST_LIB})

list(APPEND Libs Qt5::Widgets Threads::Threads ${CMAKE_DL_LIBS})
set(SRCS main.cpp mainwindow.cpp sudoku_cell_widget.cpp sudoku_grid_widget.cpp)
add_executable(sudoku-gui ${SRCS})

# rust libraries only exist after sudoku-ffi was built
add_dependencies(sudoku-gui sudoku_ffi)
target_link_libraries(sudoku-gui ${Libs})
set_target_properties(sudoku-gui PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
)
