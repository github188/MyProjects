// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2007 Anthony Williams
#ifndef THREAD_HEAP_ALLOC_HPP
#define THREAD_HEAP_ALLOC_HPP
#include <new>
#include "thread_primitives.hpp"
#include <stdexcept>
#include <boost/assert.hpp>

#if defined( BOOST_USE_WINDOWS_H )
# include <windows.h>

# ifdef HeapAlloc
# undef HeapAlloc
# endif

namespace boost
{
    namespace detail
    {
        namespace win32
        {
            using ::GetProcessHeap;
            using ::HeapAlloc;
            using ::HeapFree;
        }
    }
}

#else

# ifdef HeapAlloc
# undef HeapAlloc
# endif

namespace boost
{
    namespace detail
    {
        namespace win32
        {
            extern "C"
            {
                __declspec(dllimport) handle __stdcall GetProcessHeap();
                __declspec(dllimport) void* __stdcall HeapAlloc(handle,unsigned long,ulong_ptr);
                __declspec(dllimport) int __stdcall HeapFree(handle,unsigned long,void*);
            }
        }
    }
}

#endif

#include <boost/config/abi_prefix.hpp>

namespace boost
{
    namespace detail
    {
        inline BOOST_THREAD_DECL void* allocate_raw_heap_memory(unsigned size)
        {
            void* const heap_memory=detail::win32::HeapAlloc(detail::win32::GetProcessHeap(),0,size);
            if(!heap_memory)
            {
                throw std::bad_alloc();
            }
            return heap_memory;
        }

        inline BOOST_THREAD_DECL void free_raw_heap_memory(void* heap_memory)
        {
            BOOST_VERIFY(detail::win32::HeapFree(detail::win32::GetProcessHeap(),0,heap_memory)!=0);
        }
            
        template<typename T>
        inline T* heap_new()
        {
            void* const heap_memory=allocate_raw_heap_memory(sizeof(T));
            try
            {
                T* const data=new (heap_memory) T();
                return data;
            }
            catch(...)
            {
                free_raw_heap_memory(heap_memory);
                throw;
            }
        }

#ifdef BOOST_HAS_RVALUE_REFS
        template<typename T,typename A1>
        inline T* heap_new(A1&& a1)
        {
            void* const heap_memory=allocate_raw_heap_memory(sizeof(T));
            try
            {
                T* const data=new (heap_memory) T(static_cast<A1&&>(a1));
                return data;
            }
            catch(...)
            {
                free_raw_heap_memory(heap_memory);
                throw;
            }
        }
        template<typename T,typename A1,typename A2>
        inline T* heap_new(A1&& a1,A2&& a2)
        {
            void* const heap_memory=allocate_raw_heap_memory(sizeof(T));
            try
            {
                T* const data=new (heap_memory) T(static_cast<A1&&>(a1),static_cast<A2&&>(a2));
                return data;
            }
            catch(...)
            {
                free_raw_heap_memory(heap_memory);
                throw;
            }
        }
        template<typename T,typename A1,typename A2,typename A3>
        inline T* heap_new(A1&& a1,A2&& a2,A3&& a3)
        {
            void* const heap_memory=allocate_raw_heap_memory(sizeof(T));
            try
            {
                T* const data=new (heap_memory) T(static_cast<A1&&>(a1),static_cast<A2&&>(a2),
                                                  static_cast<A3&&>(a3));
                return data;
            }
            catch(...)
            {
                free_raw_heap_memory(heap_memory);
                throw;
            }
        }
        template<typename T,typename A1,typename A2,typename A3,typename A4>
        inline T* heap_new(A1&& a1,A2&& a2,A3&& a3,A4&& a4)
        {
            void* const heap_memory=allocate_raw_heap_memory(sizeof(T));
            try
            {
                T* const data=new (heap_memory) T(static_cast<A1&&>(a1),static_cast<A2&&>(a2),
                                                  static_cast<A3&&>(a3),static_cast<A4&&>(a4));
                return data;
            }
            catch(...)
            {
                free_raw_heap_memory(heap_memory);
                throw;
            }
        }
#else
        template<typename T,typename A1>
        inline T* heap_new_impl1(A1 a1)
        {
            void* const heap_memory=allocate_raw_heap_memory(sizeof(T));
            try
            {
                T* const data=new (heap_memory) T(a1);
                return data;
            }
            catch(...)
            {
                free_raw_heap_memory(heap_memory);
                throw;
            }
        }

        template<typename T,typename A1,typename A2>
        inline T* heap_new_impl2(A1 a1,A2 a2)
        {
            void* const heap_memory=allocate_raw_heap_memory(sizeof(T));
            try
            {
                T* const data=new (heap_memory) T(a1,a2);
                return data;
            }
            catch(...)
            {
                free_raw_heap_memory(heap_memory);
                throw;
            }
        }

