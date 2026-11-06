// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_BINDINGS_CORE_METHODWRAPPER_H
#define DINRAIL_BINDINGS_CORE_METHODWRAPPER_H

#include <dinrail/Status.h>
#include <dinrail/VectorProxy.h>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <cstddef>
#include <optional>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace dinrail::bindings::core
{

namespace detail
{

namespace py = ::pybind11;

// An output is None in Python if the call failed, to not expose partially filled outputs.
template <class T> std::tuple<std::optional<T>> collectOutput(T& value, bool success)
{
    return std::tuple<std::optional<T>>(success ? std::optional<T>(std::move(value))
                                                : std::nullopt);
}

inline bool succeeded(bool result)
{
    return result;
}

inline bool succeeded(const ::dinrail::Status& result)
{
    return result.ok();
}

template <class Result> bool succeeded(const Result&)
{
    return true;
}

// How a C++ parameter is mapped to Python. This primary template handles inputs passed by value
// or by const reference.
template <class P, class = void> struct Param
{
    using Plain = std::remove_cv_t<std::remove_reference_t<P>>;
    static constexpr bool isOutput = false;
    using Input = Plain;
    using Storage = Plain;

    static Storage fromInput(Input&& input)
    {
        return std::move(input);
    }

    static Plain& pass(Storage& storage)
    {
        return storage;
    }
};

// Non-const reference parameters are outputs.
template <class P>
struct Param<
    P,
    std::enable_if_t<std::is_lvalue_reference_v<P> && !std::is_const_v<std::remove_reference_t<P>>>>
{
    using Plain = std::remove_reference_t<P>;
    static constexpr bool isOutput = true;
    using Input = void;
    using Storage = Plain;

    static Storage makeOutput(std::size_t)
    {
        return Storage{};
    }

    static Plain& pass(Storage& storage)
    {
        return storage;
    }

    static auto collect(Storage& storage, bool success)
    {
        return collectOutput(storage, success);
    }
};

// Pointer parameters are outputs.
template <class P> struct Param<P, std::enable_if_t<std::is_pointer_v<P>>>
{
    using Plain = std::remove_cv_t<std::remove_pointer_t<P>>;
    static constexpr bool isOutput = true;
    using Input = void;
    using Storage = Plain;

    static Storage makeOutput(std::size_t)
    {
        return Storage{};
    }

    static Plain* pass(Storage& storage)
    {
        return &storage;
    }

    static auto collect(Storage& storage, bool success)
    {
        return collectOutput(storage, success);
    }
};

// Vector outputs are lists, sized in advance as requested by the size function of the method.
template <> struct Param<::dinrail::VectorProxy<double>::Ref>
{
    static constexpr bool isOutput = true;
    using Input = void;
    using Storage = std::vector<double>;

    static Storage makeOutput(std::size_t size)
    {
        return Storage(size);
    }

    static ::dinrail::VectorProxy<double>::Ref pass(Storage& storage)
    {
        return ::dinrail::VectorProxy<double>::Ref(storage);
    }

    static auto collect(Storage& storage, bool success)
    {
        return collectOutput(storage, success);
    }
};

// Vector inputs are lists.
template <class T> struct VectorInputParam
{
    static constexpr bool isOutput = false;
    using Input = std::vector<T>;
    using Storage = std::vector<T>;

    static Storage fromInput(Input&& input)
    {
        return std::move(input);
    }

    static typename ::dinrail::VectorProxy<const T>::Ref pass(Storage& storage)
    {
        return typename ::dinrail::VectorProxy<const T>::Ref(storage);
    }
};

template <> struct Param<::dinrail::VectorProxy<const double>::Ref> : VectorInputParam<double>
{
};

template <> struct Param<::dinrail::VectorProxy<const int>::Ref> : VectorInputParam<int>
{
};

template <class... Ps> struct Signature
{
    static constexpr std::size_t inputIndex(std::size_t n)
    {
        constexpr bool isOutput[] = {Param<Ps>::isOutput..., false};
        std::size_t index = 0;
        for (std::size_t i = 0; i < n; ++i)
        {
            index += isOutput[i] ? 0 : 1;
        }
        return index;
    }

    // Tuple of the Python-side input types.
    using Inputs = decltype(std::tuple_cat(
        std::declval<std::conditional_t<Param<Ps>::isOutput,
                                        std::tuple<>,
                                        std::tuple<typename Param<Ps>::Input>>>()...));
};

template <class F> struct FunctionTraits;

template <class R, class I, class... Ps> struct FunctionTraits<R (I::*)(Ps...)>
{
    using Interface = I;
    using Result = R;
    using Parameters = std::tuple<Ps...>;
};

template <class R, class I, class... Ps>
struct FunctionTraits<R (I::*)(Ps...) const> : FunctionTraits<R (I::*)(Ps...)>
{
};

// The result of the call is the last element, after the outputs.
template <class Result, class Outputs> auto assembleResult(Result&& result, Outputs&& outputs)
{
    if constexpr (std::tuple_size_v<std::decay_t<Outputs>> == 0)
    {
        return std::forward<Result>(result);
    } else
    {
        return std::tuple_cat(std::forward<Outputs>(outputs),
                              std::make_tuple(std::forward<Result>(result)));
    }
}

template <class Outputs> auto assembleVoidResult(Outputs&& outputs)
{
    constexpr std::size_t numberOfOutputs = std::tuple_size_v<std::decay_t<Outputs>>;
    if constexpr (numberOfOutputs == 0)
    {
        return;
    } else if constexpr (numberOfOutputs == 1)
    {
        return std::get<0>(std::forward<Outputs>(outputs));
    } else
    {
        return std::forward<Outputs>(outputs);
    }
}

template <class Interface,
          class Function,
          class Result,
          class SizeFunction,
          class Inputs,
          class Parameters>
struct Wrapper;

template <class Interface, class Function, class Result, class SizeFunction, class... Ins, class... Ps>
struct Wrapper<Interface, Function, Result, SizeFunction, std::tuple<Ins...>, std::tuple<Ps...>>
{
    using Sig = Signature<Ps...>;

    Function function;
    SizeFunction outputSize;

    auto operator()(Interface& self, Ins... ins) const
    {
        std::tuple<Ins...> inputs(std::move(ins)...);

        // The C++ call does not need the GIL, and it may block.
        py::gil_scoped_release release;
        return call(self, inputs, std::index_sequence_for<Ps...>{});
    }

private:
    template <std::size_t N> static auto makeStorage(std::tuple<Ins...>& inputs, std::size_t size)
    {
        using P = std::tuple_element_t<N, std::tuple<Ps...>>;
        if constexpr (Param<P>::isOutput)
        {
            return Param<P>::makeOutput(size);
        } else
        {
            return Param<P>::fromInput(std::move(std::get<Sig::inputIndex(N)>(inputs)));
        }
    }

    template <std::size_t... Ns>
    auto call(Interface& self, std::tuple<Ins...>& inputs, std::index_sequence<Ns...>) const
    {
        const std::size_t size = outputSize(self, std::as_const(inputs));
        std::tuple<typename Param<Ps>::Storage...> storage{makeStorage<Ns>(inputs, size)...};

        if constexpr (std::is_void_v<Result>)
        {
            (self.*function)(Param<Ps>::pass(std::get<Ns>(storage))...);
            return assembleVoidResult(std::tuple_cat(collect<Ps>(std::get<Ns>(storage), true)...));
        } else
        {
            Result result = (self.*function)(Param<Ps>::pass(std::get<Ns>(storage))...);
            const bool success = succeeded(result);
            return assembleResult(std::move(result),
                                  std::tuple_cat(collect<Ps>(std::get<Ns>(storage), success)...));
        }
    }

    template <class P, class Storage> static auto collect(Storage& storage, bool success)
    {
        if constexpr (Param<P>::isOutput)
        {
            return Param<P>::collect(storage, success);
        } else
        {
            return std::tuple<>{};
        }
    }
};

struct NoOutputSize
{
    template <class... Args> std::size_t operator()(Args&&...) const
    {
        return 0;
    }
};

template <class F, class SizeFunction, class... Ps>
auto makeWrapper(F function, SizeFunction outputSize, std::tuple<Ps...>*)
{
    using Traits = FunctionTraits<F>;
    using Inputs = typename Signature<Ps...>::Inputs;
    return Wrapper<typename Traits::Interface,
                   F,
                   typename Traits::Result,
                   SizeFunction,
                   Inputs,
                   std::tuple<Ps...>>{function, outputSize};
}

} // namespace detail

/**
 * @brief Create the Python-callable version of a method of a dinrail interface.
 *
 * Parameters are mapped following the conventions of the dinrail interfaces:
 * - parameters passed by value or by const reference are inputs of the Python method;
 * - non-const reference parameters, pointer parameters and `VectorProxy<double>::Ref` parameters
 *   are outputs. They are returned by the Python method before the value returned by the C++
 *   method, so that the latter can be easily discarded:
 *   `bool getEncoder(int j, double* value)` becomes `getEncoder(j) -> (float | None, bool)`;
 * - an output is `None` if the C++ method returned `false` or a non-ok `Status`, so that
 *   partially filled outputs are never exposed;
 * - input vectors (`VectorProxy<const T>::Ref`) are Python lists.
 *
 * If the C++ method has no output parameters, its return value is returned as it is. If it returns
 * `void`, only the outputs are returned (a single value if there is just one output).
 *
 * @param method Pointer to the method to wrap. Overloaded methods require a `static_cast`.
 * @param outputSize Callable `std::size_t(Interface&, const std::tuple<Inputs...>&)`, that returns
 * the size of the output vectors, if the method has any.
 */
template <class F, class SizeFunction = detail::NoOutputSize>
auto wrapMethod(F method, SizeFunction outputSize = {})
{
    using Parameters = typename detail::FunctionTraits<F>::Parameters;
    return detail::makeWrapper(method, outputSize, static_cast<Parameters*>(nullptr));
}

/**
 * @brief Output size function returning always the same size.
 */
inline auto constantOutputSize(std::size_t size)
{
    return [size](auto&, const auto&) { return size; };
}

} // namespace dinrail::bindings::core

#endif // DINRAIL_BINDINGS_CORE_METHODWRAPPER_H
