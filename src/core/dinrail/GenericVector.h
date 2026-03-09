/**
 * @file GenericContainer.h
 * @authors Stefano Dafarra
 * @copyright 2019 Istituto Italiano di Tecnologia (IIT). This software may be modified and
 * distributed under the terms of the BSD-3-Clause license.
 */
// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_GENERIC_VECTOR_H
#define DINRAIL_GENERIC_VECTOR_H

// std
#include <span>

// Template helpers
#include <dinrail/TemplateHelpers.h>

//std
#include <functional>
#include <cassert>
#include <cstddef>
#include <iterator>
#include <iostream>
#include <memory>

namespace dinrail {

/**
 * Forward declaration of Vector class. T is the type of vector (double, float, int, string,...)
 */
template <typename T>
class GenericVector;

/**
 * Utility alias to a std::shared_ptr of a Vector.
 */
template <typename T>
using GenericVector_ptr = std::shared_ptr<GenericVector<T>>;

}

/**
 * Vector is a utility class which maps another existing contiguous container.
 * It does not contain any data, but only a pointer to an existing contiguous area of memory containing a sequence of objects.
 * It also stores its size, i.e. the number of objects. It does not own this portion of memory, hence it needs to be properly
 * initialized from an existing container, such as an iDynTree::Vector, std::vector, array, yarp::sig::Vector and similar.
 * Even if it does not own the memory, it is possible to resize it. This is done through an user specified lambda, which
 * calls the corresponding "resize" method on the original object from which Vector has been initialized.
 *
 * @warning The original object from which Vector has been initialized should not be deallocated before Vector.
 * This would invalidate the pointer inside it.
 */
template <typename T>
class dinrail::GenericVector
{
public:

    /**
     * Utility aliases depending on the type T.
     *
     * Some notes:
     * - vector_element_type has a different name than the corresponding one in iDynTree::Span to avoid some compilation issues.
     * - value_type is the same as vector_element_type, but without an eventual const attribute
     */
    using vector_element_type = typename std::span<T>::element_type;
    using value_type = typename std::span<T>::value_type;
    using index_type = std::ptrdiff_t;
    using pointer = typename std::span<T>::pointer;
    using reference = typename std::span<T>::reference;
    using const_reference = const value_type&;
    using size_type = typename std::span<T>::size_type;


    /**
     * Utility aliases to define iterators
     */
    using iterator = typename std::span<T>::iterator;
    using const_iterator = iterator;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = reverse_iterator;

    /**
     * Alias for the type of lambda used to resize the original vector.
     * In particular, it takes as input the new size (of type index size)
     */
    using resize_function_type = std::function<std::span<T>(index_type)>;

protected:
    /**
     * @brief Span of the pointed vector. This allows to point to an existing container without owning it.
     */
    std::span<T> m_span;
    /**
     * @brief User specified lambda to resize the existing container.
     */
    resize_function_type m_resizeLambda;

    /**
     * The default constructor is private. In fact, once the Vector is built, it is assumed to point to an existing container.
     * To be used only if m_span and m_resizeLamba are set manually.
     */
    GenericVector() = default;

public:

    /**
     * @brief Constructor
     * @param span Span of the existing container
     * @param resizeLambda User defined lambda to resize the original container
     */
    GenericVector(std::span<T> span, resize_function_type resizeLambda)
    {
        m_span = span;
        m_resizeLambda = resizeLambda;
    }

    /**
     * @brief Constructor
     * @param span Span of the existing container
     *
     * Since no resizeLambda is provided, it is assumed that the original container cannot be resized.
     */
    GenericVector(std::span<T> span)
    {
        m_span = span;
        m_resizeLambda = [span](index_type size){unused(size); return span;};
    }

    /**
     * @brief Destructor
     */
    ~GenericVector() = default;

    /**
     * @brief Copy constructor
     *
     * @warning It has been deleted since it would not be clear if the pointer or the pointed data would be copied.
     */
    GenericVector(const GenericVector<T>& other) = delete;

    /**
     * @brief Move constructor
     * @param other Another Vector
     *
     * @warning Here the pointers are copied, the content is not duplicated.
     */
    GenericVector(GenericVector<T>&& other)
    {
        m_span = other.m_span;
        m_resizeLambda = other.m_resizeLambda;
    }

