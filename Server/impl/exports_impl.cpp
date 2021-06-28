#include <exports.hpp>
#include <malloc.h>

void* omp_malloc(size_t size) {
	return malloc(size);
}

void omp_free(void* ptr) {
	return free(ptr);
}
