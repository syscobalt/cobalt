/***
 *                                                                                      
 *      ,ad8888ba,    ,ad8888ba,    88888888ba         db         88      888888888888  
 *     d8"'    `"8b  d8"'    `"8b   88      "8b       d88b        88           88       
 *    d8'           d8'        `8b  88      ,8P      d8'`8b       88           88       
 *    88            88          88  88aaaaaa8P'     d8'  `8b      88           88       
 *    88            88          88  88""""""8b,    d8YaaaaY8b     88           88       
 *    Y8,           Y8,        ,8P  88      `8b   d8""""""""8b    88           88       
 *     Y8a.    .a8P  Y8a.    .a8P   88      a8P  d8'        `8b   88           88       
 *      `"Y8888Y"'    `"Y8888Y"'    88888888P"  d8'          `8b  88888888888  88       
 *  Cobalt is a UNIX-like operating system forked from Dennis Wölfing's Cobalt operating
 *  system, which can be found at https://github.com/dennis95/cobalt. Cobalt is licensed
 *  under the ISC license, which can be found at the file called LICENSE at the root
 *  directory of the project.
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

enum Direction {
    UP,
    LEFT,
    DOWN,
    RIGHT,
    NONE
};

struct SnakeSegment {
    int row;
    int col;
    enum Direction direction;
    struct SnakeSegment* next;
};

struct Food {
    int row;
    int col;
};

struct SnakeSegment* snakeHead;
struct SnakeSegment* snakeTail;
struct Food food;
struct termios oldTermios;
struct winsize winsize;
unsigned int score;

static bool checkCollision(void);
static void checkFood(void);
static void drawScreen(void);
static void handleInput(void);
static void initializeWorld(void);
static void move(struct SnakeSegment* snake);
static void restoreTermios(void);

int main(int argc, char* argv[]) {
    if (argc >= 2) {
        unsigned int seed = strtoul(argv[1], NULL, 10);
        srand(seed);
    } else {
        srand(time(NULL));
    }

    setbuf(stdout, NULL);

    // Set terminal attributes.
    tcgetattr(0, &oldTermios);
    atexit(restoreTermios);
    struct termios new_termios = oldTermios;
    new_termios.c_lflag &= ~(ECHO | ICANON);
    new_termios.c_cc[VMIN] = 0;
    tcsetattr(0, TCSAFLUSH ,&new_termios);

    tcgetwinsize(1, &winsize);

    initializeWorld();
    fputs("\e[?25l\e[?1049h", stdout);

    while (true) {
        // Game loop
        drawScreen();
        struct timespec sleepTime;
        sleepTime.tv_sec = 0;
        sleepTime.tv_nsec = 175000000;
        nanosleep(&sleepTime, NULL);
        handleInput();
        move(snakeHead);

        if (checkCollision()) {
            printf("\e[?25h\e[?1049lGame Over. Your score is: %u\n", score);
            return 0;
        }
    }
}

static bool checkCollision(void) {
    struct SnakeSegment* current = snakeHead;
    while (current) {
        if (current->row < 0 || current->row >= winsize.ws_row ||
                current->col < 0 || current->col >= winsize.ws_col) {
            return true;
        }

        struct SnakeSegment* other = current->next;
        while (other) {
            if (current->row == other->row && current->col == other->col) {
                return true;
            }
            other = other->next;
        }
        current = current->next;
    }
    return false;
}

static void checkFood(void) {
    if (food.row == snakeHead->row && food.col == snakeHead->col) {
        struct SnakeSegment* newSegment = malloc(sizeof(struct SnakeSegment));
        if (!newSegment) {
            puts("\e[?25h\e[?1049lsnake: allocation failure");
            exit(1);
        }

        newSegment->row = snakeTail->row;
        newSegment->col = snakeTail->col;
        // Set the direction to NONE so that the new segment will not move in
        // the current frame.
        newSegment->direction = NONE;
        newSegment->next = NULL;

        snakeTail->next = newSegment;
        snakeTail = newSegment;

        score++;

        // Create some new food at a random location.
        food.row = rand() % winsize.ws_row;
        food.col = rand() % winsize.ws_col;
    }
}

static void drawScreen(void) {
    printf("\e[2J\e[42m");

    struct SnakeSegment* current = snakeHead;
    while (current) {
        if (current->row >= 0 && current->row < winsize.ws_row &&
                current->col >= 0 && current->col < winsize.ws_col) {
            printf("\e[%d;%dH ", current->row + 1, current->col + 1);
        }
        current = current->next;
    }

    printf("\e[%d;%dH\e[41m \e[49m", food.row + 1, food.col + 1);
}

static void handleInput(void) {
    char key;
    if (read(0, &key, 1)) {
        if (key == 'q') {
            fputs("\e[?25h\e[?1049l", stdout);
            exit(0);
        }
        enum Direction newDirection = snakeHead->direction;
        switch (key) {
        case 'w': case 'W': newDirection = UP; break;
        case 'a': case 'A': newDirection = LEFT; break;
        case 's': case 'S': newDirection = DOWN; break;
        case 'd': case 'D': newDirection = RIGHT; break;
        }

        // Don't allow the player to make a 180° turn.
        if ((snakeHead->direction + 2) % 4 != newDirection) {
            snakeHead->direction = newDirection;
        }
    }
}

static void initializeWorld(void) {
    // Create a snake with 6 segments.
    snakeHead = malloc(sizeof(struct SnakeSegment));
    if (!snakeHead) {
        puts("snake: allocation failure");
        exit(1);
    }

    snakeHead->row = 20;
    snakeHead->col = 10;
    snakeHead->direction = UP;

    snakeTail = snakeHead;
    for (int i = 0; i < 5; i++) {
        struct SnakeSegment* next = malloc(sizeof(struct SnakeSegment));
        if (!next) {
            puts("snake: allocation failure");
            exit(1);
        }
        snakeTail->next = next;

        next->row = 20;
        next->col = 11 + i;
        next->direction = LEFT;
        next->next = NULL;
        snakeTail = next;
    }

    food.row = rand() % winsize.ws_row;
    food.col = rand() % winsize.ws_col;
}

static void move(struct SnakeSegment* snake) {
    switch (snake->direction) {
    case UP: snake->row--; break;
    case LEFT: snake->col--; break;
    case DOWN: snake->row++; break;
    case RIGHT: snake->col++; break;
    case NONE: break;
    }

    if (snake == snakeHead) {
        // Check for food before the other segments have moved so the old
        // position of the tail is still known.
        checkFood();
    }

    if (snake->next) {
        move(snake->next);
        snake->next->direction = snake->direction;
    }
}

static void restoreTermios(void) {
    tcsetattr(0, TCSAFLUSH, &oldTermios);
}
