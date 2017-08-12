#ifndef PARAMETER_H_INCLUDED
#define PARAMETER_H_INCLUDED

#include <string>
#include <map>
#include <set>
#include <vector>
#include <ostream>

#ifdef UNITS_SUPPORT
#include "units.hpp"
#endif


namespace SimSolve
{

#ifdef UNITS_SUPPORT
typedef Units::Quantity ParameterType;
#else
typedef double ParameterType;
#endif

    
class ParameterFactory;

class Parameter 
{
private:
  int _index;
  std::string _symbol;
  ParameterFactory *_factory;
  
public:
  Parameter(): _index(-1), _symbol(""), _factory(NULL)
  {}
  
  Parameter(int index, std::string symbol, ParameterFactory *factory);
  
  int index() const
  {return _index;}
  
  std::string symbol() const
  {return _symbol;}
  
  ParameterType& value() const;
};

typedef std::set<std::string> ParameterSet;


typedef std::vector<ParameterType> ParameterVector;

class ParameterFactory
{
public:
  typedef std::map<std::string, Parameter> KeyMapType;
 
private:
  KeyMapType _map;
  ParameterVector _value;
  
public:
  ParameterFactory()
  {  
    _value.reserve(100);
  }
  
  Parameter& get_parameter(const std::string& parameter);
  
  void set_value(int index, ParameterType value)
  {_value[index] = value;}
  
  void set_value(const std::string& parameter, ParameterType value)
  {_value[get_parameter(parameter).index()] = value;}
  
#ifdef UNITS_SUPPORT
  void set_unit(const std::string& parameter, const Units::Unit &u)
  {
      _value[get_parameter(parameter).index()].set_unit(u);
  }
#endif
  
  ParameterType& get_value(int index)
  {return _value[index];}
  
  ParameterSet parameters()const;

  int count()const
  {return _map.size();}
  
    ParameterVector &value_vector() {return _value;}
  
  void write_to_stream(std::ostream& stream) const;
  
  
};

extern ParameterFactory parameter_factory;

std::ostream& operator<<(std::ostream& stream, const ParameterFactory& pf);

}
#endif
