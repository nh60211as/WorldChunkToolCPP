#ifndef DEFAULTINITALLOCATOR_H
#define DEFAULTINITALLOCATOR_H

#include <type_traits>

// This implementation is from VC++ 2019.
// it simply disable zero initialization by not doing anything in construct() and destroy().
// Currently using C++17 standard. Might need to update the usage starting from C++20.

using true_type = std::bool_constant<true>;
using false_type = std::bool_constant<false>;

// CLASS TEMPLATE DefaultInitAllocator
template <class _Ty>
class DefaultInitAllocator {
public:
    static_assert(!std::is_const_v<_Ty>, "The C++ Standard forbids containers of const elements "
        "because allocator<const T> is ill-formed.");

    using _From_primary = DefaultInitAllocator;

    using value_type = _Ty;

    _CXX17_DEPRECATE_OLD_ALLOCATOR_MEMBERS typedef _Ty* pointer;
    _CXX17_DEPRECATE_OLD_ALLOCATOR_MEMBERS typedef const _Ty* const_pointer;

    _CXX17_DEPRECATE_OLD_ALLOCATOR_MEMBERS typedef _Ty& reference;
    _CXX17_DEPRECATE_OLD_ALLOCATOR_MEMBERS typedef const _Ty& const_reference;

    using size_type = size_t;
    using difference_type = ptrdiff_t;

    using propagate_on_container_move_assignment = true_type;
    using is_always_equal = true_type;

    template <class _Other>
    struct _CXX17_DEPRECATE_OLD_ALLOCATOR_MEMBERS rebind {
        using other = DefaultInitAllocator<_Other>;
    };

    _CXX17_DEPRECATE_OLD_ALLOCATOR_MEMBERS _NODISCARD _Ty* address(_Ty& _Val) const noexcept {
        return _STD addressof(_Val);
    }

    _CXX17_DEPRECATE_OLD_ALLOCATOR_MEMBERS _NODISCARD const _Ty* address(const _Ty& _Val) const noexcept {
        return _STD addressof(_Val);
    }

    constexpr DefaultInitAllocator() noexcept {}

    constexpr DefaultInitAllocator(const DefaultInitAllocator&) noexcept = default;
    template <class _Other>
    constexpr DefaultInitAllocator(const DefaultInitAllocator<_Other>&) noexcept {}

    void deallocate(_Ty* const _Ptr, const size_t _Count) {
        // no overflow check on the following multiply; we assume _Allocate did that check
        std::_Deallocate<std::_New_alignof<_Ty>>(_Ptr, sizeof(_Ty) * _Count);
    }

    _NODISCARD __declspec(allocator) _Ty* allocate(_CRT_GUARDOVERFLOW const size_t _Count) {
        return static_cast<_Ty*>(std::_Allocate<std::_New_alignof<_Ty>>(std::_Get_size_of_n<sizeof(_Ty)>(_Count)));
    }

    _CXX17_DEPRECATE_OLD_ALLOCATOR_MEMBERS _NODISCARD __declspec(allocator) _Ty* allocate(
        _CRT_GUARDOVERFLOW const size_t _Count, const void*) {
        return allocate(_Count);
    }

    template <class _Objty, class... _Types>
    _CXX17_DEPRECATE_OLD_ALLOCATOR_MEMBERS void construct(_Objty* const _Ptr, _Types&&... _Args) {
        //::new (const_cast<void*>(static_cast<const volatile void*>(_Ptr))) _Objty(_STD forward<_Types>(_Args)...);
    }

    template <class _Uty>
    _CXX17_DEPRECATE_OLD_ALLOCATOR_MEMBERS void destroy(_Uty* const _Ptr) {
        //_Ptr->~_Uty();
    }

    _CXX17_DEPRECATE_OLD_ALLOCATOR_MEMBERS _NODISCARD size_t max_size() const noexcept {
        return static_cast<size_t>(-1) / sizeof(_Ty);
    }
};

#endif
