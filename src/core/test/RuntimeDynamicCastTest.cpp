#include <dinrail/RuntimeDynamicCast.h>

#include <catch2/catch_test_macros.hpp>

#include <typeinfo>

namespace test_types
{

struct Animal
{
    virtual ~Animal() = default;
    int animalValue{1};
};

struct Dog final : Animal
{
    int dogValue{2};
};

struct Cat final : Animal
{
    int catValue{3};
};

struct IAlpha
{
    virtual ~IAlpha() = default;
    virtual int alpha() const = 0;
    int alphaStorage{10};
};

struct IBeta
{
    virtual ~IBeta() = default;
    virtual int beta() const = 0;
    int betaStorage{20};
};

struct AlphaBeta final : IAlpha, IBeta
{
    int alpha() const override
    {
        return 1;
    }
    int beta() const override
    {
        return 2;
    }
};

struct AlphaOnly final : IAlpha
{
    int alpha() const override
    {
        return 1;
    }
};

} // namespace test_types

namespace
{

using namespace test_types;

// Cast the source via makePolymorphicView + runtimeDynamicCast, the only public
// entry points of the API.
template <class Target, class Source> Target* runtimeCast(Source* source)
{
    return static_cast<Target*>(
        dinrail::runtimeDynamicCast(dinrail::makePolymorphicView(source), typeid(Target)));
}

} // namespace

TEST_CASE("makePolymorphicView keeps the pointer and static type together", "[runtimeDynamicCast]")
{
    AlphaBeta object;
    IAlpha* alpha = &object;

    const auto view = dinrail::makePolymorphicView(alpha);

    REQUIRE(view.data() == alpha);
    REQUIRE(view.sourceType() == typeid(IAlpha));
}

TEST_CASE("makePolymorphicView and runtimeDynamicCast are noexcept", "[runtimeDynamicCast]")
{
    AlphaBeta object;
    IAlpha* alpha = &object;

    STATIC_REQUIRE(noexcept(dinrail::makePolymorphicView(alpha)));
    STATIC_REQUIRE(
        noexcept(dinrail::runtimeDynamicCast(dinrail::PolymorphicView{}, typeid(IBeta))));
}

TEST_CASE("runtimeDynamicCast returns null for a null or empty source", "[runtimeDynamicCast]")
{
    Animal* nullAnimal = nullptr;

    REQUIRE(runtimeCast<Dog>(nullAnimal) == nullptr);
    REQUIRE(dinrail::runtimeDynamicCast(dinrail::PolymorphicView{}, typeid(IBeta)) == nullptr);
}

TEST_CASE("runtimeDynamicCast handles an identity cast", "[runtimeDynamicCast]")
{
    AlphaBeta object;
    IAlpha* alpha = &object;

    REQUIRE(runtimeCast<IAlpha>(alpha) == alpha);
}

TEST_CASE("runtimeDynamicCast performs downcasts like dynamic_cast", "[runtimeDynamicCast]")
{
    Dog dog;
    Animal* asDog = &dog;
    REQUIRE(runtimeCast<Dog>(asDog) == dynamic_cast<Dog*>(asDog));
    REQUIRE(runtimeCast<Dog>(asDog) == &dog);

    Cat cat;
    Animal* asCat = &cat;
    REQUIRE(runtimeCast<Dog>(asCat) == dynamic_cast<Dog*>(asCat));
    REQUIRE(runtimeCast<Dog>(asCat) == nullptr);
}

TEST_CASE("runtimeDynamicCast performs cross-casts like dynamic_cast", "[runtimeDynamicCast]")
{
    AlphaBeta object;
    IAlpha* alpha = &object;
    REQUIRE(runtimeCast<IBeta>(alpha) == dynamic_cast<IBeta*>(alpha));
    REQUIRE(runtimeCast<IBeta>(alpha) != nullptr);

    AlphaOnly alphaOnly;
    IAlpha* alphaOnlyPtr = &alphaOnly;
    REQUIRE(runtimeCast<IBeta>(alphaOnlyPtr) == nullptr);
}

TEST_CASE("runtimeDynamicCast returns null for unrelated types", "[runtimeDynamicCast]")
{
    AlphaBeta object;
    IAlpha* alpha = &object;

    REQUIRE(runtimeCast<Animal>(alpha) == nullptr);
}
