

add_executable(simsolve parameter.cpp equation.cpp system.cpp solve.cpp main.cpp)
if(${Units_FOUND})
message(info "Units found, enabling units support")
target_compile_definitions(simsolve PRIVATE UNITS_SUPPORT)
target_compile_definitions(simsolve PRIVATE UNITS_INCLUDE_DIR=${Units_INCLUDE_DIRS})
endif(${Units_FOUND})

target_link_libraries(simsolve dl nlopt units)
install(TARGETS simsolve RUNTIME DESTINATION bin)

