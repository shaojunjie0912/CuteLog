#pragma once

#include <concepts>
#include <cstdint>
#include <format>
#include <iostream>
#include <source_location>

namespace cutelog {

// 宏定义与宏展开技术
#define CUTELOG_FOREACH_LOG_LEVEL(f) \
    f(TRACE) f(DEBUG) f(INFO) f(CRITICAL) f(WARNING) f(ERROR) f(FATAL)

enum class LogLevel : uint8_t {
// 注意这里 name 后的逗号,
// TRACE,
// DUBUG,
// ...
#define _FUNCTION(name) name,
    // 宏展开, 替换所有_FUNCTION
    CUTELOG_FOREACH_LOG_LEVEL(_FUNCTION)
#undef _FUNCTION
};

std::string LogLevelName(LogLevel lev) {
    switch (lev) {
#define _FUNCTION(name)  \
    case LogLevel::name: \
        return #name;  // # 返回字符串 name -> "name"
        CUTELOG_FOREACH_LOG_LEVEL(_FUNCTION);
#undef _FUNCTION
    }
    return "unknown";
}

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
void Log(with_source_location<std::format_string<Args...>> fmt, Args &&...args) {
    const auto &loc = fmt.location();
    std::cout << loc.file_name() << ":" << loc.line() << " [Info]"
              << std::vformat(fmt.format().get(), std::make_format_args(args...)) << "\n";
}

}  // namespace cutelog
