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
    o1 <<"Solving\n";
    for(auto const &e:_equations)o1<<"  "<<_equation_list[e].equation()<<"\n";
    o1 <<"---\n";
    
    ParameterVector &set_parameters = parameter_factory.value_vector();
    std::vector<double> current_parameters(_current_parameters.size());
    int cp=0;
    
    for(auto const &p: _current_parameters)
    {
#ifdef UNITS_SUPPORT
        double d = set_parameters[parameter_factory.get_parameter(p).index()].value();
#else
        double d = set_parameters[parameter_factory.get_parameter(p).index()];
#endif
        if(std::isnan(d))
            d = 0.1;
        current_parameters[cp++] = d;
    }
    
    nlopt::opt opt(nlopt::LN_COBYLA, _current_parameters.size());
    
    opt.set_stopval(1e-14);
    opt.set_ftol_rel(1e-14);
    opt.set_ftol_abs(1e-14);
    opt.set_min_objective(&func, this);
    
    double f;
    
    nlopt::result result = opt.optimize(current_parameters, f);
    
    o1<<"Converged to: "<<"\n";
    
    cp = 0;
    for(auto const &p: _current_parameters)
    {
#ifdef UNITS_SUPPORT
        set_parameters[parameter_factory.get_parameter(p).index()].set_value(current_parameters[cp++]);
#else
        set_parameters[parameter_factory.get_parameter(p).index()] = current_parameters[cp++];
#endif
        o1<<" "<<p<<" ="<<set_parameters[parameter_factory.get_parameter(p).index()]<<"\n";
    }
    o1<<"With error: "<<opt.last_optimum_value()<<"\n";
    o1<<"--------------\n";
}   

}
