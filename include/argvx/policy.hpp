// argvx - Copyright (C) XnLogicaL 2025
// Licensed under GNU GPL v3.0; see {root}/LICENSE

#pragma once

#include <cstddef>
#include <string_view>
#include <type_traits>

namespace argvx {
namespace detail {

template <size_t N>
struct static_string {
  char data[N];

  consteval static_string() = default;
  consteval static_string(const char (&str)[N]) {
    for (size_t i = 0; i < N; i++) data[i] = str[i];
  }

  template <size_t M>
  consteval bool operator==(const static_string<M>& other) const {
    if constexpr (N != M) return false;
    for (std::size_t i = 0; i < N; ++i)
      if (data[i] != other.data[i]) return false;
    return true;
  }

  consteval size_t size() { return N - 1; }
};

}  // namespace detail

template <detail::static_string Long, detail::static_string Short>
  requires(Long != Short)
struct prefix_policy {
  static constexpr std::string_view long_prefix = Long.data;
  static constexpr std::string_view short_prefix = Short.data;
};

template <char Asgn, char Sep>
  requires(Asgn != Sep)
struct delim_policy {
  static constexpr char assign_delim = Asgn;
  static constexpr char seperator_delim = Sep;
};

namespace detail {

template <typename T>
struct is_prefix_policy : std::false_type {};

template <detail::static_string Long, detail::static_string Short>
struct is_prefix_policy<prefix_policy<Long, Short>> : std::true_type {};

template <typename T>
concept prefix_policy = is_prefix_policy<T>::value;

template <typename T>
struct is_delim_policy : std::false_type {};

template <char Asgn, char Sep>
struct is_delim_policy<delim_policy<Asgn, Sep>> : std::true_type {};

template <typename T>
concept delim_policy = is_delim_policy<T>::value;

}  // namespace detail
}  // namespace argvx
