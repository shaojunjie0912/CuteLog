#pragma once

#include <chrono>
#include <concepts>
#include <cstdint>
#include <format>
#include <fstream>
#include <iostream>
#include <source_location>

namespace cutelog {

template <typename T>
struct with_source_location {
private:
    T inner;
    std::source_location loc;

public:
    template <typename U>
        requires std::constructible_from<T, U>
    consteval with_source_location(U &&inner,
                                   std::source_location loc = std::source_location::current())
        : inner(std::forward<U>(inner)), loc(std::move(loc)) {}
    constexpr const T &format() const {
        return inner;
    }
    constexpr const std::source_location &location() const {
        return loc;
    }
};

}  // namespace cutelog
