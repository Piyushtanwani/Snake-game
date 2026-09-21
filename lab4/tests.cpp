#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#define main snake_game_application_main
#include "../main.cpp"
#undef main

namespace {

int failures = 0;

void expectTrue(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}

class StubRandomSource : public RandomSource {
private:
    std::vector<int> values;
    std::size_t nextValue;

public:
    explicit StubRandomSource(std::vector<int> valuesToReturn)
        : values(valuesToReturn), nextValue(0) {}

    int next(int upperExclusive) override {
        if (nextValue >= values.size()) {
            std::cerr << "FAIL: random stub ran out of values\n";
            ++failures;
            return 0;
        }

        const int value = values[nextValue++];
        expectTrue(value >= 0 && value < upperExclusive, "stub value is within the requested range");
        return value;
    }
};

void testMoveAdvancesOneCell() {
    Snake snake;

    snake.move();

    expectTrue(snake.getHead().x == 11, "moving right changes the head x-coordinate to 11");
    expectTrue(snake.getHead().y == 10, "moving right leaves the head y-coordinate at 10");
}

void testGameStartsPlayable() {
    std::srand(1);

    GameBoard board;

    expectTrue(!board.isGameOver(), "a new game is not over");
}

void testEatingFoodIncreasesScoreByTen() {
    StubRandomSource random({11, 10, 0, 0});
    GameBoard board(random);

    board.update();

    std::ostringstream renderedGame;
    std::streambuf* originalOutput = std::cout.rdbuf(renderedGame.rdbuf());
    board.render();
    std::cout.rdbuf(originalOutput);

    expectTrue(renderedGame.str().find("Score: 10") != std::string::npos,
               "eating one fruit renders a score of exactly ten");
}

}  // namespace

int main() {
    testMoveAdvancesOneCell();
    testGameStartsPlayable();
    testEatingFoodIncreasesScoreByTen();

    if (failures == 0) {
        std::cout << "All tests passed\n";
        return 0;
    }

    std::cerr << failures << " test assertion(s) failed\n";
    return 1;
}
