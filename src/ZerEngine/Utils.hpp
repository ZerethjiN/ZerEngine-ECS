#pragma once

class ICompDestructor {
public:
    virtual constexpr ~ICompDestructor() noexcept = default;
    virtual constexpr void del(void* ptr) const noexcept = 0;
};

template <typename T>
class CompDestructor final: public ICompDestructor {
public:
    constexpr void del(void* ptr) const noexcept override final {
        static_cast<T*>(ptr)->~T();
    }
};

using Type = std::size_t;
using Ent = std::size_t;

template <typename... Filters>
struct With final {};

template <typename... Filters>
constinit With<Filters...> with;

template <typename... Excludes>
struct Without final {};

template <typename... Excludes>
constinit Without<Excludes...> without;

struct TypeInfos final {
    std::size_t size;
    std::size_t offset;

    constexpr TypeInfos(const std::size_t newSize, const std::size_t newOffset) noexcept:
        size(newSize),
        offset(newOffset) {
    }
};

struct LateUpgradeAddData final {
    const Type type;
    const std::size_t size;
    void* data;

    constexpr LateUpgradeAddData(const Type newType, const std::size_t newSize, void* newData = nullptr) noexcept:
        type(newType),
        size(newSize),
        data(newData) {
    }
};

struct LateUpgradeDelCompData final {
    const Type type;
    const std::size_t size;

    constexpr LateUpgradeDelCompData(const Type newType, const std::size_t newSize) noexcept:
        type(newType),
        size(newSize) {
    }
};