    /**
     * @brief Copies the content of the vector
     * @param other Vector from which to copy
     * @return true in case of success. False if the two have different size and this is not resizable.
     *
     * @warning It performs memory allocation if this is resizable and the sizes are different.
     */
    bool clone(const GenericVector<T>& other)
    {
        if (size() != other.size())
        {
            if (!resizeVector(other.size()))
            {
                std::cerr << "[dinrail::Vector] Failed to resize. Copy aborted" << std::endl;
                return false;
            }
        }

        for (index_type i = 0; i < size(); ++i)
        {
            this->operator[](i) = other[i];
        }

        return true;
    }

    /**
     * @brief Copies the content of the vector
     * @param other Span from which to copy
     * @return true in case of success. False if the two have different size and this is not resizable.
     *
     * @warning It performs memory allocation if this is resizable and the sizes are different.
     */
    bool clone(std::span<T> other)
    {
        if (size() != other.size())
        {
            if (!resizeVector(other.size()))
            {
                std::cerr << "[dinrail::Vector] Failed to resize. Copy aborted" << std::endl;
                return false;
            }
        }

        for (index_type i = 0; i < size(); ++i)
        {
            this->operator[](i) = other[i];
        }

        return true;
    }

    /**
     * @brief operator = Copies the content
     * @param other Vector from which to copy
     * @returns A reference to the vector.
     *
     * It calls clone(). There is an assert on its return value.
     *
     * @warning It performs memory allocation if this is resizable and the sizes are different.
     */
    GenericVector<T>& operator=(const GenericVector<T>& other)
    {
        bool ok = clone(other);
        assert(ok);
        unused(ok);
        return *this;
    }

    /**
     * @brief operator = Copies the content
     * @param other Vector from which to copy
     * @returns A reference to the vector.
     *
     * It calls clone(). There is an assert on its return value.
     *
     * @warning It performs memory allocation if this is resizable and the sizes are different.
     */
    GenericVector<T>& operator=(std::span<T> other)
    {
        bool ok = clone(other);
        assert(ok);
        unused(ok);
        return *this;
    }

    /**
     * @brief Move operator = Copies the content
     * @param other Vector from which to copy
     * @returns A reference to the vector.
     *
     * It calls clone(). There is an assert on its return value.
     *
     * @warning It performs memory allocation if this is resizable and the sizes are different.
     */
    GenericVector<T>& operator=(GenericVector<T>&& other)
    {
        bool ok = clone(other);
        assert(ok);
        unused(ok);
        return *this;
    }

    /**
     * @brief resizeVector It resize the original vector (if possible)
     * @param newSize New size of the vector
     * @return true if the new size matches the desired. False otherwise.
     *
     * @warning It may perform memory allocation if the new size is different from the current and the resizeLambda has been specified.
     */
    bool resizeVector(index_type newSize)
    {
        m_span = m_resizeLambda(newSize);
        return m_span.size() == newSize;
    }

    /**
     * @brief resize It resize the original vector (if possible)
     * @param newSize New size of the vector
     * It calls resizeVector(). There is an assert on its return value.
     *
     * @warning It may perform memory allocation if the new size is different from the current and the resizeLambda has been specified.
     */
    void resize(index_type newSize)
    {
        bool ok = resizeVector(newSize);
        assert(ok);
        unused(ok);
    }

    /**
     * @brief size Get the size of the Vector
     * @return The size of the Vector
     */
    index_type size() const
    {
        return m_span.size();
    }

    /**
     * @brief empty Checks if the container is empty (zero size)
     * @return True if empty.
     */
    bool empty() const {
        return m_span.empty();
    }

    /**
     * @brief operator [] Accessor
     * @param idx The index to be accessed
     * @return A const reference to the requested element.
     */
    const_reference operator[](index_type idx) const
    {
        return m_span[idx];
    }

    /**
     * @brief operator [] Accessor
     * @param idx The index to be accessed
     * @return A reference to the requested element.
     */
    reference operator[](index_type idx)
    {
        return m_span[idx];
    }

