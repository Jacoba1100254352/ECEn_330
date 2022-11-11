
#include "ticTacToeControl.h"
#include "buttons.h"
#include "display.h"
#include "minimax.h"
#include "ticTacToeDisplay.h"
#include "touchscreen.h"
#include <stdbool.h>
#include <stdio.h>

#define TIMER_DONE 60
#define TEXT_SIZE 2
#define CHAR_HEIGHT DISPLAY_CHAR_HEIGHT *TEXT_SIZE

#define ERASE true
#define DONT_ERASE false

#define ERASE_TEXT true
#define DONT_ERASE_TEXT false

#define TOUCHED true
#define RELEASED false

#define RESET_COUNTER 0
#define RESET_TIMER 0

#define INTRO_LINE_1 "Touch board to play X"
#define INTRO_LINE_2 "-or-"
#define INTRO_LINE_3 "wait for the computer"
#define INTRO_LINE_4 "and play O"

#define INTRO_LINE_1_X                                                         \
  (DISPLAY_WIDTH / 2) - (DISPLAY_CHAR_WIDTH * TEXT_SIZE * 21 / 2)
#define INTRO_LINE_1_Y (DISPLAY_HEIGHT / 2) - DISPLAY_CHAR_HEIGHT *TEXT_SIZE * 2
#define INTRO_LINE_2_X                                                         \
  (DISPLAY_WIDTH / 2) - (DISPLAY_CHAR_WIDTH * TEXT_SIZE * 4 / 2)
#define INTRO_LINE_2_Y (DISPLAY_HEIGHT / 2) - DISPLAY_CHAR_HEIGHT *TEXT_SIZE
#define INTRO_LINE_3_X                                                         \
  (DISPLAY_WIDTH / 2) - (DISPLAY_CHAR_WIDTH * TEXT_SIZE * 21 / 2)
#define INTRO_LINE_3_Y (DISPLAY_HEIGHT / 2)
#define INTRO_LINE_4_X                                                         \
  (DISPLAY_WIDTH / 2) - (DISPLAY_CHAR_WIDTH * TEXT_SIZE * 11 / 2)
#define INTRO_LINE_4_Y (DISPLAY_HEIGHT / 2) + DISPLAY_CHAR_HEIGHT *TEXT_SIZE

#define TOP 0
#define LEFT 0
#define MID 1

#define PRINT_DEBUG_STATEMENTS false

enum ticTacToe_st_t {
  init_st,
  intro_st,
  new_game_st,
  player_turn_st,
  touched_st,
  get_location_st,
  draw_players_move_st,
  comp_turn_st,
  game_over_st
};
static enum ticTacToe_st_t currentState;

static uint8_t threeSecTimer;
static bool is_Xs_turn;
static bool wasTouched;
static tictactoe_board_t board;
static tictactoe_location_t move;

// Print introductory text
static void writeIntro(bool erase) {
  display_setTextSize(TEXT_SIZE);
  display_setTextColor(erase ? DISPLAY_BLACK : DISPLAY_GREEN);
  display_setCursor(INTRO_LINE_1_X, INTRO_LINE_1_Y);
  display_println(INTRO_LINE_1);
  display_setCursor(INTRO_LINE_2_X, INTRO_LINE_2_Y);
  display_println(INTRO_LINE_2);
  display_setCursor(INTRO_LINE_3_X, INTRO_LINE_3_Y);
  display_println(INTRO_LINE_3);
  display_setCursor(INTRO_LINE_4_X, INTRO_LINE_4_Y);
  display_println(INTRO_LINE_4);
}

// Clear all the squares
static void clearSquares() {
  for (uint8_t row = 0; row < TICTACTOE_BOARD_ROWS; row++) {
    for (uint8_t col = 0; col < TICTACTOE_BOARD_COLUMNS; col++) {
      move.row = row;
      move.column = col;
      ticTacToeDisplay_drawX(move, ERASE);
      ticTacToeDisplay_drawO(move, ERASE);
    }
  }
}

// Print the given state passed in by the state variable
static void printStateString(enum ticTacToe_st_t state) {
  // Print the given state
  switch (state) {
  case init_st:
    printf("init_st");
    break;
  case intro_st:
    printf("intro_st");
    break;
  case new_game_st:
    printf("new_game_st");
    break;
  case player_turn_st:
    printf("player_turn_st");
    break;
  case touched_st:
    printf("touched_st");
    break;
  case get_location_st:
    printf("get_location_st");
    break;
  case draw_players_move_st:
    printf("draw_players_move_st");
    break;
  case comp_turn_st:
    printf("comp_turn_st");
    break;
  case game_over_st:
    printf("game_over_st");
    break;
  default:
    printf("UNKNOWN_ST");
    break;
  }
}

// Debug function for printing state changes
static void debug() {
  static enum ticTacToe_st_t previousState = init_st;
  // Print state change when a change occurs
  if (previousState != currentState) {
    printf("\nDEBUG: ");
    printStateString(previousState);
    printf(" -> ");
    printStateString(currentState);

    previousState = currentState;
  }
}

