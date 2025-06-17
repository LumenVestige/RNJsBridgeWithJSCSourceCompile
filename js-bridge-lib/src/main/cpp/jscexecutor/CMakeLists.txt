


include_directories(./*)
include_directories(./jsi)
include_directories(./include)

file(GLOB_RECURSE SOURCE
        ./*.cpp
        ../bridge/JSExecutor.cpp
        ../bridge/JSLogging.cpp
        ./OnLoad.cpp
)

add_library(jscexecutor
        SHARED
        ${SOURCE}
)

target_include_directories(jscexecutor
        PRIVATE
        ${CMAKE_SOURCE_DIR}
)

target_link_libraries(jscexecutor
        fbjni
        folly_json
        js-bridge
        JavaScriptCore
)
