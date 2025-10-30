#pragma once

#include <fmt/ostream.h>
#include <fmt/color.h>

using fmt::color;
using fmt::terminal_color;
using fmt::emphasis;

namespace fmt {
template <typename... T>
void eprint(format_string<T...> fmt, T&&... args) {
  fmt::print(std::cerr, fmt, std::forward<T>(args)...);
}

template <typename... T>
void eprintln(format_string<T...> fmt, T&&... args) {
  fmt::println(std::cerr, fmt, std::forward<T>(args)...);
}

template <typename... T>
void eprint(text_style ts, format_string<T...> fmt, T&&... args) {
  fmt::print(std::cerr, "{}", fmt::format(ts, fmt, std::forward<T>(args)...));
}

template <typename... T>
void eprintln(text_style ts, format_string<T...> fmt, T&&... args) {
  fmt::println(std::cerr, "{}", fmt::format(ts, fmt, std::forward<T>(args)...));
}

}  // namespace fmt
