#include "cutelog.h"

#include <concepts>
#include <format>
#include <iostream>
#include <source_location>
#include <string>
#include <string_view>

using std::cout;
using std::endl;
using namespace cutelog;

// void CuteLog(std::string_view str, std::source_location loc = std::source_location::current()) {
//     cout << std::format("{}:{}:{}:{}:{}", loc.file_name(), loc.function_name(), loc.line(),
//                         loc.column(), str)
//          << endl;
// }

template <typename T>
concept Incrementable = requires(T x) {
    x++;
    ++x;
};

template <Incrementable T>
void process(const T& t) {}

int main() {
    // CuteLog("hello");
    Incrementable auto value1{1};
    // std::string str1 = "dsadsa"s;
    LogInfo("dsadsa, {}", 43);
    return 0;
}
