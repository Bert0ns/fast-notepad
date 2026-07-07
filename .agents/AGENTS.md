# Fast Notepad - AI Agent Guidelines (`AGENTS.md`)

## 1. Project Context & Tooling

- **Core Language:** C++
- **UI Framework:** Dear ImGui
- **Build System:** CMake
- **Testing:** Catch2 (run via `ctest` or `./scripts/run-tests.sh`)
- **Formatting:** `clang-format` for C/C++ files, `prettier` for others. Handled via Node.js/npm and `lint-staged`.

## 2. Product Goals & Constraints

- A compact, cross-platform text editor.
- Focus on fast startup time, a minimal UI, and practical editing tools.
- Key features: Multi-tab support, syntax highlighting, find & replace, session persistence, and native OS file dialogs.
- Keep dependencies lightweight (e.g., `stb_image`, `portable-file-dialogs`).

## 3. SOLID Principles

All code written for this project must strictly respect **SOLID principles**:

- **S - Single Responsibility Principle (SRP):** Each class, struct, or module should have one, and only one, reason to change.
- **O - Open/Closed Principle (OCP):** Software entities should be open for extension but closed for modification.
- **L - Liskov Substitution Principle (LSP):** Subtypes must be substitutable for their base types.
- **I - Interface Segregation Principle (ISP):** Do not force components to depend on interfaces they do not use.
- **D - Dependency Inversion Principle (DIP):** Depend on abstractions, not concretions.

## 4. Coding Standards & Best Practices

- **Never git commit without approval.**
- Write clean, readable code and use `clang-format` (or let the pre-commit hook do it) before submitting.
- Avoid memory leaks: prefer modern C++ constructs like smart pointers (`std::unique_ptr`, `std::shared_ptr`) over raw pointers when managing ownership.
- Do not block the Dear ImGui rendering loop with heavy synchronous tasks.

## 5. Project Structure

- `src/core/`: Application core state, models, and business logic.
- `src/io/`: File I/O operations and native dialog integrations.
- `src/ui/`: UI rendering logic, window panes, and Dear ImGui abstractions.
- `src/utils/`: Generic helper functions and utilities.
- `tests/`: Automated unit tests using Catch2.
- `scripts/`: Helper shell scripts for building and testing.

## 6. Effectively Using Your AI Agent abilities

Do not start mindlessly writing code! Develop a full-fledged plan first.
Put it in `docs/plans`, you can either choose to write a `.md` file or an `.html` file.

- **Precision & Exhaustiveness:** The plan should always describe all features and changes with precision.
- **Task Tracking:** If you have to do a lot of work, split it into smaller tasks (Divide et impera principle). Use Markdown checkboxes (`[ ]` and `[x]`) in your plan file to track progress. Update the file as you complete each step.
- **Tests in Plan:** Always include a sketch of the required Catch2 unit tests as part of your implementation plan.
- **Confirmation:** After writing a first version of the plan, always ask for confirmation from the human. If there are ambiguities, ask.
- **Simplicity:** Always prefer elegant simple solutions that achieve 95% of the goal, rather than complex solutions.
- **Verification:** After completing a coding task, verify your work by compiling the project (`cmake --build .`) and running the tests (`ctest` or `./scripts/run-tests.sh`) to ensure existing features aren't broken.
- **Subagents for Research:** If a task requires extensive reading of the codebase or searching the web, delegate the research phase to a background subagent.

## 7. UI Development & Modularity

- **Reuse, Modularity & Component Independence:** Reuse existing UI abstractions and internal components.
- **Focus:** New UI windows/widgets should be small, focused, and independent. Keep UI state separate from business logic.
- **Testability:** Separate core logic from ImGui calls as much as possible so the logic can be unit-tested without rendering.
