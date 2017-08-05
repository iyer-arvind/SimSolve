#include "equation.hpp"

#include <cmath>
#include <iostream>

#include <nlopt.hpp>


namespace SimSolve
{

double func(const std::vector<double> &x, std::vector<double> &grad, void* _f_data)
{
    EquationGroup *grp = (EquationGroup*)_f_data;
    return grp->evaluate(x);
}

void EquationGroup::solve()
{
    std::cout <<"Solving group"<<_index<<std::endl;
    for(auto const &e:_equations)std::cout<<_equation_list[e].equation()<<std::endl;
    std::cout <<"---"<<std::endl;
    
    DoubleVector &set_parameters = parameter_factory.value_vector();
    DoubleVector current_parameters(_current_parameters.size());
    int cp=0;
    
    for(auto const &p: _current_parameters)
    {
        double d = set_parameters[parameter_factory.get_parameter(p).index()];
        if(std::isnan(d))
            d = 0.1;
        current_parameters[cp++] = d;
    }
    
    nlopt::opt opt(nlopt::LN_BOBYQA, _current_parameters.size());
    
    opt.set_stopval(1e-8);
    opt.set_ftol_rel(1e-8);
    opt.set_ftol_abs(1e-8);
    opt.set_min_objective(&func, this);
    
    double f;
    
    nlopt::result result = opt.optimize(current_parameters, f);
    
    std::cout <<"Converged to: [";
    for(auto const &x: current_parameters)std::cout<<x<<' ';
    std::cout<<"]\n--------------"<<std::endl;
    
    cp = 0;
    for(auto const &p: _current_parameters)set_parameters[parameter_factory.get_parameter(p).index()] = current_parameters[cp++];
    
}   

}
