#include "system.hpp"

#include <fstream>
#include <iostream>
#include <iomanip>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <cstdlib>
#include <algorithm>
#include <regex>
#include <assert.h>
#include <dlfcn.h>

#ifdef UNITS_SUPPORT
extern unsigned char units_header[];
extern unsigned int units_header_len;
#endif

#ifndef HASH_FUNC
#define HASH_FUNC MurmurHash64A
#endif

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
  ParameterSet set_parameters;
  
  if(!ifh.is_open())
        throw std::invalid_argument("File missing?");
    
  while(! ifh.eof())
  {
    ifh.getline(line, 1000);
    std::string l = line;
    int cpos = l.find('#');
    if(cpos != std::string::npos)
        l = l.substr(0, cpos);
    
    trim(l);
    if(! l.length()) continue;

    if(ssize_t pos = l.find("::") != std::string::npos)
    {
            _load_external_library(l);
    }
    
    ssize_t ini_pos = l.find('~');
    ssize_t equ_pos = l.find(":=");
    if( ini_pos != std::string::npos || equ_pos != std::string::npos)
    {
        int pos = (equ_pos!= std::string::npos?equ_pos:ini_pos);
        int off = (equ_pos!= std::string::npos?2:1);
        std::string p;
#ifdef UNITS_SUPPORT
        int dposo = l.find('[');
        if(dposo != std::string::npos)
        {
            int dposc = l.find(']');
            assert(dposc != std::string::npos);
           
            p = l.substr(0, pos);
            trim(p);
            Units::Quantity q(std::stof(l.substr(pos+off, dposo)),
                                        Units::Unit(l.substr(dposo+1,dposc)));
            parameter_factory.set_value(p, q);
        }
        else
        {
            p = l.substr(0, pos);
            trim(p);
            parameter_factory.set_value(p, std::stof(l.substr(pos+off)));
        }
#else
        p = l.substr(0, pos);
        trim(p);
        parameter_factory.set_value(p, std::stof(l.substr(pos+off)));
#endif
        if(equ_pos!= std::string::npos)
        {
            o3<<"Fixing value of "<<p<<"\n";
            set_parameters.insert(p);
        }
    }
    else if(l.find('=') != std::string::npos)
    {
      _equation_list.push_back(Equation(l));
    }
  }
  _partition_equations(set_parameters);
  _dl_handle = _emit_code();
  
  for(auto &eg:_equation_groups)
  {
        eg.load_function(_dl_handle);
  }
}

void System::_load_external_library(std::string lib)
{
    ssize_t pos = lib.find("::");
    std::string libname = lib.substr(0, pos);
    trim(libname);
    o3<<"Loading '"<<libname<<"'\n";
    void *handle = dlopen(libname.c_str(), RTLD_LAZY);
    
    
    if(!handle)
        throw std::invalid_argument(std::string("Library '") + libname + "' not found.");
    
    const char *h = (char*)dlsym(handle, "header");
    unsigned int *hl = (unsigned int*)dlsym(handle, "header_len");
    
    if(!h or !hl)
        throw std::invalid_argument(std::string("Library does not seem to contain the header."));
    
    std::string s = lib.substr(pos+2);
    
    std::smatch sm;
    std::regex e("[a-zA-Z_][a-zA-Z0-9_]*");
    
    while(std::regex_search(s, sm, e))
    {
        for(auto x:sm)
        {
            std::string function = x.str();
            trim(function);
            o3<<"Adding function: '"<<function<<"'"<<"\n";
            Equation::functions.insert(function);
        }
        s = sm.suffix().str();
    }
    _external_libs[libname] = handle;
    
}

