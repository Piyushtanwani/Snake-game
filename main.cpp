#include <iostream>
#include <deque>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <sstream>

// Platform-specific includes
#ifdef _WIN32
    #include <windows.h>
    #include <conio.h>
#else
    #include <termios.h>
    #include <unistd.h>
    #include <sys/ioctl.h>
#endif

using namespace std;

// Constants
const int GRID_SIZE = 20;
const int INITIAL_SNAKE_LENGTH = 3;

// Direction enumeration
enum Direction { UP, DOWN, LEFT, RIGHT };

// Position structure
struct Position {
    int x, y;
    Position(int _x = 0, int _y = 0) : x(_x), y(_y) {}
    bool operator==(const Position& other) const {
        return x == other.x && y == other.y;
    }
};

// Platform-specific terminal control functions
#ifdef _WIN32
    void clearScreen() {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        COORD coordScreen = {0, 0};
        DWORD cCharsWritten;
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        DWORD dwConSize;
        
        GetConsoleScreenBufferInfo(hConsole, &csbi);
        dwConSize = csbi.dwSize.X * csbi.dwSize.Y;
        
        FillConsoleOutputCharacter(hConsole, (TCHAR)' ', dwConSize, coordScreen, &cCharsWritten);
        GetConsoleScreenBufferInfo(hConsole, &csbi);
        FillConsoleOutputAttribute(hConsole, csbi.wAttributes, dwConSize, coordScreen, &cCharsWritten);
        SetConsoleCursorPosition(hConsole, coordScreen);
    }
    
    void moveCursorToHome() {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        COORD coordScreen = {0, 0};
        SetConsoleCursorPosition(hConsole, coordScreen);
    }
    
    void hideCursor() {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(hConsole, &cursorInfo);
        cursorInfo.bVisible = FALSE;
        SetConsoleCursorInfo(hConsole, &cursorInfo);
    }
    
    void showCursor() {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(hConsole, &cursorInfo);
        cursorInfo.bVisible = TRUE;
        SetConsoleCursorInfo(hConsole, &cursorInfo);
    }
    
    int kbhit() {
        return _kbhit();
    }
    
    char getCharacter() {
        return _getch();
    }
    
    char getArrowKey() {
        char c = getCharacter();
        
        // Arrow keys in Windows send 224 (0xE0) or 0 followed by key code
        if (c == 0 || c == -32) {  // 224 as signed char is -32
            c = _getch();
            switch (c) {
                case 72: return 'I';  // Up arrow
                case 80: return 'K';  // Down arrow
                case 77: return 'L';  // Right arrow
                case 75: return 'J';  // Left arrow
            }
        }
        
        return c;
    }
    
    void sleepMs(int ms) {
        Sleep(ms);
    }
    
    void setupConsole() {
        // Set UTF-8 code page for emojis
        SetConsoleOutputCP(CP_UTF8);
        hideCursor();
    }
    
    void cleanupConsole() {
        showCursor();
        clearScreen();
    }
#else
    struct termios orig_termios;
    
    void disableRawMode() {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
    }
    
    void enableRawMode() {
        tcgetattr(STDIN_FILENO, &orig_termios);
        atexit(disableRawMode);
        
        struct termios raw = orig_termios;
        raw.c_lflag &= ~(ECHO | ICANON);
        raw.c_cc[VMIN] = 0;
        raw.c_cc[VTIME] = 0;
        
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    }
    
    void clearScreen() {
        cout << "\033[2J\033[H";
    }
    
    void moveCursorToHome() {
        cout << "\033[H";
    }
    
    void hideCursor() {
        cout << "\033[?25l";
    }
    
    void showCursor() {
        cout << "\033[?25h";
    }
    
    int kbhit() {
        int bytesWaiting;
        ioctl(STDIN_FILENO, FIONREAD, &bytesWaiting);
        return bytesWaiting;
    }
    
    char getCharacter() {
        char c;
        if (read(STDIN_FILENO, &c, 1) == 1) return c;
        return 0;
    }
    
    char getArrowKey() {
        char c = getCharacter();
        
        if (c == 27) {  // ESC
            char seq[2];
            
            if (read(STDIN_FILENO, &seq[0], 1) != 1) return c;
            if (read(STDIN_FILENO, &seq[1], 1) != 1) return c;
            
            if (seq[0] == '[') {
                switch (seq[1]) {
                    case 'A': return 'I';  // Up arrow
                    case 'B': return 'K';  // Down arrow
                    case 'C': return 'L';  // Right arrow
                    case 'D': return 'J';  // Left arrow
                }
            }
        }
        
        return c;
    }
    
    void sleepMs(int ms) {
        usleep(ms * 1000);
    }
    
    void setupConsole() {
        enableRawMode();
        hideCursor();
    }
    
    void cleanupConsole() {
        showCursor();
        clearScreen();
    }
