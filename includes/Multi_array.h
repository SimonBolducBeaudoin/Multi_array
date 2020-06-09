#pragma once
#include<malloc.h>

#include <pybind11/pybind11.h>
#include <pybind11/complex.h>
#include <pybind11/numpy.h>

namespace py = pybind11 ;
using namespace pybind11::literals ;
typedef std::complex<float> complex_f ;
typedef std::complex<double> complex_d ;
typedef unsigned int uint ;

/*Function pointer types*/
// typedef void* (*alloc_ptr)(size_t size);
// typedef void (*free_ptr)(size_t size);

// see : https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/aligned-malloc?view=vs-2019
// https://stackoverflow.com/questions/44659924/returning-numpy-arrays-via-pybind11
template <size_t alignment= 64>
void* _64B_aligned_malloc( size_t size );

/////// IDEA
	// Add a macro that can be turned on and off that checks if called indexes are out of bounds.

/*
Multi_array is a custom class for runtime allocation of fixed size multidimentionnal (but low dimensionnal) arrays.
	- It is meant to optimize for execution speed at the expense of excutable size and/or compilation time.
	- No run-time or compile time verification are made.
		- Altough compile time verification could be implemented in the futur.
	- Dim (the number of dimension) has to be declared at compile time.
		- It is currently only implemented up to 3 dimensions. But general templated implementation for arbitrary dimension possible/feasible in the futur.
		- No resizing implementation is planned.
	- Memory is going to be allocated to be maximaly local, with index logic beeing indexes(uint64_t lless_local, less_local, local)
		- Therefore fastest iterating loop must always be iterating on last index for optimal performance.
		- The last index is always denoted by the subscript "i".
	- Memory will be aligned be default : 
		- Using 128 bits (aka 16 bytes) alignement
			- allocation function by default is _aligned_malloc
			- free function bvy default is _aligned_free
			- see : https://embeddedartistry.com/blog/2017/02/22/generating-aligned-memory/
	- User can give their own allocating and freeing function in constructors
		- Ex : fftw_malloc fftw_free (for fftw library)
	- Dereferencing is implemented such to minimize indirections that would arise from an implementation using double ( or triple, or quadruple , ... ) pointers
		- Instead arrays are allocated linearly and deallocation is doen through pointer arithmetic using strides.
	- Default strides suppose maximally dense memory allocation for the given type.
		- Stride can also be custom
	- It follow the following Mom to daugther relation in general.
		- General statement : The Multi_array Mom object is by default responsible for memory and dauthers are simply a copy of their mother's attibute but 
			they dont posses allocation and freeing functions.
		- This general statement can be bypassed (for flexibility) by many means depending on the context. 
			Here are some exemples:
			- Mom can be of any other type. This can be acheived by using the "from an existing pointer" constructor. 
				The constructed daughter array will have acces to memory but wont be responsible to free it.
			- Mom and daughter can be of type Multi_array but of different subtype (i.e. different template arguments)
				This is useful for situation where the same memory location is intended to be used for data that is going to be of different type along a computing process.
				This is similar to the concept of a Union ( https://en.cppreference.com/w/cpp/language/union ) but in supplement it carries all the goodies associated with the array class.
				For example : It is common to do in-place fft (see:fftw's library) as they are faster.
			- The constructor of Daughter allows for explicit declaration of stride to simplifie this process.
			- To allow for memory responsibilities inheritance Mom can be explicitly constructed without a free function and 
				the appropriate freeing function can be explicitly given to Daughter (with allocation function = NULL) so she can cleanup her mother's memory.
			- Copy constructor are implemented so that Mom keeps memory responsabilites.
				- Consequentially a copy asignement operator is using the same logic ( when the object to be copied is an lvalue)
			- Move constructor are implemented so that Daughter get memory responsabilites.
				- Consequentially a move asignement operator is using the same logic ( when the object to be copied is an rvalue)
					This is optimal for speed when a function returns a Multi_array and removes the hassle of pointer and memory responsabilities when exiting a function.
			- More scenarios are imaginable.
	- Dereferencing is done using pointer arithmetic on a char type (1 byte) and respects strides logic, allowing for some flexibility if Daughter is a typecast of Mom. 
	- The IndexType can be modified to modified the maximal lenght of the linearized array. As all indexes (n_i,n_j,...) are of this type, pointer arithmetic could overflow 
		if the total array lenght also corresponds to an overflow of IndexType.
		- By default IndexType is unsigned int (optimal for speed on a given machine) but can be set to any unsigned type by the user.	
*/

