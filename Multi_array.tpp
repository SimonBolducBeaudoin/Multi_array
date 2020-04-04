#include "Multi_array.h"

// see : https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/aligned-malloc?view=vs-2019
// https://stackoverflow.com/questions/44659924/returning-numpy-arrays-via-pybind11
void* _128bit_aligned_malloc( size_t size )
{
	void    *ptr;
    size_t  alignment = 16 ;
	
	ptr = _aligned_malloc(size, alignment);
    if (ptr == NULL) {throw std::runtime_error("Error allocation aligned memory.");}
	
    /* 
	if (((unsigned long long)ptr % alignment ) == 0)
        printf_s( "This pointer, %p, is aligned on %zu\n",
                  ptr, alignment);
    else
        printf_s( "This pointer, %p, is not aligned on %zu\n",
                  ptr, alignment); 
	*/
	// printf("_128bit_aligned_malloc");
	return ptr ;
};

template<class Type>
Multi_array<Type,1>::Multi_array()
: 
	alloc_func(NULL) , 
	free_func(NULL) , 
	ptr( NULL ),
	n_i(0),
	stride_i(0)
{};

template<class Type>
Multi_array<Type,1>::Multi_array
( 
	uint64_t n_i , // Number of elements in i
	size_t stride_i , // The number of Bytes of one element 
	void* (*alloc_func)(size_t size) , // Custom allocation function 
	void (*free_func)(void* ptr)
)
: 
	alloc_func(alloc_func) , 
	free_func(free_func) , 
	ptr( (Type*)alloc_func(n_i*stride_i) ),
	n_i(n_i),
	stride_i(stride_i)
{};

template<class Type>
Multi_array<Type,1>::Multi_array( Type* ptr , uint64_t n_i , size_t stride_i)
:
	alloc_func(NULL), /* No memory manegement allowed */
	free_func(NULL),
	ptr(ptr),
	n_i(n_i),
	stride_i(stride_i)
{};

template<class Type>
Multi_array<Type,1> Multi_array<Type,1>::numpy( py::array_t<Type, py::array::c_style> np_array )
{
	py::buffer_info buffer = np_array.request() ;
	
	if (buffer.ndim != 1) 
    {
		throw std::runtime_error("Number of dimensions must be one");
	}
	
	return Multi_array<Type,1>( (Type*)buffer.ptr , buffer.shape[0] , buffer.strides[0] );
};

/* Copy constructor */
template<class Type>
Multi_array<Type,1>::Multi_array( Multi_array& Mom)
:
	alloc_func(NULL), /* No memory manegement allowed */
	free_func(NULL),
	ptr(Mom.ptr),
	n_i(Mom.n_i),
	stride_i(Mom.stride_i)
{};

template<class Type>
Multi_array<Type,1>::Multi_array(const Multi_array& Mom)
:
	alloc_func(NULL), /* No memory manegement allowed */
	free_func(NULL),
	ptr(Mom.ptr),
	n_i(Mom.n_i),
	stride_i(Mom.stride_i)
{};

// Destructor
template<class Type>
Multi_array<Type,1>::~Multi_array()
{
	_free_func();
};

// template<class Type>
// void Multi_array<Type,1>::check_overflow()
// {
    // if ()
	// {
		// throw std::runtime_error(" l_kernel is not odd dont expect this to work... ");
	// }
// }

template<class Type>
void Multi_array<Type,1>::_free_func()
{
	
	if ( (free_func!=NULL) and (ptr!=NULL) )
	{  free_func(ptr) ;  }
	ptr = NULL ;	
};

template<class Type>
inline char* Multi_array<Type,1>::displace( uint64_t n_Bytes )
{
	return ((char*)ptr) + n_Bytes ;
};

template<class Type>
inline char* Multi_array<Type,1>::displace( uint64_t n_Bytes ) const
{
	return ((char*)ptr) + n_Bytes ;
};

// OPERATOR OVERLOAD
template<class Type>
inline Type& Multi_array<Type,1>::operator()( uint64_t i )
{
	return  *( (Type*)displace( stride_i*i ) ) ;
};

template<class Type>
inline Type& Multi_array<Type,1>::operator()( uint64_t i ) const
{
	return *( (Type*)displace( stride_i*i ) ) ;
};

template<class Type>
inline Type& Multi_array<Type,1>::operator[]( uint64_t i )
{
	return  *( (Type*)displace( stride_i*i ) ) ;
};

template<class Type>
inline Type& Multi_array<Type,1>::operator[]( uint64_t i ) const
{
	return *( (Type*)displace( stride_i*i ) ) ;
};
////

template<class Type>
py::array_t<Type, py::array::c_style> Multi_array<Type,1>::get_py_copy()
{
	// This class keeps ownership of the allocated memory. 
	// It will be freed only when the object is garbage collected in python.
	return py::array_t<Type, py::array::c_style>
	(
		{n_i},      // shape
		{stride_i},   // C-style contiguous strides for double
		ptr      	// the data pointer
	);
};

