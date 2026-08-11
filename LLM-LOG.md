# LLM-LOG.md

## 1. Setup

| | |
|---|---|
| Model(s) | Claude Sonnet 5 (Anthropic) |
| Tool / harness | Claude.ai web chat |
| IDE / editor | VS Code |
| Pasted this assignment document into it? | Yes — the full lab assignment page (`lab_1_snake.html`) was uploaded at the start of the session, before any code was shared. |

## 2. The session

**Prompt 1.**
Uploaded `main.cpp` — the complete file assigned to our group (599 lines, one file, the whole program). Along with the file, we gave the model the specific list of functions we had already identified as needing to change: `Snake` (class), `GameBoard::update()`, `GameBoard::handleInput()`, `checkSelfCollision()`, `GameBoard::render()`, `main()`, and `ensureFoodNotOnSnake()`. This list came from our own reading of the code before this session, not from the model. Assignment Text as HTML file was also given.

**Prompt 2.**
"okay now create two versions withh divided work to keep modular structure of code" — resumed and triggered the actual code generation using the file and function list already provided in Prompt 1.

**Which attempt worked, and what was wrong with earlier ones:**
Prompt 2 was the only generation attempt. Both output files were compiled with `g++ -std=c++17 -Wall -c main.cpp` immediately after generation and produced zero warnings/errors on the first try. No revisions were needed.

**Total number of prompts it took to get working code:** 2 (one to supply the file + context and pause, one to trigger generation).

**What code we gave it:** The whole file — `main.cpp`, the complete, unmodified program as assigned to our group (599 lines). We did not need to hunt for which files to supply; there is only one source file in this repo. We supplemented it with our own manually pre-identified list of functions expected to change (see Prompt 1) rather than asking the model to find them from scratch.

## 3. What the model produced (key diffs, not full listing)

**Version 1 — core multiplayer engine:**
- `Snake` constructor parameterized (`Snake(int startX, int startY, Direction startDir)`) so two snakes can spawn at different positions/directions instead of both starting at grid center.
- `getArrowKey()` (both Windows and Unix branches) changed to return distinct codes (`'I'`,`'K'`,`'J'`,`'L'`) for arrow-key presses instead of reusing `'w'/'a'/'s'/'d'` — the original code mapped arrow keys to the same chars as the WASD letters, which would have made Player 1 (arrows) and Player 2 (WASD) indistinguishable in `handleInput()`. This function was not on our predicted list.
- `GameBoard` now holds `snake1`, `snake2`, `score1`, `score2`, and a `loser` flag instead of a single snake/score.
- `handleInput()` routes arrow-codes to `snake1` and literal WASD to `snake2`.
- `update()` moves both snakes, then checks wall/self/cross-snake collision for both using the existing `isOnPosition()` method (no new collision function needed), and food-eating priority (snake1 checked first on a tie).
- `ensureFoodNotOnSnake()`, `saveHighScore()`, `reset()` updated to account for two snakes/scores.
- `main()` changed `if (kbhit())` to `while (kbhit())` so both players' key presses register within the same tick.

**Version 2 — extends Version 1:**
- `render()` rewritten with a local lambda (`cellSymbol`) to give each snake a distinct symbol (🐍/🔵 for P1, 🟡/🟢 for P2) instead of drawing both snakes identically.
- Score line now shows both players' scores plus high score.
- Game-over screen now announces which player lost (or a draw, if both died on the same tick) instead of a generic message.
- Controls panel updated to show arrows → P1, WASD → P2.

## 4. Note on `checkSelfCollision()`
This was on our predicted list but ultimately required **no code change** — it only ever checks a snake against its own body, and that logic is unaffected by a second snake existing. Cross-snake collision was implemented separately in `update()` by reusing the already-public `isOnPosition()` method. This is flagged here since Part D asks whether the prediction matched reality.
