module;

#include <unordered_set>

export module ZerengineCore:View;

import :Utils;
import :Archetype;

export template <typename... Ts>
class View final {
friend class Registry;
friend class LiteRegistry;
private:
    constexpr View(std::unordered_set<Archetype*>&& newArchs) noexcept:
        archs(std::move(newArchs)) {
    }

public:
    [[nodiscard]] bool empty() const noexcept {
        if (archs.empty()) {
            return true;
        }
        for (const auto* arch: archs) {
            if (arch->size() > 0) {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] std::size_t size() const noexcept {
        std::size_t newSize = 0;
        for (const auto* arch: archs) {
            newSize += arch->size();
        }
        return newSize;
    }

private:
    class ViewIterator final {
    friend class View;
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = std::tuple<const Ent, Ts&...>;
        using element_type = value_type;
        using pointer = value_type*;
        using reference = value_type&;
        using difference_type = std::ptrdiff_t;

    public:
        ViewIterator(const std::unordered_set<Archetype*>& newArchs, std::unordered_set<Archetype*>::const_iterator newArchsIt) noexcept:
            archsIt(newArchsIt),
            archs(newArchs) {
            if (archsIt != newArchs.end()) {
                entsIt = (*archsIt)->ents.begin();
            }
        }

        [[nodiscard]] value_type operator *() const noexcept {
            return (*archsIt)->getTupleWithEnt<Ts...>((*entsIt));
        }

        [[nodiscard]] ViewIterator& operator ++() noexcept {
            entsIt++;
            if (entsIt == (*archsIt)->ents.end()) {
                archsIt++;
                if (archsIt != archs.end()) {
                    entsIt = (*archsIt)->ents.begin();
                }
            }
            return *this;
        }

        [[nodiscard]] friend constexpr bool operator !=(const ViewIterator& a, const ViewIterator& b) noexcept {
            return a.archsIt != b.archsIt;
        }

    private:
        std::unordered_set<Archetype*>::const_iterator archsIt;
        std::unordered_set<Ent>::iterator entsIt;
        const std::unordered_set<Archetype*>& archs;
    };

public:
    [[nodiscard]] constexpr ViewIterator begin() const noexcept {
        return {archs, archs.begin()};
    }

    [[nodiscard]] constexpr ViewIterator end() const noexcept {
        return {archs, archs.end()};
    }

private:
    const std::unordered_set<Archetype*> archs;
};