template<class Type,uint8_t Dim,class IndexType=uint>
class Multi_array
{

};

template<class Type, class IndexType>
class Multi_array<Type,1,IndexType>
{
	public :
    /* Default constructor */
    Multi_array();
    
	/* constructor default stride */
	Multi_array 
	(
		IndexType n_i , // Number of elements in i
		void* (*alloc_func)(size_t size) = &_64B_aligned_malloc, // Custom allocation function 
		void (*free_func)(void* ptr) = &_aligned_free
	);
	
	/* usual constructor */
	Multi_array 
	(
		IndexType n_i , // Number of elements in i
		size_t stride_i , // The number of Bytes of one element 
		void* (*alloc_func)(size_t size) = &_64B_aligned_malloc, // Custom allocation function 
		void (*free_func)(void* ptr) = &_aligned_free
	);
	
	/* Constructing from an existing pointer */
	Multi_array ( Type* prt, IndexType n_i , size_t stride_i = sizeof(Type) );
	
	/* Constructing from a 1D Numpy array */
	static Multi_array numpy( py::array_t<Type,py::array::c_style>& np_array );
	
	/* Copy constructor */
	Multi_array( Multi_array& Mom );  
	Multi_array( const Multi_array& Mom );
		
	/* Move constructor */
	// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2006/n2027.html#Move_Semantics
	// https://en.cppreference.com/w/cpp/language/move_constructor
	Multi_array( Multi_array&& Mom ); 
	
	/* Destructor */
	~Multi_array();
    
    /* Assignment operators (not implemented)*/
	/*
		Copy assignment operator
			https://en.cppreference.com/w/cpp/language/copy_assignment
	*/
	
	/*
		Move assignment operator
			https://en.cppreference.com/w/cpp/language/move_assignment
	*/
	
    /* Indexing operators */ 
	Type& operator()( IndexType i ); /* Returns a reference to an element */
	Type& operator[]( IndexType i ); /* Returns a reference to an element */
	
	/*  
		Same behavior for const Multi_array
		See : https://en.cppreference.com/w/cpp/language/member_functions#const-_and_volatile-qualified_member_functions
	*/
	Type& operator()( IndexType i ) const ; /* Returns a reference to an element */
	Type& operator[]( IndexType i ) const ; /* Returns a reference to an element */
	
	
	Type* get_ptr(){ return ptr; } ;
	Type* get_ptr()const{ return ptr; }  ;
    
	/* Copy to numpy methods */
	
	/* 
		move_py : 
		Same meaning as move constructor.
		"steal" the resources held by the current object and gives them to a numpy array 
		and leave the current object in valid but empty state.
		Python inherits all memory responsibilities 
	*/
	py::array_t<Type, py::array::c_style> move_py();
	py::array_t<Type, py::array::c_style> move_py(IndexType n_i); // subset move : numpy array has shape {n_i}

	// 	copy_py : Same meaning as copy constructor. Makes a copy of the current object in a  numpy array format and gives it to python (with all responsibilities) 
	py::array_t<Type, py::array::c_style> copy_py();
	py::array_t<Type, py::array::c_style> copy_py(IndexType n_i); // subset copy : partial copy. numpy array has shape {n_i}
	
	//	share_py : Doesn't make a copy and give access to current memmory to python (with no responsibilities)
	py::array_t<Type, py::array::c_style> share_py();
	py::array_t<Type, py::array::c_style> share_py(IndexType n_i); // subset share 
	
	// 	affect_py : Doesn't make a copy and give access to current memmory to python (with all responsibilities) 
	py::array_t<Type, py::array::c_style> affect_py();
	py::array_t<Type, py::array::c_style> affect_py(IndexType n_i); // subset affect 
	
	
	IndexType get_n_i(){return n_i;};
	IndexType get_n_i()const{return n_i;}  ;
    
	size_t get_stride_i(){return stride_i;};
	size_t get_stride_i()const{return stride_i;};
	
	// alloc_ptr get_alloc_func(){return alloc_func;};
	// free_ptr get_free_func(){return free_func};
	
	private :
	void* (*alloc_func)(size_t size) ;
	void (*free_func)(void* ptr) ;
	
	Type* ptr ;
	IndexType n_i ;
	size_t stride_i ;
    
