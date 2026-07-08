# Implementation Plan: HTML Embedded Language Support

## Overview

We need to support embedded languages (CSS and JavaScript) inside HTML files within `fast-notepad`. Currently, `ImGuiColorTextEdit` evaluates a single `LanguageDefinition` for an entire file. To properly highlight `<style>` and `<script>` blocks, we need to introduce a generic "SubLanguage" mechanism to `ImGuiColorTextEdit` that allows regions of text to be evaluated by different `LanguageDefinition` instances, and then configure `fast-notepad`'s HTML parser to utilize this new mechanism.

## Architecture Decisions

- **SubLanguage Support in `ImGuiColorTextEdit`**: We will extend `LanguageDefinition` with a `mSubLanguages` vector. Each sub-language will define a start string/regex, an end string/regex, and a pointer to the target `LanguageDefinition`.
- **Character-level Language Tracking**: We will add a `uint8_t mLanguageIndex` to the `TextEditor::Glyph` struct. During `ColorizeInternal` (which already scans the file top-to-bottom to resolve multi-line comments), we will also track and assign the active sub-language index to every character.
- **Segmented Tokenization**: In `ColorizeRange`, instead of applying the main language's regexes to the entire line at once, we will partition the line into segments based on `mLanguageIndex` and apply the corresponding `LanguageDefinition` to each segment. This is generic, reusable, and prevents HTML regexes from bleeding into CSS blocks.

## Task List

### Phase 1: Foundation (ImGuiColorTextEdit Library)

- [x] **Task 1: Extend Data Structures**
  - **Description**: Add the `SubLanguage` struct to `LanguageDefinition` and `mLanguageIndex` to `TextEditor::Glyph`.
  - **Acceptance criteria**:
    - `LanguageDefinition` contains a way to define sub-languages.
    - `Glyph` size is kept compact while including `mLanguageIndex` (defaulting to 0).
  - **Verification**: Ensure `ImGuiColorTextEdit` compiles without errors.
  - **Estimated scope**: Small (1 file)

- [x] **Task 2: Track Language State in `ColorizeInternal`**
  - **Description**: Update the character-by-character scan in `ColorizeInternal` to detect start/end markers of sub-languages and assign the correct `mLanguageIndex` to each `Glyph`.
  - **Acceptance criteria**:
    - When a start marker is found, subsequent characters receive the new language index.
    - When an end marker is found, the state reverts to the parent language (index 0).
  - **Verification**: Run unit tests or a manual debug session to ensure `mLanguageIndex` is set correctly across multiple lines.
  - **Estimated scope**: Medium (1 file)

- [x] **Task 3: Segmented Regex Application in `ColorizeRange`**
  - **Description**: Modify `ColorizeRange` to group contiguous glyphs with the same `mLanguageIndex` and apply the correct `LanguageDefinition`'s regexes and keywords to that segment.
  - **Acceptance criteria**:
    - Regex tokenization respects segment boundaries (CSS regexes won't match HTML outside the segment).
    - Keyword coloring uses the correct dictionary for the segment.
  - **Verification**: Compile and link successfully.
  - **Estimated scope**: Medium (1 file)

### Checkpoint: Foundation

- [x] `ImGuiColorTextEdit` compiles cleanly.
- [x] Existing `fast-notepad` features (Markdown, C++, etc.) are unaffected and tests pass.

### Phase 2: Core Features (fast-notepad integration)

- [x] **Task 4: Implement CSS and JS Definitions**
  - **Description**: Create `GetCssDefinition()` and `GetJsDefinition()` in `ThemeManager` with strict, standalone regexes and keywords for each language.
  - **Acceptance criteria**:
    - CSS definition correctly highlights properties, classes, and IDs.
    - JS definition correctly highlights keywords and standard punctuation.
  - **Verification**: Can be temporarily assigned as the main language to verify correctness.
  - **Estimated scope**: Small (2 files: `ThemeManager.h/cpp`)

- [x] **Task 5: Link SubLanguages in HTML Definition**
  - **Description**: Revert the temporary merged HTML parser in `GetHtmlDefinition()` to a strict HTML parser, and link the CSS and JS definitions as sub-languages.
  - **Acceptance criteria**:
    - `<style>` blocks trigger CSS highlighting.
    - `<script>` blocks trigger JS highlighting.
  - **Verification**: Open an HTML file in `fast-notepad` containing both CSS and JS blocks and visually confirm perfect highlighting separation.
  - **Estimated scope**: Small (1 file)

### Checkpoint: Complete

- [x] Manual check of `test.html` with complex nested blocks.
- [x] All Catch2 tests pass.
- [x] Ready for PR to fork and merge to main application.

## Risks and Mitigations

| Risk              | Impact | Mitigation                                                                                                                                                                |
| ----------------- | ------ | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Regex Performance | Medium | Sub-language boundaries evaluated line-by-line using basic string matching instead of expensive regex if possible, minimizing performance overhead in `ColorizeInternal`. |
| Memory Bloat      | Low    | `Glyph` struct alignment usually has padding. Adding `uint8_t` will likely not increase the struct size, keeping memory allocations flat.                                 |

## Open Questions

- Should the `SubLanguage` start/end markers be strict strings (e.g. `<style>`) or Regexes (e.g. `<style.*?>`)? Regexes are more accurate for HTML attributes, but string matching is faster. I recommend Regex for start markers and strict strings for end markers (`</style>`).
