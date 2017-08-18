#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#include <iostream>
#include <ostream>

namespace SimSolve
{

struct Output
{
    static int ref_level;
    int vlevel;
    Output(int vlevel):vlevel(vlevel)
    {}
};

template<typename T>
Output& operator<<(Output& output, const T &t)
{
    if(output.vlevel <= Output::ref_level)
        std::cout<<t;
    return output;
}

extern Output o1, o2, o3, o4;
}
#endif