    char* displace( IndexType n_Bytes );
	char* displace( IndexType n_Bytes ) const;
    
    // void check_overflow();
	
	void _free_func();
	
};
////////////////////////////////////////////////////
template<class Type, class IndexType>
class Multi_array<Type,2,IndexType>
{
	public :
	/* with default strides */
	Multi_array
	( 
		IndexType n_j , 
		IndexType n_i , 
		void* (*alloc_func)(size_t size) = &_64B_aligned_malloc,
		void (*free_func)(void* ptr) = &_aligned_free
	);
	/* declaring strides */
	Multi_array
	( 
		IndexType n_j , 
		IndexType n_i , 
		size_t stride_j , 
		size_t stride_i = sizeof(Type) ,
		void* (*alloc_func)(size_t size) = &_64B_aligned_malloc, 
		void (*free_func)(void* ptr) = &_aligned_free
	);
	
	/* 	Constructing from an existing pointer with default strides */
	Multi_array
	(
		Type* prt ,
		IndexType n_j ,
		IndexType n_i 
	);
	/* 	Constructing from an existing pointer declaring strides */
	Multi_array
	(
		Type* prt ,
		IndexType n_j ,
		IndexType n_i ,
		size_t stride_j , 
		size_t stride_i = sizeof(Type) 
	);
	
	/* Constructing from a 2D Numpy array */
	static Multi_array numpy( py::array_t<Type,py::array::c_style>& np_array );
	
	/* Copy constructors */
	Multi_array( Multi_array& Mom ) ;
	Multi_array( const Multi_array& Mom ) ;
	
	/* Move constructor */
	Multi_array( Multi_array&& Mom );
	
	/* Destructor */
	~Multi_array();
	
    /* Indexing operators */ 
	Type& operator()( IndexType j , IndexType i ); /* Returns a reference to an element */
	Type* operator()( IndexType j ) ; /* Returns a pointer to a row */ 
	Type* operator[]( IndexType j ) ; /* Returns a pointer to a row */ 
	
	/* Same behavior for const Multi_array	*/
	Type& operator()( IndexType j , IndexType i ) const ; /* Returns a reference to an element */
	Type* operator()( IndexType j ) const ; /* Returns a pointer to a row */ 
	Type* operator[]( IndexType j ) const ; /* Returns a pointer to a row */ 
	
	Type* get_ptr(){ return ptr; } ;
	Type* get_ptr()const{ return ptr; }  ;
	
	/* Copy to numpy methods */
	py::array_t<Type, py::array::c_style> move_py();
	py::array_t<Type, py::array::c_style> move_py(IndexType n_j,IndexType n_i);
	py::array_t<Type, py::array::c_style> copy_py();
	py::array_t<Type, py::array::c_style> copy_py(IndexType n_j,IndexType n_i);
	py::array_t<Type, py::array::c_style> share_py();
	py::array_t<Type, py::array::c_style> share_py(IndexType n_j,IndexType n_i);
	py::array_t<Type, py::array::c_style> affect_py();
	py::array_t<Type, py::array::c_style> affect_py(IndexType n_j,IndexType n_i);
	
	IndexType get_n_j(){return n_j;};
	IndexType get_n_i(){return n_i;};
	IndexType get_n_j()const{return n_j;};
	IndexType get_n_i()const{return n_i;};
	
	size_t get_stride_j(){return stride_j;};
	size_t get_stride_i(){return stride_i;};
	size_t get_stride_j()const{return stride_j;};
	size_t get_stride_i()const{return stride_i;};
	
	private :
	void* (*alloc_func)(size_t size) ;
	void (*free_func)(void* ptr) ;
	
	Type* ptr ;
	IndexType n_j ;
	IndexType n_i ;
	
	size_t stride_j ;
	size_t stride_i ;
    
    char* displace( IndexType n_Bytes );
	char* displace( IndexType n_Bytes ) const;
	
