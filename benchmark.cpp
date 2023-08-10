#include <Multi_array.h>
#include <benchmark/benchmark.h>

static void MULTI_ARRAY_1D_CONSTRUCT(benchmark::State& state)
{
	uint l_data = state.range(0);
	for (auto _ : state)
		Multi_array<double,1> data(l_data) ;
	state.SetComplexityN(state.range(0));
}
BENCHMARK(MULTI_ARRAY_1D_CONSTRUCT)->Range(1024, 8<<26)->Complexity(benchmark::oN);

BENCHMARK_MAIN();