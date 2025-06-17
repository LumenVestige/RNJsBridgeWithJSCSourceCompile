

include_directories(.)

set(SOURCE "folly/json.cpp"
        "folly/Unicode.cpp"
        "folly/Conv.cpp"
        "folly/Demangle.cpp"
        "folly/memory/detail/MallocImpl.cpp"
        "folly/String.cpp"
        "folly/dynamic.cpp"
        "folly/Format.cpp"
        "folly/json_pointer.cpp"
        "folly/lang/ColdClass.cpp"
        "folly/detail/Demangle.cpp"
        "folly/hash/SpookyHashV2.cpp"
        "folly/container/detail/F14Table.cpp"
        "folly/ScopeGuard.cpp")


add_library(folly_json
        SHARED
        ${SOURCE}
)
set_target_properties(folly_json PROPERTIES
        CXX_VISIBILITY_PRESET default
        VISIBILITY_INLINES_HIDDEN OFF
)
target_link_libraries(folly_json
        glog
        double-conversion
        boost)
#target_include_directories(folly_json
#        PRIVATE
#        ${CMAKE_SOURCE_DIR}/double_conversion
#)