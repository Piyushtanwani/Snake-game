#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <ncurses.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#endif

#include <vector>
#include <cstdlib>
#include <ctime>
#include <iostream>

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

#ifdef _WIN32
    void sleep_ms(int ms) { Sleep(ms); }
#else
    void sleep_ms(int ms) { usleep(ms * 1000); }
#endif

#ifndef _WIN32
    int kbhit(void) {
        struct termios oldt, newt;
        int ch;
        int oldf;
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
        ch = getchar();
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        fcntl(STDIN_FILENO, F_SETFL, oldf);
        if (ch != EOF) {
            ungetc(ch, stdin);
            return 1;
        }
        return 0;
    }
#endif

    void initGame() {
        srand(time(nullptr));
#ifdef _WIN32
        gameWidth = 50;
        gameHeight = 20;
        
        SetConsoleOutputCP(CP_UTF8);
        
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(hOut, &cursorInfo);
        cursorInfo.bVisible = false;
        SetConsoleCursorInfo(hOut, &cursorInfo);
        
        system("cls");
#else
        initscr();
        noecho();
        curs_set(0);
        keypad(stdscr, TRUE);
        nodelay(stdscr, TRUE);
        gameWidth = COLS - 2;
        gameHeight = LINES - 4;
#endif
        snake.clear();
        snake.push_back(Position(gameWidth / 2, gameHeight / 2));
        direction = Position(1, 0);
        score = 0;
        gameOver = false;
        generateFood();
    }

    void endGame() {
#ifdef _WIN32
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(hOut, &cursorInfo);
        cursorInfo.bVisible = true;
        SetConsoleCursorInfo(hOut, &cursorInfo);
#else
        nodelay(stdscr, FALSE);
        endwin();
#endif
        std::cout << "\nGame Over! Final Score: " << score << std::endl;
    }

    void generateFood() {
        do {
            food.x = rand() % gameWidth + 1;
            food.y = rand() % gameHeight + 1;
        } while (isSnakePosition(food));
    }

    bool isSnakePosition(const Position& pos) {
        for (const auto& s : snake) {
            if (s == pos) return true;
        }
        return false;
    }

    void draw() {
#ifdef _WIN32
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        COORD coord = {0, 0};
        SetConsoleCursorPosition(hOut, coord);
        
        std::string buffer;
        buffer.reserve((gameWidth + 3) * (gameHeight + 3));
        
        for (int y = 0; y < gameHeight + 2; ++y) {
            for (int x = 0; x < gameWidth + 2; ++x) {
                if (y == 0 && x == 0)
                    buffer += "┏";
                else if (y == 0 && x == gameWidth + 1)
                    buffer += "┓";
                else if (y == gameHeight + 1 && x == 0)
                    buffer += "┗";
                else if (y == gameHeight + 1 && x == gameWidth + 1)
                    buffer += "┛";
                else if (y == 0 || y == gameHeight + 1)
                    buffer += "━";
                else if (x == 0 || x == gameWidth + 1)
                    buffer += "┃";
                else if (x == food.x && y == food.y)
                    buffer += "*";
                else {
                    bool printed = false;
                    for (size_t i = 0; i < snake.size(); ++i) {
                        if (snake[i].x == x && snake[i].y == y) {
                            if (i == 0)
                                buffer += "O";
                            else
                                buffer += "o";
                            printed = true;
                            break;
                        }
                    }
                    if (!printed) buffer += " ";
                }
            }
            buffer += "\n";
        }
        
        buffer += "Score: " + std::to_string(score) + "  Use Arrow Keys or WASD to move, X to exit";
        
        DWORD written;
        WriteConsoleA(hOut, buffer.c_str(), buffer.length(), &written, NULL);
#else
        for (int y = 0; y < gameHeight + 2; ++y) {
            for (int x = 0; x < gameWidth + 2; ++x) {
                if (y == 0 && x == 0)
                    mvaddch(y, x, ACS_ULCORNER);
                else if (y == 0 && x == gameWidth + 1)
                    mvaddch(y, x, ACS_URCORNER);
                else if (y == gameHeight + 1 && x == 0)
                    mvaddch(y, x, ACS_LLCORNER);
                else if (y == gameHeight + 1 && x == gameWidth + 1)
                    mvaddch(y, x, ACS_LRCORNER);
                else if (y == 0 || y == gameHeight + 1)
                    mvaddch(y, x, ACS_HLINE);
                else if (x == 0 || x == gameWidth + 1)
                    mvaddch(y, x, ACS_VLINE);
                else if (x == food.x && y == food.y)
                    mvaddch(y, x, '*');
                else {
                    bool printed = false;
                    for (size_t i = 0; i < snake.size(); ++i) {
                        if (snake[i].x == x && snake[i].y == y) {
                            if (i == 0)
                                mvaddch(y, x, 'O');
                            else
                                mvaddch(y, x, 'o');
                            printed = true;
                            break;
                        }
                    }
                    if (!printed) mvaddch(y, x, ' ');
                }
            }
        }
        mvprintw(gameHeight + 3, 0, "Score: %d  ", score);
        refresh();
#endif
    }

    void input() {
#ifdef _WIN32
        if (_kbhit()) {
            int c = _getch();
            
            if (c == 0 || c == 224) {
                c = _getch(); 
                switch (c) {
                    case 72: 
                        if (direction.y != 1) direction = Position(0, -1);
                        break;
                    case 80: 
                        if (direction.y != -1) direction = Position(0, 1);
                        break;
                    case 75: 
                        if (direction.x != 1) direction = Position(-1, 0);
                        break;
                    case 77: 
                        if (direction.x != -1) direction = Position(1, 0);
                        break;
                }
            } else {
                switch (c) {
                    case 'w':
                    case 'W':
                        if (direction.y != 1) direction = Position(0, -1);
                        break;
                    case 's':
                    case 'S':
                        if (direction.y != -1) direction = Position(0, 1);
                        break;
                    case 'a':
                    case 'A':
                        if (direction.x != 1) direction = Position(-1, 0);
                        break;
                    case 'd':
                    case 'D':
                        if (direction.x != -1) direction = Position(1, 0);
                        break;
                    case 'x':
                    case 'X':
                        gameOver = true;
                        break;
                }
            }
        }
#else
        int c = getch();
        switch (c) {
            case KEY_UP:
            case 'w':
            case 'W':
                if (direction.y != 1) direction = Position(0, -1);
                break;
            case KEY_DOWN:
            case 's':
            case 'S':
                if (direction.y != -1) direction = Position(0, 1);
                break;
            case KEY_LEFT:
            case 'a':
            case 'A':
                if (direction.x != 1) direction = Position(-1, 0);
                break;
            case KEY_RIGHT:
            case 'd':
            case 'D':
                if (direction.x != -1) direction = Position(1, 0);
                break;
            case 'x':
            case 'X':
                gameOver = true;
                break;
        }
#endif
    }

    void logic() {
        Position newHead = snake.front();
        newHead.x += direction.x;
        newHead.y += direction.y;

        if (newHead.x <= 0 || newHead.x >= gameWidth + 1 || newHead.y <= 0 || newHead.y >= gameHeight + 1)
            gameOver = true;

        for (auto& s : snake) {
            if (s == newHead) gameOver = true;
        }

        snake.insert(snake.begin(), newHead);

        if (newHead == food) {
            score += 10;
            generateFood();
        } else {
            snake.pop_back();
        }
    }

public:
    void run() {
        initGame();
        while (!gameOver) {
            draw();
            input();
            logic();
            sleep_ms(100);
        }
        endGame();
    }
};

int main() {
    SnakeGame game;
    game.run();
    return 0;
}