    /**
     * @brief getVal Accessor
     * @param idx The index to be accessed
     * @return A const reference to the requested element.
     */
    vector_element_type getVal(index_type idx) const
    {
        return this->operator[](idx);
    }

    /**
     * @brief setVal Setter
     * @param idx Index of the values to be set
     * @param val The new value
     * @return False if idx is not within [0, size()).
     */
    bool setVal(index_type idx, vector_element_type val)
    {
        if (idx >= 0 && idx < size())
        {
            this->operator[](idx) = val;
            return true;
        }
        return false;
    }

    /**
     * @brief at Accessor
     * @param idx The index to be accessed
     * @return A const reference to the requested element.
     */
    const_reference at(index_type idx) const
    {
        return this->operator[](idx);
    }

    /**
     * @brief at Accessor
     * @param idx The index to be accessed
     * @return A reference to the requested element.
     */
    reference at(index_type idx)
    {
        return this->operator[](idx);
    }

    /**
     * @brief operator () Accessor
     * @param idx The index to be accessed
     * @return A const reference to the requested element.
     */
    const_reference operator()(index_type idx) const
    {
        return this->operator[](idx);
    }

    /**
     * @brief operator () Accessor
     * @param idx The index to be accessed
     * @return A reference to the requested element.
     */
    reference operator()(index_type idx)
    {
        return this->operator[](idx);
    }

    /**
     * @brief data Raw pointer to the data
     * @return THe raw pointer to the data
     */
    pointer data() const
    {
        return m_span.data();
    }

    /**
     * @brief begin Iterator
     * @return An iterator to the end of the sequence
     */
    iterator begin()
    {
        return std::begin(m_span);
    }

    /**
     * @brief end Iterator
     * @return An iterator to the end of the sequence
     * @warning This element acts as a placeholder; attempting to access it results in undefined behavior.
     */
    iterator end()
    {
        return std::end(m_span);
    }

    /**
     * @brief begin Iterator
     * @return A const iterator to the end of the sequence
     */
    const_iterator begin() const
    {
        return std::begin(m_span);
    }

    /**
     * @brief end Iterator
     * @return A const iterator to the end of the sequence
     * @warning This element acts as a placeholder; attempting to access it results in undefined behavior.
     */
    const_iterator end() const
    {
        return std::end(m_span);
    }

    /**
     * @brief cbegin Iterator
     * @return A const iterator to the end of the sequence
     */
    const_iterator cbegin() const
    {
        return std::begin(m_span);
    }

    /**
     * @brief cend Iterator
     * @return A const iterator to the end of the sequence
     * @warning This element acts as a placeholder; attempting to access it results in undefined behavior.
     */
    const_iterator cend() const
    {
        return std::end(m_span);
    }

    /**
     * @brief rbegin Iterator to the first element of the reversed vector
     * @return Returns a reverse iterator to the first element of the reversed vector.
     * It corresponds to the last element of the non-reversed vector
     */
    reverse_iterator rbegin()
    {
        return std::rbegin(m_span);
    }

    /**
     * @brief rend Iterator to the element following the last element of the reversed vector.
     * @return Returns a reverse iterator to the element following the last element of the reversed vector.
     *  It corresponds to the element preceding the first element of the non-reversed vector.
     * @warning This element acts as a placeholder; attempting to access it results in undefined behavior.
     */
    reverse_iterator rend()
    {
        return std::rend(m_span);
    }

    /**
     * @brief rbegin Iterator to the first element of the reversed vector
     * @return Returns a const reverse iterator to the first element of the reversed vector.
     * It corresponds to the last element of the non-reversed vector
     */
    const_reverse_iterator rbegin() const
    {
        return std::rbegin(m_span);
    }

    /**
     * @brief rend Iterator to the element following the last element of the reversed vector.
     * @return Returns a reverse iterator to the element following the last element of the reversed vector.
     *  It corresponds to the element preceding the first element of the non-reversed vector.
     * @warning This element acts as a placeholder; attempting to access it results in undefined behavior.
     */
    const_reverse_iterator rend() const
    {
        return std::rend(m_span);
    }

