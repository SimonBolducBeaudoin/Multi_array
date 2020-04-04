#pragma once
#include<malloc.h>

// #include <fftw3.h>

#include <pybind11/pybind11.h>
#include <pybind11/complex.h>
#include <pybind11/numpy.h>

namespace py = pybind11;
using namespace pybind11::literals;
typedef std::complex<float> complex_f;
typedef std::complex<double> complex_d;
typedef unsigned int uint;

// see : https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/aligned-malloc?view=vs-2019
// https://stackoverflow.com/questions/44659924/returning-numpy-arrays-via-pybind11
void* _128bit_aligned_malloc( size_t size );


/////// IDEA
	// Add a macro that can be turned on and off that checks if called indexes are out of bounds.


template<class Type, uint8_t Dim>
class Multi_array
{
	/*
	Custom class for runtime allocation of fixed size multidimentionnal arrays.
		- Memory is going to be allocated to be maximaly local, with index logic beeing indexes(uint64_t lless_local, less_local, local)
			- Therefore fastest iterating loop must always be iterating on last index for optimal performance.
		- Memory will be aligned be default : 
			https://embeddedartistry.com/blog/2017/02/22/generating-aligned-memory/
		- which size are known only at runtime but are of fixed size.
		- Dim (the number of dimension) has to be declared at compile time.
		- (Optional not implemented yet) The constructor takes memory allocation and free function pointers (ex for use with fftw's fftw_alloc and fftw_free)
	*/
};

template<class Type>
class Multi_array<Type,1>
{
	public :
    /* Default constructor */
    Multi_array();
    
	/* usual constructor */
	Multi_array 
	(
		uint64_t n_i , // Number of elements in i
		size_t stride_i = sizeof(Type) , // The number of Bytes of one element 
		void* (*alloc_func)(size_t size) = &_128bit_aligned_malloc, // Custom allocation function 
		void (*free_func)(void* ptr) = &_aligned_free
	);
	
	/* Constructing from an existing pointer */
	Multi_array ( Type* prt, uint64_t n_i , size_t stride_i = sizeof(Type) );
	
	/* Constructing from a 1D Numpy array */
	static Multi_array numpy( py::array_t<Type,py::array::c_style> np_array );
	
	/* Copy constructor */
	Multi_array( Multi_array& Mom);
	Multi_array(const Multi_array& Mom);
	
	~Multi_array();
    
     /* Asignment operator */
    void operator= (const Multi_array& Mom)
    {
        alloc_func = NULL ;
        free_func = NULL ;
	
        ptr = Mom.get() ;
        n_i = Mom.get_n_i();
 
        stride_i = Mom.get_stride_i() ;     
    }
	
    /* Indexing operators */ 
	Type& operator()( uint64_t i ); /* Returns a reference to an element */
	Type& operator[]( uint64_t i ); /* Returns a reference to an element */
	
	/*  
		Same behavior for const Multi_array
		See : https://en.cppreference.com/w/cpp/language/member_functions#const-_and_volatile-qualified_member_functions
	*/
	Type& operator()( uint64_t i ) const ; /* Returns a reference to an element */
	Type& operator[]( uint64_t i ) const ; /* Returns a reference to an element */
	
	
	Type* get(){ return ptr; } ;
	Type* get()const{ return ptr; }  ;
    
	py::array_t<Type, py::array::c_style> get_py_copy();
	py::array_t<Type, py::array::c_style> get_py_no_copy();
	
	uint64_t get_n_i(){return n_i;};
	uint64_t get_n_i()const{return n_i;}  ;
    
	size_t get_stride_i(){return stride_i;};
	size_t get_stride_i()const{return stride_i;};
	
	private :
	void* (*alloc_func)(size_t size) ;
	void (*free_func)(void* ptr) ;
	
	Type* ptr ;
	uint64_t  n_i ;
	size_t stride_i ;
    
    char* displace( uint64_t n_Bytes );
	char* displace( uint64_t n_Bytes ) const;
    
    // void check_overflow();
	
	void _free_func();
	
};
////////////////////////////////////////////////////
template<class Type>
class Multi_array<Type,2>
{
	public :
	/* with default strides */
	Multi_array
	( 
		uint64_t n_j , // Number of elements in j
		uint64_t n_i , // Number of elements in i
		void* (*alloc_func)(size_t size) = &_128bit_aligned_malloc, // Custom allocation function 
		void (*free_func)(void* ptr) = &_aligned_free
	);
	/* declaring strides */
	Multi_array
	( 
		uint64_t n_j , // Number of elements in j
		uint64_t n_i , // Number of elements in i
		size_t stride_j , // The number of Bytes of complete row of elements
		size_t stride_i = sizeof(Type) , // The number of Bytes of one element
		void* (*alloc_func)(size_t size) = &_128bit_aligned_malloc, // Custom allocation function 
		void (*free_func)(void* ptr) = &_aligned_free
	);
	
	/* 	Constructing from an existing pointer with default strides */
	Multi_array
	(
		Type* prt ,
		uint64_t n_j ,
		uint64_t n_i 
	);
	/* 	Constructing from an existing pointer declaring strides */
	Multi_array
	(
		Type* prt ,
		uint64_t n_j ,
		uint64_t n_i ,
		size_t stride_j , // The number of Bytes of complete row of elements
		size_t stride_i = sizeof(Type) // The number of Bytes of one element
	);
	
	/* Constructing from a 2D Numpy array */
	static Multi_array numpy( py::array_t<Type,py::array::c_style> np_array );
	
