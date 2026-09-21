# Lab 4 — Group A__

| | |
|---|---|
| Repository | `Piyushtanwani/Snake-game` |
| Base tag | `lab4-base` at commit `6068bd8` |
| Pull request | Pending from `lab4` → `main`: `https://github.com/Piyushtanwani/Snake-game/compare/main...lab4?expand=1` |

---

## 1. Five rules — [5]

Written before opening the source. Behaviour, with an observable outcome.

| # | Rule |
|---|---|
| 1 | Moving advances the snake by exactly one cell in its current direction. |
| 2 | Eating one fruit increases the score by exactly ten points. |
| 3 | Eating fruit increases the snake's length. |
| 4 | The game ends when the snake's head enters a cell occupied by its body. |
| 5 | Fruit never appears on a cell occupied by the snake. |

These rules were stated from the expected behaviour of the game before inspecting its source. My initial draft guessed that one fruit added exactly one point; after inspection I corrected the amount to ten rather than leaving a known false claim in the report.

---

## 2. What you could test, and what stopped you — [10]

No source changes in this part. Every `file:line` below is a line in `lab4-base`.

| # | Rule | Test written? | Blocking dependency (`file:line` + what it is) |
|---|---|---|---|
| 1 | Moving advances the snake by exactly one cell in its current direction. | Yes — `lab4/tests.cpp:testMoveAdvancesOneCell` | None; `main.cpp:239` exposes `move()` and `main.cpp:260` exposes the head. |
| 2 | Eating one fruit increases the score by exactly ten points. | No | `main.cpp:283` — `score` is private with no observer; `main.cpp:207-208` also choose the food position through global `rand()`. |
| 3 | Eating fruit increases the snake's length. | No | `main.cpp:281-282` — `GameBoard` privately owns both `Snake` and `Food`; the inaccessible eat path at `main.cpp:349-350` is what calls `grow()`. Calling public `Snake::grow()` would test the primitive, not eating. |
| 4 | The game ends when the snake's head enters a cell occupied by its body. | No | `main.cpp:281` — `GameBoard` owns a private `Snake`, so a test cannot arrange a self-colliding board state before the decision at `main.cpp:343`. |
| 5 | Fruit never appears on a cell occupied by the snake. | No | `main.cpp:282` — `GameBoard` owns private food and exposes no position observer for the guard at `main.cpp:310`. |

> **Rules testable without modifying the source: 1 / 5**

Only movement is verified through the original public API. I did not count a direct `Snake::grow()` check as a test of eating, and I did not count `testGameStartsPlayable` as one of the five rule tests: it is a coverage probe whose assertion checks only that the game starts.

---

## 3. Coverage, and what it missed — [6]

| | |
|---|---|
| Line coverage | 52.34% of `main.cpp` |
| Branch coverage | 57.67% executed; 34.19% taken at least once |
| Command used | `g++ --coverage -O0 -g lab4/tests.cpp -o tests && ./tests && gcov -b tests.gcno` |
| Toolchain | WSL Ubuntu, `g++`/`gcov` 13.3.0 |

**One rule that is executed by the suite but not verified by it:**

| | |
|---|---|
| Rule | Fruit never appears on a cell occupied by the snake. |
| Line that runs | `main.cpp:332` — the `ensureFoodNotOnSnake` loop condition (`main.cpp:310` in `lab4-base`) |
| The assertion that is missing | An assertion comparing the resulting fruit position with every snake segment. |

These figures are a fresh run of the exact suite committed on the final `lab4` branch, after Part D, so the command is reproducible from the submitted files. `testGameStartsPlayable` constructs a board, so the fruit-overlap decision contributes to coverage; its only assertion is `!board.isGameOver()`, which says nothing about fruit placement. This is execution without verification.

---

## 4. The seam — [10]

| | |
|---|---|
| Rule made testable | Eating one fruit increases the score by exactly ten points. |
| Commit 1 (seam) | `0bc0b37` — Add injectable random source seam |
| Commit 2 (test) | Test score increase through random seam |
| Seam kind | object |
| Enabling point | `GameBoard` accepts a `RandomSource&`, and `Food::spawn()` asks it for each coordinate. The test observes the existing public `render()` output. |
| What production code gave up | Exclusive control over choosing food coordinates. Production still supplies the default `rand()` adapter, so normal play keeps the same coordinate algorithm and call order. |

I chose the score rule as the hardest for me because a test had to place food on the snake's next cell despite private board state and global randomness, then observe the score without a score accessor. Self-collision was also blocked at board level, but its lower-level collision predicate already had a public query; the eat-and-score decision had neither controllable input nor a direct observer.

The default constructor still resolves to `CStdlibRandomSource`, whose implementation is the original `rand() % GRID_SIZE`. Building the game after commit 1 produced no regression, and no score getter was added solely for testing.

---

## 5. The double — [4]

| | |
|---|---|
| What you passed through the seam | stub |
| The method under test | `GameBoard::update()` |

The game asks the collaborator a question — `next(20)` — and uses the returned coordinate. The double is therefore a stub: it supplies predetermined answers (`11, 10, 0, 0`), and the test asserts `Score: 10` in the existing rendered output rather than asserting that calls occurred.

---

## 6. Two smells in your own tests — [5]

| | Smell | `file:line` | One-line fix |
|---|---|---|---|
| 1 | General fixture | `lab4/tests.cpp:13` | Return each test's failures to the runner instead of sharing the mutable global `failures` counter. |
| 2 | Mystery guest | `lab4/tests.cpp:56` | Inject high-score persistence or create an explicit temporary score file for board tests. |

The global counter couples every test through shared state. Constructing `GameBoard` also reads `highscore.txt` internally, so the test does filesystem work that is invisible at its call site even though this assertion does not need persistence.
