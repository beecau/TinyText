═══════════════════════════════════════════════════════════════
  CROSS-PLATFORM NOTEPAD - QUICK START GUIDE
═══════════════════════════════════════════════════════════════

📋 PROJECT STRUCTURE
├── notepad.c          (Main source code)
├── Makefile          (Simple build with Make)
├── CMakeLists.txt    (Build with CMake)
├── BUILD.md          (Detailed installation guide)
└── QUICK_START.txt   (This file)

═══════════════════════════════════════════════════════════════
🐧 LINUX (Ubuntu/Debian)
═══════════════════════════════════════════════════════════════

Install:
  sudo apt-get install build-essential libgtk-3-dev pkg-config

Build with Make:
  make
  ./notepad

Build with CMake:
  mkdir build && cd build
  cmake ..
  make
  ./notepad

Build manually:
  gcc -o notepad notepad.c $(pkg-config --cflags --libs gtk+-3.0)
  ./notepad

═══════════════════════════════════════════════════════════════
🍎 macOS (with Homebrew)
═══════════════════════════════════════════════════════════════

Install:
  brew install gtk+3 pkg-config

Build:
  make
  ./notepad

or CMake:
  mkdir build && cd build
  cmake ..
  make
  ./notepad

═══════════════════════════════════════════════════════════════
🪟 WINDOWS (MSYS2/MinGW64)
═══════════════════════════════════════════════════════════════

Install MSYS2: https://www.msys2.org/

In MSYS2 terminal:
  pacman -S mingw-w64-x86_64-toolchain mingw-w64-x86_64-gtk3

Build:
  make
  ./notepad.exe

═══════════════════════════════════════════════════════════════
✨ FEATURES
═══════════════════════════════════════════════════════════════

✓ File Management
  - New, Open, Save, Save As
  - Unsaved changes indicator
  - Safe exit with save prompt

✓ Text Editing
  - Cut, Copy, Paste
  - Select All
  - Find & Search
  - Line wrapping

✓ User Interface
  - Black & white minimalist design
  - Native window decorations
  - Menu bar (File, Edit, Help)
  - Monospace font
  - Status responsive to changes

✓ Cross-Platform
  - Compiles on Linux, macOS, Windows
  - Single C source file
  - No external dependencies except GTK+

═══════════════════════════════════════════════════════════════
🔧 TROUBLESHOOTING
═══════════════════════════════════════════════════════════════

If "gtk/gtk.h: No such file or directory":
  → Install libgtk-3-dev (Linux) or gtk+3 (macOS)

If "pkg-config: command not found":
  → Install pkg-config package for your OS

If build fails on Windows:
  → Make sure you're in MSYS2 terminal, not Command Prompt
  → Run: pacman -S mingw-w64-x86_64-toolchain

═══════════════════════════════════════════════════════════════
🚀 RUN IT
═══════════════════════════════════════════════════════════════

After building:
  ./notepad       (Linux/macOS)
  notepad.exe     (Windows)

Done! 🎉
