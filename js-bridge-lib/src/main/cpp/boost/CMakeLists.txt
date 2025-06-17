

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DBOOST_REGEX_CONFIG_HPP=TRUE")
#set(CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})

include_directories(./boost_1_63_0)
file(GLOB SOURCE ./boost_1_63_0/boost/*.hpp)

add_library(boost
        STATIC
        ${SOURCE})

SET_TARGET_PROPERTIES(boost PROPERTIES LINKER_LANGUAGE CXX)