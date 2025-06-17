
#include_directories(./)
#include_directories(../)
#include_directories(./glog/src)

set(SOURCE
        glog/src/demangle.cc
        glog/src/logging.cc
        glog/src/raw_logging.cc
        glog/src/signalhandler.cc
        glog/src/symbolize.cc
        glog/src/utilities.cc
        glog/src/vlog_is_on.cc)

add_library(glog
        SHARED
        ${SOURCE}
)
target_include_directories(glog PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../)
set_target_properties(glog PROPERTIES
        CXX_VISIBILITY_PRESET default
        VISIBILITY_INLINES_HIDDEN OFF
)