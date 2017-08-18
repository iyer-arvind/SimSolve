#ifndef EQUATION_H_INCLUDED
#define EQUATION_H_INCLUDED

#include <exception>
#include <set>
#include <list>
#include <string>

#include "parameter.hpp"

namespace SimSolve
{
  
class InvalidEquation: public std::exception
{
private:
  std::string _msg;

public:
  InvalidEquation(const std::string &msg):
    _msg(msg)
  {}
};

class Equation
{
public:
  static int equation_count;
  static std::set<std::string> functions;
  
private:
  int _index;
  std::string _equation;
  std::string _error_expr;

  ParameterSet _parameters;
  void _parse_equation();
  
    
public:
  Equation(const std::string &equation);
  const std::string& equation() const {return _equation;}
  const ParameterSet& parameters() const {return _parameters;}
  const std::string& error_expr() const {return _error_expr;}
  
};

typedef std::vector<Equation> EquationList;
typedef std::vector<int> EquationIndexList;

class EquationGroup
{
private:
    int _index;
    const EquationList &_equation_list;
    
    ParameterSet _set_parameters;
    ParameterSet _current_parameters;
    EquationIndexList _equations;
    
    typedef void (*ErrorFunction)(const ParameterType * const,
                                  const ParameterType * const,
                                  ParameterType *);
    ErrorFunction _err_func;
public:
    EquationGroup(int index,
                  const EquationList &equation_list,
                  const ParameterSet& set_parameters,
                  const ParameterSet& current_parameters,
                  EquationIndexList equations):
                    _index(index),
                    _equation_list(equation_list),
                    _set_parameters(set_parameters),
                    _current_parameters(current_parameters),
                    _equations(equations),
                    _err_func(NULL)
    {}
    
    void emit_code(std::ostream &stream) const;
    void load_function(void *dl_handle);
    double evaluate(const std::vector<double> &current_parameters);
    void solve();
};

typedef std::list<EquationGroup> EquationGroupList; 

} // namespace SimSolve

#endif
