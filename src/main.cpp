#include <iostream>
#include "tclap/CmdLine.h"
#include "system.hpp"

int main(int argc, char **argv)
{
    try
    {
        TCLAP::CmdLine cmd("Simultaneous Nonlinear Equation Solver", ' ', "0.0.1");
        TCLAP::ValueArg<std::string> input("i","input","Input file for solving", true,"","filename");
        cmd.add(input);
        TCLAP::MultiSwitchArg verb("V","verbose","Increase Verbosity");
        cmd.add(verb);
        
        cmd.parse(argc, argv);
        
        // Get the value parsed by each arg. 
        std::string  ifile = input.getValue();
        
        SimSolve::Output::ref_level = verb.getValue();
        
        SimSolve::System system(ifile);
        system.solve();
        std::cout << SimSolve::parameter_factory;
    }
    catch (TCLAP::ArgException &e)
	{
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl; 
    }
    
    return 0;
}
