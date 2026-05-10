# Design Patterns in Modern C++

A self-contained, educational reference that translates all 23 Gang-of-Four design patterns into **C++23**, grounded in concrete, real-world mini-projects. Every pattern comes with a _Why_ section that explains the decision process — what alternatives were considered and why this pattern wins in that context.

> **Who is this for?**  
> Developers who already know basic OOP and want to understand _when_ to reach for a pattern, not just _how_ to implement it. Each example is intentionally small and self-contained so you can read it in isolation.
> If you're getting started with C++, check [Learn C++](https://www.learncpp.com/) for a comprehensive introduction and [LearnCPP](https://github.com/bbalouki/LearnCpp) for configuration and usage of C++ in VS Code.

---

## What's Inside

| File                                                       | Category   | Patterns (count) |
| ---------------------------------------------------------- | ---------- | ---------------- |
| [src/creational_patterns.cpp](src/creational_patterns.cpp) | Creational | 5                |
| [src/structural_patterns.cpp](src/structural_patterns.cpp) | Structural | 7                |
| [src/behavioral_patterns.cpp](src/behavioral_patterns.cpp) | Behavioral | 11               |

Each pattern section follows the same structure:

```
PROJECT   — a real-world mini-project that motivates the pattern
WHY       — decision process: why this pattern fits here
DECISION  — alternatives that were ruled out and why
CODE      — full, runnable C++ implementation with a demo main()
```

---

## Pattern Reference

### Creational — _How objects are created_

| Pattern          | Real-World Project        | Key Decision                                               |
| ---------------- | ------------------------- | ---------------------------------------------------------- |
| Abstract Factory | Cross-platform UI widgets | Stable widget _kinds_, growing _families_ (Windows/Mac)    |
| Builder          | Custom PC configurator    | 4+ optional parts; same Director, different output         |
| Factory Method   | Game enemy spawner        | Level knows the algorithm; subclass decides which enemy    |
| Prototype        | Magic spell palette       | Spells vary in _state_ not class; runtime mod registration |
| Singleton        | App config manager        | Uniqueness is a _domain requirement_, not convenience      |

### Structural — _How objects are composed_

| Pattern         | Real-World Project          | Key Decision                                          |
| --------------- | --------------------------- | ----------------------------------------------------- |
| Adapter         | Legacy payment gateway      | Third-party SDK, can't modify; interface mismatch     |
| Bridge          | Remote ↔ Device (N×M → N+M) | Two independent axes both need subclassing            |
| Composite       | File system (File/Folder)   | Part-whole hierarchy; uniform `size()` call           |
| Decorator       | Coffee shop add-ons         | Combinatorial combinations at runtime                 |
| Façade          | Home theater (8 subsystems) | Simple interface; power users bypass if needed        |
| Flyweight       | Forest of 100 000 trees     | Intrinsic (species) shared; extrinsic (x,y) passed in |
| Proxy (Virtual) | Lazy image loading          | Defer expensive disk I/O until first `draw()`         |

### Behavioral — _How objects communicate_

| Pattern                 | Real-World Project                 | Key Decision                                      |
| ----------------------- | ---------------------------------- | ------------------------------------------------- |
| Chain of Responsibility | Support ticket escalation          | Dynamic handler set; no fixed receiver            |
| Command                 | Smart bulb with undo               | First-class actions; undo stack                   |
| Interpreter             | Arithmetic expression evaluator    | Simple grammar (≤5 rules); AST = Composite        |
| Iterator                | Music playlist (forward + shuffle) | Hide internal representation; multiple traversals |
| Mediator                | Chat room                          | N×(N-1) → hub; colleagues know only the room      |
| Memento                 | Text editor undo/redo              | Snapshot without breaking encapsulation           |
| Observer                | Stock price tracker                | Unknown number of dependents; pull model          |
| State                   | Traffic light cycle                | Behavior fully depends on state; self-transitions |
| Strategy                | Runtime sorting selector           | Whole algorithm swapped at runtime                |
| Template Method         | CSV/HTML report generator          | Structure fixed; steps vary by subclass           |
| Visitor                 | Shopping cart (tax/discount/price) | Stable element types; growing operations          |

---

## Building

The project uses **CMake 3.25+** and **C++23**. Three executables are produced, one per pattern category.

### Prerequisites

| Tool                                   | Minimum version                               |
| -------------------------------------- | --------------------------------------------- |
| CMake                                  | 3.25                                          |
| C++ compiler                           | GCC 13 / Clang 17 / MSVC 19.38 (VS 2022 17.8) |
| Ninja (Linux/macOS)                    | any recent version                            |
| MinGW-w64 / MSYS2 UCRT64 (Windows GCC) | GCC 13+                                       |

### Windows — MinGW / MSYS2

```powershell
# Debug
cmake --preset mingw
cmake --build build

# Release
cmake --preset mingw-rel
cmake --build build
```

### Windows — MSVC (Visual Studio 2022)

```powershell
cmake --preset win
cmake --build build --config Debug
cmake --build build --config Release
```

### Linux / macOS

```bash
# Debug
cmake --preset cross
cmake --build build

# Release
cmake --preset cross-rel
cmake --build build
```

### Run the demos

After building, the executables land in `build/` (single-config generators) or `build/<Debug|Release>/` (MSVC):

```bash
./build/creational_patterns
./build/structural_patterns
./build/behavioral_patterns
```

### VS Code

Open the repo, select a **CMake Kit** matching your toolchain, then use the **CMake: Build** command (`Ctrl+Shift+B`). The provided [.vscode/tasks.json](.vscode/tasks.json) exposes all presets as named build tasks.

---

## Project Layout

```
DesignPatterns/
├── src/
│   ├── creational_patterns.cpp   # Abstract Factory, Builder, Factory Method, Prototype, Singleton
│   ├── structural_patterns.cpp   # Adapter, Bridge, Composite, Decorator, Façade, Flyweight, Proxy
│   └── behavioral_patterns.cpp   # Chain of Responsibility, Command, Interpreter, Iterator, Mediator,
│                                 # Memento, Observer, State, Strategy, Template Method, Visitor
├── CMakeLists.txt
├── CMakePresets.json
├── .clang-format
├── .clang-tidy
└── .vscode/
```

---

## Further Reading

- _Design Patterns: Elements of Reusable Object-Oriented Software_ — Gamma, Helm, Johnson, Vlissides (GoF)
- _Head First Design Patterns_ — Freeman & Robson — gentler introduction with visual diagrams
- [refactoring.guru/design-patterns](https://refactoring.guru/design-patterns) — free online reference with UML and multi-language examples
