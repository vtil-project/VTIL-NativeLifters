#pragma once
#include <new>
#include <algorithm>

namespace mem
{
    // Allocates <size> bytes of read/write/execute memory.
    //
    void* allocate_rwx( size_t size );

    // Frees <size> byets of read/write/execute memory at <pointer>.
    //
    void free_rwx( void* pointer ) noexcept;

    // A standard C++ allocator wrapping our helpers.
    //
    template <typename T>
    struct rwx_allocator
    {
	    using value_type = T;

	    rwx_allocator() = default;

	    template <typename T2> 
	    constexpr rwx_allocator( const rwx_allocator<T2>& ) noexcept {}

	    inline T* allocate( size_t count ) { return ( T* ) allocate_rwx( count * sizeof( T ) ); }
	    inline void deallocate( T* pointer, size_t ) noexcept { free_rwx( pointer ); }

        template <typename T2> constexpr bool operator==( const rwx_allocator<T2>& ) { return true; }
        template <typename T2> constexpr bool operator!=( const rwx_allocator<T2>& ) { return false; }
    };
};