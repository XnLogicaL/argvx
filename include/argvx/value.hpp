// argvx - Copyright (C) XnLogicaL 2025
// Licensed under GNU GPL v3.0; see {root}/LICENSE

#pragma once

#include <concepts>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <utility>
#include <variant>

namespace argvx {
namespace fs = std::filesystem;
namespace detail {

template <typename T>
struct value_type;

template <>
struct value_type<bool> {
  using type = bool;
  static constexpr auto name = "bool";
};

template <std::integral T>
struct value_type<T> {
  using type = std::conditional_t<std::is_signed_v<T>, int64_t, uint64_t>;
  static constexpr auto name = std::is_signed_v<T> ? "int64" : "uint64";
};

template <std::floating_point T>
struct value_type<T> {
  using type = double;
  static constexpr auto name = "double";
};

template <>
struct value_type<std::string> {
  using type = std::string;
  static constexpr auto name = "string";
};

template <>
struct value_type<fs::path> {
  using type = fs::path;
  static constexpr auto name = "path";
};

template <typename T>
using value_type_t = typename value_type<T>::type;

template <typename T>
concept value_alternative = requires { typename value_type<T>::type; };

}  // namespace detail

using value =
    std::variant<bool, int64_t, uint64_t, double, std::string, fs::path>;

class default_value_parser final {
 public:
  template <typename T>
  static std::expected<T, std::string> parse_as(std::string_view sv);
  static std::expected<value, std::string> parse(std::string_view sv,
                                                 const std::type_info& type);
};

template <>
inline std::expected<bool, std::string> default_value_parser::parse_as<bool>(
    std::string_view sv) {
  if (sv == "true" || sv == "on" || sv == "yes") return true;
  if (sv == "false" || sv == "off" || sv == "no") return false;
  return std::unexpected("bad value (boolean)");
}

template <>
inline std::expected<int64_t, std::string>
default_value_parser::parse_as<int64_t>(std::string_view sv) {
  int64_t v = 0;
  auto [ptr, ec] = std::from_chars(sv.begin(), sv.end(), v);
  if (ec == std::errc::invalid_argument)
    return std::unexpected("bad value (int64)");
  if (ec == std::errc::result_out_of_range)
    return std::unexpected("bad value (int64 out of range)");
  if (ptr != sv.end())
    return std::unexpected("bad value (trailing characters in int64)");
  return v;
}

template <>
inline std::expected<uint64_t, std::string>
default_value_parser::parse_as<uint64_t>(std::string_view sv) {
  uint64_t v = 0;
  auto [ptr, ec] = std::from_chars(sv.begin(), sv.end(), v);
  if (ec == std::errc::invalid_argument)
    return std::unexpected("bad value (uint64)");
  if (ec == std::errc::result_out_of_range)
    return std::unexpected("bad value (uint64 out of range)");
  if (ptr != sv.end())
    return std::unexpected("bad value (trailing characters in uint64)");
  return v;
}

template <>
inline std::expected<double, std::string>
default_value_parser::parse_as<double>(std::string_view sv) {
  double v = 0.0;
  auto [ptr, ec] = std::from_chars(sv.begin(), sv.end(), v);
  if (ec == std::errc::invalid_argument)
    return std::unexpected("bad value (double)");
  if (ec == std::errc::result_out_of_range)
    return std::unexpected("bad value (double out of range)");
  if (ptr != sv.end())
    return std::unexpected("bad value (trailing characters in double)");
  return v;
}

inline std::expected<value, std::string> default_value_parser::parse(
    std::string_view sv, const std::type_info& type) {
  if (type == typeid(bool)) {
    auto r = parse_as<bool>(sv);
    if (!r) return std::unexpected(r.error());
    return value(*r);
  }

  if (type == typeid(int64_t)) {
    auto r = parse_as<int64_t>(sv);
    if (!r) return std::unexpected(r.error());
    return value(*r);
  }

  if (type == typeid(uint64_t)) {
    auto r = parse_as<uint64_t>(sv);
    if (!r) return std::unexpected(r.error());
    return value(*r);
  }

  if (type == typeid(double)) {
    auto r = parse_as<double>(sv);
    if (!r) return std::unexpected(r.error());
    return value(*r);
  }

  if (type == typeid(std::string)) return value(std::string(sv));
  if (type == typeid(std::filesystem::path))
    return value(std::filesystem::path(sv));

  std::unreachable();
}

namespace detail {

template <typename T>
concept value_parser = requires {
  {
    T::parse(std::string_view{}, typeid(int))
  } -> std::same_as<std::expected<value, std::string>>;
};

template <value_alternative T>
consteval std::string_view type_name() {
  return value_type<T>::name;
}

constexpr std::string_view type_name(const value& value) {
  return std::visit(
      [](auto&& underlying) -> std::string_view {
        return type_name<std::decay_t<decltype(underlying)>>();
      },
      value);
}

constexpr std::string type_name(const std::type_info& ti) {
  if (ti == typeid(bool)) return "boolean";
  if (ti == typeid(int64_t)) return "int64";
  if (ti == typeid(uint64_t)) return "uint64";
  if (ti == typeid(double)) return "double";
  if (ti == typeid(std::filesystem::path)) return "path";
  if (ti == typeid(std::string)) return "string";
  return ti.name();
}

inline std::string to_string(const value& value) {
  return std::visit(
      [](auto&& underlying) -> std::string {
        using U = std::decay_t<decltype(underlying)>;
        if constexpr (std::is_same_v<U, bool>)
          return underlying ? "true" : "false";
        else if constexpr (std::is_same_v<U, fs::path>)
          return underlying.string();
        else if constexpr (std::is_same_v<U, std::string>)
          return underlying;
        else
          return std::to_string(underlying);
      },
      value);
}

}  // namespace detail
}  // namespace argvx
