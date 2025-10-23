#include "singleton.h"

namespace {
	constexpr int k_MaxFinalizersSize = 256;
	std::mutex g_Mutex;
	int g_NumFinalizersSize = 0;
	SingletonFinalizer::FinalizerFunc g_Finalizers[k_MaxFinalizersSize];
}

void SingletonFinalizer::addFinalizer(FinalizerFunc func)
{
	std::lock_guard<std::mutex> lock(g_Mutex);
	assert(g_NumFinalizersSize < k_MaxFinalizersSize);
	g_Finalizers[g_NumFinalizersSize++] = func;
}

void SingletonFinalizer::Finalize()
{
	std::lock_guard<std::mutex> lock(g_Mutex);
	for (int i = g_NumFinalizersSize - 1; i >= 0; --i) {
		(*g_Finalizers[i])();
	}
	g_NumFinalizersSize = 0;
}