// Initialize the tic-tac-toe controller state machine, providing the tick
// period, in seconds.
void ticTacToeControl_init(double period_s) {
  display_init();
  buttons_init();

  display_fillScreen(DISPLAY_BLACK);

  wasTouched = RELEASED;
  threeSecTimer = RESET_TIMER;

  currentState = init_st;
}

// Tick the tic-tac-toe controller state machine
void ticTacToeControl_tick() {
  // Toggle debug function
  if (PRINT_DEBUG_STATEMENTS)
    debug();

  // Perform state update first.
  switch (currentState) {
  case init_st:
    writeIntro(DONT_ERASE_TEXT);
    currentState = intro_st;
    break;

  case intro_st:
    // When timer expires, start game
    if (threeSecTimer > TIMER_DONE) {
      currentState = new_game_st;

      // Clear the squares
      minimax_initBoard(&board);
      // Erase the intro text
      writeIntro(ERASE_TEXT);
      // Draw the grid
      ticTacToeDisplay_init();
      threeSecTimer = RESET_TIMER;
    }
    break;

  case new_game_st:
    // Computer starts after 3 seconds, otherwise check for user input
    if (threeSecTimer > TIMER_DONE) {
      // Play for the computer to start
      board.squares[TOP][LEFT] = MINIMAX_X_SQUARE;
      tictactoe_location_t squareOne = {TOP, LEFT};
      ticTacToeDisplay_drawX(squareOne, DONT_ERASE_TEXT);

      // O/The Player's turn now
      currentState = player_turn_st;
      is_Xs_turn = false; // Computer played X, player plays O

      threeSecTimer = RESET_TIMER;
    } else if (display_isTouched()) {
      currentState = touched_st;
      is_Xs_turn = true;
    }
    break;

  case player_turn_st:
    // Check if the game is over or the player touched the screen after previous
    // touch is cleared
    if (minimax_isGameOver(minimax_computeBoardScore(&board, is_Xs_turn)))
      currentState = game_over_st;
    else if (!wasTouched && display_isTouched())
      currentState = touched_st;
    break;

  case touched_st:
    currentState = get_location_st;
    break;

  case get_location_st:
    // If the square is not empty return to previous state to wait for a new
    // press Otherwise, log mark/press in grid
    if (board.squares[move.row][move.column] != MINIMAX_EMPTY_SQUARE)
      currentState = player_turn_st;
    else {
      board.squares[move.row][move.column] =
          is_Xs_turn ? MINIMAX_X_SQUARE : MINIMAX_O_SQUARE;
      currentState = draw_players_move_st;
    }
    break;

  case draw_players_move_st:
    currentState =
        (minimax_isGameOver(minimax_computeBoardScore(&board, is_Xs_turn)))
            ? game_over_st
            : comp_turn_st;
    break;

  case comp_turn_st:
    currentState = player_turn_st;
    break;

  case game_over_st:
    // If btn0 is pressed, reinitialize the board.
    if (buttons_read() & BUTTONS_BTN0_MASK) {
      currentState = new_game_st;

      // Clear the squares
      minimax_initBoard(&board);
      // Reinitialize the display
      clearSquares();
      threeSecTimer = RESET_TIMER;
    }
    break;

  default:
    break;
  }

  // State action switch statement
  switch (currentState) {
  // Fall through to both increment 3 second timer
  case intro_st:
  case new_game_st:
    threeSecTimer++;
    break;

  case player_turn_st:
    // Check for release
    if (wasTouched && !display_isTouched())
      wasTouched = RELEASED;
    break;

  case touched_st:
    display_clearOldTouchData();
    wasTouched = TOUCHED;
    touchscreen_ack_touch(); // I think this is where
    break;

  case get_location_st:
    move = ticTacToeDisplay_getLocationFromPoint(touchscreen_get_location());
    break;

  case draw_players_move_st:
    // Check for release
    if (wasTouched && !display_isTouched())
      wasTouched = RELEASED;

    // Draw in the correct square/symbol
    is_Xs_turn ? ticTacToeDisplay_drawX(move, DONT_ERASE_TEXT)
               : ticTacToeDisplay_drawO(move, DONT_ERASE_TEXT);
    break;

  case comp_turn_st:
    // Check for release
    if (wasTouched && !display_isTouched())
      wasTouched = RELEASED;

    // Force the computer to take the center if possible
    if (board.squares[MID][MID] == MINIMAX_EMPTY_SQUARE) {
      board.squares[MID][MID] =
          is_Xs_turn ? MINIMAX_O_SQUARE : MINIMAX_X_SQUARE;
      tictactoe_location_t squareOne = {MID, MID};
      is_Xs_turn ? ticTacToeDisplay_drawO(squareOne, DONT_ERASE_TEXT)
                 : ticTacToeDisplay_drawX(squareOne, DONT_ERASE_TEXT);
      move = squareOne;
    } else { // Let the computer move
      move = minimax_computeNextMove(&board, !is_Xs_turn);
      board.squares[move.row][move.column] =
          is_Xs_turn ? MINIMAX_O_SQUARE : MINIMAX_X_SQUARE;
      is_Xs_turn ? ticTacToeDisplay_drawO(move, DONT_ERASE_TEXT)
                 : ticTacToeDisplay_drawX(move, DONT_ERASE_TEXT);
    }
    break;

  default:
    break;
  }
}