template<class Type>
py::array_t<Type, py::array::c_style> Multi_array<Type,1>::get_py_no_copy()
{
	// This class keeps ownership of the allocated memory. 
	// It will be freed only when the object is garbage collected in python.
	py::capsule free_dummy(	ptr, [](void *f){;} );
	return py::array_t<Type, py::array::c_style>
	(
		{n_i},      // shape
		{stride_i},   // C-style contiguous strides for double
		ptr ,       // the data pointer
		free_dummy // numpy array references this parent
	);
};
////////////////////////////////////
/* with default strides */
template<class Type>
Multi_array<Type,2>::Multi_array
( 
	uint64_t n_j , // Number of elements in j
	uint64_t n_i , // Number of elements in i
	void* (*alloc_func)(size_t size) , // Custom allocation function 
	void (*free_func)(void* ptr)
)
: 
	alloc_func(alloc_func) , 
	free_func(free_func) , 
	ptr( (Type*)alloc_func( n_j*n_i*sizeof(Type)) ),
	n_j(n_j) , n_i(n_i),
	stride_j(n_i*sizeof(Type)) , stride_i(sizeof(Type))
{};
/* declaring strides */
template<class Type>
Multi_array<Type,2>::Multi_array
( 
	uint64_t n_j , // Number of elements in j
	uint64_t n_i , // Number of elements in i
	size_t stride_j , // The number of Bytes of complete row of elements
	size_t stride_i , // The number of Bytes of one element
	void* (*alloc_func)(size_t size) , // Custom allocation function 
	void (*free_func)(void* ptr)
)
: 
	alloc_func(alloc_func) , 
	free_func(free_func) , 
	ptr( (Type*)alloc_func( n_j*stride_j ) ),
	n_j(n_j) , n_i(n_i),
	stride_j(stride_j) , stride_i(stride_i)
{};
/* 	Constructing from an existing pointer with default strides */	
template<class Type>
Multi_array<Type,2>::Multi_array( Type* ptr , uint64_t n_j , uint64_t n_i )
:
	alloc_func(NULL), /* No memory manegement allowed */
	free_func(NULL),
	ptr(ptr),
	n_j(n_j), n_i(n_i),
	stride_j(n_i*sizeof(Type)) , stride_i(sizeof(Type))
{};
/* 	Constructing from an existing pointer declaring strides */
template<class Type>
Multi_array<Type,2>::Multi_array
( 
	Type* ptr , 
	uint64_t n_j , 
	uint64_t n_i , 
	size_t stride_j , 
	size_t stride_i 
)
:
	alloc_func(NULL), /* No memory manegement allowed */
	free_func(NULL),
	ptr(ptr),
	n_j(n_j), n_i(n_i),
	stride_j(stride_j) , stride_i(stride_i)
{};
/* Constructing from a 2D Numpy array */
template<class Type>
Multi_array<Type,2> Multi_array<Type,2>::numpy( py::array_t<Type, py::array::c_style> np_array )
{
	py::buffer_info buffer = np_array.request() ;
	
	if (buffer.ndim != 2) 
    {
		throw std::runtime_error("Number of dimensions must be two");
	}
	return Multi_array<Type,2>( (Type*)buffer.ptr , buffer.shape[0] , buffer.shape[1] , buffer.strides[0] , buffer.strides[1] );
};

/* Copy constructor */
template<class Type>
Multi_array<Type,2>::Multi_array( Multi_array& Mom )
:
	alloc_func(NULL), /* No memory manegement allowed */
	free_func(NULL),
	ptr(Mom.ptr),
	n_j(Mom.n_j), n_i(Mom.n_i),
	stride_j(Mom.stride_j) , stride_i(Mom.stride_i)
{};

template<class Type>
Multi_array<Type,2>::Multi_array(const Multi_array& Mom)
:
	alloc_func(NULL), /* No memory manegement allowed */
	free_func(NULL),
	ptr(Mom.ptr),
	n_j(Mom.n_j), n_i(Mom.n_i),
	stride_j(Mom.stride_j) , stride_i(Mom.stride_i)
{};

// Destructor
template<class Type>
Multi_array<Type,2>::~Multi_array()
{
	_free_func();
};

template<class Type>
void Multi_array<Type,2>::_free_func()
{	
	
	if ( (free_func!=NULL) and (ptr!=NULL) )
	{free_func(ptr) ; }
	ptr = NULL ;
};

template<class Type>
inline char* Multi_array<Type,2>::displace( uint64_t n_Bytes )
{
	return ((char*)ptr) + n_Bytes ;
};

template<class Type>
inline char* Multi_array<Type,2>::displace( uint64_t n_Bytes ) const
{
	return ((char*)ptr) + n_Bytes ;
};

