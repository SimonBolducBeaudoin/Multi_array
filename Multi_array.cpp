#include "Multi_array.h"
#include "Multi_array.tpp"

// Explicit template instanciation
// See: https://docs.microsoft.com/en-us/cpp/cpp/explicit-instantiation?view=vs-2019

#define MULTI_ARRAY(Type)\
template class Multi_array<Type,1>;\
template class Multi_array<Type,2>;\
template class Multi_array<Type,3>;


MULTI_ARRAY(int8_t) ;
MULTI_ARRAY(uint8_t) ;
MULTI_ARRAY(int16_t) ;
MULTI_ARRAY(uint16_t) ;
MULTI_ARRAY(int32_t) ;
MULTI_ARRAY(uint32_t) ;
MULTI_ARRAY(int64_t) ;
MULTI_ARRAY(uint64_t) ;
MULTI_ARRAY(float) ;
MULTI_ARRAY(double) ;
MULTI_ARRAY(complex_f) ;
MULTI_ARRAY(complex_d) ;
// MULTI_ARRAY(fftw_complex) ; //did not acheive to do it

#undef MULTI_ARRAY