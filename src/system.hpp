#ifndef SYSTEM_HPP_INCLUDED
#define SYSTEM_HPP_INCLUDED

#include "equation.hpp"

namespace SimSolve
{
  

class System
{
private:
  std::map<std::string, void *> _external_libs;
  EquationList _equation_list;
  EquationGroupList _equation_groups;
  
  void _load_external_library(std::string);
  void _partition_equations(ParameterSet);
  void* _emit_code() const;
  void *_dl_handle;
  
  void execute_equation_group(int g);
  
public:
  System(const std::string& fname);
  void solve();
  virtual ~System();
  
};
}

#endif