// OPERATOR OVERLOAD 
template<class Type>
inline Type& Multi_array<Type,2>::operator ()( uint64_t j , uint64_t i )
{
	return *( (Type*)displace( stride_j*j+stride_i*i ) ) ;
};

template<class Type>
inline Type* Multi_array<Type,2>::operator()( uint64_t j )
{
	return (Type*)displace( stride_j*j ) ;
	
};

template<class Type>
inline Type* Multi_array<Type,2>::operator[]( uint64_t j )
{
	return (Type*)displace( stride_j*j ) ;
};

template<class Type>
inline Type& Multi_array<Type,2>::operator ()( uint64_t j , uint64_t i ) const
{
	return  *( (Type*)displace( stride_j*j+stride_i*i ) ) ;
};

template<class Type>
inline Type* Multi_array<Type,2>::operator()( uint64_t j ) const
{
	return (Type*)displace( stride_j*j ) ;
};

template<class Type>
inline Type* Multi_array<Type,2>::operator[]( uint64_t j ) const
{
	return (Type*)displace( stride_j*j ) ;
};
////

template<class Type>
Type* Multi_array<Type,2>::get()
{
	return ptr ;
};
	
template<class Type>
py::array_t<Type, py::array::c_style> Multi_array<Type,2>::get_py_copy()
{
	// Numpy will copy the array when using the assignement operator=
	return py::array_t<Type, py::array::c_style>
	(
		{n_j,n_i},      // shape
		{stride_j,stride_i},   // C-style contiguous strides for double
		ptr       // the data pointer
	);
};

template<class Type>
py::array_t<Type, py::array::c_style> Multi_array<Type,2>::get_py_no_copy()
{
	// Numpy will not copy the array when using the assignement operator=
	py::capsule free_dummy(	ptr, [](void *f){;} );
	
	return py::array_t<Type, py::array::c_style>
	(
		{n_j,n_i},      // shape
		{stride_j,stride_i},   // C-style contiguous strides for double
		ptr ,       // the data pointer
		free_dummy // numpy array references this parent
	);
};

////////////////////////////////////
/* with default strides */
template<class Type>
Multi_array<Type,3>::Multi_array
( 
	uint64_t n_k , // Number of elements in j
	uint64_t n_j , // Number of elements in j
	uint64_t n_i , // Number of elements in i
	void* (*alloc_func)(size_t size) , // Custom allocation function 
	void (*free_func)(void* ptr)
)
: 
	alloc_func(alloc_func) , 
	free_func(free_func) , 
	ptr( (Type*)alloc_func( n_k*n_j*n_i*sizeof(Type)) ),
	n_k(n_k) , n_j(n_j) , n_i(n_i),
	stride_k(n_j*n_i*sizeof(Type)) , stride_j(n_i*sizeof(Type)) , stride_i(sizeof(Type))
{};

/* declaring strides */
template<class Type>
Multi_array<Type,3>::Multi_array
( 
	uint64_t n_k , // Number of elements in k
	uint64_t n_j , // Number of elements in j
	uint64_t n_i , // Number of elements in i
	size_t stride_k , // The number of Bytes of n_i*n_j elements
	size_t stride_j , // The number of Bytes of a complete row of elements
	size_t stride_i , // The number of Bytes of one element
	void* (*alloc_func)(size_t size) , // Custom allocation function 
	void (*free_func)(void* ptr)
)
: 
	alloc_func(alloc_func) , 
	free_func(free_func) , 
	ptr( (Type*)alloc_func(n_k*stride_k) ),
	n_k(n_k) , n_j(n_j) , n_i(n_i),
	stride_k(stride_k) , stride_j(stride_j) , stride_i(stride_i)
{};
/* 	Constructing from an existing pointer with default strides */
template<class Type>
Multi_array<Type,3>::Multi_array( Type* ptr , uint64_t n_k , uint64_t n_j , uint64_t n_i )
:
	alloc_func(NULL), /* No memory manegement allowed */
	free_func(NULL),
	ptr(ptr),
	n_k(n_k) , n_j(n_j) , n_i(n_i),
	stride_k(n_j*n_i*sizeof(Type)) , stride_j(n_i*sizeof(Type)) , stride_i(sizeof(Type))
{};
/* 	Constructing from an existing pointer declaring strides */
template<class Type>
Multi_array<Type,3>::Multi_array
(
	Type* ptr ,
	uint64_t n_k ,
	uint64_t n_j ,
	uint64_t n_i ,
	size_t stride_k , // The number of Bytes of n_i*n_j elements
	size_t stride_j , // The number of Bytes of complete row of elements
	size_t stride_i // The number of Bytes of one element
)
:
	alloc_func(NULL), /* No memory manegement allowed */
	free_func(NULL),
	ptr(ptr),
	n_k(n_k) , n_j(n_j) , n_i(n_i),
	stride_k(stride_k) , stride_j(stride_j) , stride_i(stride_i)
{};

