# File originally from a rust_qt_binding_generator demo

project (sudoku-gui)

cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
cmake_policy(SET CMP0046 NEW)
cmake_policy(SET CMP0063 NEW)
cmake_policy(SET CMP0071 NEW)
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

### fetch and build rust bindings ###

ExternalProject_Add(
    sudoku_ffi
    PREFIX sudoku_ffi
    #GIT_REPOSITORY file:///home/shared/Code/sudoku/sudoku-ffi
    #GIT_TAG dev
    GIT_REPOSITORY https://github.com/emerentius/sudoku-ffi
    GIT_TAG dd8b141742030f33ee0eb003c8e48726978b37d4
    BUILD_IN_SOURCE 1
    BUILD_COMMAND "${Cargo_EXECUTABLE}" build "${RUST_BUILD_FLAG}"
    CONFIGURE_COMMAND ""
    INSTALL_COMMAND ""
)

ExternalProject_Get_property(sudoku_ffi DOWNLOAD_DIR)
set(sudoku_ffi_crate_dir "${DOWNLOAD_DIR}")
unset(DOWNLOAD_DIR)

ExternalProject_Get_Property(sudoku_ffi SOURCE_DIR)
set(RUST_DIR "${SOURCE_DIR}")
unset(SOURCE_DIR)
SET(RUST_LIB "${RUST_DIR}/${RUST_TARGET_DIR}/libsudoku_ffi.a")
list(APPEND Libs ${RUST_LIB})

list(APPEND Libs Qt5::Widgets Threads::Threads ${CMAKE_DL_LIBS})
set(SRCS src/main.cpp src/mainwindow.cpp src/sudoku_cell_widget.cpp src/sudoku_grid_widget.cpp)

add_executable(sudoku-gui ${SRCS})
target_include_directories(sudoku-gui PRIVATE src)
target_include_directories(sudoku-gui PRIVATE "${sudoku_ffi_crate_dir}")

# rust libraries only exist after sudoku-ffi was built
add_dependencies(sudoku-gui sudoku_ffi)
target_link_libraries(sudoku-gui ${Libs})
set_target_properties(sudoku-gui PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

# Copy resource files such as icons to build dir
# so the executable can find them.
# Does this work with in-source builds? Don't know, don't care.
file(COPY "icons" DESTINATION "${CMAKE_BINARY_DIR}")
