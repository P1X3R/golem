## [1.0] - 2026-01-07

### 🚀 Features

- Add gitignore
- Add attack look-up tables
- Add zobrist tables
- *(board)* Add fen parsing
- *(movegen)* Generate non-evasion moves (pseudo-legal)
- *(board)* Add move legality check
- *(board)* Add move making
- *(movegen)* Add en passant captures generation
- *(board)* Add efficient copy-make
- Add draw detection and history tracking; refactor move generation
- Add incremental evaluation function
- *(search,uci)* Implement alpha-beta search and UCI protocol
- *(search)* Add quiescence search and move ordering
- Add transposition tables
- Add PVS
- *(uci)* Add `setoption`

### 🐛 Bug Fixes

- *(board)* Update castling rights when a rook captures another rook
- Add color for ZOBRIST_PIECES

### 💼 Other

- Switch to clang and unify CFLAGS for C11

### 🚜 Refactor

- Create helper for random u64
- Fix all compiler warnings
- *(evaluation)* Improve PSQTs
- *(board, eval)* Replace int16_t with int for mg_score and eg_score
- *(core)* Rename rand.h to misc.h and update function signatures

### ⚡ Performance

- Enable LTO on release mode

### 🎨 Styling

- Move evaluation function definition to header file

### ⚙️ Miscellaneous Tasks

- Init
- Add .clang-format
- Add makefile
- Include .cache in .gitignore
- Add a readme
