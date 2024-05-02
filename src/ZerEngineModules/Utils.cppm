module;

#include <cstddef>

export module ZerengineCore:Utils;

export using Ent = std::size_t;
export using Type = std::size_t;

template <typename... Filters>
struct With final {};
export template <typename... Filters>
constinit With<Filters...> with;

template <typename... Excludes>
struct Without final {};
export template <typename... Excludes>
constinit Without<Excludes...> without;

struct WithInactive final {};
export constinit WithInactive withInactive;

export class IsInactive final {};