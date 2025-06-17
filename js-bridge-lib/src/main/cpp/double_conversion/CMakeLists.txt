

file(GLOB SOURCE "double_conversion/bignum.cc"
        "double_conversion/bignum-dtoa.cc"
        "double_conversion/cached-powers.cc"
        "double_conversion/diy-fp.cc"
        "double_conversion/double-conversion.cc"
        "double_conversion/fast-dtoa.cc"
        "double_conversion/fixed-dtoa.cc"
        "double_conversion/strtod.cc ")

add_library(double-conversion
        STATIC
        ${SOURCE}
)