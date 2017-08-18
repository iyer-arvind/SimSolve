#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#include <iostream>
#include <ostream>
#include <cstdint>
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


#ifndef MURMURHASH2_64_H_INCLUDED
#define MURMURHASH2_64_H_INCLUDED

uint64_t MurmurHash64A ( const void * key, int len, unsigned int seed=1 );
uint64_t MurmurHash64B ( const void * key, int len, unsigned int seed=1 );

#endif
