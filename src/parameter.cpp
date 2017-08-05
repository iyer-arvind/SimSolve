#include "parameter.hpp"

#include <cmath>

namespace SimSolve
{

ParameterFactory parameter_factory;
  
  
Parameter::Parameter(int index, std::string symbol, ParameterFactory* factory):
  _index(index),
  _symbol(symbol),
  _factory(factory)
{

}

Parameter& ParameterFactory::get_parameter(const std::string& parameter)
{
  auto i = _map.find(parameter);
  if(i == _map.end())
  {
    _map[parameter] = Parameter(_map.size(), parameter, this);
    _value.push_back(std::nan(""));
  }
  return _map[parameter];
}

double Parameter::value() const
{
  return _factory->get_value(_index);
}

void ParameterFactory::write_to_stream(std::ostream& stream) const
{
  stream<<"Total "<<count()<<" parameters"<<std::endl;
  for(auto const &i :_map) stream<<i.first<<" = "<<i.second.value()<<std::endl;
}

std::set<std::string> ParameterFactory::parameters()const
{
  std::set<std::string> pset;
  for(auto const&i :_map)
  {
    pset.insert(i.first);
  }
  return pset;
}

std::ostream& operator<<(std::ostream& stream, const ParameterFactory& pf)
{
  pf.write_to_stream(stream);
  return stream;
}

}
