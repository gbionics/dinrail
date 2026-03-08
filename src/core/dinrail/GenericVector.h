// SPDX-FileCopyrightText: Generative Bionics S.R.L.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef DINRAIL_GENERICVECTOR_H
#define DINRAIL_GENERICVECTOR_H

#include <cassert>
#include <cstddef>
#include <functional>
#include <iterator>
#include <span>
#include <type_traits>
#include <utility>

namespace dinrail
{

template <typename T> class GenericVector
{
public:
    using vector_element_type = T;
    using value_type = std::remove_cv_t<T>;
    using index_type = std::size_t;
    using pointer = typename std::span<T>::pointer;
    using reference = typename std::span<T>::reference;
    using const_reference = const value_type&;
    using size_type = typename std::span<T>::size_type;

    using iterator = typename std::span<T>::iterator;
    using const_iterator = iterator;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = reverse_iterator;

    using resize_function_type = std::function<std::span<T>(index_type)>;

    class Ref;

protected:
    std::span<T> m_span;
    resize_function_type m_resizeLambda;

    GenericVector() = default;

public:
    explicit GenericVector(std::span<T> span)
        : m_span(span)
        , m_resizeLambda([span](index_type) { return span; })
    {
    }

    GenericVector(std::span<T> span, resize_function_type resizeLambda)
        : m_span(span)
        , m_resizeLambda(std::move(resizeLambda))
    {
    }

    bool clone(const GenericVector<T>& other)
    {
        if constexpr (std::is_const_v<T>)
        {
            return false;
        } else
        {
            if (size() != other.size() && !resizeVector(other.size()))
            {
                return false;
            }

            for (index_type i = 0; i < size(); ++i)
            {
                (*this)[i] = other[i];
            }
            return true;
        }
    }

    bool clone(std::span<const value_type> other)
    {
        if constexpr (std::is_const_v<T>)
        {
            return false;
        } else
        {
            if (size() != other.size() && !resizeVector(other.size()))
            {
                return false;
            }

            for (index_type i = 0; i < size(); ++i)
            {
                (*this)[i] = other[i];
            }
            return true;
        }
    }

    bool resizeVector(index_type newSize)
    {
        m_span = m_resizeLambda(newSize);
        return m_span.size() == newSize;
    }

    void resize(index_type newSize)
    {
        const bool ok = resizeVector(newSize);
        assert(ok);
        static_cast<void>(ok);
    }

    index_type size() const
    {
        return m_span.size();
    }

    bool empty() const
    {
        return m_span.empty();
    }

    const_reference operator[](index_type idx) const
    {
        return m_span[idx];
    }

    reference operator[](index_type idx)
    {
        return m_span[idx];
    }

    const_reference at(index_type idx) const
    {
        return (*this)[idx];
    }

    reference at(index_type idx)
    {
        return (*this)[idx];
    }

    pointer data() const
    {
        return m_span.data();
    }

    iterator begin()
    {
        return m_span.begin();
    }

    iterator end()
    {
        return m_span.end();
    }

    const_iterator begin() const
    {
        return m_span.begin();
    }

    const_iterator end() const
    {
        return m_span.end();
    }

    const_iterator cbegin() const
    {
        return m_span.cbegin();
    }

    const_iterator cend() const
    {
        return m_span.cend();
    }

    reverse_iterator rbegin()
    {
        return reverse_iterator(end());
    }

    reverse_iterator rend()
    {
        return reverse_iterator(begin());
    }

    const_reverse_iterator rbegin() const
    {
        return const_reverse_iterator(end());
    }

    const_reverse_iterator rend() const
    {
        return const_reverse_iterator(begin());
    }

    const_reverse_iterator crbegin() const
    {
        return const_reverse_iterator(cend());
    }

    const_reverse_iterator crend() const
    {
        return const_reverse_iterator(cbegin());
    }
};

template <typename T> class GenericVector<T>::Ref : public GenericVector<T>
{
private:
    using Base = GenericVector<T>;

public:
    Ref() = delete;

    Ref(const Ref& other)
    {
        this->m_span = other.m_span;
        this->m_resizeLambda = other.m_resizeLambda;
    }

    Ref(Ref&& other)
    {
        this->m_span = other.m_span;
        this->m_resizeLambda = std::move(other.m_resizeLambda);
    }

    explicit Ref(Base& other)
    {
        this->m_span = other.m_span;
        this->m_resizeLambda = other.m_resizeLambda;
    }

    explicit Ref(Base&& other)
    {
        this->m_span = other.m_span;
        this->m_resizeLambda = std::move(other.m_resizeLambda);
    }

    template <typename Container>
        requires(!std::is_const_v<T>)
                && requires(Container& c) { std::span<typename Base::value_type>(c); }
    explicit Ref(Container& input)
    {
        this->m_span = std::span<T>(input);

        if constexpr (requires(Container& c, std::size_t n) { c.resize(n); })
        {
            Container* inputPtr = &input;
            this->m_resizeLambda = [inputPtr](typename Base::index_type newSize) {
                inputPtr->resize(newSize);
                return std::span<T>(*inputPtr);
            };
        } else
        {
            const std::span<T> copiedSpan = this->m_span;
            this->m_resizeLambda = [copiedSpan](typename Base::index_type) { return copiedSpan; };
        }
    }

    template <typename Container>
        requires std::is_const_v<T>
                 && requires(const Container& c) { std::span<const typename Base::value_type>(c); }
    explicit Ref(const Container& input)
    {
        this->m_span = std::span<const typename Base::value_type>(input);
        const std::span<T> copiedSpan = this->m_span;
        this->m_resizeLambda = [copiedSpan](typename Base::index_type) { return copiedSpan; };
    }

    Ref& operator=(const Ref& other)
    {
        this->m_span = other.m_span;
        this->m_resizeLambda = other.m_resizeLambda;
        return *this;
    }

    Ref& operator=(Ref&& other)
    {
        this->m_span = other.m_span;
        this->m_resizeLambda = std::move(other.m_resizeLambda);
        return *this;
    }
};

} // namespace dinrail

#endif // DINRAIL_GENERICVECTOR_H
