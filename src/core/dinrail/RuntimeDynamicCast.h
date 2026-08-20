// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_RUNTIMEDYNAMICCAST_H
#define DINRAIL_RUNTIMEDYNAMICCAST_H

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <typeinfo>

namespace dinrail
{

/**
 * @brief Non-owning view of one statically typed polymorphic subobject.
 *
 * Bundles the data that must describe the same source instance for an
 * ABI-level runtime cast.
 *
 * It is only used to pass the source pointer and its static type to runtimeDynamicCast().
 *
 * The pointed-to object must remain alive while the view is used.
 */
class PolymorphicView final
{
public:
    /** @brief Construct an empty view (source type `void`, null pointer). */
    PolymorphicView() noexcept = default;

    /**
     * @brief Construct a view from a source pointer and its static type.
     * @param sourcePointer Exact pointer to the static source subobject.
     * @param sourceType `typeid(Source)` for that pointer.
     */
    PolymorphicView(void* sourcePointer,
                    const std::type_info& sourceType,
                    const std::ptrdiff_t vfDelta = 0) noexcept
        : m_sourcePointer(sourcePointer)
        , m_sourceType(&sourceType)
        , m_vfDelta(vfDelta)
    {
    }

    /** @brief Return the exact static source-subobject pointer. */
    [[nodiscard]] void* data() const noexcept
    {
        return m_sourcePointer;
    }

    /** @brief Return the static source type represented by this view. */
    [[nodiscard]] const std::type_info& sourceType() const noexcept
    {
        return *m_sourceType;
    }

    /** @brief Return the source-to-RTTI-vfptr byte displacement on MSVC, or 0 on non-MSVC
     * compilers. */
    [[nodiscard]] std::ptrdiff_t vfDelta() const noexcept
    {
        return m_vfDelta;
    }

private:
    void* m_sourcePointer{nullptr};
    const std::type_info* m_sourceType{&typeid(void)};

    std::ptrdiff_t m_vfDelta{0};
};

/**
 * @brief Create a view from a typed source pointer.
 *
 * @p Source must be a polymorphic class (i.e. a class that inherits from at least a virtual
 * method).
 *
 * @code{.cpp}
 * IBase* base = getObject();
 * auto view = dinrail::makePolymorphicView(base);
 * @endcode
 */
template <class Source> [[nodiscard]] PolymorphicView makePolymorphicView(Source* source) noexcept
{
    using UnqualifiedSource = std::remove_cv_t<Source>;

    static_assert(!std::is_const_v<Source>,
                  "const source objects are not supported by this void*-returning API");
    static_assert(!std::is_volatile_v<Source>, "volatile source objects are not supported");
    static_assert(std::is_class_v<UnqualifiedSource>, "Source must be a class type");
    static_assert(std::is_polymorphic_v<UnqualifiedSource>, "Source must be polymorphic");

#if defined(_MSC_VER)
    // Byte displacement from the source pointer to the subobject that owns the
    // RTTI vfptrx
    const auto sourceAddress = reinterpret_cast<std::intptr_t>(static_cast<void*>(source));
    const auto rttiAddress = reinterpret_cast<std::intptr_t>(
        static_cast<void*>(static_cast<UnqualifiedSource*>(source)));
    const auto vfDelta = static_cast<std::ptrdiff_t>(rttiAddress - sourceAddress);
    return PolymorphicView{static_cast<void*>(source), typeid(UnqualifiedSource), vfDelta};
#else
    return PolymorphicView{static_cast<void*>(source), typeid(UnqualifiedSource)};
#endif
}

/**
 * @brief Perform a dynamic cast at runtime.
 *
 * Runtime-type equivalent of the genuinely dynamic part of a pointer
 * `dynamic_cast` (down-casts and cross-casts).
 *
 * @param source Non-owning view of the exact static source subobject, created by
 * `dinrail::makePolymorphicView`.
 * @param testedType `typeid(TargetClass)` (a class type).
 *
 * @return A correctly adjusted target pointer, or `nullptr` when the cast
 * fails.
 *
 * @code{.cpp}
 * IBase* base = getObject();
 * auto view = dinrail::makePolymorphicView(base);
 * IDerived* derived = static_cast<IDerived*>(
 *     dinrail::runtimeDynamicCast(view, typeid(IDerived)));
 * @endcode
 *
 * To support compilation on platforms different that do not support the Itanium ABI or MSVC ABI,
 * the implementation is not compiled if DINRAIL_ENABLE_INTEROP_PLUGINS CMake option is set to OFF,
 * and this function will always return nullptr.
 *
 */
[[nodiscard]]
void* runtimeDynamicCast(const PolymorphicView& source, const std::type_info& testedType) noexcept;

} // namespace dinrail

#endif // DINRAIL_RUNTIMEDYNAMICCAST_H
