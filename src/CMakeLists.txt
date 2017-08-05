

add_executable(simsolve parameter.cpp equation.cpp system.cpp solve.cpp main.cpp)
target_link_libraries(simsolve dl nlopt)
install(TARGETS simsolve RUNTIME DESTINATION bin)

