macro(ADD_EXT_LIBRARY EXT_LIB_NAME)

add_custom_command(
    OUTPUT ${EXT_LIB_NAME}_ext.cpp
    COMMAND cat ${CMAKE_CURRENT_SOURCE_DIR}/${EXT_LIB_NAME}.hpp | sed '/.*units.hpp.*/d' > header
    COMMAND xxd -i header >${EXT_LIB_NAME}_ext.cpp
)

add_library(${EXT_LIB_NAME} SHARED ${EXT_LIB_NAME}.cpp ${EXT_LIB_NAME}_ext.cpp)
endmacro(ADD_EXT_LIBRARY)