    /**
     * @brief crbegin Iterator to the first element of the reversed vector
     * @return Returns a const reverse iterator to the first element of the reversed vector.
     * It corresponds to the last element of the non-reversed vector
     */
    const_reverse_iterator crbegin() const
    {
        return std::rbegin(m_span);
    }

    /**
     * @brief crend Iterator to the element following the last element of the reversed vector.
     * @return Returns a reverse iterator to the element following the last element of the reversed vector.
     *  It corresponds to the element preceding the first element of the non-reversed vector.
     * @warning This element acts as a placeholder; attempting to access it results in undefined behavior.
     */
    const_reverse_iterator crend() const
    {
        return std::rend(m_span);
    }

    /**
     * Forward declaration of Ref, which is used as a reference to the Vector (as &).
     */
    class Ref;

};

namespace dinrail {

/**
 * is_vector is a utility metafunction used to check if T is a dinrail::Vector.
 */
template <typename T>
struct is_vector : std::false_type
{
};

/**
 * is_vector is a utility metafunction used to check if T is a dinrail::Vector.
 */
template <typename T>
struct is_vector<GenericVector<T>> : std::true_type
{
};

/**
 * is_span_constructible is a utility metafunction to check if iDynTree::Span is constructible given a reference to Class.
 */
template <typename Class, typename = void>
struct is_span_constructible : std::false_type
{};

/**
 * is_span_constructible is a utility metafunction to check if iDynTree::Span is constructible given a reference to Class.
 */
template <typename Class>
struct is_span_constructible<Class,
                             typename std::enable_if<
                                 std::is_constructible<std::span<typename container_data<Class>::type>, Class&>::value>::type>
    : std::true_type
{};

/**
 * is_vector_constructible is a utility metafunction to check if dinrail::Vector is constructible given a type T.
 */
template <typename T, typename = void, typename = void>
struct is_vector_constructible : std::false_type
{
};

/**
 * is_vector_constructible is a utility metafunction to check if dinrail::Vector is constructible given a type T.
 * This specialization first checks if T is an array, or if it is possible to deduce the type of vector.
 * If not, this specialization is not used (SFINAE). Otherwise, given the vector type, it checks if a iDynTree::Span is construbile
 * or if the methods <code>data()<\code> and <code>size()<\code> are available. In addition, the type has not to be <code>bool<\code>.
 * If all the above checks are true, <code>is_vector_constructible<T>::value = true<\code>.
 */
template <typename T>
    struct is_vector_constructible<T,
                               typename std::enable_if<(std::is_array<T>::value || has_type_member<T>::value || is_data_available<T>::value)>::type,
    typename std::enable_if<(is_span_constructible<T>::value ||(is_data_available<T>::value && is_size_available<T>::value)) &&
                             !std::is_same<typename container_data<T>::type, bool>::value>::type> : std::true_type
{
};

/**
 * is_vector_ref_constructible is a utility metafunction to check if dinrail::Vector::Ref is constructible given a type T and a Container.
 */
template <typename T, typename Container, typename = void, typename = void>
struct is_vector_ref_constructible : std::false_type
{
};

/**
 * is_vector_constructible is a utility metafunction to check if dinrail::Vector::Ref is constructible given a type T and a Container.
 * This specialization first checks if a dinrail::Vector can be constructed given the Container.
 * If not, this specialization is not used (SFINAE). Otherwise, it checks that the data type of the Container is the same as T.
 * This is useful to avoid ambiguity in interfaces with overloaded methods using different types of Ref.
 * If all the above checks are true, <code>is_vector_ref_constructible<T>::value = true<\code>.
 */
template <typename T, typename Container>
struct is_vector_ref_constructible<T,
                                   Container,
                                   typename std::enable_if<dinrail::is_vector_constructible<Container>::value>::type,
                                   typename std::enable_if<std::is_same<std::remove_cv_t<T>, typename std::remove_cv_t<typename container_data<Container>::type>>::value>::type> : std::true_type
{
};

/**
 * @brief The VectorResizeMode enum
 * It defines if the vector can be resized or not.
 */
enum class VectorResizeMode
{
    Resizable,
    Fixed
};

/**
 * @brief Utility function with return a suitable <code>resize_function_type<\code> given the input class.
 * @param input Reference to vector from which the resize lambda is created
 * @returns It returns a lambda calling the <code>resize()<\code> method of the input class.
 *
 * \warning Do not deallocate input if the lambda returned by this method is still used, to avoid segfault.
 * In fact, the output lambda contains a pointer to input.
 */
template<typename Class>
typename GenericVector<typename container_data<Class>::type>::resize_function_type DefaultVectorResizer(Class& input)
{
    static_assert (is_resizable<Class>::value, "Class type is not resizable.");
    static_assert (is_span_constructible<Class>::value || (is_data_available<Class>::value && is_size_available<Class>::value),
                  "Cannot create a span given the provided class.");

    using value_type = typename container_data<Class>::type;

    if constexpr (is_span_constructible<Class>::value)
    {
        using index_type = typename GenericVector<value_type>::index_type;
        using resize_function = typename GenericVector<value_type>::resize_function_type;

        Class* inputPtr = &input;
        resize_function resizeLambda =
            [inputPtr](index_type newSize) -> std::span<value_type>
        {
            inputPtr->resize(newSize);
            return std::span(*inputPtr);
        };

        return resizeLambda;
    }
    else
    {
        using index_type = decltype(std::declval<Class>().size());
        using resize_function = typename GenericVector<value_type>::resize_function_type;

        Class* inputPtr = &input;
        resize_function resizeLambda =
            [inputPtr](index_type newSize) -> std::span<value_type>
        {
            inputPtr->resize(newSize);
            return std::span(inputPtr->data(), inputPtr->size());
        };

        return resizeLambda;
    }
}

/**
 * @brief Utility function to create a dinrail::Vector from a reference to another vector.
 * @param input The refence to an existing vector.
 * @param mode The resize mode. By default the output Vector is <code>Fixed<\code>.
 * @returns A dinrail::Vector pointing to the input vector.
 *
 * @warning The input object from which dinrail::Vector has been initialized should not be deallocated before it.
 * This would invalidate the pointer inside dinrail::Vector.
 */
template<typename Class>
GenericVector<typename container_data<Class>::type>
make_vector(Class& input, VectorResizeMode mode = VectorResizeMode::Fixed)
{
    static_assert (!std::is_same<typename container_data<Class>::type, bool>::value,
                  "Cannot create a Vector of bool type. Memory is not contiguos." );
    static_assert (is_span_constructible<Class>::value || (is_data_available<Class>::value && is_size_available<Class>::value),
                  "Cannot create a span given the provided class.");

    using value_type = typename container_data<Class>::type;

    std::span<value_type> span;

    if constexpr (is_span_constructible<Class>::value)
    {
        span = std::span(input);
    }
    else
    {
        span = std::span(input.data(), input.size());
    }

    if constexpr (is_resizable<Class>::value)
    {
        if (mode == VectorResizeMode::Resizable)
        {
            return GenericVector(span, DefaultVectorResizer(input));
        }
        else
        {
            return GenericVector(span);
        }
    }
    else
    {
        if (mode == VectorResizeMode::Resizable)
        {
            std::cerr << "[dinrail::Vector] " << type_name<Class>()
                      << " is not resizable. Returning a non-resizable container." << std::endl;
        }

        return GenericVector(span);
    }
}

/**
 * @brief Utility function to create a dinrail::Vector from a reference to another vector.
 * @param input The refence to an existing vector.
 * @param mode The resize mode. By default the output Vector is <code>Fixed<\code>.
 * @returns A dinrail::Vector pointing to the input vector.
 *
 * @warning This version is called if the input type is const. Hence, it cannot be resized.
 * It throws a warning if the selected mode is <code>VectorResizeMode::Resizable<\code>.
 *
 * @warning The input object from which dinrail::Vector has been initialized should not be deallocated before it.
 * This would invalidate the pointer inside dinrail::Vector.
 */
template <typename Class>
GenericVector<const typename container_data<Class>::type>
make_vector(const Class& input, VectorResizeMode mode = VectorResizeMode::Fixed)
{
    static_assert (!std::is_same<typename container_data<Class>::type, bool>::value,
                  "Cannot create a Vector of bool type. Memory is not contiguos." );
    static_assert(is_span_constructible<Class>::value
                      || (is_data_available<Class>::value && is_size_available<Class>::value),
                  "Cannot create a span given the provided class.");

    using value_type = typename container_data<decltype(input)>::type;

    std::span<value_type> span;

    if constexpr (is_span_constructible<Class>::value)
    {
        span = std::span(input);
    } else
    {
        span = std::span(input.data(), input.size());
    }

    if (mode == VectorResizeMode::Resizable)
    {
        std::cerr << "[dinrail::Vector] The input type is const. Returning a non-resizable container." << std::endl;
    }

    return GenericVector(span);
}

/**
 * @brief Utility function to create a dinrail::GenericVector_ptr from a reference to another vector.
 * @param input The refence to an existing vector.
 * @param mode The resize mode. By default the output Vector is <code>Fixed<\code>.
 * @returns A dinrail::GenericVector_ptr.
 *
 * @warning The input object from which the underlying dinrail::Vector has been initialized should not be deallocated before it.
 * This would invalidate the pointer inside dinrail::Vector.
 */
template<typename Class>
GenericVector_ptr<typename container_data<Class>::type>
make_vector_ptr(Class& input, VectorResizeMode mode = VectorResizeMode::Fixed)
{
    return std::make_shared<GenericVector<typename container_data<Class>::type>>(make_vector(input,mode));
}

/**
 * @brief Utility function to create a dinrail::GenericVector_ptr from a reference to another vector.
 * @param input The refence to an existing vector.
 * @param mode The resize mode. By default the output Vector is <code>Fixed<\code>.
 * @returns A dinrail::GenericVector_ptr.
 *
 * @warning This version is called if the input type is const. Hence, it cannot be resized.
 * It throws a warning if the selected mode is <code>VectorResizeMode::Resizable<\code>.
 *
 * @warning The input object from which the underlying dinrail::Vector has been initialized should not be deallocated before it.
 * This would invalidate the pointer inside dinrail::Vector.
 */
template<typename Class>
GenericVector_ptr<const typename container_data<Class>::type>
make_vector_ptr(const Class& input, VectorResizeMode mode = VectorResizeMode::Fixed)
{
    return std::make_shared<GenericVector<const typename container_data<Class>::type>>(make_vector(input,mode));
}

/**
 * @brief Utility function to create a dinrail::GenericVector_ptr from a reference to another vector.
 * @param span Span of the existing container
 * @returns A dinrail::GenericVector_ptr.
 *
 * Since no resizeLambda is provided, it is assumed that the original container cannot be resized.
 *
 * @warning The input object from which the underlying dinrail::Vector has been initialized should not be deallocated before it.
 * This would invalidate the pointer inside dinrail::Vector.
 */
template<typename T>
GenericVector_ptr<T>
make_vector_ptr(std::span<T> span)
{
    return std::make_shared<GenericVector<T>>(span);
}

/**
 * @brief Utility function to create a dinrail::GenericVector_ptr from a reference to another vector.
 * @param span Span of the existing container
 * @param resizeLambda User defined lambda to resize the original container
 *
 * @warning The input object from which the underlying dinrail::Vector has been initialized should not be deallocated before it.
 * This would invalidate the pointer inside dinrail::Vector.
 */
template<typename T>
GenericVector_ptr<T>
make_vector_ptr(std::span<T> span, typename GenericVector<T>::resize_function_type resizeLambda)
{
    return std::make_shared<GenericVector<T>>(span, resizeLambda);
}

}

