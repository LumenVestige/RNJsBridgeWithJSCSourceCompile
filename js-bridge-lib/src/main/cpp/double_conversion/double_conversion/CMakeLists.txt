
set(CXX11_FLAGS "${CXX11_FLAGS} -Wno-unused-local-typedefs")
set(CMAKE_C_FLAGS ${CXX11_FLAGS})
set(CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu++0x -D__STDC_INT64__")
set(headers
        bignum.h
        cached-powers.h
        diy-fp.h
        double-conversion.h
        fast-dtoa.h
        fixed-dtoa.h
        ieee.h
        strtod.h
        utils.h
        ${headers}
        )
#file(GLOB SOURCE "double-conversion/bignum.cc"
#        "double-conversion/bignum-dtoa.cc"
#        "double-conversion/cached-powers.cc"
#        "double-conversion/diy-fp.cc"
#        "double-conversion/double-conversion.cc"
#        "double-conversion/fast-dtoa.cc"
#        "double-conversion/fixed-dtoa.cc"
#        "double-conversion/strtod.cc ")

add_library(double-conversion
        STATIC
        bignum.cc
        bignum-dtoa.cc
        cached-powers.cc
        diy-fp.cc
        double-conversion.cc
        fast-dtoa.cc
        fixed-dtoa.cc
        strtod.cc
        )