
#ifndef _LIVE_P2PCOMMON2_BASE_PPL_DATA_ALLOCATOR_IMPL_H_
#define _LIVE_P2PCOMMON2_BASE_PPL_DATA_ALLOCATOR_IMPL_H_

#include <stddef.h>
#include <ppl/std/class.h>


template <typename T, typename AllocT>
class allocator_impl
{
public:
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef T* pointer;
	typedef const T* const_pointer;
	typedef T& reference;
	typedef const T& const_reference;
	typedef T value_type;

	allocator_impl() { }
	allocator_impl(const allocator_impl<T, AllocT>&) { }

#if _MSC_VER > 1300
	template<class _Other>
	allocator_impl(const allocator_impl<_Other, AllocT>&) { }

	template<class _Other>
	allocator_impl<T, AllocT>& operator=(const allocator_impl<_Other, AllocT>&) { return (*this); }
#endif

	pointer address(reference x) const { return &x; }
	const_pointer address(const_reference x) const { return &x; }

	pointer allocate(size_type n, const void* p = 0)
	{
		return static_cast<pointer>(AllocT::allocate(sizeof(T) * n));
	}
	char* _Charalloc(size_type n)
	{
		return static_cast<char*>(AllocT::allocate(sizeof(T) * n));
	}
	void deallocate(void* p, size_type)
	{
		AllocT::deallocate(p);
	}
	void construct(pointer p, const T& val)
	{
		ppl::construct(p, val);
	}
	void destroy(pointer p)
	{
		ppl::destruct(p);
	}
	size_t max_size() const
	{
		size_t n = (size_t)(-1) / sizeof (T);
		return (0 < n ? n : 1);
	}

	template<typename _Other>
	struct rebind
	{	// convert an allocator<_Ty> to an allocator <_Other>
		typedef allocator_impl<_Other, AllocT> other;
	};

};

template<typename T1, typename T2, typename AllocT> 
inline bool operator==(const allocator_impl<T1, AllocT>&, const allocator_impl<T2, AllocT>&)
{
	// test for allocator equality (always true)
	return true;
}

#endif