#endif

// Food Class
class Food {
private:
    Position pos;
public:
    Food() { spawn(); }
    
    void spawn() {
        pos.x = rand() % GRID_SIZE;
        pos.y = rand() % GRID_SIZE;
    }
    
    Position getPosition() const { return pos; }
};

// Snake Class
class Snake {
private:
    deque<Position> body;
    Direction dir;
    bool growing;
    bool alive;
    
public:
    Snake(int startX = GRID_SIZE / 2, int startY = GRID_SIZE / 2, Direction startDir = RIGHT)
        : dir(startDir), growing(false), alive(true) {
        for (int i = 0; i < INITIAL_SNAKE_LENGTH; i++) {
            switch (startDir) {
                case RIGHT: body.push_back(Position(startX - i, startY)); break;
                case LEFT:  body.push_back(Position(startX + i, startY)); break;
                case DOWN:  body.push_back(Position(startX, startY - i)); break;
                case UP:    body.push_back(Position(startX, startY + i)); break;
            }
        }
    }
    
    void setDirection(Direction newDir) {
        if ((dir == UP && newDir != DOWN) ||
            (dir == DOWN && newDir != UP) ||
            (dir == LEFT && newDir != RIGHT) ||
            (dir == RIGHT && newDir != LEFT)) {
            dir = newDir;
        }
    }
    
    void move() {
        Position head = body.front();
        
        switch (dir) {
            case UP:    head.y--; break;
            case DOWN:  head.y++; break;
            case LEFT:  head.x--; break;
            case RIGHT: head.x++; break;
        }
        
        body.push_front(head);
        
        if (!growing) {
            body.pop_back();
        } else {
            growing = false;
        }
    }
    
    void grow() { growing = true; }
    
    Position getHead() const { return body.front(); }
    
    bool checkSelfCollision() const {
        Position head = body.front();
        for (size_t i = 1; i < body.size(); i++) {
            if (body[i] == head) return true;
        }
        return false;
    }
    
    bool isOnPosition(const Position& pos) const {
        for (const auto& segment : body) {
            if (segment == pos) return true;
        }
        return false;
    }

    bool isAlive() const { return alive; }
    void kill() { alive = false; }
};

// GameBoard Class
class GameBoard {
private:
    Snake snake1;
    Snake snake2;
    Food food;
    int score1;
    int score2;
    int highScore;
    bool gameOver;
    int loser; // 0 = none, 1 = player1, 2 = player2, 3 = both
    mutable bool firstRender;
    
    void loadHighScore() {
        ifstream file("highscore.txt");
        if (file.is_open()) {
            file >> highScore;
            file.close();
        } else {
            highScore = 0;
        }
    }
    
    void saveHighScore() {
        int bestScore = (score1 > score2) ? score1 : score2;
        if (bestScore > highScore) {
            highScore = bestScore;
            ofstream file("highscore.txt");
            if (file.is_open()) {
                file << highScore;
                file.close();
            }
        }
    }
    
    void ensureFoodNotOnSnake() {
        while (snake1.isOnPosition(food.getPosition()) || snake2.isOnPosition(food.getPosition())) {
            food.spawn();
        }
    }
    
public:
    GameBoard()
        : snake1(GRID_SIZE / 2 - 4, GRID_SIZE / 2, RIGHT),
          snake2(GRID_SIZE / 2 + 4, GRID_SIZE / 2, LEFT),
          score1(0), score2(0), gameOver(false), loser(0), firstRender(true) {
        loadHighScore();
        ensureFoodNotOnSnake();
    }
    
    void handleInput(char input) {
        switch (input) {
            case 'I': snake1.setDirection(UP); break;
            case 'K': snake1.setDirection(DOWN); break;
            case 'J': snake1.setDirection(LEFT); break;
            case 'L': snake1.setDirection(RIGHT); break;
            case 'w': case 'W': snake2.setDirection(UP); break;
            case 's': case 'S': snake2.setDirection(DOWN); break;
            case 'a': case 'A': snake2.setDirection(LEFT); break;
            case 'd': case 'D': snake2.setDirection(RIGHT); break;
        }
    }
    
