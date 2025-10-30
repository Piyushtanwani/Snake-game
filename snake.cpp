#include <ncurses.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <unistd.h>

class Position {
public:
    int x, y;
    Position(int x = 0, int y = 0) : x(x), y(y) {}
    bool operator==(const Position& other) const {
        return x == other.x && y == other.y;
    }
};

class SnakeGame {
private:
    std::vector<Position> snake;
    Position food;
    Position direction;
    int score;
    int gameWidth, gameHeight;
    bool gameOver;

    void initGame() {
        srand(time(nullptr));
        gameWidth = COLS - 2;
        gameHeight = LINES - 4;

        snake.clear();
        snake.push_back(Position(gameWidth / 2, gameHeight / 2));

        direction = Position(1, 0);
        score = 0;
        gameOver = false;

        generateFood();
    }

    void generateFood() {
        do {
            food.x = rand() % gameWidth + 1;
            food.y = rand() % gameHeight + 1;
        } while (isSnakePosition(food));
    }

    bool isSnakePosition(const Position& pos) {
        for (const auto& segment : snake) {
            if (segment == pos) return true;
        }
        return false;
    }

    void moveSnake() {
        Position newHead = snake[0];
        newHead.x += direction.x;
        newHead.y += direction.y;

        if (newHead.x < 1 || newHead.x > gameWidth ||
            newHead.y < 1 || newHead.y > gameHeight ||
            isSnakePosition(newHead)) {
            gameOver = true;
            return;
        }

        snake.insert(snake.begin(), newHead);

        if (newHead == food) {
            score += 10;
            generateFood();
        } else {
            snake.pop_back();
        }
    }

    void draw() {
        clear();

        box(stdscr, 0, 0);

        mvprintw(0, 2, " SNAKE GAME ");
        mvprintw(LINES - 1, 2, " Score: %d | Controls: WASD/Arrow Keys | Q to quit ", score);

        for (const auto& segment : snake) {
            mvaddch(segment.y, segment.x, '#');
        }

        mvaddch(food.y, food.x, '*');

        if (gameOver) {
            int msgY = LINES / 2;
            int msgX = (COLS - 20) / 2;
            mvprintw(msgY, msgX, "GAME OVER!");
            mvprintw(msgY + 1, msgX, "Final Score: %d", score);
            mvprintw(msgY + 2, msgX, "Press R to restart");
        }

        refresh();
    }

    void handleInput() {
        int ch = getch();

        switch (ch) {
            case 'w': case 'W': case KEY_UP:
                if (direction.y != 1) direction = Position(0, -1);
                break;
            case 's': case 'S': case KEY_DOWN:
                if (direction.y != -1) direction = Position(0, 1);
                break;
            case 'a': case 'A': case KEY_LEFT:
                if (direction.x != 1) direction = Position(-1, 0);
                break;
            case 'd': case 'D': case KEY_RIGHT:
                if (direction.x != -1) direction = Position(1, 0);
                break;
            case 'q': case 'Q':
                gameOver = true;
                break;
            case 'r': case 'R':
                if (gameOver) {
                    initGame();
                }
                break;
        }
    }

public:
    SnakeGame() {
        initscr();
        cbreak();
        noecho();
        nodelay(stdscr, TRUE);
        keypad(stdscr, TRUE);
        curs_set(0);

        if (LINES < 10 || COLS < 30) {
            endwin();
            printf("Terminal too small! Minimum size: 30x10\n");
            exit(1);
        }

        initGame();
    }

    ~SnakeGame() {
        endwin();
    }

    void run() {
        while (true) {
            handleInput();

            if (!gameOver) {
                moveSnake();
            }

            draw();

            if (gameOver && getch() == 'q') {
                break;
            }

            usleep(150000);
        }
    }
};

int main() {
    try {
        SnakeGame game;
        game.run();
    } catch (const std::exception& e) {
        endwin();
        printf("Error: %s\n", e.what());
        return 1;
    }

    return 0;
}