/**
 * The class ref is used a substitution to a classical reference to a Vector. The advantage of using this,
 * is that custom vectors (all those supported by dinrail::Vector) can be implicitly casted to Ref.
 * Ref does not allocate any memory in construction, hence can be used as a parameter to be passed by copy.
 * The = operator clones the content.
 * Ref inherits GenericVector<T>, hence it can be used as it was a GenericVector<T>.
 */
template <typename T>
class dinrail::GenericVector<T>::Ref : public dinrail::GenericVector<T>
{
public:

    /**
     * @brief A reference cannot exist on its own.
     */
    Ref() = delete;

    /**
     * @brief Copy constructor
     * @param other The ref from which to copy the context.
     */
    Ref(dinrail::GenericVector<T>::Ref& other)
    {
        m_span = other.m_span;
        m_resizeLambda = other.m_resizeLambda;
    }

    /**
     * @brief Move constructor
     * @param other The ref from which to get the context.
     */
    Ref(dinrail::GenericVector<T>::Ref&& other)
    {
        m_span = other.m_span;
        m_resizeLambda = other.m_resizeLambda;
    }

    /**
     * @brief Constructor from a dinrail::GenericVector<T>&
     * @param other The input vector from which the context is copied
     */
    Ref(dinrail::GenericVector<T>& other)
    {
        m_span = other.m_span;
        m_resizeLambda = other.m_resizeLambda;
    }

