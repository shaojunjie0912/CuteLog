#pragma once

#include <chrono>
#include <concepts>
#include <cstdint>
#include <cstdlib>
#include <format>
#include <fstream>
#include <iostream>
#include <source_location>

namespace cutelog {

// 因为宏函数是没有名称空间的，因此需要给宏加上库名

/**
 * #define 一般是一行，因此会产生 \
 */

// 宏定义与宏展开技术
// 好好好！XMACRO 大法，省去写if else if else...
// 同步修改所有等级也很方便
#define CUTELOG_FOREACH_LOG_LEVEL(f) \
    f(TRACE) f(DEBUG) f(INFO) f(CRITICAL) f(WARNING) f(ERROR) f(FATAL)

// 新学习到的 enum 定义方法
// 注意这里 name 后的逗号,
// TRACE,
// DUBUG,
// ...
// 宏展开, 替换所有_FUNCTION
enum class LogLevel : uint8_t {
#define _FUNCTION(name) name,
    CUTELOG_FOREACH_LOG_LEVEL(_FUNCTION)
#undef _FUNCTION
};

// 将仅在cutelog内部使用的函数封装起来
// 用户不需知道details中细节
namespace details {

// 终端带颜色输出
#if defined(__linux__) || defined(__APPLE__)
// 设置终端颜色
inline constexpr char k_level_ansi_colors[(std::uint8_t)LogLevel::FATAL + 1][8] = {
    "\E[37m", "\E[35m", "\E[32m", "\E[34m", "\E[33m", "\E[31m", "\E[31;1m",
};
// 恢复终端颜色
inline constexpr char k_reset_ansi_color[4] = "\E[m";
#define _MINILOG_IF_HAS_ANSI_COLORS(x) x
#else
// 如果是 windows 则宏设为空
#define _MINILOG_IF_HAS_ANSI_COLORS(x)
inline constexpr char k_level_ansi_colors[(std::uint8_t)log_level::fatal + 1][1] = {
    "", "", "", "", "", "", "",
};
inline constexpr char k_reset_ansi_color[1] = "";
#endif

// #是字符串化运算符，将宏参数转换为字符串
// 注意这里的inline，在头文件中定义函数
// 防止多个cpp文件包含该头文件
// LogLevel -> string
inline std::string LogLevelToName(LogLevel lev) {
    switch (lev) {
#define _FUNCTION(name)  \
    case LogLevel::name: \
        return #name;
        CUTELOG_FOREACH_LOG_LEVEL(_FUNCTION);
#undef _FUNCTION
    }
    return "unknown";
}

// string -> LogLevel
inline LogLevel LogLevelFromName(std::string lev) {
#define _FUNCTION(name) \
    if (lev == #name)   \
        return LogLevel::name;
    CUTELOG_FOREACH_LOG_LEVEL(_FUNCTION);
#undef _FUNCTION
    return LogLevel::INFO;
}

/**
 * 包装模板类
 * NOTE: 解决变长模板参数后不能使用形参默认值问题？
 */
template <typename T>
struct WithSourceLocation {
private:
    T inner;
    std::source_location loc;

public:
    template <typename U>
        requires std::constructible_from<T, U>
    // consteval 强制编译期
    // 这里 consteval 的原因在于：GenericLog参数std::format_string在编译期构造
    consteval WithSourceLocation(U &&inner,
                                 std::source_location loc = std::source_location::current())
        : inner(std::forward<U>(inner)), loc(std::move(loc)) {}
    // constexpr 可以编译期/运行时
    constexpr const T &format() const {
        return inner;
    }
    constexpr const std::source_location &location() const {
        return loc;
    }
};

// 内联变量[c++17]
// 使得多个cpp文件中的max_level相同
// 注意这里inline变量在多个文件中共享 [c++17]
// static 无法实现该效果：每个cpp文件都是自己的max_level
// 先从环境变量中获取，若无则设为 INFO
// static initialization 大法
// lambda 就地调用
inline LogLevel g_max_level = []() -> LogLevel {
    if (auto lev = std::getenv("CUTELOG_LEVEL")) {
        return details::LogLevelFromName(lev);
    }
    return LogLevel::INFO;
}();

// 日志文件
inline std::ofstream g_log_file = []() -> std::ofstream {
    if (auto path = std::getenv("CUTELOG_FILE")) {
        return std::ofstream(path, std::ios::app);
    }
    return std::ofstream();
}();

inline void OutputLog(LogLevel lev, std::string msg, const std::source_location &loc) {
    // 获取当前时间
    std::chrono::zoned_time now{std::chrono::current_zone(),
                                std::chrono::high_resolution_clock::now()};
    msg =
        std::format("{} {}:{} [{}] {}", now, loc.file_name(), loc.line(), LogLevelToName(lev), msg);
    if (g_log_file) {
        g_log_file << msg + '\n';
    }
    if (lev >= g_max_level) {
        // 单个<<是线程安全的
        std::cout << _MINILOG_IF_HAS_ANSI_COLORS(k_level_ansi_colors[(std::uint8_t)lev] +)
                             msg _MINILOG_IF_HAS_ANSI_COLORS(+k_reset_ansi_color) +
                         '\n';
    }
}

}  // namespace details

inline void SetLogLevel(LogLevel lev) {
    details::g_max_level = lev;
}

inline void SetLogFile(std::string path) {
    // 文件追加模式
    details::g_log_file = std::ofstream(path, std::ios::app);
}

/**
 * 通用日志函数
 */
template <typename... Args>
void GenericLog(LogLevel lev, details::WithSourceLocation<std::format_string<Args...>> fmt,
                Args &&...args) {
    const auto &loc = fmt.location();
    auto msg = std::vformat(fmt.format().get(), std::make_format_args(args...));
    details::OutputLog(lev, msg, loc);
}

// ##是连接运算符，它会将宏参数拼接起来
#define _FUNCTION(name)                                                                 \
    template <typename... Args>                                                         \
    void LOG_##name(details::WithSourceLocation<std::format_string<Args...>> fmt,       \
                    Args &&...args) {                                                   \
        return GenericLog(LogLevel::name, std::move(fmt), std::forward<Args>(args)...); \
    }
CUTELOG_FOREACH_LOG_LEVEL(_FUNCTION);
#undef _FUNCTION

/**
 * DEBUG 模式打印变量
 * NOTE: ::cutelog::确保是全局命名空间的cutelog命名空间
 */
#define LOG_P(x) ::cutelog::LOG_DEBUG(#x "={}", x)

}  // namespace cutelog