        template<typename T,typename A1,typename A2,typename A3>
        inline T* heap_new_impl3(A1 a1,A2 a2,A3 a3)
        {
            void* const heap_memory=allocate_raw_heap_memory(sizeof(T));
            try
            {
                T* const data=new (heap_memory) T(a1,a2,a3);
                return data;
            }
            catch(...)
            {
                free_raw_heap_memory(heap_memory);
                throw;
            }
        }

        template<typename T,typename A1,typename A2,typename A3,typename A4>
        inline T* heap_new_impl4(A1 a1,A2 a2,A3 a3,A4 a4)
        {
            void* const heap_memory=allocate_raw_heap_memory(sizeof(T));
            try
            {
                T* const data=new (heap_memory) T(a1,a2,a3,a4);
                return data;
            }
            catch(...)
            {
                free_raw_heap_memory(heap_memory);
                throw;
            }
        }


        template<typename T,typename A1>
        inline T* heap_new(A1 const& a1)
        {
            return heap_new_impl1<T,A1 const&>(a1);
        }
        template<typename T,typename A1>
        inline T* heap_new(A1& a1)
        {
            return heap_new_impl1<T,A1&>(a1);
        }
        
        template<typename T,typename A1,typename A2>
        inline T* heap_new(A1 const& a1,A2 const& a2)
        {
            return heap_new_impl2<T,A1 const&,A2 const&>(a1,a2);
        }
        template<typename T,typename A1,typename A2>
        inline T* heap_new(A1& a1,A2 const& a2)
        {
            return heap_new_impl2<T,A1&,A2 const&>(a1,a2);
        }
        template<typename T,typename A1,typename A2>
        inline T* heap_new(A1 const& a1,A2& a2)
        {
            return heap_new_impl2<T,A1 const&,A2&>(a1,a2);
        }
        template<typename T,typename A1,typename A2>
        inline T* heap_new(A1& a1,A2& a2)
        {
            return heap_new_impl2<T,A1&,A2&>(a1,a2);
        }

        template<typename T,typename A1,typename A2,typename A3>
        inline T* heap_new(A1 const& a1,A2 const& a2,A3 const& a3)
        {
            return heap_new_impl3<T,A1 const&,A2 const&,A3 const&>(a1,a2,a3);
        }
        template<typename T,typename A1,typename A2,typename A3>
        inline T* heap_new(A1& a1,A2 const& a2,A3 const& a3)
        {
            return heap_new_impl3<T,A1&,A2 const&,A3 const&>(a1,a2,a3);
        }
        template<typename T,typename A1,typename A2,typename A3>
        inline T* heap_new(A1 const& a1,A2& a2,A3 const& a3)
        {
            return heap_new_impl3<T,A1 const&,A2&,A3 const&>(a1,a2,a3);
        }
        template<typename T,typename A1,typename A2,typename A3>
        inline T* heap_new(A1& a1,A2& a2,A3 const& a3)
        {
            return heap_new_impl3<T,A1&,A2&,A3 const&>(a1,a2,a3);
        }

        template<typename T,typename A1,typename A2,typename A3>
        inline T* heap_new(A1 const& a1,A2 const& a2,A3& a3)
        {
            return heap_new_impl3<T,A1 const&,A2 const&,A3&>(a1,a2,a3);
        }
        template<typename T,typename A1,typename A2,typename A3>
        inline T* heap_new(A1& a1,A2 const& a2,A3& a3)
        {
            return heap_new_impl3<T,A1&,A2 const&,A3&>(a1,a2,a3);
        }
        template<typename T,typename A1,typename A2,typename A3>
        inline T* heap_new(A1 const& a1,A2& a2,A3& a3)
        {
            return heap_new_impl3<T,A1 const&,A2&,A3&>(a1,a2,a3);
        }
        template<typename T,typename A1,typename A2,typename A3>
        inline T* heap_new(A1& a1,A2& a2,A3& a3)
        {
            return heap_new_impl3<T,A1&,A2&,A3&>(a1,a2,a3);
        }