    void update() {
        if (gameOver) return;
        
        snake1.move();
        snake2.move();

        Position head1 = snake1.getHead();
        Position head2 = snake2.getHead();

        bool snake1Dead = false;
        bool snake2Dead = false;

        if (head1.x < 0 || head1.x >= GRID_SIZE || head1.y < 0 || head1.y >= GRID_SIZE) {
            snake1Dead = true;
        }
        if (head2.x < 0 || head2.x >= GRID_SIZE || head2.y < 0 || head2.y >= GRID_SIZE) {
            snake2Dead = true;
        }

        if (snake1.checkSelfCollision()) snake1Dead = true;
        if (snake2.checkSelfCollision()) snake2Dead = true;

        if (snake2.isOnPosition(head1)) snake1Dead = true;
        if (snake1.isOnPosition(head2)) snake2Dead = true;

        if (snake1Dead || snake2Dead) {
            gameOver = true;
            if (snake1Dead) snake1.kill();
            if (snake2Dead) snake2.kill();
            if (snake1Dead && snake2Dead) loser = 3;
            else if (snake1Dead) loser = 1;
            else loser = 2;
            saveHighScore();
            return;
        }
        
        if (head1 == food.getPosition()) {
            snake1.grow();
            score1 += 10;
            food.spawn();
            ensureFoodNotOnSnake();
        } else if (head2 == food.getPosition()) {
            snake2.grow();
            score2 += 10;
            food.spawn();
            ensureFoodNotOnSnake();
        }
    }
    
    void render() const {
        // Build complete frame in buffer
        ostringstream buffer;
        
        if (gameOver) {
            // When game over, clear screen completely and show only game over screen
            if (firstRender || true) {  // Always clear on game over
                clearScreen();
                firstRender = false;
            }
           #ifdef _WIN32
                system("cls");
            #else
                system("clear");
            #endif
            buffer << "\n\n\n\n\n\n\n\n\n";
            buffer << "    ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓\n";
            buffer << "    ┃  ▓▒░   💥 G A M E   O V E R 💥   ░▒▓   ┃\n";
            buffer << "    ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛\n";
            buffer << "                                            \n";
            buffer << "          P1 Score: " << score1 << "   P2 Score: " << score2 << "        \n";
            buffer << "          ⭐ High Score:  " << highScore << "                 \n";
            buffer << "                                             \n";
            buffer << "    ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓\n";
            buffer << "    ┃                                        ┃\n";
            buffer << "    ┃     🔄 R - Restart    ❌ Q - Quit      ┃\n";
            buffer << "    ┃                                        ┃\n";
            buffer << "    ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛\n";
            buffer << "\n\n\n\n\n\n\n\n\n";
        } else {
            if (firstRender) {
                clearScreen();
                firstRender = false;
            }
            moveCursorToHome();
            
            // Show normal game board
            buffer << "\n    ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓\n";
            buffer << "    ┃         ** SNAKE GAME **              ┃\n";
            buffer << "    ┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫\n";
            buffer << "       P1: " << score1 << "  |  P2: " << score2 << "  |  High: " << highScore << "     \n";
            buffer << "    ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛\n\n";
            
            // Game board with controls side by side
            buffer << "    ┏";
            for (int i = 0; i < GRID_SIZE * 2; i++) buffer << "━";
            buffer << "┓    ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓\n";
            
            // First row with controls header
            buffer << "    ┃";
            for (int x = 0; x < GRID_SIZE; x++) {
                Position current(x, 0);
                if (snake1.getHead() == current || snake2.getHead() == current) {
                    buffer << "🐍";
                } else if (snake1.isOnPosition(current) || snake2.isOnPosition(current)) {
                    buffer << "🔵";
                } else if (food.getPosition() == current) {
                    buffer << "🍎";
                } else {
                    buffer << "▒▒";
                }
            }
            buffer << "┃    ┃         🎮 CONTROLS           ┃\n";
            
            // Second row
            buffer << "    ┃";
            for (int x = 0; x < GRID_SIZE; x++) {
                Position current(x, 1);
                if (snake1.getHead() == current || snake2.getHead() == current) {
                    buffer << "🐍";
                } else if (snake1.isOnPosition(current) || snake2.isOnPosition(current)) {
                    buffer << "🔵";
                } else if (food.getPosition() == current) {
                    buffer << "🍎";
                } else {
                    buffer << "▒▒";
                }
            }
            buffer << "┃    ┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫\n";
            
            // Rows 3-8 (showing keyboard)
            for (int y = 2; y < 8; y++) {
                buffer << "    ┃";
                for (int x = 0; x < GRID_SIZE; x++) {
                    Position current(x, y);
                    if (snake1.getHead() == current || snake2.getHead() == current) {
                        buffer << "🐍";
                    } else if (snake1.isOnPosition(current) || snake2.isOnPosition(current)) {
                        buffer << "🔵";
                    } else if (food.getPosition() == current) {
                        buffer << "🍎";
                    } else {
                        buffer << "▒▒";
                    }
                }
                buffer << "┃    ┃";
                
                // Add control text based on row
                switch(y) {
                    case 2: buffer << "                               ┃\n"; break;
                    case 3: buffer << "    ┌───┐            ┌───┐     ┃\n"; break;
                    case 4: buffer << "    │ W │            │ ↑ │     ┃\n"; break;
                    case 5: buffer << " ┌──┼───┼──┐      ┌──┼───┼──┐  ┃\n"; break;
                    case 6: buffer << " │A │ S │ D│      │← │ ↓ │ →│  ┃\n"; break;
                    case 7: buffer << " └──┴───┴──┘      └──┴───┴──┘  ┃\n"; break;
                }
            }
            
            // Row 9
            buffer << "    ┃";
            for (int x = 0; x < GRID_SIZE; x++) {
                Position current(x, 8);
                if (snake1.getHead() == current || snake2.getHead() == current) {
                    buffer << "🐍";
                } else if (snake1.isOnPosition(current) || snake2.isOnPosition(current)) {
                    buffer << "🔵";
                } else if (food.getPosition() == current) {
                    buffer << "🍎";
                } else {
                    buffer << "▒▒";
                }
            }
            buffer << "┃    ┃                               ┃\n";
            
            // Row 10
            buffer << "    ┃";
            for (int x = 0; x < GRID_SIZE; x++) {
                Position current(x, 9);
                if (snake1.getHead() == current || snake2.getHead() == current) {
                    buffer << "🐍";
                } else if (snake1.isOnPosition(current) || snake2.isOnPosition(current)) {
                    buffer << "🔵";
                } else if (food.getPosition() == current) {
                    buffer << "🍎";
                } else {
                    buffer << "▒▒";
                }
            }
            buffer << "┃    ┃                               ┃\n";
            
            // Remaining rows (11-19)
            for (int y = 10; y < GRID_SIZE - 1; y++) {
                buffer << "    ┃";
                for (int x = 0; x < GRID_SIZE; x++) {
                    Position current(x, y);
                    if (snake1.getHead() == current || snake2.getHead() == current) {
                        buffer << "🐍";
                    } else if (snake1.isOnPosition(current) || snake2.isOnPosition(current)) {
                        buffer << "🔵";
                    } else if (food.getPosition() == current) {
                        buffer << "🍎";
                    } else {
                        buffer << "▒▒";
                    }
                }
                buffer << "┃    ┃";
                
                if (y == 10) buffer << " ❌ Q - Quit                   ┃\n";
                else if (y == 11) buffer << " 🔄 R - Restart                ┃\n";
                else buffer << "                               ┃\n";
            }
            
            // Last row
            buffer << "    ┃";
            for (int x = 0; x < GRID_SIZE; x++) {
                Position current(x, GRID_SIZE - 1);
                if (snake1.getHead() == current || snake2.getHead() == current) {
                    buffer << "🐍";
                } else if (snake1.isOnPosition(current) || snake2.isOnPosition(current)) {
                    buffer << "🔵";
                } else if (food.getPosition() == current) {
                    buffer << "🍎";
                } else {
                    buffer << "▒▒";
                }
            }
            buffer << "┃    ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛\n";
            
            buffer << "    ┗";
            for (int i = 0; i < GRID_SIZE * 2; i++) buffer << "━";
            buffer << "┛\n";
        }
        
        // Single write operation
        cout << buffer.str();
        cout.flush();
    }
    
