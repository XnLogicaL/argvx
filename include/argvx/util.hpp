// argvx - Copyright (C) XnLogicaL 2025
// Licensed under GNU GPL v3.0; see {root}/LICENSE

#pragma once

#include <cstdlib>
#include <format>
#include <iostream>

namespace argvx {
namespace detail {

template <typename... Args>
[[noreturn]] void panic(std::format_string<Args...> fmt, Args&&... args) {
  std::println(std::cerr, "argvx::panic() called: {}",
               std::format(fmt, std::forward<Args>(args)...));
  std::abort();
}

template <typename T, typename... Args>
void require(T value, std::format_string<Args...> fmt, Args&&... args) {
  if (!value) {
    panic(fmt, std::forward<Args>(args)...);
  }
}

}  // namespace detail
}  // namespace argvx
