# Contributing to Sticky Paws

Thank you for your interest in contributing to Sticky Paws! This document provides guidelines for contributing to the project.

## Code Standards

### Formatting

We use `clang-format` to maintain consistent code formatting. Before submitting any code:

```bash
make format
```

Configuration: `.clang-format`
- Based on LLVM style
- 2-space indentation
- 100 character line limit
- Linux brace style

### Static Analysis

We use `clang-tidy` for static analysis. Configuration is in `.clang-tidy`.

Run linting:
```bash
make lint
```

### Naming Conventions

- **Structs/Types**: `PascalCase` (e.g., `GameContext`, `Bear`)
- **Functions**: `camelCase` (e.g., `handleSpeed`, `drawBear`)
- **Variables**: `camelBack` (e.g., `totalSpeed`, `mouseDelta`)
- **Constants**: `UPPER_CASE` (e.g., `TOTAL_SPEED_MAX`, `WIDTH`)
- **Macros**: `UPPER_CASE` (e.g., `MAX_SOUNDS`, `FADE_SPEED`)

### Documentation

- Add file header comments to all new source files
- Document all public functions with Doxygen-style comments
- Include `@brief`, `@param`, and `@return` tags
- Comment complex logic inline

Example:
```c
/**
 * @brief Handle sticky collision between paw and honey jar
 * @param ctx Game context
 * @param paw Bear paw
 * @param jar Target honey jar
 * @param sb Sound buffer array
 */
void handleStickyJar(GameContext *ctx, Bear *paw, Target *jar, Sound sb[]);
```

### Best Practices

1. **Const Correctness**: Use `const` for read-only parameters
2. **Named Constants**: Replace magic numbers with named constants
3. **Error Handling**: Validate resources and provide informative error messages
4. **Code Clarity**: Prefer explicit if-else over complex ternary operations
5. **Memory Safety**: Always check bounds and validate pointers
6. **Minimal Scope**: Declare variables in the smallest scope needed

### Code Organization

```
src/
├── main.c      - Entry point, game loop, state management
├── game.c/h    - Core game logic, physics, collisions
├── ui.c/h      - User interface rendering
├── sound.c/h   - Audio management
└── assets/     - Game resources (textures, sounds)
```

## Building

### Prerequisites

- CMake 3.25+
- C compiler (Clang, GCC, or MSVC)
- Git

### Build Commands

```bash
# Configure
cmake -S . -B build

# Build
cmake --build build

# Run
./build/sticky-paws/sticky-paws
```

Or using make:
```bash
make build_osx   # macOS
make build_linux # Linux
make run         # Build and run (macOS)
```

## Testing

Currently, the project does not have automated tests. Manual testing checklist:

- [ ] Game starts without errors
- [ ] All sounds load successfully
- [ ] Mouse controls paw smoothly
- [ ] Objects stick to paw on collision
- [ ] Timer counts down correctly
- [ ] Win condition works (honey to bottom)
- [ ] Fail conditions work (timer runs out, too fast)
- [ ] Fullscreen toggle works (SPACE)
- [ ] Debug mode toggle works (TAB)

## Submitting Changes

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Make your changes
4. Format your code (`make format`)
5. Test your changes thoroughly
6. Commit your changes (`git commit -m 'Add amazing feature'`)
7. Push to the branch (`git push origin feature/amazing-feature`)
8. Open a Pull Request

### Commit Message Guidelines

- Use present tense ("Add feature" not "Added feature")
- Use imperative mood ("Move cursor to..." not "Moves cursor to...")
- First line should be 50 characters or less
- Reference issues and pull requests after the first line

## Questions?

Feel free to open an issue for:
- Bug reports
- Feature requests
- Questions about the codebase
- Suggestions for improvement

## License

By contributing to Sticky Paws, you agree that your contributions will be licensed under the same license as the project.
