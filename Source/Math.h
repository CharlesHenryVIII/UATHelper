#pragma once
#include <cstdint>
#include <cassert>

using s8  = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;
using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using f32 = float;
using f64 = double;


template <typename T>
struct ExitScope
{
    T lambda;
    ExitScope(T lambda): lambda(lambda){ }
    ~ExitScope(){ lambda();}
};

struct ExitScopeHelp
{
    template<typename T>
    ExitScope<T> operator+(T t) { return t; }
};
#define _UATH_CONCAT(a, b) a ## b
#define UATH_CONCAT(a, b) _UATH_CONCAT(a, b)
#define DEFER auto UATH_CONCAT(defer__, __LINE__) = ExitScopeHelp() + [&]()
#define arrsize(arr__) (sizeof(arr__) / sizeof(arr__[0]))

template <typename T>
[[nodiscard]] T Min(T a, T b)
{
    return a < b ? a : b;
}

 template <typename T>
[[nodiscard]] T Max(T a, T b)
{
    return a > b ? a : b;
}

template <typename T>
[[nodiscard]] T Clamp(T v, T min, T max)
{
    return Max(min, Min(max, v));
}
static void Swap(void* a, void* b, const s32 size);
static s32 Partition(u8* array, const s32 itemSize, s32 iBegin, s32 iEnd, s32(*compare)(const void*, const void*), s32* comparisons);
static void QuickSortInternal(u8* array, const s32 itemSize, s32 iBegin, s32 iEnd, s32(*compare)(const void*, const void*), s32* comparisons);
static s32 QuickSort(u8* data, const s32 arrayCount, const s32 itemSize, s32(*compare)(const void* a, const void* b));

void Swap(void* a, void* b, const s32 size)
{
    u8* c = (u8*)a;
    u8* d = (u8*)b;
    for (s32 i = 0; i < size; i++)
    {

		u8 temp = c[i];
		c[i] = d[i];
		d[i] = temp;
    }
}

s32 Partition(u8* array, const s32 itemSize, s32 iBegin, s32 iEnd, s32 (*compare)(const void*, const void*), s32* comparisons)
{
    assert(array != nullptr);
    u8* pivot = &array[iEnd * itemSize];
    assert(pivot != nullptr);
    s32 lowOffset = iBegin;

	for (s32 i = iBegin; i < iEnd; i++)
	{
        (*comparisons)++;
		if (compare(&array[i * itemSize], pivot) > 0)
		{

			Swap(&array[lowOffset * itemSize], &array[i * itemSize], itemSize);
			lowOffset++;
		}
	}

    Swap(&array[lowOffset * itemSize], &array[iEnd * itemSize], itemSize);
    return lowOffset;
}

void QuickSortInternal(u8* array, const s32 itemSize, s32 iBegin, s32 iEnd, s32 (*compare)(const void*, const void*), s32* comparisons)
{
    (*comparisons)++;
	if (iBegin < iEnd)
	{
        s32 pivotIndex = Partition(array, itemSize, iBegin, iEnd, compare, comparisons);
		QuickSortInternal(array, itemSize, iBegin, pivotIndex - 1, compare, comparisons); //Low Sort
		QuickSortInternal(array, itemSize, pivotIndex + 1, iEnd, compare, comparisons); //High Sort
	}
}

//Returns the amount of values compared (comparison count)
s32 QuickSort(u8* data, const s32 arrayCount, const s32 itemSize, s32 (*compare)(const void* a, const void* b))
{
    s32 comparisons = 0;
    QuickSortInternal(data, itemSize, 0, arrayCount - 1, compare, &comparisons);
    return comparisons;
}
