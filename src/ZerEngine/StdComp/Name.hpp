#ifndef ZERENGINE_STDCOMP_NAME_HPP
#define ZERENGINE_STDCOMP_NAME_HPP

#include <string>

struct Name {
public:
    constexpr Name(const std::string& newName) noexcept:
        name(std::make_shared<std::string>(newName)) {
    }

    constexpr operator const std::string&() const noexcept {
        return *name.get();
    }

    constexpr const std::string& operator =(const std::string& newName) noexcept {
        *name.get() = newName;
        return *name.get();
    }

public:
    std::shared_ptr<std::string> name;
};

#endif /* ZERENGINE_STDCOMP_NAME_HPP */