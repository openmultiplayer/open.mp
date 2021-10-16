#pragma once

#include <cstddef>
#include <new>

#ifdef OMP_EXPORTS
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#define OMP_API __declspec(dllexport)
#else
#define OMP_API __attribute__((visibility("default")))
#endif
#else
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#define OMP_API __declspec(dllimport)
#else
#define OMP_API
#endif
#endif

/// A version of std::malloc to use for API allocations
/// Used to keep ABI compatibility between the API providers and consumers
/// @param size The number of bytes to allocate
/// @return On success, returns the pointer to the beginning of newly allocated memory
/// To avoid a memory leak, the returned pointer must be deallocated with std::free() or std::realloc()
/// On failure, returns a null pointer
OMP_API void* omp_malloc(size_t size);

/// A version of std::free to use for API allocations
/// Used to keep ABI compatibility between the API providers and consumers
/// @param ptr Pointer to the memory to deallocate
OMP_API void omp_free(void* ptr);
