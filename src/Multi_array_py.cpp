#include "Multi_array_py.h"

// CLASS MACROS
#define PY_MULTI_ARRAY(Type,Dim)\
	py::class_<Multi_array<Type,Dim>>( m , "Multi_array_"#Type"_"#Dim)\
	.def( py::init(&Multi_array<Type,Dim>::numpy_copy), "numpy_array"_a.noconvert() ) \
	.def("get_copy", 	( py::array_t<int8_t,py::array::c_style> (Multi_array<Type,Dim>::*)()) &Multi_array<Type,Dim>::copy_py )\
	.def("get", 		( py::array_t<int8_t,py::array::c_style> (Multi_array<Type,Dim>::*)()) &Multi_array<Type,Dim>::move_py )\
	;
	
void init_Multi_array(py::module &m)
{
	PY_MULTI_ARRAY(int8_t,1);
	PY_MULTI_ARRAY(int8_t,2);
	PY_MULTI_ARRAY(int8_t,3);
}

// CLOSE MACRO SCOPES
#undef PY_MULTI_ARRAY