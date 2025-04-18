/*
 * Made by catdotjs 2025, I love rats
 * This implementation of snake uses a counting clock instead of constraint
 * based implementation of original snake. It works pretty well at small boards
 *
 * Libraries you need:
 * stdc++ (of course)
 * ncurses
 * thats it...
 */

#include <algorithm>
#include <chrono>
#include <csignal>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <ncurses.h>
#include <string>
#include <thread>
#include <vector>

#define SNAKE '#'
#define APPLE '$'
#define BASEVAL 4
#define MULTI 5

struct vec2 {
  int16_t x, y = 0;
  vec2(uint16_t x = 0, uint16_t y = 0) : x(x), y(y) {}
  vec2 &operator+=(vec2 p) {
    *this = *this + p;
    return *this;
  }
  vec2 &operator-=(vec2 p) {
    *this = *this - p;
    return *this;
  }
  vec2 &operator*=(int p) {
    *this = *this * p;
    return *this;
  }
  vec2 &operator/=(int p) {
    *this = *this / p;
    return *this;
  }
  vec2 operator+(const vec2 &q) { return vec2(x + q.x, y + q.y); }
  vec2 operator-(const vec2 &q) { return vec2(x - q.x, y - q.y); }
  vec2 operator*(const uint16_t &q) { return vec2(x * q, y * q); }
  vec2 operator/(const uint16_t &q) { return vec2(x / q, y / q); }
};

class Game {
public:
  Game(uint16_t boardX = 90, uint16_t boardY = 30, uint16_t framesPerSec = 10)
      : delay(1000 / framesPerSec), boardSize(boardX, boardY),
        snakeHead(boardX / 2, boardY / 2), GameBoard(boardX * boardY, 0),
        snakeDir(0, -1) {
    initscr();

    // Accept keypad key presses KEY_UP, KEY_DOWN, KEY_RIGHT, KEY_LEFT and do
    // not block for input
    noecho();
    cbreak();
    nodelay(stdscr, true);
    keypad(stdscr, true);
    srand(time(0));

    GameBoard[posOnArray(snakeHead)] = snakeLen;
  };
  ~Game() { endwin(); };

  void start() {
    drawBorder();
    placeApple();
    while (isRunning) {
      input();
      checkFail();
      draw();
      refresh();
      std::this_thread::sleep_for(delay);
    }
  }
  void end() { isRunning = false; }

private:
  uint16_t posOnArray(vec2 obj) { return obj.x + (obj.y * boardSize.x); }

  void drawBorder() {
    std::string width(boardSize.x, '-');
    mvprintw(boardSize.y, 0, width.c_str());
    for (uint16_t i = 0; i < boardSize.y; i++) {
      mvaddch(i, boardSize.x, '|');
    }
  }

  void input() {
    int key = getch();
    switch (key) {
    case KEY_UP:
      if (snakeDir.y != 1) {
        snakeDir.y = -1;
        snakeDir.x = 0;
      }
      break;
    case KEY_DOWN:
      if (snakeDir.y != -1) {
        snakeDir.y = 1;
        snakeDir.x = 0;
      }
      break;
    case KEY_LEFT:
      if (snakeDir.x != 1) {
        snakeDir.y = 0;
        snakeDir.x = -1;
      }
      break;
    case KEY_RIGHT:
      if (snakeDir.x != -1) {
        snakeDir.y = 0;
        snakeDir.x = 1;
      }
      break;
    default:
      break;
    }
    snakeHead += snakeDir;
  }

  void checkFail() {
    bool xOverflow = snakeHead.x < 0 || snakeHead.x >= boardSize.x;
    bool yOverflow = snakeHead.y < 0 || snakeHead.y >= boardSize.y;
    if (xOverflow || yOverflow || GameBoard[posOnArray(snakeHead)] != 0) {
      end();
      gameOver();
      return;
    }
    GameBoard[posOnArray(snakeHead)] = snakeLen;
  }

  void gameOver() {
    vec2 scr = boardSize / 2;
    std::string gameover = "Game Over! Exiting in 5 seconds!";
    uint16_t x = std::max((int)(scr.x - (gameover.length() / 2)), 0);
    mvprintw(scr.y - 1, x, gameover.c_str());
    mvprintw(scr.y, x, "Your score: %d", (snakeLen - BASEVAL) * MULTI);
    refresh();

    std::this_thread::sleep_for(std::chrono::seconds(5));
  }

  void placeApple() {
    do {
      apple.x = rand() % boardSize.x;
      apple.y = rand() % boardSize.y;
    } while (GameBoard[posOnArray(apple)] != 0);
  }

  void draw() {
    for (uint16_t i = 0; i < boardSize.x * boardSize.y; i++) {
      uint16_t x = i % boardSize.x;
      uint16_t y = i / boardSize.x;
      if (x != apple.x || y != apple.y) {
        uint16_t val = GameBoard[i];
        mvaddch(y, x, val > 0 ? SNAKE : ' ');

        // Tick down the snake
        if (val > 0) {
          GameBoard[i]--;
        }
      } else {
        mvaddch(y, x, APPLE);
      }
    }

    if (apple.x == snakeHead.x && apple.y == snakeHead.y) {
      snakeLen++;
      placeApple();
    }
  }

  vec2 snakeHead, boardSize, apple, snakeDir;
  uint16_t snakeLen = BASEVAL;
  bool isRunning = true;

  std::vector<uint16_t> GameBoard;
  std::chrono::milliseconds delay;
};
std::unique_ptr<Game> game;

void handleSignal(int signal) { game->end(); }

int main(int argc, char *argv[]) {
  game = std::make_unique<Game>();
  std::signal(SIGTERM, handleSignal);
  std::signal(SIGINT, handleSignal);
  game->start();
  return 0;
}
