#pragma once

#include <cassert>
#include <cstdint>

namespace ciph {

// One field packed into a 64-bit word: `Width` bits starting at bit `Shift`,
// read and written as a `T` (bool, an enum, or a small unsigned number).
//
// Fields are declared one after another with `Next`, so each starts where the
// previous one ends and nobody counts bit positions by hand:
//
//     using First  = BitField<bool, 0, 1>;
//     using Second = First::Next<MyEnum, 3>;   // bits 1..3
//
// A region that must stay put for others to build on, like the base node's
// bits, reserves an end and checks it with a static_assert.
template <class T, unsigned Shift, unsigned Width>
struct BitField {
    static_assert(Width > 0, "a field needs at least one bit");
    static_assert(Shift + Width <= 64, "the field does not fit in 64 bits");

    using Type = T;
    static constexpr unsigned kShift = Shift;
    static constexpr unsigned kWidth = Width;
    static constexpr unsigned kEnd = Shift + Width;
    static constexpr uint64_t kMax = Width == 64 ? ~0ull : (1ull << Width) - 1;
    static constexpr uint64_t kMask = kMax << Shift;

    // The field declared directly after this one.
    template <class T2, unsigned Width2>
    using Next = BitField<T2, kEnd, Width2>;

    [[nodiscard]] static constexpr T
    get(uint64_t bits) {
        return static_cast<T>((bits & kMask) >> Shift);
    }

    [[nodiscard]] static constexpr uint64_t
    set(uint64_t bits, T value) {
        const uint64_t raw = static_cast<uint64_t>(value);
        assert(raw <= kMax && "value does not fit in the field");
        return (bits & ~kMask) | (raw << Shift);
    }
};

} // namespace ciph
