# argvx

## Modern, exception-free, header-only argument parser for C++

<p align="left">
  <img src="https://img.shields.io/github/license/XnLogicaL/argvx" alt="License">
  <img src="https://img.shields.io/github/languages/top/XnLogicaL/argvx" alt="Top Language">
  <img src="https://github.com/XnLogicaL/argvx/actions/workflows/ci.yml/badge.svg" alt="CI">
</p>

```cpp
int count, coeff;
bool triple = false;

argvx::parser<> parser(argc, argv);
parser.positional("count", count).required();
parser.option({"--coefficient", "-co"}, coeff).required();
parser.option({"--triple"}, triple);

if (auto error = parser.parse())
  ...
```

> [!WARNING]
> This project is still in its early stages.
> The foundation is solid, but more features are planned.

### Why yet another parser?

There are other great parsers for C++ out there- but all of them had cons that were deal-breakers for me; annoying licenses, lack of QoL/utilities, bad ergonomics, exception-oriented design, etc... So I decided to make my own and publicize it, in the hopes that someone may find it useful. It does **not** aim to replace anyone or anything.

## Installation

Just clone the repository and add `{root}/include` to your include directories.

## Roadmap

- 🟨 Core
  - 🟩 Positional arguments
  - 🟥 Subcommands
  - 🟩 Long & short options
  - 🟩 Basic values (bool, int, uint, float, string, path)
- 🟥 Extra
  - 🟥 Packed options (e.g. `-abc <value>` instead of `-a -b -c <value>`)
  - 🟥 Comma-seperated values (e.g. `-opt a,b,c`)
  - 🟥 IO values (e.g. `--` for stdout)
- 🟨 Ergonomics
  - 🟩 Typed value binding
  - 🟥 Validators/policies
  - 🟥 Auto-generated help command

<details>
  <summary>Legend</summary>
  - 🟩 = Completed
  - 🟨 = In progress
  - 🟥 = Planned
</details>

## Notes

This project was initially a part of [via-lang](https://github.com/XnLogicaL/via-lang), but was forked off because I felt like it.
