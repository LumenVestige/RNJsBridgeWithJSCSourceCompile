

include_directories(./)

file(GLOB SOURCE *.cpp)
add_library(cxxmodule
        SHARED
        ${SOURCE})

target_link_libraries(cxxmodule
        js-bridge)
