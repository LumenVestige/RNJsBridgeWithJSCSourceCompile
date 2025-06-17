

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-parameter -fexceptions -fno-omit-frame-pointer")
#set(CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})

include_directories(./)

set(SOURCE glog_init.cpp)


add_library(glog_init
        SHARED
        ${SOURCE})
set_target_properties(glog_init PROPERTIES
        CXX_VISIBILITY_PRESET default
        VISIBILITY_INLINES_HIDDEN OFF
)
target_link_libraries(glog_init
        log
        glog)
