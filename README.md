# Monster Game

> A Pokémon-inspired monster catching and battling game built from scratch in **C99** using the **SDL2** library — no game engine, no shortcuts.

> **This project is in very early development and is not yet playable in a meaningful way. Expect rough edges.**

---

## About

Monster Game is a personal solo project inspired by the Pokémon series. The goal is to build a game where you can explore a world, encounter monsters, catch them, and use them in turn-based battles — all built from the ground up in C with SDL2 as the only graphical backbone.

This is primarily a learning project: an opportunity to go deep on C, SDL2, game architecture, and everything that goes into a game without the safety net of an engine. Because it's a one-person effort developed around a university schedule, progress is incremental and the scope is intentionally modest compared to a commercial title.

---

## Features (Planned / In Progress)

- **World exploration** — tile-based map movement
- **Monster encounters** — find and catch monsters in the wild
- **Turn-based battles** — use your monsters to fight
- **Data-driven design** — game data loaded from external files

---

## Tech Stack

| | |
|---|---|
| **Language** | C99 |
| **Graphics / Input / Audio** | SDL2, SDL2\_image, SDL2\_ttf, SDL2\_mixer |
| **Build system** | GNU Make |
| **Platform** | Linux (primary) |

---

## Project Structure

```
monster_game/
├── src/           # All C source and header files
├── data/          # Game data files (maps, monster definitions, etc.)
├── resources/     # Sprites, fonts, audio assets
├── map_file.bin   # Map layout (custom binary format, subject to change)
└── Makefile
```

---

## Building & Running

### Prerequisites

Install the SDL2 dependencies (Ubuntu/Debian):

```bash
sudo apt-get install build-essential libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev libsdl2-mixer-dev
```

### Compile

```bash
make
```

The Makefile is being set up to statically link all SDL2 dependencies into the executable, so the final binary should be self-contained and runnable on any compatible system without requiring SDL2 to be installed separately.

### Run

```bash
./main
```

---

## Development Status

This project is **very early in development**. It is a solo side project built around a university schedule, so updates happen in bursts rather than continuously. The current codebase is exploratory — expect things to change, break, or be missing entirely.

There is no roadmap or release timeline. The scope is deliberately smaller than a full Pokémon-style game, as this is being built entirely without a game engine by a single developer.

---

## Motivation

> *"Building a game without an engine is the best way to understand what an engine actually does for you."*

This project exists to learn — C, SDL2, memory management, game loops, rendering, and everything else you normally take for granted.

---

## License

No license has been specified yet. All rights reserved by the author unless stated otherwise.