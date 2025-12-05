# argvx

> [!WARNING]
> This project is still in its early stages.
> The foundation is solid, but more features are planned.

argvx (argv eXtended) is a modern, exception-free, header-only argument parser for C++.

```cpp
#include <print>
#include <argvx/parser.hpp>

int main(int argc, char** argv) {
  int count = 0;
  bool triple = false;

  argvx::parser<> parser(argc, argv);
  parser.positional("count", count)
    .required();
    .help("just an arbitrary number");

  parser.option({"triple", "tri"}, triple)
    .help("triple the <count> argument");

  if (auto error = parser.parse()) {
    std::println(std::cerr, "error: {}", *error);
    return 1;
  }

  if (triple)
    count *= 3;

  std::println("count: {}", count);
  return 0;
}
```

# Installation

Just clone the repository and add `{root}/include` to your include directories.

# Roadmap

- Subcommands
- Validators
- CSV arguments (e.g. `-opt a,b,c`)
- Packed short options (e.g. `-abc <value>` instead of `-a -b -c <value>`)
- Quality of life

# Notes

This project was initially a part of [via-lang](https://github.com/XnLogicaL/via-lang), but was forked off because I felt like it.
