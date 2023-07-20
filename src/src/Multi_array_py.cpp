#include <Multi_array_py.h>

// CLASS MACROS
#define PY_MULTI_ARRAY(Type,Dim)\
	py::class_<Multi_array<Type,Dim>>( m , "Multi_array_"#Type"_"#Dim)\
	.def( py::init(&Multi_array<Type,Dim>::numpy), "numpy_array"_a.noconvert() ) \
	.def("get_copy", 	( py::array_t<int8_t,py::array::c_style> (Multi_array<Type,Dim>::*)()) &Multi_array<Type,Dim>::copy_py )\
	.def("get", 		( py::array_t<int8_t,py::array::c_style> (Multi_array<Type,Dim>::*)()) &Multi_array<Type,Dim>::move_py )\
	;
	// py::array_t<Type, py::array::c_style> copy_py();
	
	// (np_double (*)(np_double&,np_double&) ) &interpolation_linear_py 
	
	// m.def("lin_interpol_1D", , "data"_a.noconvert() , "new_abs"_a.noconvert() ) ;
/* void omp_test()
{
	int n_threads = 2 ;
	uint l_fft = 128 ;
	Multi_array<double,1> gs(n_threads,(l_fft/2 + 1));
	
	printf("line number %d in file %s\n", __LINE__, __FILE__);
	#pragma omp parallel
	{	
		manage_thread_affinity();
		
		int this_thread = omp_get_thread_num();
		printf("this_thread = %d \n", this_thread);
		// complex_d tmp ; // Intermediate variable 
		
		 // Reset g to zeros.
		printf("(l_fft/2+1) = %d \n" , (l_fft/2+1) );
		for( uint k=0; k < (l_fft/2+1) ; k++ )
		{
			printf( "gs(%d,%d) \n" , this_thread , k );
			gs(this_thread,k) = 0;
		}
	}
	
} */
	
void init_Multi_array(py::module &m)
{
	PY_MULTI_ARRAY(int8_t,1);
	PY_MULTI_ARRAY(int8_t,2);
	PY_MULTI_ARRAY(int8_t,3);
}

// CLOSE MACRO SCOPES
#undef PY_MULTI_ARRAY