	/* Copy constructors */
	Multi_array( Multi_array& Mom );
	Multi_array(const Multi_array& Mom);
	
	~Multi_array();
	
    /* Asignment operator */
    // void operator= (const Multi_array& Mom)
    // {
        // alloc_func = NULL ;
        // free_func = NULL ;
	
        // ptr = Mom.get() ;
        // n_j = Mom.get_n_j() ;
        // n_i = Mom.get_n_i();
        
        // stride_j = Mom.get_stride_j() ;
        // stride_i = Mom.get_stride_i() ;     
    // }

    /* Indexing operators */ 
	Type& operator()( uint64_t j , uint64_t i ); /* Returns a reference to an element */
	Type* operator()( uint64_t j ) ; /* Returns a pointer to a row */ 
	Type* operator[]( uint64_t j ) ; /* Returns a pointer to a row */ 
	
	/* Same behavior for const Multi_array	*/
	Type& operator()( uint64_t j , uint64_t i ) const ; /* Returns a reference to an element */
	Type* operator()( uint64_t j ) const ; /* Returns a pointer to a row */ 
	Type* operator[]( uint64_t j ) const ; /* Returns a pointer to a row */ 
	
	Type* get();
	py::array_t<Type, py::array::c_style> get_py_copy();
	py::array_t<Type, py::array::c_style> get_py_no_copy();
	
	uint64_t get_n_j(){return n_j;};
	uint64_t get_n_i(){return n_i;};
	size_t get_stride_j(){return stride_j;};
	size_t get_stride_i(){return stride_i;};
	
	private :
	void* (*alloc_func)(size_t size) ;
	void (*free_func)(void* ptr) ;
	
	Type* ptr ;
	uint64_t n_j ;
	uint64_t n_i ;
	
	size_t stride_j ;
	size_t stride_i ;
    
    char* displace( uint64_t n_Bytes );
	char* displace( uint64_t n_Bytes ) const;
	
	void _free_func();
};
//////////////////////////////////////////////////
template<class Type>
class Multi_array<Type,3>
{
	public :
	/* with default strides */
	Multi_array 
	( 
		uint64_t n_k , // Number of elements in k
		uint64_t n_j , // Number of elements in j
		uint64_t n_i , // Number of elements in i
		void* (*alloc_func)(size_t size) = &_128bit_aligned_malloc, // Custom allocation function 
		void (*free_func)(void* ptr) = &_aligned_free
	);
	/* declaring strides */
	Multi_array
	( 
		uint64_t n_k , // Number of elements in k
		uint64_t n_j , // Number of elements in j
		uint64_t n_i , // Number of elements in i
		size_t stride_k , // The number of Bytes of n_i*n_j elements
		size_t stride_j , // The number of Bytes of a complete row of elements
		size_t stride_i = sizeof(Type) , // The number of Bytes of one element
		void* (*alloc_func)(size_t size) = &_128bit_aligned_malloc, // Custom allocation function 
		void (*free_func)(void* ptr) = &_aligned_free
	);
	
	/* 	Constructing from an existing pointer with default strides */
	Multi_array ( Type* prt, uint64_t n_k , uint64_t n_j , uint64_t n_i );
	/* 	Constructing from an existing pointer declaring strides */
	Multi_array 
	(
		Type* prt ,
		uint64_t n_k ,
		uint64_t n_j ,
		uint64_t n_i ,
		size_t stride_k , // The number of Bytes of n_i*n_j elements
		size_t stride_j , // The number of Bytes of complete row of elements
		size_t stride_i = sizeof(Type) // The number of Bytes of one element
	);
	
	/* Constructing from a 3D Numpy array */
	static Multi_array numpy( py::array_t<Type,py::array::c_style> np_array );
	
	/* Copy constructor */
	Multi_array( Multi_array& Mom);
	Multi_array(const Multi_array& Mom);
	
	~Multi_array();
	
	Type& operator()( uint64_t k , uint64_t j , uint64_t i ); /* Returns a reference to an element */
	Type* operator()( uint64_t k , uint64_t j  ); /* Returns a pointer to kj'th row */
	Type* operator()( uint64_t k ) ; /* Returns a pointer to the k'th 2D subarray */ 
	Type* operator[]( uint64_t k ) ; /* Returns a pointer to the k'th 2D subarray */ 
	
	/* Same behavior for const Multi_array	*/
	Type& operator()( uint64_t k , uint64_t j , uint64_t i ) const ; /* Returns a reference to an element */
	Type* operator()( uint64_t k , uint64_t j  ) const ; /* Returns a pointer to kj'th row */
	Type* operator()( uint64_t k ) const ; /* Returns a pointer to the k'th 2D subarray */ 
	Type* operator[]( uint64_t k ) const ; /* Returns a pointer to the k'th 2D subarray */ 
	
	Type* get();
	py::array_t<Type, py::array::c_style> get_py_copy();
	py::array_t<Type, py::array::c_style> get_py_no_copy();
	
	uint64_t get_n_i(){return n_i;};
	uint64_t get_n_j(){return n_j;};
	uint64_t get_n_k(){return n_k;};
	
	private :
	void* (*alloc_func)(size_t size) ;
	void (*free_func)(void* ptr) ;
	
	Type* ptr ;
	uint64_t n_k ; 
	uint64_t n_j ;
	uint64_t n_i ;
	
	size_t stride_k ;
	size_t stride_j ;
	size_t stride_i ;
    
    char* displace( uint64_t n_Bytes );
	char* displace( uint64_t n_Bytes ) const;
	
	void _free_func();
};

