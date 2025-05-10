cmake_minimum_required(VERSION 3.5)

project(vector-engine LANGUAGES CXX VERSION 0.0.0)

set(VECTOR_ENGINE_CXX_STANDARD 17)
message(STATUS "vector-engine build with CXX standard: c++${VECTOR_ENGINE_CXX_STANDARD}")

if(NOT WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++${VECTOR_ENGINE_CXX_STANDARD}")
else()
    if(MSVC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++${VECTOR_ENGINE_CXX_STANDARD}")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++${VECTOR_ENGINE_CXX_STANDARD}")
    endif()
endif()

# libuv dependency
find_path(VECTOR_ENGINE_ASYNC_LIB_HEADER NAMES uv.h)
find_library(VECTOR_ENGINE_ASYNC_LIB uv)

set(VECTOR_ENGINE_HEADER_DIR src)

set(VECTOR_ENGINE_SOURCE_DIR src/sw/vector-engine)

set(VECTOR_ENGINE_LIB_SOURCES
    ""
)

set(VECTOR_ENGINE_APP_SOURCES
        "${VECTOR_ENGINE_SOURCE_DIR}/connection.cpp"
        "${VECTOR_ENGINE_SOURCE_DIR}/reactor.cpp"
        "${VECTOR_ENGINE_SOURCE_DIR}/read_buffer.cpp"
        "${VECTOR_ENGINE_SOURCE_DIR}/resp.cpp"
        "${VECTOR_ENGINE_SOURCE_DIR}/uv_utils.cpp"
        "${VECTOR_ENGINE_SOURCE_DIR}/worker.cpp"
        "${VECTOR_ENGINE_SOURCE_DIR}/utils.cpp"
        "${VECTOR_ENGINE_SOURCE_DIR}/ping_task.cpp"
        "${VECTOR_ENGINE_SOURCE_DIR}/unknown_task.cpp"
        "${VECTOR_ENGINE_SOURCE_DIR}/protocol.cpp"
        "${VECTOR_ENGINE_SOURCE_DIR}/main.cpp"
)

list(APPEND VECTOR_ENGINE_APP_SOURCES ${VECTOR_ENGINE_LIB_SOURCES})

# Build static library
option(VECTOR_ENGINE_BUILD_STATIC "Build static library" ON)
message(STATUS "vector-engine build static library: ${VECTOR_ENGINE_BUILD_STATIC}")

#if(VECTOR_ENGINE_BUILD_STATIC)
#    set(STATIC_LIB vector-engine_static)
#
#    add_library(${STATIC_LIB} STATIC ${VECTOR_ENGINE_LIB_SOURCES})
#    add_library(vector-engine::${STATIC_LIB} ALIAS ${STATIC_LIB})
#
#    list(APPEND VECTOR_ENGINE_TARGETS ${STATIC_LIB})
#
#    target_include_directories(${STATIC_LIB} PUBLIC
#            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${VECTOR_ENGINE_HEADER_DIR}>
#            $<BUILD_INTERFACE:${VECTOR_ENGINE_ASYNC_LIB_HEADER}>
#            $<INSTALL_INTERFACE:include>)
#    target_link_libraries(${STATIC_LIB} PUBLIC ${VECTOR_ENGINE_ASYNC_LIB})
#
#    if (WIN32)
#        target_compile_definitions(${STATIC_LIB} PRIVATE NOMINMAX)
#        set_target_properties(${STATIC_LIB} PROPERTIES CXX_STANDARD ${VECTOR_ENGINE_CXX_STANDARD})
#        set_target_properties(${STATIC_LIB} PROPERTIES OUTPUT_NAME vector-engine_static)
#    else()
#        target_compile_options(${STATIC_LIB} PRIVATE "-Wall" "-Wextra" "-Werror")
#        set_target_properties(${STATIC_LIB} PROPERTIES OUTPUT_NAME vector-engine)
#    endif()
#
#    set_target_properties(${STATIC_LIB} PROPERTIES CLEAN_DIRECT_OUTPUT 1)
#    set_target_properties(${STATIC_LIB} PROPERTIES CXX_EXTENSIONS OFF)
#
#    option(VECTOR_ENGINE_BUILD_STATIC_WITH_PIC "Build static library with position independent code" ON)
#    message(STATUS "vector-engine build static library with position independent code: ${VECTOR_ENGINE_BUILD_STATIC_WITH_PIC}")
#
#    if(VECTOR_ENGINE_BUILD_STATIC_WITH_PIC)
#        set_target_properties(${STATIC_LIB} PROPERTIES POSITION_INDEPENDENT_CODE ON)
#    endif()
#endif()
#
## Build shared library
#option(VECTOR_ENGINE_BUILD_SHARED "Build shared library" ON)
#message(STATUS "vector-engine build shared library: ${VECTOR_ENGINE_BUILD_SHARED}")
#
#if(VECTOR_ENGINE_BUILD_SHARED)
#    set(SHARED_LIB vector-engine)
#
#    add_library(${SHARED_LIB} SHARED ${VECTOR_ENGINE_LIB_SOURCES})
#    add_library(vector-engine::${SHARED_LIB} ALIAS ${SHARED_LIB})
#    list(APPEND VECTOR_ENGINE_TARGETS ${SHARED_LIB})
#
#    target_include_directories(${SHARED_LIB} PUBLIC
#            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${VECTOR_ENGINE_HEADER_DIR}>
#            $<BUILD_INTERFACE:${VECTOR_ENGINE_ASYNC_LIB_HEADER}>
#            $<INSTALL_INTERFACE:include>)
#    target_link_libraries(${SHARED_LIB} PUBLIC ${VECTOR_ENGINE_ASYNC_LIB})
#
#    if(WIN32)
#        target_compile_definitions(${SHARED_LIB} PRIVATE NOMINMAX)
#        set_target_properties(${SHARED_LIB} PROPERTIES CXX_STANDARD ${VECTOR_ENGINE_CXX_STANDARD})
#        set_target_properties(${SHARED_LIB} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
#    else()
#        target_compile_options(${SHARED_LIB} PRIVATE "-Wall" "-Wextra" "-Werror")
#    endif()
#
#    set_target_properties(${SHARED_LIB} PROPERTIES OUTPUT_NAME vector-engine)
#    set_target_properties(${SHARED_LIB} PROPERTIES CLEAN_DIRECT_OUTPUT 1)
#    set_target_properties(${SHARED_LIB} PROPERTIES CXX_EXTENSIONS OFF)
#    set_target_properties(${SHARED_LIB} PROPERTIES POSITION_INDEPENDENT_CODE ON)
#    set_target_properties(${SHARED_LIB} PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR})
#endif()

# Build application
option(VECTOR_ENGINE_BUILD_APP "Build application" ON)
message(STATUS "vector-engine build application: ${VECTOR_ENGINE_BUILD_APP}")

if(VECTOR_ENGINE_BUILD_APP)
    set(VECTOR_ENGINE_APP vector-engine-app)

    add_executable(${VECTOR_ENGINE_APP} ${VECTOR_ENGINE_APP_SOURCES})

    list(APPEND VECTOR_ENGINE_TARGETS ${VECTOR_ENGINE_APP})

    target_include_directories(${VECTOR_ENGINE_APP} PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${VECTOR_ENGINE_HEADER_DIR}>
            $<BUILD_INTERFACE:${VECTOR_ENGINE_ASYNC_LIB_HEADER}>
            $<INSTALL_INTERFACE:include>)
    target_link_libraries(${VECTOR_ENGINE_APP} PUBLIC ${VECTOR_ENGINE_ASYNC_LIB})

    if (WIN32)
        target_compile_definitions(${VECTOR_ENGINE_APP} PRIVATE NOMINMAX)
        set_target_properties(${VECTOR_ENGINE_APP} PROPERTIES CXX_STANDARD ${VECTOR_ENGINE_CXX_STANDARD})
        set_target_properties(${VECTOR_ENGINE_APP} PROPERTIES OUTPUT_NAME vector-engine)
    else()
        target_compile_options(${VECTOR_ENGINE_APP} PRIVATE "-Wall" "-Wextra" "-Werror")
        set_target_properties(${VECTOR_ENGINE_APP} PROPERTIES OUTPUT_NAME vector-engine)
    endif()

    set_target_properties(${STATIC_LIB} PROPERTIES CLEAN_DIRECT_OUTPUT 1)
    set_target_properties(${STATIC_LIB} PROPERTIES CXX_EXTENSIONS OFF)
endif()

include(GNUInstallDirs)

install(TARGETS ${VECTOR_ENGINE_TARGETS}
        EXPORT vector-engine-targets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Install headers.
set(HEADER_PATH "sw/vector-engine")
file(GLOB HEADERS
    "${VECTOR_ENGINE_SOURCE_DIR}/*.h*")

install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${HEADER_PATH})
