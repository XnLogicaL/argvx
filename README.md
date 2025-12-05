# argvx

> [!WARNING]
> This project currently features a very minimal implementation.
> It is planned to be expanded with significantly more features.

argvx (argv eXtended) is a feature-rich, exception-free argument parser for modern C++

```cpp
#include <print>
#include <iostream>
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

This project was initially a part of [via-lang](https://github.com/XnLogicaL/via-lang) but was forked off because yeah.

# Installation

Just clone the repository and add `{root}/include` to your include directories.

# Planned

- Subcommands
- Validators
- CSV values (e.g. `-opt a,b,c`)
- Option packing (e.g. `-abc <value>` instead of `-a -b -c <value>`)