    /**
     * @brief Constructor from a dinrail::GenericVector<T>&
     * @param other The input vector from which the context is taken
     * In principle, Ref should be the reference of a GenericVector<T> which should remain alive while
     * Ref is alive. On the other hand, GenericVector<T> is only a pointer to some data which does not own.
     * Hence, Ref can remain alive even if the GenericVector<T> is deleted, provided that the original container
     * stays alive.
     */
    Ref(dinrail::GenericVector<T>&& other)
    {
        m_span = other.m_span;
        m_resizeLambda = other.m_resizeLambda;
    }

    /**
     * Constructor from another container.
     * This is used if:
     * - the input container is not a dinrail::Vector, to avoid ambiguities with other constructors
     * - the input container is not a string. This allows using Ref and string with overloaded methods.
     * - a dinrail::GenericVector<T>::Ref can be constructed from the Container
     * - T is not const
     * - the input container is not const.
     */
    template <class Vector, typename = typename std::enable_if<!dinrail::is_vector<Vector>::value &&
                                                               !std::is_same<Vector, std::string>::value &&
                                                               dinrail::is_vector_ref_constructible<T,Vector>::value &&
                                                               !std::is_const_v<T> &&
                                                               !is_container_const<Vector>::value>::type>
    Ref(Vector& input)
    {
        if constexpr (is_span_constructible<Vector>::value)
        {
            m_span = std::span(input);
        } else
        {
            m_span = std::span(input.data(), input.size());
        }

        if constexpr (dinrail::is_resizable<Vector>::value)
        {
            m_resizeLambda = DefaultVectorResizer(input);
        }
        else
        {
            std::span<T> copiedSpan = m_span;
            m_resizeLambda = [copiedSpan](index_type size){unused(size); return copiedSpan;};
        }

    }

