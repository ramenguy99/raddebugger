// Copyright (c) 2024 Epic Games Tools
// Licensed under the MIT license (https://opensource.org/license/mit/)

#ifndef BASE_INS_H
#define BASE_INS_H

////////////////////////////////
// NOTE(allen): Implementations of Intrinsics

#if OS_WINDOWS

# include <windows.h>
# include <tmmintrin.h>
# include <wmmintrin.h>
# include <intrin.h>

# if ARCH_X64
#  define ins_atomic_u64_eval(x) InterlockedAdd((volatile LONG *)(x), 0)
#  define ins_atomic_u64_inc_eval(x) InterlockedIncrement64((volatile __int64 *)(x))
#  define ins_atomic_u64_dec_eval(x) InterlockedDecrement64((volatile __int64 *)(x))
#  define ins_atomic_u64_eval_assign(x,c) InterlockedExchange64((volatile __int64 *)(x),(c))
#  define ins_atomic_u64_add_eval(x,c) InterlockedAdd((volatile LONG *)(x), c)
#  define ins_atomic_u32_eval_assign(x,c) InterlockedExchange((volatile LONG *)(x),(c))
#  define ins_atomic_u32_eval_cond_assign(x,k,c) InterlockedCompareExchange((volatile LONG *)(x),(k),(c))
#  define ins_atomic_ptr_eval_assign(x,c) (void*)ins_atomic_u64_eval_assign((volatile __int64 *)(x), (__int64)(c))
# endif

#elif OS_LINUX

#include <atomic>

internal U32
ins_atomic_u32_eval_cond_assign_helper(volatile std::atomic<U32>* dest, U32 exchange, U32 comperand)
{
    std::atomic_compare_exchange_strong(dest, &exchange, comperand);
    return exchange;
}

#define ins_atomic_u64_eval(x) std::atomic_load((volatile std::atomic<U64> *)(x))
#define ins_atomic_u64_inc_eval(x) std::atomic_fetch_add((volatile std::atomic<U64> *)(x), 1)
#define ins_atomic_u64_add_eval(x,c) std::atomic_fetch_add((volatile std::atomic<U64>*)(x), c)
#define ins_atomic_u64_dec_eval(x) std::atomic_fetch_sub((volatile std::atomic<U64> *)(x), 1)
#define ins_atomic_u64_eval_assign(x,c) std::atomic_exchange((volatile std::atomic<U64>*)(x), c)
#define ins_atomic_u32_eval_assign(x,c) std::atomic_exchange((volatile std::atomic<U32>*)(x), c)
#define ins_atomic_u32_eval_cond_assign(x,k,c) ins_atomic_u32_eval_cond_assign_helper((volatile std::atomic<U32>*)(x), k, c)
#define ins_atomic_ptr_eval_assign(x,c) (void*)ins_atomic_u64_eval_assign((volatile std::atomic<U64>*)(x), (U64)(c))

#else
// TODO(allen): 
#endif

////////////////////////////////
// NOTE(allen): Intrinsic Checks

#if ARCH_X64

# if !defined(ins_atomic_u64_inc_eval)
# error missing: ins_atomic_u64_inc_eval
# endif

#else
# error the intrinsic set for this arch is not developed
#endif


#endif //BASE_INS_H
