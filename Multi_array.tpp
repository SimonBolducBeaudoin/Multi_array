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

/* Default constructor */
template<class Type , class IndexType>
Multi_array<Type,1,IndexType>::Multi_array()
: 
	alloc_func(NULL) , 
	free_func(NULL) , 
	ptr( NULL ),
	n_i(0),
	stride_i(0)
{};

/* usual constructor */
template<class Type, class IndexType>
Multi_array<Type,1,IndexType>::Multi_array
( 
	IndexType n_i , // Number of elements in i
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

/* constructor no strides */
template<class Type, class IndexType>
Multi_array<Type,1,IndexType>::Multi_array 
(
	IndexType n_i , // Number of elements in i
	void* (*alloc_func)(size_t size) , // Custom allocation function 
	void (*free_func)(void* ptr)
)
: 
	alloc_func(alloc_func) , 
	free_func(free_func) , 
	ptr( (Type*)alloc_func(n_i*sizeof(Type)) ),
	n_i(n_i),
	stride_i(sizeof(Type))
{};

/* Constructing from an existing pointer */
template<class Type, class IndexType>
Multi_array<Type,1,IndexType>::Multi_array( Type* ptr , IndexType n_i , size_t stride_i)
:
	alloc_func(NULL), /* No memory manegement allowed */
	free_func(NULL),
	ptr(ptr),
	n_i(n_i),
	stride_i(stride_i)
{};

template<class Type, class IndexType>
Multi_array<Type,1,IndexType> Multi_array<Type,1,IndexType>::numpy( py::array_t<Type, py::array::c_style> np_array )
{
	py::buffer_info buffer = np_array.request() ;
	
	if (buffer.ndim != 1) 
    {
		throw std::runtime_error("Number of dimensions must be one");
	}
	
	return Multi_array<Type,1,IndexType>( (Type*)buffer.ptr , buffer.shape[0] , buffer.strides[0] );
};

/* Copy constructors */
template<class Type, class IndexType>
Multi_array<Type,1,IndexType>::Multi_array(Multi_array& Mom)
:
	alloc_func(Mom.alloc_func), 
	free_func(Mom.free_func),
	ptr( (Type*)alloc_func(Mom.n_i*Mom.stride_i) ),
	n_i(Mom.n_i),
	stride_i(Mom.stride_i)
{};

template<class Type, class IndexType>
Multi_array<Type,1,IndexType>::Multi_array(const Multi_array& Mom)
:
	alloc_func(Mom.alloc_func),
	free_func(Mom.free_func),
	ptr( (Type*)alloc_func(Mom.n_i*Mom.stride_i) ),
	n_i(Mom.n_i),
	stride_i(Mom.stride_i)
{};

/* Move constructor */
template<class Type, class IndexType>
Multi_array<Type,1,IndexType>::Multi_array( Multi_array&& Mom )
:
	alloc_func( Mom.alloc_func ), /* Son, you're now responsible of my memory */
	free_func( Mom.free_func ),
	ptr( Mom.ptr ),
	n_i( Mom.n_i ),
	stride_i( Mom.stride_i )
{
	Mom.alloc_func = NULL ; // Not necessary
	Mom.free_func = NULL ;
	Mom.ptr = NULL ; // Not necessary
	Mom.n_i = 0 ; // Not necessary
	Mom.stride_i = 0 ; // Not necessary
};

// Destructor
template<class Type, class IndexType>
Multi_array<Type,1,IndexType>::~Multi_array()
{
	_free_func();
};

template<class Type, class IndexType>
void Multi_array<Type,1,IndexType>::_free_func()
{
	if ( (free_func!=NULL) and (ptr!=NULL) )
	{  free_func(ptr) ;  }
	ptr = NULL ;	
};

/* COPY TO NUMPY */
template<class Type, class IndexType>
py::array_t<Type, py::array::c_style> Multi_array<Type,1,IndexType>::move_py()
{
	py::capsule free_when_done( ptr, free_func );
	py::array_t<Type, py::array::c_style> out
	(
		{n_i},      // shape
		{stride_i}, // C-style contiguous strides for double
		ptr,     	// the data pointer
		free_when_done // Free method
	);
	
	alloc_func = NULL ; 
	free_func = NULL ;
	ptr = NULL ; 
	n_i = 0 ; 
	stride_i = 0 ; 
	
	return out ;
};

template<class Type, class IndexType>
py::array_t<Type, py::array::c_style> Multi_array<Type,1,IndexType>::move_py(IndexType n_i)
{
	py::capsule free_when_done( ptr, free_func );
	py::array_t<Type, py::array::c_style> out
	(
		{n_i},      // shape
		{stride_i}, // C-style contiguous strides for double
		ptr,     	// the data pointer
		free_when_done // Free method
	);
	
	alloc_func = NULL ; 
	free_func = NULL ;
	ptr = NULL ; 
	this->n_i = 0 ; 
	stride_i = 0 ; 
	
	return out ;
};

template<class Type, class IndexType>
py::array_t<Type, py::array::c_style> Multi_array<Type,1,IndexType>::copy_py()
{
	/* copy the data */
	size_t num_bytes = n_i*stride_i ; 
	Type* destination = (Type*)alloc_func(n_i*stride_i) ;
	memcpy ( (void*)destination, (void*)get_ptr(), num_bytes ) ;
	
	/* Python responsabilites */
	py::capsule free_when_done( destination, free_func );
	
	return py::array_t<Type, py::array::c_style>
	(
		{n_i},      // shape
		{stride_i}, // C-style contiguous strides for double
		destination,     	// the data pointer
		free_when_done // Free method
	);
};

template<class Type, class IndexType>
py::array_t<Type, py::array::c_style> Multi_array<Type,1,IndexType>::copy_py(IndexType n_i)
{
	/* copy the data */
	size_t num_bytes = n_i*stride_i ; 
	Type* destination = (Type*)alloc_func(n_i*stride_i) ;
	memcpy ( (void*)destination, (void*)get_ptr(), num_bytes ) ;
	
	/* Python responsabilites */
	py::capsule free_when_done( destination, free_func );
	
	return py::array_t<Type, py::array::c_style>
	(
		{n_i},      // shape
		{stride_i}, // C-style contiguous strides for double
		destination,     	// the data pointer
		free_when_done // Free method
	);
};

template<class Type, class IndexType>
py::array_t<Type, py::array::c_style> Multi_array<Type,1,IndexType>::share_py()
{
	/* Python responsabilites */
	py::capsule free_dummy(	ptr, [](void *f){;} ); 
	/* 
		This is just a workarround cause Pybind11 currently makes a copy if no capsule is declared
		I dunno if the mentionned behaviour is supported or if it's just for current patch/compiler...
		Anyway the workarround should be stable
	*/
	
	return py::array_t<Type, py::array::c_style>
	(
		{n_i},      // shape
		{stride_i}, // C-style contiguous strides for double
		ptr,     	// the data pointer
		free_dummy // Free method
	);
};

template<class Type, class IndexType>
py::array_t<Type, py::array::c_style> Multi_array<Type,1,IndexType>::share_py(IndexType n_i)
{
	/* Python responsabilites */
	py::capsule free_dummy(	ptr, [](void *f){;} ); 
	/* 
		This is just a workarround cause Pybind11 currently makes a copy if no capsule is declared
		I dunno if the mentionned behaviour is supported or if it's just for current patch/compiler...
		Anyway the workarround should be stable
	*/
	
	return py::array_t<Type, py::array::c_style>
	(
		{n_i},      // shape
		{stride_i}, // C-style contiguous strides for double
		ptr,     	// the data pointer
		free_dummy // Free method
	);
};

template<class Type, class IndexType>
py::array_t<Type, py::array::c_style> Multi_array<Type,1,IndexType>::affect_py()
{
	/* Python responsabilites */
	py::capsule free_when_done( ptr, free_func );
	
	/* Remove C++ responsability */
	alloc_func = NULL ; 
	free_func = NULL ;
	
	return py::array_t<Type, py::array::c_style>
	(
		{n_i},      // shape
		{stride_i}, // C-style contiguous strides for double
		ptr,     	// the data pointer
		free_when_done // Free method
	);
};

template<class Type, class IndexType>
py::array_t<Type, py::array::c_style> Multi_array<Type,1,IndexType>::affect_py(IndexType n_i)
{
	/* Python responsabilites */
	py::capsule free_when_done( ptr, free_func );
	
	/* Remove C++ responsability */
	alloc_func = NULL ; 
	free_func = NULL ;
	
	return py::array_t<Type, py::array::c_style>
	(
		{n_i},      // shape
		{stride_i}, // C-style contiguous strides for double
		ptr,     	// the data pointer
		free_when_done // Free method
	);
};

template<class Type, class IndexType>
inline char* Multi_array<Type,1,IndexType>::displace( IndexType n_Bytes )
{
	return ((char*)ptr) + n_Bytes ;
};

template<class Type, class IndexType>
inline char* Multi_array<Type,1,IndexType>::displace( IndexType n_Bytes ) const
{
	return ((char*)ptr) + n_Bytes ;
};

// OPERATOR OVERLOAD
template<class Type, class IndexType>
inline Type& Multi_array<Type,1,IndexType>::operator()( IndexType i )
{
	return  *( (Type*)displace( stride_i*i ) ) ;
};

template<class Type, class IndexType>
inline Type& Multi_array<Type,1,IndexType>::operator()( IndexType i ) const
{
	return *( (Type*)displace( stride_i*i ) ) ;
};

template<class Type, class IndexType>
inline Type& Multi_array<Type,1,IndexType>::operator[]( IndexType i )
{
	return  *( (Type*)displace( stride_i*i ) ) ;
};

template<class Type, class IndexType>
inline Type& Multi_array<Type,1,IndexType>::operator[]( IndexType i ) const
{
	return *( (Type*)displace( stride_i*i ) ) ;
};
////

////////////////////////////////////
/* with default strides */
template<class Type, class IndexType>
Multi_array<Type,2,IndexType>::Multi_array
( 
	IndexType n_j , // Number of elements in j
	IndexType n_i , // Number of elements in i
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
template<class Type, class IndexType>
Multi_array<Type,2,IndexType>::Multi_array
( 
	IndexType n_j , // Number of elements in j
	IndexType n_i , // Number of elements in i
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
template<class Type, class IndexType>
Multi_array<Type,2,IndexType>::Multi_array( Type* ptr , IndexType n_j , IndexType n_i )
:
	alloc_func(NULL), /* No memory manegement allowed */
	free_func(NULL),
	ptr(ptr),
	n_j(n_j), n_i(n_i),
	stride_j(n_i*sizeof(Type)) , stride_i(sizeof(Type))
{};
/* 	Constructing from an existing pointer declaring strides */
template<class Type, class IndexType>
Multi_array<Type,2,IndexType>::Multi_array
( 
	Type* ptr , 
	IndexType n_j , 
	IndexType n_i , 
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
template<class Type, class IndexType>
Multi_array<Type,2,IndexType> Multi_array<Type,2,IndexType>::numpy( py::array_t<Type, py::array::c_style> np_array )
{
	py::buffer_info buffer = np_array.request() ;
	
	if (buffer.ndim != 2) 
    {
		throw std::runtime_error("Number of dimensions must be two");
	}
	return Multi_array<Type,2,IndexType>( (Type*)buffer.ptr , buffer.shape[0] , buffer.shape[1] , buffer.strides[0] , buffer.strides[1] );
};

/* Copy constructors */
template<class Type, class IndexType>
Multi_array<Type,2,IndexType>::Multi_array( Multi_array& Mom )
:
	alloc_func(Mom.alloc_func),
	free_func(Mom.free_func),
	ptr( (Type*)alloc_func( Mom.n_j*Mom.stride_j ) ),
	n_j(Mom.n_j), n_i(Mom.n_i),
	stride_j(Mom.stride_j) , stride_i(Mom.stride_i)
{};

template<class Type, class IndexType>
Multi_array<Type,2,IndexType>::Multi_array(const Multi_array& Mom)
:
	alloc_func(Mom.alloc_func),
	free_func(Mom.free_func),
	ptr( (Type*)alloc_func( Mom.n_j*Mom.stride_j ) ),
	n_j(Mom.n_j), n_i(Mom.n_i),
	stride_j(Mom.stride_j) , stride_i(Mom.stride_i)
{};

/* Move constructor */
template<class Type, class IndexType>
Multi_array<Type,2,IndexType>::Multi_array( Multi_array&& Mom )
:
alloc_func( Mom.alloc_func ), /* Son, you're now responsible of my memory */
free_func( Mom.free_func ),
ptr( Mom.ptr ),
n_j( Mom.n_j ),
n_i( Mom.n_i ),
stride_j( Mom.stride_j ),
stride_i( Mom.stride_i )
{
	Mom.alloc_func = NULL ; // Not necessary
	Mom.free_func = NULL ;
	Mom.ptr = NULL ; // Not necessary
	Mom.n_j = 0 ; // Not necessary
	Mom.n_i = 0 ; // Not necessary
	Mom.stride_j = 0 ; // Not necessary
	Mom.stride_i = 0 ; // Not necessary
};

// Destructor
template<class Type, class IndexType>
Multi_array<Type,2,IndexType>::~Multi_array()
{
	_free_func();
};

template<class Type, class IndexType>
void Multi_array<Type,2,IndexType>::_free_func()
{	
	
	if ( (free_func!=NULL) and (ptr!=NULL) )
	{free_func(ptr) ; }
	ptr = NULL ;
};

/* COPY TO NUMPY */
template<class Type, class IndexType>
py::array_t<Type, py::array::c_style> Multi_array<Type,2,IndexType>::move_py()
{
	py::capsule free_when_done( ptr, free_func );
	py::array_t<Type, py::array::c_style> out
	(
		{n_j,n_i},      
		{stride_j,stride_i},   
		ptr,     	
		free_when_done 
	);
	
	alloc_func = NULL ; 
	free_func = NULL ;
	ptr = NULL ; 
	n_j = 0 ; 
	n_i = 0 ; 
	stride_j = 0 ; 
	stride_i = 0 ; 
	
	return out ;
};

template<class Type, class IndexType>
py::array_t<Type, py::array::c_style> Multi_array<Type,2,IndexType>::move_py(IndexType n_j,IndexType n_i)
{
	py::capsule free_when_done( ptr, free_func );
	py::array_t<Type, py::array::c_style> out
	(
		{n_j,n_i},      
		{stride_j,stride_i},   
		ptr,     	
		free_when_done 
	);
	
	alloc_func = NULL ; 
	free_func = NULL ;
	ptr = NULL ; 
	this->n_j = 0 ; 
	this->n_i = 0 ; 
	stride_j = 0 ; 
	stride_i = 0 ; 
	
	return out ;
};

template<class Type, class IndexType>
py::array_t<Type, py::array::c_style> Multi_array<Type,2,IndexType>::copy_py()
{
	size_t num_bytes = n_j*stride_j ; 
	Type* destination = (Type*)alloc_func(n_j*stride_j) ;
	memcpy ( (void*)destination, (void*)get_ptr(), num_bytes ) ;
	
	py::capsule free_when_done( destination, free_func );
	
	return py::array_t<Type, py::array::c_style>
	(
		{n_j,n_i},      
		{stride_j,stride_i},   
		destination,     
		free_when_done 
	);
};

template<class Type, class IndexType>
py::array_t<Type, py::array::c_style> Multi_array<Type,2,IndexType>::copy_py(IndexType n_j,IndexType n_i)
{
	size_t num_bytes = n_j*stride_j ; 
	Type* destination = (Type*)alloc_func(n_j*stride_j) ;
	memcpy ( (void*)destination, (void*)get_ptr(), num_bytes ) ;
	
	py::capsule free_when_done( destination, free_func );
	
	return py::array_t<Type, py::array::c_style>
	(
		{n_j,n_i},      
		{stride_j,stride_i},   
		destination,     
		free_when_done 
	);
};

template<class Type, class IndexType>
py::array_t<Type, py::array::c_style> Multi_array<Type,2,IndexType>::share_py()
{
	py::capsule free_dummy(	ptr, [](void *f){;} ); 
	
	return py::array_t<Type, py::array::c_style>
	(
		{n_j,n_i},      
		{stride_j,stride_i},   
		ptr,     	
		free_dummy 
	);
};

template<class Type, class IndexType>
py::array_t<Type, py::array::c_style> Multi_array<Type,2,IndexType>::share_py(IndexType n_j,IndexType n_i)
{
	py::capsule free_dummy(	ptr, [](void *f){;} ); 
	
	return py::array_t<Type, py::array::c_style>
	(
		{n_j,n_i},      
		{stride_j,stride_i},   
		ptr,     	
		free_dummy 
	);
};

template<class Type, class IndexType>
py::array_t<Type, py::array::c_style> Multi_array<Type,2,IndexType>::affect_py()
{
	py::capsule free_when_done( ptr, free_func );
	
	alloc_func = NULL ; 
	free_func = NULL ;
	
	return py::array_t<Type, py::array::c_style>
	(
		{n_j,n_i},      
		{stride_j,stride_i},   
		ptr,   
		free_when_done
	);
};

template<class Type, class IndexType>
py::array_t<Type, py::array::c_style> Multi_array<Type,2,IndexType>::affect_py(IndexType n_j,IndexType n_i)
{
	py::capsule free_when_done( ptr, free_func );
	
	alloc_func = NULL ; 
	free_func = NULL ;
	
	return py::array_t<Type, py::array::c_style>
	(
		{n_j,n_i},      
		{stride_j,stride_i},   
		ptr,   
		free_when_done
	);
};

template<class Type, class IndexType>
inline char* Multi_array<Type,2,IndexType>::displace( IndexType n_Bytes )
{
	return ((char*)ptr) + n_Bytes ;
};

template<class Type, class IndexType>
inline char* Multi_array<Type,2,IndexType>::displace( IndexType n_Bytes ) const
{
	return ((char*)ptr) + n_Bytes ;
};

// OPERATOR OVERLOAD 
template<class Type, class IndexType>
inline Type& Multi_array<Type,2,IndexType>::operator ()( IndexType j , IndexType i )
{
	return *( (Type*)displace( stride_j*j+stride_i*i ) ) ;
};

template<class Type, class IndexType>
inline Type* Multi_array<Type,2,IndexType>::operator()( IndexType j )
{
	return (Type*)displace( stride_j*j ) ;
	
};

template<class Type, class IndexType>
inline Type* Multi_array<Type,2,IndexType>::operator[]( IndexType j )
{
	return (Type*)displace( stride_j*j ) ;
};

template<class Type, class IndexType>
inline Type& Multi_array<Type,2,IndexType>::operator ()( IndexType j , IndexType i ) const
{
	return  *( (Type*)displace( stride_j*j+stride_i*i ) ) ;
};

template<class Type, class IndexType>
inline Type* Multi_array<Type,2,IndexType>::operator()( IndexType j ) const
{
	return (Type*)displace( stride_j*j ) ;
};

template<class Type, class IndexType>
inline Type* Multi_array<Type,2,IndexType>::operator[]( IndexType j ) const
{
	return (Type*)displace( stride_j*j ) ;
};
////

////////////////////////////////////
/* with default strides */
template<class Type, class IndexType>
Multi_array<Type,3,IndexType>::Multi_array
( 
	IndexType n_k , // Number of elements in j
	IndexType n_j , // Number of elements in j
	IndexType n_i , // Number of elements in i
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
template<class Type, class IndexType>
Multi_array<Type,3,IndexType>::Multi_array
( 
	IndexType n_k , // Number of elements in k
	IndexType n_j , // Number of elements in j
	IndexType n_i , // Number of elements in i
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
template<class Type, class IndexType>
Multi_array<Type,3,IndexType>::Multi_array( Type* ptr , IndexType n_k , IndexType n_j , IndexType n_i )
:
	alloc_func(NULL), /* No memory manegement allowed */
	free_func(NULL),
	ptr(ptr),
	n_k(n_k) , n_j(n_j) , n_i(n_i),
	stride_k(n_j*n_i*sizeof(Type)) , stride_j(n_i*sizeof(Type)) , stride_i(sizeof(Type))
{};
/* 	Constructing from an existing pointer declaring strides */
template<class Type, class IndexType>
Multi_array<Type,3,IndexType>::Multi_array
(
	Type* ptr ,
	IndexType n_k ,
	IndexType n_j ,
	IndexType n_i ,
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

template<class Type, class IndexType>
Multi_array<Type,3,IndexType> Multi_array<Type,3,IndexType>::numpy( py::array_t<Type, py::array::c_style> np_array )
{
	py::buffer_info buffer = np_array.request() ;
	
	if (buffer.ndim != 3) 
    {
		throw std::runtime_error("Number of dimensions must be two");
	}
	
	return Multi_array<Type,3,IndexType>
	( 	
		(Type*)buffer.ptr , 
		buffer.shape[0] , buffer.shape[1], buffer.shape[2] ,
		buffer.strides[0] , buffer.strides[1], buffer.strides[2]
	);
};

/* Copy constructors */
template<class Type, class IndexType>
Multi_array<Type,3,IndexType>::Multi_array( Multi_array& Mom )
:
	alloc_func(Mom.alloc_func), /* No memory manegement allowed */
	free_func(Mom.free_func),
	ptr( (Type*)alloc_func(Mom.n_k*Mom.stride_k) ),
	n_k(Mom.n_k) , n_j(Mom.n_j) , n_i(Mom.n_i),
	stride_k(Mom.stride_k) , stride_j(Mom.stride_j) , stride_i(Mom.stride_i)
{};
	
template<class Type, class IndexType>
Multi_array<Type,3,IndexType>::Multi_array(const Multi_array& Mom)
:
	alloc_func(Mom.alloc_func), /* No memory manegement allowed */
	free_func(Mom.free_func),
	ptr( (Type*)alloc_func(Mom.n_k*Mom.stride_k) ),
	n_k(Mom.n_k) , n_j(Mom.n_j) , n_i(Mom.n_i),
	stride_k(Mom.stride_k) , stride_j(Mom.stride_j) , stride_i(Mom.stride_i)
{};

/* Move constructor */
template<class Type, class IndexType>
Multi_array<Type,3,IndexType>::Multi_array( Multi_array&& Mom )
:
alloc_func( Mom.alloc_func ), /* Son, you're now responsible of my memory */
free_func( Mom.free_func ),
ptr( Mom.ptr ),
n_k( Mom.n_k ),
n_j( Mom.n_j ),
n_i( Mom.n_i ),
stride_k( Mom.stride_k ),
stride_j( Mom.stride_j ),
stride_i( Mom.stride_i )
{
	Mom.alloc_func = NULL ; // Not necessary
	Mom.free_func = NULL ;
	Mom.ptr = NULL ; // Not necessary
	Mom.n_k = 0 ; // Not necessary
	Mom.n_j = 0 ; // Not necessary
	Mom.n_i = 0 ; // Not necessary
	Mom.stride_k = 0 ; // Not necessary
	Mom.stride_j = 0 ; // Not necessary
	Mom.stride_i = 0 ; // Not necessary
};

// Destructor
template<class Type, class IndexType>
Multi_array<Type,3,IndexType>::~Multi_array()
{
	_free_func();
};

template<class Type, class IndexType>
void Multi_array<Type,3,IndexType>::_free_func()
{
	
	if ( (free_func!=NULL) and (ptr!=NULL) )
	{ free_func(ptr) ; }
	ptr = NULL ;
};

/* COPY TO NUMPY */
template<class Type, class IndexType>
py::array_t<Type, py::array::c_style> Multi_array<Type,3,IndexType>::move_py()
{
	py::capsule free_when_done( ptr, free_func );
	py::array_t<Type, py::array::c_style> out
	(
		{n_k,n_j,n_i},      
		{stride_k,stride_j,stride_i},   
		ptr,     	
		free_when_done 
	);
	
	alloc_func = NULL ; 
	free_func = NULL ;
	ptr = NULL ; 
	n_k = 0 ; 
	n_j = 0 ; 
	n_i = 0 ; 
	stride_k = 0 ; 
	stride_j = 0 ; 
	stride_i = 0 ; 
	
	return out ;
};

template<class Type, class IndexType>
py::array_t<Type, py::array::c_style> Multi_array<Type,3,IndexType>::move_py(IndexType n_k,IndexType n_j,IndexType n_i)
{
	py::capsule free_when_done( ptr, free_func );
	py::array_t<Type, py::array::c_style> out
	(
		{n_k,n_j,n_i},      
		{stride_k,stride_j,stride_i},   
		ptr,     	
		free_when_done 
	);
	
	alloc_func = NULL ; 
	free_func = NULL ;
	ptr = NULL ; 
	this->n_k = 0 ; 
	this->n_j = 0 ; 
	this->n_i = 0 ; 
	stride_k = 0 ; 
	stride_j = 0 ; 
	stride_i = 0 ; 
	
	return out ;
};

template<class Type, class IndexType>
py::array_t<Type, py::array::c_style> Multi_array<Type,3,IndexType>::copy_py()
{
	size_t num_bytes = n_k*stride_k ; 
	Type* destination = (Type*)alloc_func(n_j*stride_j) ;
	memcpy ( (void*)destination, (void*)get_ptr(), num_bytes ) ;
	
	py::capsule free_when_done( destination, free_func );
	
	return py::array_t<Type, py::array::c_style>
	(
		{n_k,n_j,n_i},      
		{stride_k,stride_j,stride_i},     
		destination,     
		free_when_done 
	);
};

template<class Type, class IndexType>
py::array_t<Type, py::array::c_style> Multi_array<Type,3,IndexType>::copy_py(IndexType n_k,IndexType n_j,IndexType n_i)
{
	size_t num_bytes = n_k*stride_k ; 
	Type* destination = (Type*)alloc_func(n_j*stride_j) ;
	memcpy ( (void*)destination, (void*)get_ptr(), num_bytes ) ;
	
	py::capsule free_when_done( destination, free_func );
	
	return py::array_t<Type, py::array::c_style>
	(
		{n_k,n_j,n_i},      
		{stride_k,stride_j,stride_i},     
		destination,     
		free_when_done 
	);
};

template<class Type, class IndexType>
py::array_t<Type, py::array::c_style> Multi_array<Type,3,IndexType>::share_py()
{
	py::capsule free_dummy(	ptr, [](void *f){;} ); 
	
	return py::array_t<Type, py::array::c_style>
	(
		{n_k,n_j,n_i},      
		{stride_k,stride_j,stride_i},  
		ptr,     	
		free_dummy 
	);
};

template<class Type, class IndexType>
py::array_t<Type, py::array::c_style> Multi_array<Type,3,IndexType>::share_py(IndexType n_k,IndexType n_j,IndexType n_i)
{
	py::capsule free_dummy(	ptr, [](void *f){;} ); 
	
	return py::array_t<Type, py::array::c_style>
	(
		{n_k,n_j,n_i},      
		{stride_k,stride_j,stride_i},  
		ptr,     	
		free_dummy 
	);
};

template<class Type, class IndexType>
py::array_t<Type, py::array::c_style> Multi_array<Type,3,IndexType>::affect_py()
{
	py::capsule free_when_done( ptr, free_func );
	
	alloc_func = NULL ; 
	free_func = NULL ;
	
	return py::array_t<Type, py::array::c_style>
	(
		{n_k,n_j,n_i},      
		{stride_k,stride_j,stride_i}, 
		ptr,   
		free_when_done
	);
};

template<class Type, class IndexType>
py::array_t<Type, py::array::c_style> Multi_array<Type,3,IndexType>::affect_py(IndexType n_k,IndexType n_j,IndexType n_i)
{
	py::capsule free_when_done( ptr, free_func );
	
	alloc_func = NULL ; 
	free_func = NULL ;
	
	return py::array_t<Type, py::array::c_style>
	(
		{n_k,n_j,n_i},      
		{stride_k,stride_j,stride_i}, 
		ptr,   
		free_when_done
	);
};

template<class Type, class IndexType>
inline char* Multi_array<Type,3,IndexType>::displace( IndexType n_Bytes )
{
	return ((char*)ptr) + n_Bytes ;
};

template<class Type, class IndexType>
inline char* Multi_array<Type,3,IndexType>::displace( IndexType n_Bytes ) const
{
	return ((char*)ptr) + n_Bytes ;
};

// OPERATOR OVERLOAD 	
template<class Type, class IndexType>
inline Type& Multi_array<Type,3,IndexType>::operator ()( IndexType k , IndexType j , IndexType i )
{
	return *( (Type*)displace( stride_k*k+stride_j*j+stride_i*i ) ) ;
};

template<class Type, class IndexType>
inline Type* Multi_array<Type,3,IndexType>::operator()( IndexType k , IndexType j )
{
	return (Type*)displace( stride_k*k + stride_j*j ) ;
};

template<class Type, class IndexType>
inline Type* Multi_array<Type,3,IndexType>::operator()( IndexType k )
{
	return (Type*)displace( stride_k*k ) ;
};

template<class Type, class IndexType>
inline Type* Multi_array<Type,3,IndexType>::operator[]( IndexType k )
{
	return (Type*)displace( stride_k*k ) ;
};

template<class Type, class IndexType>
inline Type& Multi_array<Type,3,IndexType>::operator ()( IndexType k , IndexType j , IndexType i ) const
{
	return *( (Type*)displace( stride_k*k+stride_j*j+stride_i*i ) ) ;
};

template<class Type, class IndexType>
inline Type* Multi_array<Type,3,IndexType>::operator()( IndexType k , IndexType j ) const
{
	return (Type*)displace( stride_k*k + stride_j*j ) ;
};

template<class Type, class IndexType>
inline Type* Multi_array<Type,3,IndexType>::operator()( IndexType k ) const
{
	return  (Type*)displace( stride_k*k ) ;
};

template<class Type, class IndexType>
inline Type* Multi_array<Type,3,IndexType>::operator[]( IndexType k ) const
{
	return  (Type*)displace( stride_k*k ) ;
};
////


