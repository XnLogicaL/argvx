// argvx - Copyright (C) XnLogicaL 2025
// Licensed under GNU GPL v3.0; see {root}/LICENSE

#pragma once

#include <cassert>
#include <cstdlib>
#include <format>
#include <optional>
#include <type_traits>
#include <typeinfo>
#include <unordered_map>

#include "argument.hpp"
#include "policy.hpp"
#include "util.hpp"
#include "value.hpp"

namespace argvx {
namespace detail {

template <typename T>
struct binder {
  T* bind;
  std::string name;

  template <typename U>
  std::optional<std::string> operator()(const U& val) const {
    if constexpr (std::is_same_v<T, U>) {
      *bind = val;
      return std::nullopt;
    } else {
      return std::format("{}: expected {}, got {}", name, type_name<T>(),
                         type_name<U>());
    }
  }
};

}  // namespace detail

template <detail::prefix_policy Pp = prefix_policy<"--", "-">,
          detail::delim_policy Dp = delim_policy<'=', ','>>
class parser final {
 public:
  friend class argument;

 public:
  explicit parser(size_t argc, const char* const* argv) : m_argv(argv, argc) {}

 public:
  template <detail::value_alternative T>
  argument& positional(std::string name, T& bind) {
    detail::require(!name.empty(), "positional must have non-empty name");

    // REMINDER: do NOT use std::make_shared here as it breaks std::function
    // move/copy semantics (nice one committee :|)
    auto ptr = std::shared_ptr<argument>(new argument{
        typeid(T),
        {name},
        [&bind, name](const value& value) -> detail::bind_result_t {
          return std::visit(detail::binder<T>{&bind, name}, value);
        }});

    m_positionals.push_back(ptr);
    return *ptr;
  }

  template <detail::value_alternative T>
  argument& option(detail::option_names option_names, T& bind) {
    detail::require(option_names.one_defined(),
                    "option must have at least one name");

    std::string lname = option_names.long_name;
    std::string sname = option_names.short_name;
    std::vector<std::string> names;

    if (!lname.empty()) {
      detail::require(lname.starts_with(Pp::long_prefix),
                      "long option name must start with long prefix");
      names.push_back(lname);
    }

    if (!sname.empty()) {
      detail::require(sname.starts_with(Pp::short_prefix),
                      "short option name must start with short prefix");
      names.push_back(sname);
    }

    std::string name = names.front();

    // REMINDER: do NOT use std::make_shared here as it breaks std::function
    // move/copy semantics (nice one committee :|)
    auto ptr = std::shared_ptr<argument>(new argument{
        typeid(T), std::move(names),
        [&bind, name](const value& value) -> detail::bind_result_t {
          return std::visit(detail::binder<T>{&bind, name}, value);
        }});

    if (!lname.empty()) m_options[lname] = ptr;
    if (!sname.empty()) m_options[sname] = ptr;
    return *ptr;
  }

  template <detail::value_parser Vp = default_value_parser>
  std::optional<std::string> parse() {
    size_t position = 0;
    for (size_t index = 1; index < m_argv.size(); ++index) {
      std::string_view token = m_argv[index];
      if (token.starts_with(Pp::long_prefix)) {
        if (auto error = m_parse_long_opt<Vp>(token)) return *error;
      } else if (token.starts_with(Pp::short_prefix)) {
        if (auto error = m_parse_short_opt<Vp>(token, index)) return *error;
      } else {
        if (auto error = m_parse_positional<Vp>(token, position)) return *error;
      }
    }
    if (auto error = m_check_required()) return *error;
    return std::nullopt;
  }

 private:
  template <detail::value_parser Vp>
  std::optional<std::string> m_parse_long_opt(std::string_view token) {
    auto delim = token.find(Dp::assign_delim);
    std::string_view option, raw;

    if (delim != std::string::npos) {
      option = token.substr(0, delim);
      raw = token.substr(delim + 1);
    } else {
      option = token;
      raw = "";
    }

    auto it = m_options.find(std::string(option));
    if (it == m_options.end()) {
      return std::format("unknown option: {}", option);
    }

    auto& opt = it->second;
    opt->m_provided = true;

    auto value = Vp::parse(raw, opt->m_type);
    if (!value.has_value()) {
      if (opt->m_type == typeid(bool)) {
        value = argvx::value(true);
        goto bind_value;
      }
      return std::format("{}: {}", token, value.error());
    }

    if (auto error = m_type_check(token, *value, *opt)) return *error;

  bind_value:
    if (auto error = opt->m_bind(*value)) return *error;
    return std::nullopt;
  }

  template <detail::value_parser Vp>
  std::optional<std::string> m_parse_short_opt(std::string_view token,
                                               size_t& index) {
    auto it = m_options.find(std::string(token));
    if (it == m_options.end()) {
      return std::format("unknown option: {}", token);
    }

    auto& opt = it->second;
    opt->m_provided = true;

    if (opt->m_type == typeid(bool)) {
      if (auto error = opt->m_bind(value(true))) {
        return *error;
      }
    } else {
      if (index == m_argv.size() - 1) {
        return std::format("{}: missing value", token);
      }

      std::string_view next = m_argv[++index];
      auto value = Vp::parse(next, opt->m_type);
      if (auto error = m_type_check(token, *value, *opt)) return *error;
      if (auto error = opt->m_bind(*value)) return *error;
    }
    return std::nullopt;
  }

  template <detail::value_parser Vp>
  std::optional<std::string> m_parse_positional(std::string_view token,
                                                size_t& position) {
    if (position >= m_positionals.size()) {
      return std::format("unexpected positional argument #{}", position);
    }

    auto positional = m_positionals.at(position);
    positional->m_provided = true;

    auto value = Vp::parse(token, positional->m_type);
    if (!value.has_value()) return value.error();
    if (auto error = m_type_check(token, *value, *positional)) return *error;
    if (auto error = positional->m_bind(*value)) return *error;
    position++;
    return std::nullopt;
  }

  inline std::optional<std::string> m_type_check(std::string_view name,
                                                 const value& value,
                                                 const argument& arg) {
    const std::type_info& type = std::visit(
        [](const auto& underlying) -> const std::type_info& {
          return typeid(underlying);
        },
        value);

    if (type != arg.m_type)
      return std::format("{}: expected {}, got {} '{}'", name,
                         detail::type_name(arg.m_type), detail::type_name(type),
                         detail::to_string(value));
    return std::nullopt;
  }

  inline std::optional<std::string> m_check_required() {
    for (const auto& ptr : m_positionals)
      if (ptr->m_required && !ptr->m_provided)
        return std::format("missing required positional: {}", ptr->name());
    for (const auto& it : m_options)
      if (it.second->m_required && !it.second->m_provided)
        return std::format("missing required option: {}", it.second->name());
    return std::nullopt;
  }

 private:
  std::span<const char* const> m_argv;
  std::vector<std::shared_ptr<argument>> m_positionals;
  std::unordered_map<std::string, std::shared_ptr<argument>> m_options;
};

}  // namespace argvx
