#include "Multi_array.h"
#include "Multi_array.tpp"

// Explicit template instanciation
// See: https://docs.microsoft.com/en-us/cpp/cpp/explicit-instantiation?view=vs-2019

#define MULTI_ARRAY(Type,IndexType)\
template class Multi_array<Type,1,IndexType>;\
template class Multi_array<Type,2,IndexType>;\
template class Multi_array<Type,3,IndexType>;

/*
Type : is the type of an element of the array
IndexType : is the type of the for all indexes and index counters
	It should always be unsigned int unsell it's to small to fit the largest index of the array (in Bytes)
	
	On my machine uint == uint32_t and I need to use IndexType=uint64_t for arrays with more than 2**32 bytes
*/

MULTI_ARRAY(int8_t,uint32_t) 	; MULTI_ARRAY(int8_t,uint64_t) ;
MULTI_ARRAY(uint8_t,uint32_t) 	; MULTI_ARRAY(uint8_t,uint64_t) ;
MULTI_ARRAY(int16_t,uint32_t) 	; MULTI_ARRAY(int16_t,uint64_t) ;
MULTI_ARRAY(uint16_t,uint32_t) 	; MULTI_ARRAY(uint16_t,uint64_t) ;
MULTI_ARRAY(int32_t,uint32_t) 	; MULTI_ARRAY(int32_t,uint64_t) ;
MULTI_ARRAY(uint32_t,uint32_t) 	; MULTI_ARRAY(uint32_t,uint64_t) ;
MULTI_ARRAY(int64_t,uint32_t) 	; MULTI_ARRAY(int64_t,uint64_t) ;
MULTI_ARRAY(uint64_t,uint32_t)	; MULTI_ARRAY(uint64_t,uint64_t) ;
MULTI_ARRAY(float,uint32_t) 	; MULTI_ARRAY(float,uint64_t) ;
MULTI_ARRAY(double,uint32_t) 	; MULTI_ARRAY(double,uint64_t) ;
MULTI_ARRAY(complex_f,uint32_t) ; MULTI_ARRAY(complex_f,uint64_t) ; 
MULTI_ARRAY(complex_d,uint32_t) ; MULTI_ARRAY(complex_d,uint64_t) ;
// MULTI_ARRAY(fftw_complex) ; //did not acheive to do it

#undef MULTI_ARRAY