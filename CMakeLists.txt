cmake_minimum_required(VERSION 3.25)
project(smu-server-sdk-c LANGUAGES C VERSION 1.0.0)


option(SDK_C_BUILD_TESTS "Need to build tests?" ON)
option(SDK_C_ENABLE_SANITIZERS "Enable sanitizers (only for Linux)?" OFF)


set(SDK_NAME smu-server-sdk-c)
set(SDK_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(SDK_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(CMAKE_INSTALL_PREFIX /usr CACHE PATH "Install path prefix" FORCE)


# VERSIONING
set(ABI_VERSION 1)

set(PARSON_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/helpers/parson/parson.c)


file(GLOB_RECURSE SDK_SOURCES "${SDK_SRC_DIR}/*.c")
file(GLOB_RECURSE SDK_HEADERS "${SDK_INCLUDE_DIR}/*.h")


add_library(${SDK_NAME} SHARED ${SDK_SOURCES} ${SDK_HEADERS} ${PARSON_SOURCES})
target_include_directories(${SDK_NAME} PUBLIC ${SDK_INCLUDE_DIR})


set_target_properties(${SDK_NAME} PROPERTIES VERSION 1.0 SOVERSION 1)


install(TARGETS ${SDK_NAME}
        LIBRARY DESTINATION lib/smu-server/sdk/c
        ARCHIVE DESTINATION lib/smu-server/sdk/c
        RUNTIME DESTINATION lib/smu-server/sdk/c)


install(DIRECTORY ${SDK_INCLUDE_DIR}/
        DESTINATION include/smu-server/sdk/c
        FILES_MATCHING PATTERN "*.h")


configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/include/version.h.in
  ${CMAKE_CURRENT_SOURCE_DIR}/include/version.h
  @ONLY
)


target_compile_options(${SDK_NAME} PRIVATE
  -pedantic
  -Wall -Wextra
  -Wcast-align
  -Wunused
  -Wpedantic
  -Wconversion
  -Wsign-conversion
  -Wlogical-op
  -Wuseless-cast
)


if(CMAKE_BUILD_TYPE STREQUAL "Release")
  message(STATUS "Enabling O3 optimization")
  target_compile_options(${SDK_NAME} PRIVATE $<$<CONFIG:Release>:-O3>)
endif()


if((CMAKE_C_COMPILER_ID MATCHES "Clang|GNU") AND SDK_C_ENABLE_SANITIZERS)
    message(STATUS "Using GCC/Clang sanitizer configuration")

    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -Wextra -Wpedantic -g")
    add_compile_definitions(_GLIBCXX_DEBUG)

    set(SANITIZER_FLAGS "-fsanitize=address,undefined,leak -fno-omit-frame-pointer")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${SANITIZER_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} ${SANITIZER_FLAGS}")
endif()



# Configuring tests
if(${SDK_C_BUILD_TESTS})
    message(STATUS "Configuring tests...")
    enable_testing()

    include(FetchContent)
    FetchContent_Declare(
        unity
        GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
        GIT_TAG v2.6.1
        EXCLUDE_FROM_ALL
    )
    FetchContent_MakeAvailable(unity)

    file(GLOB TEST_SUBDIRS LIST_DIRECTORIES true "tests/*")

    foreach(TEST_DIR ${TEST_SUBDIRS})
        if(IS_DIRECTORY ${TEST_DIR})
            get_filename_component(TEST_NAME ${TEST_DIR} NAME)

            message(STATUS "Found test: ${TEST_NAME}")

            set(TARGET_NAME "test_${TEST_NAME}")

            file(GLOB_RECURSE CURRENT_TEST_SOURCES "${TEST_DIR}/*.c")
            file(GLOB_RECURSE CURRENT_TEST_HEADERS "${TEST_DIR}/*.h")

            add_executable(${TARGET_NAME} ${CURRENT_TEST_SOURCES} ${CURRENT_TEST_HEADERS})

            target_link_libraries(${TARGET_NAME} PRIVATE ${SDK_NAME} unity)

            target_include_directories(${TARGET_NAME} PRIVATE ${SDK_NAME})

            add_test(NAME ${TARGET_NAME} COMMAND ${TARGET_NAME})
        endif()
    endforeach()
endif()



# =========== PACKAGING ===========

# Get system processor
string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" SYSTEM_ARCH)

# Normalize common Debian architectures
if(SYSTEM_ARCH STREQUAL "x86_64")
    set(SYSTEM_ARCH "amd64")
elseif(SYSTEM_ARCH STREQUAL "i386" OR SYSTEM_ARCH STREQUAL "i686")
    set(SYSTEM_ARCH "i386")
elseif(SYSTEM_ARCH STREQUAL "aarch64")
    set(SYSTEM_ARCH "arm64")
endif()


# Enable DEB generator
set(CPACK_GENERATOR "DEB")

# Tell CPack to create separate packages for components
set(CPACK_DEB_COMPONENT_INSTALL ON)

set(CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})

# Common package metadata
set(CPACK_PACKAGE_NAME ${SDK_NAME})
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Maksim Shchavelev <maksimshchavelev@gmail.com>")
set(CPACK_PACKAGE_FILE_NAME "${SDK_NAME}-${CPACK_PACKAGE_VERSION}")

# --- Runtime component ---
install(TARGETS ${SDK_NAME}
    COMPONENT runtime
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

# --- Development component ---
# Install all headers + parson.h
install(DIRECTORY
    include/
    COMPONENT dev
    DESTINATION include/smu-server/sdk/c
)

install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/helpers/parson/parson.h
    DESTINATION include/smu-server/sdk/c/helpers/parson
    COMPONENT dev
)

# Define components
set(CPACK_COMPONENTS_ALL runtime dev)

set(CPACK_DEBIAN_ENABLE_COMPONENT_DEPENDS ON)

# Runtime package settings
set(CPACK_COMPONENT_RUNTIME_DESCRIPTION "Runtime libraries for ${SDK_NAME}")
set(CPACK_DEBIAN_RUNTIME_PACKAGE_NAME "${SDK_NAME}")
set(CPACK_DEBIAN_RUNTIME_PACKAGE_SECTION "libs")
set(CPACK_DEBIAN_RUNTIME_FILE_NAME "${SDK_NAME}-${PROJECT_VERSION}-linux-${SYSTEM_ARCH}.deb")

# Development package settings
set(CPACK_COMPONENT_DEV_DESCRIPTION "Development files for ${SDK_NAME} (headers, parson, cmake configs)")
set(CPACK_DEBIAN_DEV_PACKAGE_NAME "${SDK_NAME}-dev")
set(CPACK_DEBIAN_DEV_PACKAGE_SECTION "devel")
set(CPACK_DEBIAN_DEV_FILE_NAME "${SDK_NAME}-dev-${PROJECT_VERSION}-linux-${SYSTEM_ARCH}.deb")
# dev package depends on runtime package
set(CPACK_DEBIAN_DEV_PACKAGE_DEPENDS "${SDK_NAME} (= ${PROJECT_VERSION})")

# Include CPack
include(CPack)

