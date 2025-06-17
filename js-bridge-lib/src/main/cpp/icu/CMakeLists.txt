

add_library(ICU_data STATIC IMPORTED GLOBAL)
set_target_properties(ICU_data PROPERTIES IMPORTED_LOCATION
        "${CMAKE_CURRENT_SOURCE_DIR}/${ANDROID_ABI}/libicudata.a")
add_library(ICU::data ALIAS ICU_data)


add_library(ICU_I18N STATIC IMPORTED GLOBAL)
set_target_properties(ICU_I18N PROPERTIES IMPORTED_LOCATION
        "${CMAKE_CURRENT_SOURCE_DIR}/${ANDROID_ABI}/libicui18n.a")
add_library(ICU::i18n ALIAS ICU_I18N)


add_library(ICU_UC STATIC IMPORTED GLOBAL)
set_target_properties(ICU_UC PROPERTIES IMPORTED_LOCATION
        "${CMAKE_CURRENT_SOURCE_DIR}/${ANDROID_ABI}/libicuuc.a")
add_library(ICU::uc ALIAS ICU_UC)
