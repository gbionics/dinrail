// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#include <dinrail/RuntimeDynamicCast.h>

#if defined(DINRAIL_ENABLE_INTEROP_PLUGINS)

#if defined(_MSC_VER)

#if !defined(_CPPRTTI)
#error "dinrail::runtimeDynamicCast requires RTTI; enable /GR"
#endif

#include <limits>
#include <rttidata.h>
#include <windows.h>

#elif defined(__GXX_ABI_VERSION)

#if !defined(__GXX_RTTI) && !defined(__cpp_rtti)
#error "dinrail::runtimeDynamicCast requires RTTI; do not use -fno-rtti"
#endif

#include <cxxabi.h>

// Apple's <cxxabi.h> does not expose __class_type_info or __dynamic_cast in
// its public headers even though they are present in the libc++abi runtime.
// Forward-declare them so we can call through the Itanium ABI on Apple platforms.
#if defined(__APPLE__)
#include <cstddef>

namespace __cxxabiv1
{
class __class_type_info;

extern "C" void* __dynamic_cast(const void* sourcePointer,
                                const __class_type_info* sourceType,
                                const __class_type_info* targetType,
                                std::ptrdiff_t sourceToTargetOffset);
} // namespace __cxxabiv1
#endif

#else

#error \
    "dinrail::runtimeDynamicCast supports only the Microsoft C++ ABI and the GCC/Clang Itanium C++ ABI, to compile on other platforms please set DINRAIL_ENABLE_INTEROP_PLUGINS CMake option to OFF"

#endif

#endif // DINRAIL_ENABLE_INTEROP_PLUGINS

namespace dinrail
{

void* runtimeDynamicCast(const PolymorphicView& source, const std::type_info& testedType) noexcept
{
    void* const basePtr = source.data();
    if (basePtr == nullptr)
    {
        return nullptr;
    }

    const std::type_info& baseType = source.sourceType();

    // Identity: the source pointer already designates the requested type.
    if (baseType == testedType)
    {
        return basePtr;
    }

#if !defined(DINRAIL_ENABLE_INTEROP_PLUGINS)

    // ABI-specific casts are not compiled in; only the identity cast above is supported.
    return nullptr;

#elif defined(_MSC_VER)

    // Check that vfDelta is well formed
    const std::ptrdiff_t vfDelta = source.vfDelta();
    if (vfDelta < (std::numeric_limits<LONG>::min)()
        || vfDelta > (std::numeric_limits<LONG>::max)())
    {
        return nullptr;
    }

    // __RTDynamicCast receives the source-object pointer and uses vfDelta to
    // locate the RTTI vfptr.
    void* const rttiPtr = basePtr;

#if defined(_CPPUNWIND) || defined(__EXCEPTIONS)
    try
    {
        return __RTDynamicCast(rttiPtr,
                               static_cast<LONG>(vfDelta),
                               const_cast<std::type_info*>(&baseType),
                               const_cast<std::type_info*>(&testedType),
                               0);
    } catch (...)
    {
        // Preserve the noexcept/null-on-failure contract if the Microsoft RTTI
        // runtime reports malformed or unavailable RTTI through an exception.
        return nullptr;
    }
#else
    return __RTDynamicCast(rttiPtr,
                           static_cast<LONG>(vfDelta),
                           const_cast<std::type_info*>(&baseType),
                           const_cast<std::type_info*>(&testedType),
                           0);
#endif

#else

    using ClassTypeInfo = __cxxabiv1::__class_type_info;

#if defined(__APPLE__)
    // libc++abi keeps __class_type_info incomplete in its public headers.
    const auto* const sourceClass = reinterpret_cast<const ClassTypeInfo*>(&baseType);
    const auto* const targetClass = reinterpret_cast<const ClassTypeInfo*>(&testedType);
#else
    const auto* const sourceClass = dynamic_cast<const ClassTypeInfo*>(&baseType);
    const auto* const targetClass = dynamic_cast<const ClassTypeInfo*>(&testedType);

    if (sourceClass == nullptr || targetClass == nullptr)
    {
        return nullptr;
    }
#endif

    // -1 means no source-to-destination offset hint is available; always valid.
    return __cxxabiv1::__dynamic_cast(basePtr, sourceClass, targetClass, -1);

#endif
}

} // namespace dinrail
