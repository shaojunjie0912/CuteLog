#pragma once

#include <concepts>
#include <cstdint>
#include <format>
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

template <typename... Args>
void LogInfo(with_source_location<std::format_string<Args...>> fmt, Args &&...args) {
    const auto &loc = fmt.location();
    std::cout << loc.file_name() << ":" << loc.line() << " [Info]"
              << std::vformat(fmt.format().get(), std::make_format_args(args...)) << "\n";
}

}  // namespace cutelog
