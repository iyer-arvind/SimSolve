#include "equation.hpp"

#include <iostream>
#include <sstream>
#include <regex>
#include <cmath>

#include <dlfcn.h>

namespace SimSolve
{

int Equation::equation_count = 0;
std::set<std::string> Equation::functions {"cos", "sin", "tan", "acos", "asin", "atan", "atan2", "cosh", "sinh", "tanh", "acosh", "asinh", "atanh", "exp", "log", "log10", "exp2", "expm1", "log1p", "log2", "logb", "pow", "sqrt", "cbrt", "hypot", "erf", "erfc"};


Equation::Equation(const std::string &equation):
  _index(Equation::equation_count++),
  _equation(equation)
{
  _parse_equation();
}

void Equation::_parse_equation()
{
  std::size_t pos = _equation.find('=');
  if(pos!=std::string::npos)
  {
    std::stringstream stream;
    stream<<"("<<_equation.substr(0, pos)<<")-("<<_equation.substr(pos+1)<<")";
    _error_expr = stream.str();
    
    std::smatch sm;
    std::regex e("[a-zA-Z][a-zA-Z0-9]*");
    std::string s = _error_expr;
    while(std::regex_search(s, sm, e))
    {
      for(auto x:sm)
      {
        //std::cout<<"'"<<x<<"' '"<<sm.suffix().str()<<"'"<<std::endl;
        if(x.str().size())
        {
            if(Equation::functions.find(x.str()) == Equation::functions.end())
            {
                parameter_factory.get_parameter(x.str()).index();
                //std::cout << "Found variable: " << x << " idx: " << idx << std::endl;
                _parameters.insert(x.str());
            }
        }
        s = sm.suffix().str();
      }
    };
  }
  else
  {
    throw InvalidEquation(_equation);
  }
}

void EquationGroup::emit_code(std::ostream &stream) const
{
    stream<<"// Equation Group "<<_index<<std::endl;
    stream<<"// "<<std::endl;
    stream<<"// Following variables have already been set:"<<std::endl;
    for(auto const &x:_set_parameters) stream<<"//   "<<x<<" = _set_parameters["<<parameter_factory.get_parameter(x).index()<<"]"<<std::endl;
    stream<<"// Following variables will be set now:"<<std::endl;    
    int cp = 0;
    for(auto const &x:_current_parameters) stream<<"//   "<<x<<"= current_parameters["<<cp++<<"]"<<std::endl;
    stream<<"extern \"C\"\n{\n  void equation_group_"<<_index<<"(const double * const _current_parameters, const double * const _set_parameters, double * _errors);\n}"<<std::endl;
    stream<<"void equation_group_"<<_index<<"(const double * const _current_parameters, const double * const _set_parameters, double * _errors)"<<std::endl;
    stream<<"{\n";
    for(auto const &x:_set_parameters) stream<<"  const double &"<<x<<" = _set_parameters["<<parameter_factory.get_parameter(x).index()<<"];"<<std::endl;
    cp = 0;
    for(auto const &x:_current_parameters) stream<<"  const double &"<<x<<" = _current_parameters["<<cp++<<"];"<<std::endl;
    
    int ei = 0;
    for(auto const &e:_equations)stream<<"  _errors["<<ei++<<"] ="<<_equation_list[e].error_expr()<<";"<<std::endl;
    stream<<"}\n";
    stream<<"\n\n\n";
}

void EquationGroup::load_function(void *dl_handle)
{
    std::stringstream ss;
    ss<<"equation_group_"<<_index;
    _err_func = (ErrorFunction)dlsym(dl_handle, ss.str().c_str());
}

double EquationGroup::evaluate(const DoubleVector &current_parameters)
{
    const DoubleVector &set_parameters = parameter_factory.value_vector();
    const double *set_parameters_ptr = &set_parameters[0];
    const double *current_parameters_ptr = &current_parameters[0];
    DoubleVector err(_equations.size());
    double *err_ptr = &err[0];
    
    _err_func(current_parameters_ptr, set_parameters_ptr, err_ptr);
    
    double sqerr = 0;
    for(auto &e:err)sqerr+=e*e;
    
    return sqerr;
}
    
    
}
