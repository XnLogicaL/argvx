// argvx - Copyright (C) XnLogicaL 2025
// Licensed under GNU GPL v3.0; see {root}/LICENSE

#pragma once

#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "policy.hpp"
#include "value.hpp"

namespace argvx {
namespace detail {

struct option_names {
  std::string long_name{}, short_name{};

  inline bool one_defined() {
    return !(long_name.empty() && short_name.empty());
  }
};

using bind_result_t = std::optional<std::string>;
using bind_function_t = std::function<bind_result_t(
    const value&)>;  // TODO: use std::copyable_function when C++26

}  // namespace detail

#define SELF(...) \
  do {            \
    __VA_ARGS__;  \
  } while (0);    \
  return *this;

class argument final {
 public:
  template <detail::prefix_policy, detail::delim_policy>
  friend class parser;

 public:
  explicit argument(const std::type_info& type,
                    std::vector<std::string>&& names,
                    detail::bind_function_t&& bind)
      : m_type(type), m_names(names), m_bind(bind) {}

 public:
  auto name() const { return m_names.front(); }
  argument& required() { SELF(m_required = true); }
  argument& help(std::string help) { SELF(m_help = help); }

 private:
  const std::type_info& m_type;
  std::vector<std::string> m_names;
  detail::bind_function_t m_bind;

  bool m_provided = false;
  bool m_required = false;
  std::string m_help;
};

#undef SELF

}  // namespace argvx