    /**
     * Constructor from another container.
     * This is used if:
     * - the input container is not a dinrail::Vector, to avoid ambiguities with other constructors
     * - the input container is not a string. This allows using Ref and string with overloaded methods.
     * - a dinrail::GenericVector<T>::Ref can be constructed from the Container
     * - T is const.
     */
    template <class Vector, typename = typename std::enable_if<!dinrail::is_vector<Vector>::value &&
                                                               !std::is_same<Vector, std::string>::value &&
                                                               dinrail::is_vector_ref_constructible<T,Vector>::value &&
                                                               std::is_const_v<T>>::type>
    Ref(const Vector& input)
    {
        if constexpr (is_span_constructible<Vector>::value)
        {
            m_span = std::span(input);
        } else
        {
            m_span = std::span(input.data(), input.size());
        }

        std::span<T> copiedSpan = m_span;
        m_resizeLambda = [copiedSpan](index_type size){unused(size); return copiedSpan;};

    }

    /**
    * Default constructor
    */
   ~Ref() = default;

   /**
    * @brief Copy operator
    * @param other The other Ref, from which the data is copied
    * @return A reference to the vector resulting from the copy.
    */
   Ref operator=(const Ref& other)
   {
       return static_cast<dinrail::GenericVector<T>&>(*this) = other;
   }

   /**
    * @brief Copy operator
    * @param other The other Ref, from which the data is copied
    * @return A reference to the vector resulting from the copy.
    */
   Ref operator=(Ref&& other)
   {
       return static_cast<dinrail::GenericVector<T>&>(*this) = other;
   }

};


#endif // DINRAIL_GENERIC_VECTOR_H
