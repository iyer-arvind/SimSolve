

add_executable(simsolve parameter.cpp equation.cpp system.cpp solve.cpp main.cpp)
target_compile_definitions(simsolve PRIVATE UNITS_SUPPORT)

target_link_libraries(simsolve dl nlopt units)
install(TARGETS simsolve RUNTIME DESTINATION bin)

