#include <argvx/paser.hpp>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

using namespace std::string_literals;

static std::vector<std::string> arg_storage;

static std::vector<char*> make_argv(std::initializer_list<std::string> args) {
  arg_storage.clear();
  for (auto& s : args) arg_storage.emplace_back(s);
  std::vector<char*> out;
  for (auto& s : arg_storage) out.push_back(s.data());
  return out;
}

static int failures = 0;
static const char* current_test = "<unnamed>";

static void fail_msg(const std::string& msg) {
  std::cerr << "[X] test: " << current_test << ": " << msg << "\n";
  failures++;
}

#define TEST(name)          \
  static void name();       \
  static struct name##_t {  \
    name##_t() {            \
      current_test = #name; \
      name();               \
    }                       \
  } name##_instance;        \
  static void name()

static void check(bool cond, const std::string& msg) {
  if (!cond) {
    fail_msg(msg);
  }
}

template <typename A, typename B>
static void check_eq(const A& actual, const B& expected,
                     const std::string& msg) {
  if (!(actual == expected)) {
    fail_msg(msg + " (actual: " + std::string(actual) +
             ", expected: " + std::string(expected) + ")");
  }
}

static std::string to_string_any(const std::filesystem::path& p) {
  return p.string();
}

static std::string to_string_any(const char* s) {
  return s ? std::string(s) : "<null>";
}

static std::string to_string_any(const std::string& s) { return s; }

static std::string to_string_any(bool b) { return b ? "true" : "false"; }
template <typename A, typename B>
static void check_eq_any(const A& actual, const B& expected,
                         const std::string& msg) {
  if (!(actual == expected)) {
    fail_msg(msg + " (actual: " + to_string_any(actual) +
             ", expected: " + to_string_any(expected) + ")");
  }
}

TEST(missing_required_positional) {
  auto argv = make_argv({"prog"});

  std::filesystem::path in;
  argvx::parser parser(argv.size(), argv.data());

  parser.positional("input", in).required();

  auto err = parser.parse();
  check(err.has_value(), "required positional should fail when missing");
}

TEST(missing_required_option) {
  auto argv = make_argv({"prog", "input"});

  std::filesystem::path in, out;
  argvx::parser parser(argv.size(), argv.data());

  parser.positional("input", in).required();
  parser.option({"--output", "-o"}, out).required();

  auto err = parser.parse();
  check(err.has_value(), "required option should fail when missing");
}

TEST(unknown_option) {
  auto argv = make_argv({"prog", "--nope"});

  std::filesystem::path in, out;
  argvx::parser parser(argv.size(), argv.data());

  parser.positional("input", in);
  parser.option({"--output", "-o"}, out);

  auto err = parser.parse();
  check(err.has_value(), "unknown option should produce an error");
}

TEST(bool_flag_sets_true) {
  auto argv = make_argv({"prog", "--do-thing", "file", "-o", "out"});

  bool do_thing = false;
  std::filesystem::path in, out;

  argvx::parser parser(argv.size(), argv.data());
  parser.positional("input", in).required();
  parser.option({"--output", "-o"}, out);
  parser.option({"--do-thing", "-do"}, do_thing);

  auto err = parser.parse();
  check(!err.has_value(), "bool flag parse should succeed");
  check_eq_any(do_thing, true, "bool flag should set bool to true");
}

TEST(path_option_parses) {
  auto argv = make_argv({"prog", "in.txt", "-o", "res/out.txt"});

  std::filesystem::path in, out;

  argvx::parser parser(argv.size(), argv.data());
  parser.positional("input", in).required();
  parser.option({"--output", "-o"}, out).required();

  auto err = parser.parse();
  check(!err.has_value(), "path parse should succeed");
  check_eq_any(in, "in.txt", "input path mismatch");
  check_eq_any(out, "res/out.txt", "output path mismatch");
}

TEST(full_happy_path) {
  auto argv = make_argv({"prog", "--do-thing", "src/in", "-o", "dst/out"});

  bool do_thing = false;
  std::filesystem::path in, out;

  argvx::parser parser(argv.size(), argv.data());

  parser.positional("input", in).required();
  parser.option({"--output", "-o"}, out).required();
  parser.option({"--do-thing", "-do"}, do_thing);

  auto err = parser.parse();

  check(!err.has_value(), "happy path shouldn't fail");
  check_eq_any(do_thing, true, "flag was not parsed correctly");
  check_eq_any(in, "src/in", "input mismatch");
  check_eq_any(out, "dst/out", "output mismatch");
}

TEST(error_propagates_from_value_parser) {
  auto argv = make_argv({"prog", "in", "-o"});

  std::filesystem::path in, out;

  argvx::parser parser(argv.size(), argv.data());

  parser.positional("input", in).required();
  parser.option({"--output", "-o"}, out).required();

  auto err = parser.parse();
  check(err.has_value(), "missing value should error");
}

int main() {
  if (failures == 0) {
    std::cout << "[*] all tests passed\n";
    return 0;
  }
  std::cout << "[!] " << failures << " tests failed\n";
  return 1;
}
