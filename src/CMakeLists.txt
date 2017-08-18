

add_executable(simsolve parameter.cpp equation.cpp system.cpp solve.cpp main.cpp utils.cpp)
if(${Units_FOUND})
message("Units found, enabling units support")
target_compile_definitions(simsolve PRIVATE UNITS_SUPPORT)

ADD_CUSTOM_COMMAND(
        OUTPUT units_header_text.c
	COMMAND cat ${Units_INCLUDE_DIRS}/units.hpp ${Units_INCLUDE_DIRS}/units_math.hpp >units_header
	COMMAND xxd -i units_header units_header_text.c
        COMMENT "Compiling units_header_text.h to binary")
add_library(units_header_text units_header_text.c)
target_link_libraries(simsolve units units_header_text)
endif(${Units_FOUND})

target_link_libraries(simsolve dl nlopt)
install(TARGETS simsolve RUNTIME DESTINATION bin)


macro(TESTSIM SIMFILE)
	add_test(NAME "Testing-${SIMFILE}"
		 COMMAND simsolve -i ${SIMFILE}
		 WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/examples
		 )
endmacro(TESTSIM)

file(GLOB SIMS ${PROJECT_SOURCE_DIR}/examples/*.sims)
foreach(SIM ${SIMS})
    message("Adding ${SIM}")
    TESTSIM(${SIM})
endforeach(SIM)

