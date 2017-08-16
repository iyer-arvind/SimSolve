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
#ifdef UNITS_SUPPORT
  std::string q_str = "";
#endif
  std::size_t pos = _equation.find('=');
  if(pos!=std::string::npos)
  {
    std::string s;
#ifdef UNITS_SUPPORT
    std::size_t dp_op = _equation.find('[');
    if(dp_op!=std::string::npos)
    {
        std::size_t dp_cl = _equation.find(']');
        assert(dp_cl != std::string::npos);
        q_str = _equation.substr(dp_op+1, dp_cl-dp_op-1);
        _error_expr = std::string("(") + _equation.substr(0, pos) + ") -" + "Units::Quantity("+_equation.substr(pos+1, dp_op-pos-1) + ",\""+q_str+"\");";
        s = std::string("(") + _equation.substr(0, pos) + ") -" + "("+_equation.substr(pos+1, dp_op-pos-1) + ")";
    }
    else
    {
        s = _error_expr = std::string("(") + _equation.substr(0, pos) + ") -" + "(" + _equation.substr(pos+1) + ")";
    }
#else
    {
        s = _error_expr = std::string("(") + _equation.substr(0, pos) + ") -" + "(" + _equation.substr(pos+1) + ")";
    }
#endif
    std::smatch sm;
    std::regex e("[a-zA-Z_][a-zA-Z0-9_]*");
    
    while(std::regex_search(s, sm, e))
    {
        for(auto x:sm)
        {
            //std::cout<<"'"<<x<<"' '"<<sm.suffix().str()<<"'"<<std::endl;
            if(x.str().size() and x.str() != "e")
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
#ifdef UNITS_SUPPORT
    if(q_str.size())
    {
        assert(_parameters.size()==1);
        parameter_factory.set_unit(*_parameters.begin(),Units::Unit(q_str));
    }
#endif
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
    
    stream<<"extern \"C\"\n{\n  void equation_group_"<<_index<<"(const ParameterType * const _current_parameters, const ParameterType * const _set_parameters, ParameterType * _errors);\n}"<<std::endl;
    stream<<"void equation_group_"<<_index<<"(const ParameterType * const _current_parameters, const ParameterType * const _set_parameters, ParameterType * _errors)"<<std::endl;

    stream<<"{\n";
    for(auto const &x:_set_parameters) stream<<"  const ParameterType &"<<x<<" = _set_parameters["<<parameter_factory.get_parameter(x).index()<<"];"<<std::endl;
    cp = 0;
    for(auto const &x:_current_parameters) stream<<"  const ParameterType &"<<x<<" = _current_parameters["<<cp++<<"];"<<std::endl;
    // cp = 0;
    // for(auto const &x:_current_parameters) stream<<"  cout<<_current_parameters["<<cp++<<"]<<std::endl;"<<std::endl;
    
    int ei = 0;
    for(auto const &e:_equations)stream<<"  _errors["<<ei++<<"] ="<<_equation_list[e].error_expr()<<";"<<std::endl;
    // ei = 0;
    // for(auto const &e:_equations)stream<<"  cout<< _errors["<<ei++<<"]<<std::endl;"<<std::endl;
    stream<<"}\n";
    stream<<"\n\n\n";
}

void EquationGroup::load_function(void *dl_handle)
{
    std::stringstream ss;
    ss<<"equation_group_"<<_index;
    _err_func = (ErrorFunction)dlsym(dl_handle, ss.str().c_str());
}

double EquationGroup::evaluate(const std::vector<double> &current_parameters_dbl)
{
    const ParameterVector &set_parameters = parameter_factory.value_vector();
    const ParameterType *set_parameters_ptr = &set_parameters[0];
    
#ifdef UNITS_SUPPORT
    ParameterType current_parameters_ptr [current_parameters_dbl.size()];
    int pp = 0;
    for(auto const &cpn:_current_parameters)
    {
        const ParameterType &cp = parameter_factory.get_parameter(cpn).value();
        current_parameters_ptr[pp] = Units::Quantity(current_parameters_dbl[pp] , cp.unit());
        pp++;
    }
    
#else
    const ParameterType *current_parameters_ptr = &current_parameters_dbl[0];

#endif
    ParameterVector err(_equations.size());
    ParameterType *err_ptr = &err[0];
    
    _err_func(current_parameters_ptr, set_parameters_ptr, err_ptr);
    
    double sqerr = 0;
#ifdef UNITS_SUPPORT
    for(auto &e:err)sqerr+=e.value()*e.value();
#else
    for(auto &e:err)sqerr+=e*e;
#endif
    return sqerr;
}
    
    
}
