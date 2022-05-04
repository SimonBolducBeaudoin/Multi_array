NAME = Multi_array
PYLIB_EXT = $(if $(filter $(OS),Windows_NT),.pyd,.so)
TARGET_STATIC = lib$(NAME).a
TARGET_PYLIB = ../../Python_2_7/$(NAME)$(PYLIB_EXT)

MULTI_ARRAY = ../Multi_array
OMP_EXTRA = ../Omp_extra
INTERPOLATION = ../Interpolation
LIBS = ../libs

ODIR = obj
LDIR = lib
SDIR = src

EXTERNAL_INCLUDES = -I$(MULTI_ARRAY)/$(SDIR)

SRC  = $(wildcard $(SDIR)/*.cpp)
OBJ  = $(patsubst $(SDIR)/%.cpp,$(ODIR)/%.o,$(SRC))
DEPS = $(OBJ:.o=.d)

CXX = $(OS:Windows_NT=x86_64-w64-mingw32-)g++
OPTIMIZATION = -Ofast -march=native
CPP_STD = -std=c++14
WARNINGS = -Wall
MINGW_COMPATIBLE = $(OS:Windows_NT=-DMS_WIN64 -D_hypot=hypot)
DEPS_FLAG = -MMD -MP

POSITION_INDEP = -fPIC
SHARED = -shared

PY = $(OS:Windows_NT=/c/Anaconda2/)python

PY_INCL := $(shell $(PY) -m pybind11 --includes)
ifneq ($(OS),Windows_NT)
    PY_INCL += -I /usr/include/python2.7/
endif

PY_LINKS = $(OS:Windows_NT=-L /c/Anaconda2/ -l python27)

LINKS = $(PY_LINKS)

LINKING = 	$(CXX) $(OPTIMIZATION) \
			$(POSITION_INDEP) $(SHARED) \
			-o $(TARGET_PYLIB) $(OBJ) \
			$(LINKS) \
			$(DEPS_FLAG) $(MINGW_COMPATIBLE)
			
STATIC_LIB = ar cr $(TARGET_STATIC) $(OBJ) 

INCLUDES = 	$(PY_INCL) \
			$(EXTERNAL_INCLUDES)
			
COMPILE = $(CXX) $(CPP_STD) $(OPTIMIZATION) $(POSITION_INDEP) $(WARNINGS) -c -o $@ $< $(INCLUDES) $(DEPS_FLAG) $(MINGW_COMPATIBLE)

LINK_BENCHMARK = -L$(LIBS)/benchmark/build/src -lbenchmark -lpthread -lshlwapi

LINKING_BENCHMARK = \
	$(CXX) $< -O0 -march=native \
	-static \
	$(LINK_BENCHMARK)\
	$(DEPS_FLAG) $(MINGW_COMPATIBLE) \
	-o $@ 

INCLUDES_BENCHMARK = \
	-I $(LIBS)/benchmark/include \
	$(INCLUDES)
			
COMPILE_BENCHMARK = \
	$(CXX) $(CPP_STD) $< -O0 -march=native \
	$(INCLUDES_BENCHMARK) \
	$(DEPS_FLAG) $(MINGW_COMPATIBLE) \
	-c -o $@ 

message :
	@ echo "Multi_array is now a header only library"

all : $(TARGET_PYLIB) $(TARGET_STATIC) $(OBJ)
	@ echo "Complied explicit template instantiation"

python_debug_library : $(TARGET_PYLIB)
	@ echo "Complied explicit template instantiation"
	@ echo "And python debuging code"

static_library : $(TARGET_STATIC)
	@ echo "Complied explicit template instantiation"
	@ echo "And python debuging code"
	
compile_objects : $(OBJ)

benchmark : benchmark.exe

$(TARGET_PYLIB): $(OBJ)
	@ echo " "
	@ echo "---------Linking library $(TARGET_PYLIB)---------"
	$(LINKING)
	
$(TARGET_STATIC): $(OBJ)
	@ echo " "
	@ echo "---------Compiling static library $(TARGET_STATIC)---------"
	$(STATIC_LIB)
	
benchmark.exe : benchmark.o
	@ echo " "
	@ echo "---------Compile $@ ---------"
	$(LINKING_BENCHMARK)

benchmark.o : benchmark.cpp
	@ echo " "
	@ echo "---------Compile $@ from $< ---------"
	$(COMPILE_BENCHMARK)

$(ODIR)/%.o : $(SDIR)/%.cpp
	@ echo " "
	@ echo "---------Compile object $@ from $<--------"
	$(COMPILE)
	
-include $(DEPS)

clean:
	@rm -f $(TARGET_PYLIB) $(TARGET_STATIC) $(OBJ) $(DEPS) benchmark.o benchmark.exe
 
.PHONY: all , clean , message , python_debug_library , compile_objects , benchmark