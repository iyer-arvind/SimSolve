#ifndef PARAMETER_H_INCLUDED
#define PARAMETER_H_INCLUDED

#include <string>
#include <map>
#include <set>
#include <vector>
#include <ostream>

namespace SimSolve
{
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
  {
    return _index;
  }
  
  std::string symbol() const
  {
    return _symbol;
  }
  double value() const;
};

typedef std::set<std::string> ParameterSet;
typedef std::vector<double> DoubleVector;

class ParameterFactory
{
public:
  typedef std::map<std::string, Parameter> KeyMapType;
 
private:
  KeyMapType _map;
  DoubleVector _value;
  
public:
  ParameterFactory()
  {  
    _value.reserve(100);
  }
  
  Parameter& get_parameter(const std::string& parameter);
  
  void set_value(int index, double value)
  {_value[index] = value;}
  
  void set_value(const std::string& parameter, double value)
  {_value[get_parameter(parameter).index()] = value;}
  
  double get_value(int index)
  {return _value[index];}
  
  ParameterSet parameters()const;

  int count()const
  {return _map.size();}
  
  DoubleVector &value_vector() {return _value;}
  
  void write_to_stream(std::ostream& stream) const;
  
  
};

extern ParameterFactory parameter_factory;

std::ostream& operator<<(std::ostream& stream, const ParameterFactory& pf);

}
#endif
