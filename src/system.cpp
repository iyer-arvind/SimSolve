#include "system.hpp"

#include <fstream>
#include <iostream>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <cstdlib>
#include <algorithm>

#include <assert.h>
#include <dlfcn.h>

void trim(std::string& str) 
{
  size_t endpos = str.find_last_not_of(" \t\r\n");
  if(std::string::npos != endpos )
    str = str.substr(0, endpos+1);
  
  size_t startpos = str.find_first_not_of(" \t\r\n");
  if(std::string::npos != startpos )
    str = str.substr(startpos);
}
namespace SimSolve
{
System::System(const std::string& fname)
{
  _equation_list.reserve(100);
  char line[1000];
  std::ifstream ifh(fname);
  while(! ifh.eof())
  {
    ifh.getline(line, 1000);
    std::string l = line;
    trim(l);
    if(! l.length()) continue;
    if(l.find('=') != std::string::npos)
    {
      _equation_list.push_back(Equation(l));
    }
    if(l.find('~') != std::string::npos)
    {
      int pos = l.find('~');
      std::string p = l.substr(0, pos);
      trim(p);
      parameter_factory.set_value(p, std::stof(l.substr(pos+1)));
    }
  }
  _partition_equations();
  _dl_handle = _emit_code();
  
  for(auto &eg:_equation_groups)
  {
        eg.load_function(_dl_handle);
  }
}

void System::_partition_equations()
{
  
  ParameterSet all_parameters = parameter_factory.parameters();
  ParameterSet unset_parameters = all_parameters;
  ParameterSet set_parameters;
  
  std::vector<int> unsolved_eqns;
  unsolved_eqns.reserve(all_parameters.size());
  
  for(int i=0;i<_equation_list.size();i++)
    unsolved_eqns.push_back(i);
  
  std::cout<<"Strategy: "<<std::endl;
  // While there are still parameters left to be solved for
  while(unset_parameters.size())
  {
    if(unsolved_eqns.size() != unset_parameters.size())
    {
        std::cout<<parameter_factory<<std::endl;
        std::cout<<unset_parameters.size()<<" vs "<<unsolved_eqns.size()<<std::endl;
        throw std::logic_error("Number of parameters is not the same as number of equations");
    }
    
    // These are the number of unsolved parameters
    int N = unset_parameters.size();
    
    // n is the number of equations which can be solved simultaneously
    for(int n=1; n<N+1; n++)
    {
        // n permutations of N equations 
        std::string bitmask(n, 'a');
        bitmask.resize(N, 'b');
        do
        {
            // The group of equations 
            std::vector<int> group_eqns;
            group_eqns.reserve(n);
            
            // The group of parameters in those equations
            ParameterSet group_parameters;
            
            for(int i=0; i<bitmask.size(); i++)
            {
                if(bitmask[i] == 'a')
                {
                    int e = unsolved_eqns[i];
                    group_eqns.push_back(e);
                    ParameterSet ep = _equation_list[e].parameters();
                    group_parameters.insert(ep.begin(), ep.end());
                }
            }
            
            // Remove the parameters, which we have already solved for
            for(auto const &i: set_parameters)group_parameters.erase(i);
            
            // If the number of parameters equals the number of equations in the group(n)
            // we can independently solve for this
            if(group_parameters.size() == n)
            {
                std::cout<<"Solve: [";
                for(auto const &e:group_eqns) std::cout<<_equation_list[e].equation()<<" ";
                std::cout<<"] for ";
                for(auto const &i:group_parameters) std::cout<<i<<" ";
                std::cout<<std::endl;
                
                // Save the group of equations
                _equation_groups.push_back(
                    EquationGroup(_equation_groups.size(),
                                  _equation_list,
                                  set_parameters,
                                  group_parameters,
                                  group_eqns));
                
                for(auto const&i:group_parameters)
                {
                    unset_parameters.erase(i);
                    set_parameters.insert(i);
                }
                // Get the list of new unsolved equations
                std::vector<int> new_unsolved_equations;
                std::set_difference(unsolved_eqns.begin(), unsolved_eqns.end(),
                                    group_eqns.begin(), group_eqns.end(),
                                    std::inserter(new_unsolved_equations, new_unsolved_equations.begin()));
                unsolved_eqns = new_unsolved_equations;
                
                
                
            }
            else if(group_parameters.size() < n)
            {
                throw std::logic_error("more equations than parameters");
            }
            
            // If we can indeed solve a subset of equations, break out
            if(unset_parameters.size()<N)break;
            class EquationList: public std::vector<Equation>
{
  
};
        } while(std::next_permutation(bitmask.begin(), bitmask.end()));
        
        // If we can indeed solve a subset of equations, break out
        if(unset_parameters.size()<N)break;
    }
  }; // Loop until all parameters can be solved for 
}

void* System::_emit_code() const
{
    char temp_file[] = "/tmp/simsolve-emitted-code-XXXXXX";
    mkstemp(temp_file);
    
    char cpp_file[strlen(temp_file)+5];
    sprintf(cpp_file, "%s.cpp", temp_file);
    
    std::ofstream fout(cpp_file);
    
    fout<<"#include <cmath>"<<std::endl<<"using namespace std;"<<std::endl;
    fout<<"\n\nextern \"C\" const char * name();\nconst char * name(){return \"SimSolve\";}\n";
    fout<<"// "<<_equation_groups.size()<<" equation groups to be solved"<<std::endl<<std::endl;
    for(auto const &g :_equation_groups)
    {
        g.emit_code(fout);
    }
    fout.close();
    std::stringstream sof;
        
    char so_file[strlen(temp_file)+4];
    sprintf(so_file, "%s.so", temp_file);
    
    std::stringstream cmd;
    cmd<<"g++ -fPIC -shared -rdynamic  "<<cpp_file<<" -o "<<so_file;
    std::system(cmd.str().c_str());
    
    void* handle = dlopen(so_file, RTLD_LAZY);
    std::cout <<"-----"<<std::endl;
    return handle;
}

void System::solve()
{
    for(auto &eg: _equation_groups)
    {
        eg.solve();
    }
}

System::~System()
{
    if(_dl_handle)
        dlclose(_dl_handle);
}
}