void System::_partition_equations(ParameterSet set_parameters)
{
  
  ParameterSet all_parameters = parameter_factory.parameters();
  ParameterSet unset_parameters;
  for(auto const &a:all_parameters)
  {
      if(set_parameters.find(a) == set_parameters.end())
      {
          o2<<"To solve for "<<a<<"\n";
          unset_parameters.insert(a);
      }
  }
  
  std::vector<int> unsolved_eqns;
  unsolved_eqns.reserve(all_parameters.size());
  
  for(int i=0;i<_equation_list.size();i++)
    unsolved_eqns.push_back(i);
  
  o2<<"Strategy: \n";
  // While there are still parameters left to be solved for
  while(unset_parameters.size())
  {
    if(unsolved_eqns.size() != unset_parameters.size())
    {
        std::cerr<<parameter_factory<<"\n";
        std::cerr<<unset_parameters.size()<<" vs "<<unsolved_eqns.size()<<"\n";
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
                o2<<"Solve: [";
                for(auto const &e:group_eqns) o2<<_equation_list[e].equation()<<" ";
                o2<<"] for ";
                for(auto const &i:group_parameters) o2<<i<<" ";
                o2<<"\n";
                
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
        } while(std::next_permutation(bitmask.begin(), bitmask.end()));
        
        // If we can indeed solve a subset of equations, break out
        if(unset_parameters.size()<N)break;
    }
  }; // Loop until all parameters can be solved for 
}

void* System::_emit_code() const
{
    std::stringstream fout;
    o3<<"Generating code ...\n";
    fout<<"#include <cmath>"<<"\n"<<"using namespace std;"<<"\n";
#ifdef UNITS_SUPPORT
    std::string units_header_str((const char*)units_header, units_header_len);
    fout<<"// -----------------UNITS_HEADER"<<"\n"<<units_header_str<<"\n"<<"// -----------------UNITS_HEADER"<<"\n";
    fout<<"using namespace Units;"<<"\n";
    fout<<"typedef Quantity ParameterType;"<<"\n";
    
#else
    fout<<"typedef double ParameterType;"<<"\n";
#endif
    std::string link_flags;
    for(auto const &e:_external_libs)
    {
        const char *h = (const char*)dlsym(e.second, "header");
        unsigned int *hl = (unsigned int*)dlsym(e.second, "header_len");
        std::string header(h, *hl);
        fout <<"// FROM "<<e.first<<"\n"<<header<<"\n";
        link_flags += std::string(" -l:") + e.first + " "; 
    }
    
    fout<<"\n\nextern \"C\" const char * name();\nconst char * name(){return \"SimSolve\";}\n";
    fout<<"// "<<_equation_groups.size()<<" equation groups to be solved"<<"\n"<<"\n";
    for(auto const &g :_equation_groups)
    {
        g.emit_code(fout);
    }
    
    std::string code = fout.str();
    
    uint64_t hash = HASH_FUNC(code.c_str(), code.size());
    
    std::stringstream fname;
    fname<<"/tmp/simsolve-emitted-code-0x"<<std::setfill('0')<<std::setw(16)<<std::hex<<hash;
    
    std::ifstream check(fname.str()+".cpp");
    bool exists = check.good();
    check.close();
    if(!exists)
    {
        o3 <<"Emitting code ..."<<"\n";
        std::ofstream codeout(fname.str()+".cpp");
        codeout<<code;
        codeout.close();
        
        o3<<"Compiling code ..."<<"\n";
        std::stringstream sof;
        
        std::stringstream cmd;
        cmd<<"g++ -g -fPIC -shared -rdynamic -L . "<<link_flags<<fname.str()<<".cpp -o "<<fname.str()<<".so";
        o4<<"Executing "<<cmd.str()<<"\n";
        int ret_code = std::system(cmd.str().c_str());
        if(ret_code)
            throw std::logic_error("compilation of module failed.");
    }
    
    o4<<"Loading the library..."<<"\n";
    void* handle = dlopen((fname.str()+".so").c_str(), RTLD_LAZY);
    
    o4 <<"Ready to solve."<<"\n";
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
    for(auto &x:_external_libs)
        dlclose(x.second);
    
    if(_dl_handle)
        dlclose(_dl_handle);
}
}
