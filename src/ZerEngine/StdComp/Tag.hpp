#ifndef ZERENGINE_STDCOMP_TAG_HPP
#define ZERENGINE_STDCOMP_TAG_HPP

#include <string>

struct Tag {
public:
    constexpr Tag(const std::string& newTag) noexcept:
        tag(std::make_shared<std::string>(newTag)) {
    }

    constexpr operator const std::string&() const noexcept {
        return *tag.get();
    }

    constexpr const std::string& operator =(const std::string& newTag) noexcept {
        *tag.get() = newTag;
        return *tag.get();
    }

public:
    std::shared_ptr<std::string> tag;
};

#endif /* ZERENGINE_STDCOMP_TAG_HPP */