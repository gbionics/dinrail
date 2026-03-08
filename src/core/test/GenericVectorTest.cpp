// Catch2
#include <catch2/catch_test_macros.hpp>

#include <array>
#include <chrono>
#include <memory>
#include <span>
#include <string>
#include <type_traits>
#include <vector>

#include <Eigen/Core>

#include <dinrail/GenericVector.h>

using namespace dinrail;

void foo(GenericVector<double>::Ref test)
{
    static_cast<void>(test.data());
}

void fooConst(const GenericVector<const double>::Ref test)
{
    static_cast<void>(test.data());
}

TEST_CASE("GenericVector")
{
    SECTION("Constructible")
    {
        REQUIRE((std::is_constructible_v<GenericVector<double>::Ref, std::vector<double>&>));
        REQUIRE((
            std::is_constructible_v<GenericVector<const double>::Ref, const std::vector<double>&>));
        REQUIRE((std::is_constructible_v<GenericVector<int>::Ref, std::vector<int>&>));
        REQUIRE((std::is_constructible_v<GenericVector<const int>::Ref, const std::vector<int>&>));
        REQUIRE(
            (std::is_constructible_v<GenericVector<std::string>::Ref, std::vector<std::string>&>));
        REQUIRE((std::is_constructible_v<GenericVector<double>::Ref, std::array<double, 5>&>));
        REQUIRE((std::is_constructible_v<GenericVector<const double>::Ref,
                                         const std::array<double, 5>&>));
        REQUIRE((std::is_constructible_v<GenericVector<char>::Ref, std::string&>));
        REQUIRE_FALSE((std::is_constructible_v<GenericVector<bool>::Ref, std::vector<bool>&>));
        REQUIRE_FALSE((std::is_constructible_v<GenericVector<double>::Ref, double&>));
        REQUIRE_FALSE((std::is_constructible_v<GenericVector<int>::Ref, int&>));
    }

    SECTION("Copy")
    {
        std::vector<double> vector{0.2, 0.4, 0.6, 0.8, 1.0};
        GenericVector<double>::Ref container(vector);

        std::vector<double> copiedIn(5, 0.0);
        GenericVector<double>::Ref containerToBeCopied(copiedIn);

        REQUIRE(containerToBeCopied.clone(container));

        for (std::size_t i = 0; i < container.size(); ++i)
        {
            REQUIRE(vector[i] == copiedIn[i]);
        }

        vector = {2.0, 4.0, 6.0, 8.0, 10.0};
        REQUIRE(containerToBeCopied.clone(std::span<const double>(vector)));

        for (std::size_t i = 0; i < container.size(); ++i)
        {
            REQUIRE(vector[i] == copiedIn[i]);
        }
    }

    SECTION("Impossible to resize")
    {
        std::array<double, 5> vector{{1.0, 2.0, 3.0, 4.0, 5.0}};
        GenericVector<double>::Ref container(vector);
        REQUIRE_FALSE(container.resizeVector(2));

        std::array<int, 3> fixedVector{{1, 2, 3}};
        GenericVector<int>::Ref container2(fixedVector);
        REQUIRE_FALSE(container2.resizeVector(2));
    }

    SECTION("Resize")
    {
        std::vector<double> vector;

        GenericVector<double>::Ref container(vector);
        REQUIRE(container.resizeVector(5));
        REQUIRE(vector.size() == 5);
    }

    SECTION("Resize and copy")
    {
        std::vector<double> vector{1, 2, 3, 4, 5};
        GenericVector<double>::Ref container(vector);

        std::vector<double> copiedIn;
        GenericVector<double>::Ref containerToBeCopied(copiedIn);

        REQUIRE(containerToBeCopied.clone(container));

        for (std::size_t i = 0; i < container.size(); ++i)
        {
            REQUIRE(vector[i] == copiedIn[i]);
        }
    }

    SECTION("Set/get value")
    {
        std::vector<double> vector{0.0};
        GenericVector<double>::Ref container(vector);
        container[0] = 1.0;

        REQUIRE(vector[0] == 1.0);
        REQUIRE(container[0] == 1.0);
    }

    SECTION("Create const")
    {
        const std::vector<double> constVector{1, 2, 3, 4, 5};
        GenericVector<const double>::Ref container(constVector);
        REQUIRE(container.size() == constVector.size());
    }

    SECTION("Create from self")
    {
        std::vector<double> vector{1, 2, 3, 4, 5};
        GenericVector<double>::Ref container(vector);

        GenericVector<double>::Ref inception(container);
        REQUIRE(inception.size() == container.size());
    }

    SECTION("Create from self custom resize")
    {
        std::vector<double> vector{1, 2, 3, 4, 5};
        GenericVector<double>::Ref container(vector);

        using resize_function = GenericVector<double>::resize_function_type;
        using index_type = GenericVector<double>::index_type;

        GenericVector<double>* inputPtr = &container;
        resize_function resizeLambda = [inputPtr](index_type newSize) -> std::span<double> {
            inputPtr->resizeVector(newSize);
            return std::span<double>(inputPtr->data(), inputPtr->size());
        };

        GenericVector<double> inception(std::span<double>(container.data(), container.size()),
                                        resizeLambda);
        REQUIRE(inception.resizeVector(6));
        REQUIRE(vector.size() == 6);
    }

    SECTION("String")
    {
        std::string test = "Test";
        GenericVector<char>::Ref container(test);

        container.resize(3);

        REQUIRE(test == "Tes");
    }

    SECTION("Create pointer")
    {
        std::vector<double> vector{1, 2, 3, 4, 5};
        auto container_ptr = std::make_shared<GenericVector<double>::Ref>(vector);
        REQUIRE(container_ptr);

        const std::vector<double> constVector{1, 2, 3, 4, 5};
        auto const_container_ptr = std::make_shared<GenericVector<const double>::Ref>(constVector);
        REQUIRE(const_container_ptr);

        std::vector<double> copiedIn(5, 0.0);
        container_ptr = std::make_shared<GenericVector<double>::Ref>(copiedIn);
        REQUIRE(container_ptr);

        GenericVector<double>::Ref container(vector);

        using resize_function = GenericVector<double>::resize_function_type;
        using index_type = GenericVector<double>::index_type;

        GenericVector<double>* inputPtr = &container;
        resize_function resizeLambda = [inputPtr](index_type newSize) -> std::span<double> {
            inputPtr->resizeVector(newSize);
            return std::span<double>(inputPtr->data(), inputPtr->size());
        };

        auto mappedContainerPtr
            = std::make_shared<GenericVector<double>>(std::span<double>(container.data(),
                                                                        container.size()),
                                                      resizeLambda);
        REQUIRE(mappedContainerPtr->resizeVector(6));
        REQUIRE(vector.size() == 6);
    }

    SECTION("at_call")
    {
        int arr[4] = {1, 2, 3, 4};

        {
            GenericVector<int> s{std::span<int>(arr)};
            REQUIRE(s.at(0) == 1);
        }

        {
            int arr2d[2] = {1, 6};
            GenericVector<int> s{std::span<int>(arr2d)};
            REQUIRE(s.at(0) == 1);
            REQUIRE(s.at(1) == 6);
        }
    }

    SECTION("operator_function_call")
    {
        int arr[4] = {1, 2, 3, 4};

        {
            GenericVector<int> s{std::span<int>(arr)};
            REQUIRE(s[0] == 1);
        }

        {
            int arr2d[2] = {1, 6};
            GenericVector<int> s{std::span<int>(arr2d)};
            REQUIRE(s[0] == 1);
            REQUIRE(s[1] == 6);
        }
    }

    SECTION("iterator_default_init")
    {
        GenericVector<int>::iterator it1{};
        GenericVector<int>::iterator it2{};
        bool ok = (it1 == it2);
        REQUIRE(ok);
    }

    SECTION("const_iterator_default_init")
    {
        GenericVector<int>::const_iterator it1{};
        GenericVector<int>::const_iterator it2{};
        bool ok = (it1 == it2);
        REQUIRE(ok);
    }

    SECTION("iterator_conversions")
    {
        bool ok;

        GenericVector<int>::iterator badIt{};
        GenericVector<int>::const_iterator badConstIt{};
        ok = (badIt == badConstIt);
        REQUIRE(ok);

        int a[] = {1, 2, 3, 4};
        GenericVector<int> s{std::span<int>(a)};
        const GenericVector<int>& cs = s;

        auto it = s.begin();
        auto cit = cs.begin();

        ok = (it == cit);
        REQUIRE(ok);
        ok = (cit == it);
        REQUIRE(ok);

        GenericVector<int>::const_iterator cit2 = it;
        ok = (cit2 == cit);
        REQUIRE(ok);

        GenericVector<int>::const_iterator cit3 = it + 4;
        ok = (cit3 == cs.end());
        REQUIRE(ok);
    }

    SECTION("iterator_comparisons")
    {
        int a[] = {1, 2, 3, 4};
        {
            GenericVector<int> s{std::span<int>(a)};
            const GenericVector<int>& cs = s;
            GenericVector<int>::iterator it = s.begin();
            auto it2 = it + 1;
            GenericVector<int>::const_iterator cit = cs.begin();
            bool ok;

            ok = (it == cit);
            REQUIRE(ok);
            ok = (cit == it);
            REQUIRE(ok);
            ok = (it == it);
            REQUIRE(ok);
            ok = (cit == cit);
            REQUIRE(ok);
            ok = (cit == s.begin());
            REQUIRE(ok);
            ok = (s.begin() == cit);
            REQUIRE(ok);
            ok = (it == s.begin());
            REQUIRE(ok);
            ok = (s.begin() == it);
            REQUIRE(ok);

            ok = (it != it2);
            REQUIRE(ok);
            ok = (it2 != it);
            REQUIRE(ok);
            ok = (it != s.end());
            REQUIRE(ok);
            ok = (it2 != s.end());
            REQUIRE(ok);
            ok = (s.end() != it);
            REQUIRE(ok);
            ok = (it2 != cit);
            REQUIRE(ok);
            ok = (cit != it2);
            REQUIRE(ok);

            ok = (it < it2);
            REQUIRE(ok);
            ok = (it <= it2);
            REQUIRE(ok);
            ok = (it2 <= s.end());
            REQUIRE(ok);
            ok = (it < s.end());
            REQUIRE(ok);
            ok = (it <= cit);
            REQUIRE(ok);
            ok = (cit <= it);
            REQUIRE(ok);
            ok = (cit < it2);
            REQUIRE(ok);
            ok = (cit <= it2);
            REQUIRE(ok);
            ok = (cit < s.end());
            REQUIRE(ok);
            ok = (cit <= s.end());
            REQUIRE(ok);

            ok = (it2 > it);
            REQUIRE(ok);
            ok = (it2 >= it);
            REQUIRE(ok);
            ok = (s.end() > it2);
            REQUIRE(ok);
            ok = (s.end() >= it2);
            REQUIRE(ok);
            ok = (it2 > cit);
            REQUIRE(ok);
            ok = (it2 >= cit);
            REQUIRE(ok);
        }
    }

    SECTION("begin_end")
    {
        {
            int a[] = {1, 2, 3, 4};
            GenericVector<int> s{std::span<int>(a)};

            GenericVector<int>::iterator it = s.begin();
            GenericVector<int>::iterator it2 = std::begin(s);
            bool ok;
            ok = (it == it2);
            REQUIRE(ok);

            it = s.end();
            it2 = std::end(s);
            ok = (it == it2);
            REQUIRE(ok);
        }

        {
            int a[] = {1, 2, 3, 4};
            GenericVector<int> s{std::span<int>(a)};

            auto it = s.begin();
            auto first = it;
            bool ok;

            ok = (it == first);
            REQUIRE(ok);
            ok = (*it == 1);
            REQUIRE(ok);

            auto beyond = s.end();
            ok = (it != beyond);
            REQUIRE(ok);

            ok = (beyond - first == 4);
            REQUIRE(ok);
            ok = (first - first == 0);
            REQUIRE(ok);
            ok = (beyond - beyond == 0);
            REQUIRE(ok);

            ++it;
            ok = (it - first == 1);
            REQUIRE(ok);
            ok = (*it == 2);
            REQUIRE(ok);
            *it = 22;
            ok = (*it == 22);
            REQUIRE(ok);
            ok = (beyond - it == 3);
            REQUIRE(ok);

            it = first;
            ok = (it == first);
            REQUIRE(ok);
            while (it != s.end())
            {
                *it = 5;
                ++it;
            }

            ok = (it == beyond);
            REQUIRE(ok);
            ok = (it - beyond == 0);
            REQUIRE(ok);

            for (const auto& n : s)
            {
                ok = (n == 5);
                REQUIRE(ok);
            }
        }
    }

    SECTION("cbegin_cend")
    {
        {
            int a[] = {1, 2, 3, 4};
            const GenericVector<int> s{std::span<int>(a)};

            GenericVector<int>::const_iterator cit = s.begin();
            GenericVector<int>::const_iterator cit2 = s.begin();
            bool ok;
            ok = (cit == cit2);
            REQUIRE(ok);

            cit = s.end();
            cit2 = s.end();
            ok = (cit == cit2);
            REQUIRE(ok);
        }

        {
            int a[] = {1, 2, 3, 4};
            const GenericVector<int> s{std::span<int>(a)};

            auto it = s.begin();
            auto first = it;
            bool ok;
            ok = (it == first);
            REQUIRE(ok);
            ok = (*it == 1);
            REQUIRE(ok);

            auto beyond = s.end();
            ok = (it != beyond);
            REQUIRE(ok);

            ok = (beyond - first == 4);
            REQUIRE(ok);
            ok = (first - first == 0);
            REQUIRE(ok);
            ok = (beyond - beyond == 0);
            REQUIRE(ok);

            ++it;
            ok = (it - first == 1);
            REQUIRE(ok);
            ok = (*it == 2);
            REQUIRE(ok);
            ok = (beyond - it == 3);
            REQUIRE(ok);

            int last = 0;
            it = first;
            ok = (it == first);
            REQUIRE(ok);
            while (it != s.end())
            {
                ok = (*it == last + 1);
                REQUIRE(ok);

                last = *it;
                ++it;
            }

            ok = (it == beyond);
            REQUIRE(ok);
            ok = (it - beyond == 0);
            REQUIRE(ok);
        }
    }

    SECTION("rbegin_rend")
    {
        int a[] = {1, 2, 3, 4};
        GenericVector<int> s{std::span<int>(a)};

        auto it = s.rbegin();
        auto first = it;
        bool ok;
        ok = (it == first);
        REQUIRE(ok);
        ok = (*it == 4);
        REQUIRE(ok);

        auto beyond = s.rend();
        ok = (it != beyond);
        REQUIRE(ok);

        ok = (beyond - first == 4);
        REQUIRE(ok);
        ok = (first - first == 0);
        REQUIRE(ok);
        ok = (beyond - beyond == 0);
        REQUIRE(ok);

        ++it;
        ok = (it - first == 1);
        REQUIRE(ok);
        ok = (*it == 3);
        REQUIRE(ok);
        *it = 22;
        ok = (*it == 22);
        REQUIRE(ok);
        ok = (beyond - it == 3);
        REQUIRE(ok);

        it = first;
        ok = (it == first);
        REQUIRE(ok);
        while (it != s.rend())
        {
            *it = 5;
            ++it;
        }

        ok = (it == beyond);
        REQUIRE(ok);
        ok = (it - beyond == 0);
        REQUIRE(ok);

        for (const auto& n : s)
        {
            ok = (n == 5);
            REQUIRE(ok);
        }
    }

    SECTION("crbegin_crend")
    {
        int a[] = {1, 2, 3, 4};
        const GenericVector<int> s{std::span<int>(a)};

        auto it = s.rbegin();
        auto first = it;
        bool ok;
        ok = (it == first);
        REQUIRE(ok);
        ok = (*it == 4);
        REQUIRE(ok);

        auto beyond = s.rend();
        ok = (it != beyond);
        REQUIRE(ok);
        ok = (beyond - first == 4);
        REQUIRE(ok);
        ok = (first - first == 0);
        REQUIRE(ok);
        ok = (beyond - beyond == 0);
        REQUIRE(ok);

        ++it;
        ok = (it - first == 1);
        REQUIRE(ok);
        ok = (*it == 3);
        REQUIRE(ok);
        ok = (beyond - it == 3);
        REQUIRE(ok);

        it = first;
        ok = (it == first);
        REQUIRE(ok);
        int last = 5;
        while (it != s.rend())
        {
            ok = (*it == last - 1);
            REQUIRE(ok);
            last = *it;
            ++it;
        }

        ok = (it == beyond);
        REQUIRE(ok);
        ok = (it - beyond == 0);
        REQUIRE(ok);
    }

    SECTION("To eigen")
    {
        std::vector<double> a{1, 2, 3, 4};
        std::vector<double> b{5, 6, 7, 8};
        std::vector<double> c{6, 8, 10, 12};

        std::vector<double> d(a.size(), 0.0);
        for (std::size_t i = 0; i < d.size(); ++i)
        {
            d[i] = a[i] + b[i];
        }

        REQUIRE(d == c);
    }

    SECTION("Eigen dynamic compatibility")
    {
        Eigen::VectorXd eigenVec(3);
        eigenVec << 1.0, 2.0, 3.0;

        auto resizeLambda
            = [&eigenVec](GenericVector<double>::index_type newSize) -> std::span<double> {
            eigenVec.conservativeResize(static_cast<Eigen::Index>(newSize));
            return std::span<double>(eigenVec.data(), static_cast<std::size_t>(eigenVec.size()));
        };

        GenericVector<double> mapped(std::span<double>(eigenVec.data(),
                                                       static_cast<std::size_t>(eigenVec.size())),
                                     resizeLambda);

        REQUIRE(mapped.size() == 3);
        mapped[1] = 42.0;
        REQUIRE(eigenVec[1] == 42.0);

        REQUIRE(mapped.resizeVector(5));
        REQUIRE(eigenVec.size() == 5);
        mapped[3] = 7.0;
        mapped[4] = 8.0;
        REQUIRE(eigenVec[3] == 7.0);
        REQUIRE(eigenVec[4] == 8.0);
    }

    SECTION("Eigen fixed compatibility")
    {
        Eigen::Vector3d fixedVec;
        fixedVec << 4.0, 5.0, 6.0;

        GenericVector<double> mapped(
            std::span<double>(fixedVec.data(), static_cast<std::size_t>(fixedVec.size())));

        REQUIRE(mapped.size() == 3);
        mapped[0] = 9.0;
        REQUIRE(fixedVec[0] == 9.0);
        REQUIRE_FALSE(mapped.resizeVector(4));
    }

    SECTION("Refs")
    {
        std::vector<int> vec(5);
        GenericVector<int>::Ref stdRef(vec);

        const std::vector<int>& cvec = vec;
        GenericVector<const int>::Ref stdConstRef(cvec);

        std::array<double, 3> fixed{{0.0, 1.0, 2.0}};
        GenericVector<double>::Ref fixedRef(fixed);

        std::vector<double> dyn(4, 0.0);
        GenericVector<double>::Ref dynRef(dyn);

        const std::vector<double>& cdyn = dyn;
        GenericVector<const double>::Ref dynConstRef(cdyn);

        std::string text = "abcd";
        GenericVector<char>::Ref textRef(text);

        REQUIRE(stdRef.size() == 5);
        REQUIRE(stdConstRef.size() == 5);
        REQUIRE(fixedRef.size() == 3);
        REQUIRE(dynRef.size() == 4);
        REQUIRE(dynConstRef.size() == 4);
        REQUIRE(textRef.size() == 4);
    }

    SECTION("Generic input to function")
    {
        std::vector<double> vec(5);
        foo(GenericVector<double>::Ref(vec));

        const std::vector<double>& cvec = vec;
        fooConst(GenericVector<const double>::Ref(cvec));

        std::array<double, 3> fixed{{0.0, 1.0, 2.0}};
        foo(GenericVector<double>::Ref(fixed));

        fooConst(GenericVector<const double>::Ref(vec));

        GenericVector<double>::Ref wrapped(vec);
        foo(GenericVector<double>::Ref(wrapped));
    }

    SECTION("Copy of Refs")
    {
        std::vector<double> vector{0.4, 0.8, 1.2, 1.6, 2.0};
        GenericVector<double>::Ref container(vector);

        std::vector<double> copiedIn(5, 0.0);
        GenericVector<double>::Ref containerToBeCopied(copiedIn);

        REQUIRE(containerToBeCopied.clone(container));

        for (std::size_t i = 0; i < container.size(); ++i)
        {
            REQUIRE(vector[i] == copiedIn[i]);
        }

        std::vector<double> otherCopiedIn;
        GenericVector<double>::Ref otherContainerToBeCopied(otherCopiedIn);
        REQUIRE(otherContainerToBeCopied.clone(container));

        for (std::size_t i = 0; i < container.size(); ++i)
        {
            REQUIRE(vector[i] == otherCopiedIn[i]);
        }
    }
}