template<class Type>
Multi_array<Type,3> Multi_array<Type,3>::numpy( py::array_t<Type, py::array::c_style> np_array )
{
	py::buffer_info buffer = np_array.request() ;
	
	if (buffer.ndim != 3) 
    {
		throw std::runtime_error("Number of dimensions must be two");
	}
	
	return Multi_array<Type,3>
	( 	
		(Type*)buffer.ptr , 
		buffer.shape[0] , buffer.shape[1], buffer.shape[2] ,
		buffer.strides[0] , buffer.strides[1], buffer.strides[2]
	);
};

/* Copy constructors */
template<class Type>
Multi_array<Type,3>::Multi_array( Multi_array& Mom )
:
	alloc_func(NULL), /* No memory manegement allowed */
	free_func(NULL),
	ptr(Mom.ptr),
	n_k(Mom.n_k) , n_j(Mom.n_j) , n_i(Mom.n_i),
	stride_k(Mom.stride_k) , stride_j(Mom.stride_j) , stride_i(Mom.stride_i)
{};
	
template<class Type>
Multi_array<Type,3>::Multi_array(const Multi_array& Mom)
:
	alloc_func(NULL), /* No memory manegement allowed */
	free_func(NULL),
	ptr(Mom.ptr),
	n_k(Mom.n_k) , n_j(Mom.n_j) , n_i(Mom.n_i),
	stride_k(Mom.stride_k) , stride_j(Mom.stride_j) , stride_i(Mom.stride_i)
{};

// Destructor
template<class Type>
Multi_array<Type,3>::~Multi_array()
{
	_free_func();
};

template<class Type>
void Multi_array<Type,3>::_free_func()
{
	
	if ( (free_func!=NULL) and (ptr!=NULL) )
	{ free_func(ptr) ; }
	ptr = NULL ;
};

template<class Type>
inline char* Multi_array<Type,3>::displace( uint64_t n_Bytes )
{
	return ((char*)ptr) + n_Bytes ;
};

template<class Type>
inline char* Multi_array<Type,3>::displace( uint64_t n_Bytes ) const
{
	return ((char*)ptr) + n_Bytes ;
};

// OPERATOR OVERLOAD 	
template<class Type>
inline Type& Multi_array<Type,3>::operator ()( uint64_t k , uint64_t j , uint64_t i )
{
	return *( (Type*)displace( stride_k*k+stride_j*j+stride_i*i ) ) ;
};

template<class Type>
inline Type* Multi_array<Type,3>::operator()( uint64_t k , uint64_t j )
{
	return (Type*)displace( stride_k*k + stride_j*j ) ;
};

template<class Type>
inline Type* Multi_array<Type,3>::operator()( uint64_t k )
{
	return (Type*)displace( stride_k*k ) ;
};

template<class Type>
inline Type* Multi_array<Type,3>::operator[]( uint64_t k )
{
	return (Type*)displace( stride_k*k ) ;
};

template<class Type>
inline Type& Multi_array<Type,3>::operator ()( uint64_t k , uint64_t j , uint64_t i ) const
{
	return *( (Type*)displace( stride_k*k+stride_j*j+stride_i*i ) ) ;
};

template<class Type>
inline Type* Multi_array<Type,3>::operator()( uint64_t k , uint64_t j ) const
{
	return (Type*)displace( stride_k*k + stride_j*j ) ;
};

template<class Type>
inline Type* Multi_array<Type,3>::operator()( uint64_t k ) const
{
	return  (Type*)displace( stride_k*k ) ;
};

template<class Type>
inline Type* Multi_array<Type,3>::operator[]( uint64_t k ) const
{
	return  (Type*)displace( stride_k*k ) ;
};
////

template<class Type>
Type* Multi_array<Type,3>::get()
{
	return ptr ;
};

template<class Type>
py::array_t<Type, py::array::c_style> Multi_array<Type,3>::get_py_copy()
{
	return py::array_t<Type, py::array::c_style>
	(
		{n_k,n_j,n_i},      // shape
		{stride_k,stride_j,stride_i},   // C-style contiguous strides for double
		ptr         // the data pointer
	);
};

template<class Type>
py::array_t<Type, py::array::c_style> Multi_array<Type,3>::get_py_no_copy()
{
	// Numpy will not copy the array when using the assignement operator=
	py::capsule free_dummy(	ptr, [](void *f){;} );
	
	return py::array_t<Type, py::array::c_style>
	(
		{n_k,n_j,n_i},      // shape
		{stride_k,stride_j,stride_i},   // C-style contiguous strides for double
		ptr ,       // the data pointer
		free_dummy // numpy array references this parent
	);
};

