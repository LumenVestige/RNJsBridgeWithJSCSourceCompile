add_library(trace SHARED
        TraceSection.cpp
)
target_link_libraries(
        trace
        android
)