	void _free_func();
};
//////////////////////////////////////////////////
template<class Type, class IndexType>
class Multi_array<Type,3,IndexType>
{
	public :
	/* with default strides */
	Multi_array 
	( 
		IndexType n_k , // Number of elements in k
		IndexType n_j , // Number of elements in j
		IndexType n_i , // Number of elements in i
		void* (*alloc_func)(size_t size) = &_64B_aligned_malloc, // Custom allocation function 
		void (*free_func)(void* ptr) = &_aligned_free
	);
	/* declaring strides */
	Multi_array
	( 
		IndexType n_k , // Number of elements in k
		IndexType n_j , // Number of elements in j
		IndexType n_i , // Number of elements in i
		size_t stride_k , // The number of Bytes of n_i*n_j elements
		size_t stride_j , // The number of Bytes of a complete row of elements
		size_t stride_i = sizeof(Type) , // The number of Bytes of one element
		void* (*alloc_func)(size_t size) = &_64B_aligned_malloc, // Custom allocation function 
		void (*free_func)(void* ptr) = &_aligned_free
	);
	
	/* 	Constructing from an existing pointer with default strides */
	Multi_array ( Type* prt, IndexType n_k , IndexType n_j , IndexType n_i );
	/* 	Constructing from an existing pointer declaring strides */
	Multi_array 
	(
		Type* prt ,
		IndexType n_k ,
		IndexType n_j ,
		IndexType n_i ,
		size_t stride_k , // The number of Bytes of n_i*n_j elements
		size_t stride_j , // The number of Bytes of complete row of elements
		size_t stride_i = sizeof(Type) // The number of Bytes of one element
	);
	
	/* Constructing from a 3D Numpy array */
	static Multi_array numpy( py::array_t<Type,py::array::c_style>& np_array );
	
	/* Copy constructor */
	Multi_array( Multi_array& Mom);
	Multi_array(const Multi_array& Mom);
	
	/* Move constructor */
	Multi_array( Multi_array&& Mom );
	
	/* Destructor */
	~Multi_array();
	
	Type& operator()( IndexType k , IndexType j , IndexType i ); /* Returns a reference to an element */
	Type* operator()( IndexType k , IndexType j  ); /* Returns a pointer to kj'th row */
	Type* operator()( IndexType k ) ; /* Returns a pointer to the k'th 2D subarray */ 
	Type* operator[]( IndexType k ) ; /* Returns a pointer to the k'th 2D subarray */ 
	
	/* Same behavior for const Multi_array	*/
	Type& operator()( IndexType k , IndexType j , IndexType i ) const ; /* Returns a reference to an element */
	Type* operator()( IndexType k , IndexType j  ) const ; /* Returns a pointer to kj'th row */
	Type* operator()( IndexType k ) const ; /* Returns a pointer to the k'th 2D subarray */ 
	Type* operator[]( IndexType k ) const ; /* Returns a pointer to the k'th 2D subarray */ 
	
	Type* get_ptr(){ return ptr; } ;
	Type* get_ptr()const{ return ptr; }  ;
	
	/* Copy to numpy methods */
	py::array_t<Type, py::array::c_style> move_py();
	py::array_t<Type, py::array::c_style> move_py(IndexType n_k,IndexType n_j,IndexType n_i);
	py::array_t<Type, py::array::c_style> copy_py();
	py::array_t<Type, py::array::c_style> copy_py(IndexType n_k,IndexType n_j,IndexType n_i);
	py::array_t<Type, py::array::c_style> share_py();
	py::array_t<Type, py::array::c_style> share_py(IndexType n_k,IndexType n_j,IndexType n_i);
	py::array_t<Type, py::array::c_style> affect_py();
	py::array_t<Type, py::array::c_style> affect_py(IndexType n_k,IndexType n_j,IndexType n_i);
	
	IndexType get_n_k(){return n_k;};
	IndexType get_n_j(){return n_j;};
	IndexType get_n_i(){return n_i;};
	IndexType get_n_k()const{return n_k;};
	IndexType get_n_j()const{return n_j;};
	IndexType get_n_i()const{return n_i;};

	size_t get_stride_k(){return stride_k;};
	size_t get_stride_j(){return stride_j;};
	size_t get_stride_i(){return stride_i;};
	size_t get_stride_k()const{return stride_k;};
	size_t get_stride_j()const{return stride_j;};
	size_t get_stride_i()const{return stride_i;};
	
	private :
	void* (*alloc_func)(size_t size) ;
	void (*free_func)(void* ptr) ;
	
	Type* ptr ;
	IndexType n_k ; 
	IndexType n_j ;
	IndexType n_i ;
	
	size_t stride_k ;
	size_t stride_j ;
	size_t stride_i ;
    
    char* displace( IndexType n_Bytes );
	char* displace( IndexType n_Bytes ) const;
	
	void _free_func();
};

#include "../src/Multi_array.tpp"