    bool isGameOver() const { return gameOver; }
    
    void reset() {
        snake1 = Snake(GRID_SIZE / 2 - 4, GRID_SIZE / 2, RIGHT);
        snake2 = Snake(GRID_SIZE / 2 + 4, GRID_SIZE / 2, LEFT);
        food = Food();
        score1 = 0;
        score2 = 0;
        gameOver = false;
        loser = 0;
        firstRender = true;
        ensureFoodNotOnSnake();
    }
};

int main() {
    srand(time(0));
    setupConsole();
    
    GameBoard game;
    char input;
    
    while (true) {
        game.render();
        
        if (game.isGameOver()) {
            while (true) {
                if (kbhit()) {
                    input = getArrowKey();
                    if (input == 'r' || input == 'R') {
                        game.reset();
                        break;
                    } else if (input == 'q' || input == 'Q') {
                        cleanupConsole();
                        cout << "\n  Thanks for playing! 👋\n\n";
                        return 0;
                    }
                }
                sleepMs(10);
            }
        } else {
            while (kbhit()) {
                input = getArrowKey();
                if (input == 'q' || input == 'Q') {
                    cleanupConsole();
                    cout << "\n  Thanks for playing! 👋\n\n";
                    return 0;
                }
                game.handleInput(input);
            }
            
            game.update();
            sleepMs(250);
        }
    }
    
    return 0;
}