        template<typename T,typename A1,typename A2,typename A3,typename A4>
        inline T* heap_new(A1 const& a1,A2 const& a2,A3 const& a3,A4 const& a4)
        {
            return heap_new_impl4<T,A1 const&,A2 const&,A3 const&,A4 const&>(a1,a2,a3,a4);
        }
        template<typename T,typename A1,typename A2,typename A3,typename A4>
        inline T* heap_new(A1& a1,A2 const& a2,A3 const& a3,A4 const& a4)
        {
            return heap_new_impl4<T,A1&,A2 const&,A3 const&,A4 const&>(a1,a2,a3,a4);
        }
        template<typename T,typename A1,typename A2,typename A3,typename A4>
        inline T* heap_new(A1 const& a1,A2& a2,A3 const& a3,A4 const& a4)
        {
            return heap_new_impl4<T,A1 const&,A2&,A3 const&,A4 const&>(a1,a2,a3,a4);
        }
        template<typename T,typename A1,typename A2,typename A3,typename A4>
        inline T* heap_new(A1& a1,A2& a2,A3 const& a3,A4 const& a4)
        {
            return heap_new_impl4<T,A1&,A2&,A3 const&,A4 const&>(a1,a2,a3,a4);
        }

        template<typename T,typename A1,typename A2,typename A3,typename A4>
        inline T* heap_new(A1 const& a1,A2 const& a2,A3& a3,A4 const& a4)
        {
            return heap_new_impl4<T,A1 const&,A2 const&,A3&,A4 const&>(a1,a2,a3,a4);
        }
        template<typename T,typename A1,typename A2,typename A3,typename A4>
        inline T* heap_new(A1& a1,A2 const& a2,A3& a3,A4 const& a4)
        {
            return heap_new_impl4<T,A1&,A2 const&,A3&,A4 const&>(a1,a2,a3,a4);
        }
        template<typename T,typename A1,typename A2,typename A3,typename A4>
        inline T* heap_new(A1 const& a1,A2& a2,A3& a3,A4 const& a4)
        {
            return heap_new_impl4<T,A1 const&,A2&,A3&,A4 const&>(a1,a2,a3,a4);
        }
        template<typename T,typename A1,typename A2,typename A3,typename A4>
        inline T* heap_new(A1& a1,A2& a2,A3& a3,A4 const& a4)
        {
            return heap_new_impl4<T,A1&,A2&,A3&,A4 const&>(a1,a2,a3,a4);
        }
        template<typename T,typename A1,typename A2,typename A3,typename A4>
        inline T* heap_new(A1 const& a1,A2 const& a2,A3 const& a3,A4& a4)
        {
            return heap_new_impl4<T,A1 const&,A2 const&,A3 const&,A4&>(a1,a2,a3,a4);
        }
        template<typename T,typename A1,typename A2,typename A3,typename A4>
        inline T* heap_new(A1& a1,A2 const& a2,A3 const& a3,A4& a4)
        {
            return heap_new_impl4<T,A1&,A2 const&,A3 const&,A4&>(a1,a2,a3,a4);
        }
        template<typename T,typename A1,typename A2,typename A3,typename A4>
        inline T* heap_new(A1 const& a1,A2& a2,A3 const& a3,A4& a4)
        {
            return heap_new_impl4<T,A1 const&,A2&,A3 const&,A4&>(a1,a2,a3,a4);
        }
        template<typename T,typename A1,typename A2,typename A3,typename A4>
        inline T* heap_new(A1& a1,A2& a2,A3 const& a3,A4& a4)
        {
            return heap_new_impl4<T,A1&,A2&,A3 const&,A4&>(a1,a2,a3,a4);
        }

        template<typename T,typename A1,typename A2,typename A3,typename A4>
        inline T* heap_new(A1 const& a1,A2 const& a2,A3& a3,A4& a4)
        {
            return heap_new_impl4<T,A1 const&,A2 const&,A3&,A4&>(a1,a2,a3,a4);
        }
        template<typename T,typename A1,typename A2,typename A3,typename A4>
        inline T* heap_new(A1& a1,A2 const& a2,A3& a3,A4& a4)
        {
            return heap_new_impl4<T,A1&,A2 const&,A3&,A4&>(a1,a2,a3,a4);
        }
        template<typename T,typename A1,typename A2,typename A3,typename A4>
        inline T* heap_new(A1 const& a1,A2& a2,A3& a3,A4& a4)
        {
            return heap_new_impl4<T,A1 const&,A2&,A3&,A4&>(a1,a2,a3,a4);
        }
        template<typename T,typename A1,typename A2,typename A3,typename A4>
        inline T* heap_new(A1& a1,A2& a2,A3& a3,A4& a4)
        {
            return heap_new_impl4<T,A1&,A2&,A3&,A4&>(a1,a2,a3,a4);
        }
        
#endif        
        template<typename T>
        inline void heap_delete(T* data)
        {
            data->~T();
            free_raw_heap_memory(data);
        }

        template<typename T>
        struct do_heap_delete
        {
            void operator()(T* data) const
            {
                detail::heap_delete(data);
            }
        };
    }
}

#include <boost/config/abi_suffix.hpp>


#endif
