#include "minimax.h"
#include <stdio.h>
#include <stdlib.h>

#define TWO_WIN_DIRECTIONS 2
#define PASS_ONE_THROUGH_LOOP 0
#define PASS_TWO_THROUGH_LOOP 1
#define MAX_ROW_INDEX 2
#define POSITIVE_OFFSET 1
#define NEGATIVE_OFFSET -1

#define INITIALIZE_MOVE 0

typedef struct {
  tictactoe_location_t move;
  minimax_score_t score;
} minimax_decision_t;

// Init the board to all empty squares.
void minimax_initBoard(tictactoe_board_t *board) {
  for (uint8_t row = 0; row < TICTACTOE_BOARD_ROWS; row++)
    for (uint8_t col = 0; col < TICTACTOE_BOARD_COLUMNS; col++)
      board->squares[row][col] = MINIMAX_EMPTY_SQUARE;
}

// Recursive helper function. This function will call itself, finding out which
// option is the best one available.
static minimax_decision_t minimax(tictactoe_board_t *board, bool is_Xs_turn) {

  minimax_decision_t choice = {{INITIALIZE_MOVE, INITIALIZE_MOVE},
                               MINIMAX_NOT_ENDGAME};

  // Base Case for game finished
  minimax_score_t endScore = minimax_computeBoardScore(board, is_Xs_turn);
  if (minimax_isGameOver(endScore)) {
    // Recursion base case, there has been a win or a draw.
    // Evaluate board based upon prev player's turn.
    choice.score = endScore;
    return choice;
  }

  // Otherwise, you need to recurse.
  // This loop will generate all possible boards and call
  // minimax recursively for every empty square
  for (uint8_t row = 0; row < TICTACTOE_BOARD_ROWS; row++) {
    for (uint8_t col = 0; col < TICTACTOE_BOARD_COLUMNS; col++) {
      // Check if space is empty
      if (board->squares[row][col] == MINIMAX_EMPTY_SQUARE) {

        // Simulate playing at this location
        board->squares[row][col] =
            is_Xs_turn ? MINIMAX_X_SQUARE : MINIMAX_O_SQUARE;

        // Recursively call minimax to get the best score, assuming player
        // choses to play at this location.
        minimax_decision_t newChoice = minimax(board, !is_Xs_turn);

        // If its the X's turn it likes the new strategy
        if (is_Xs_turn && (newChoice.score > choice.score ||
                           choice.score == MINIMAX_NOT_ENDGAME) ||
            !is_Xs_turn && (newChoice.score < choice.score ||
                            choice.score == MINIMAX_NOT_ENDGAME)) {
          choice.score = newChoice.score;
          choice.move.row = row;
          choice.move.column = col;
        }

        // Undo the change to the board
        board->squares[row][col] = MINIMAX_EMPTY_SQUARE;
      }
    }
  }
  choice.score +=
      (choice.score > MINIMAX_DRAW_SCORE) ? NEGATIVE_OFFSET : POSITIVE_OFFSET;

  return choice;
}

// This routine is not recursive but will invoke the recursive minimax function.
// You will call this function from the controlling state machine that you will
// implement in a later milestone. It computes the row and column of the next
// move based upon: the current board and player.
//
// When called from the controlling state machine, you will call this function
// as follows:
// 1. If the computer is playing as X, you will call this function with
// is_Xs_turn = true.
// 2. If the computer is playing as O, you will call this function with
// is_Xs_turn = false.
// This function directly passes the  is_Xs_turn argument into the minimax()
// (helper) function.
tictactoe_location_t minimax_computeNextMove(tictactoe_board_t *board,
                                             bool is_Xs_turn) {
  return minimax(board, is_Xs_turn).move;
}

// Returns the score of the board.
// This returns one of 4 values: MINIMAX_X_WINNING_SCORE,
// MINIMAX_O_WINNING_SCORE, MINIMAX_DRAW_SCORE, MINIMAX_NOT_ENDGAME
// Note: the is_Xs_turn argument indicates which player just took their
// turn and makes it possible to speed up this function.
// Assumptions:
// (1) if is_Xs_turn == true, the last thing played was an 'X'.
// (2) if is_Xs_turn == false, the last thing played was an 'O'.
// Hint: If you know the game was not over when an 'X' was played,
// you don't need to look for 'O's, and vice-versa.
minimax_score_t minimax_computeBoardScore(tictactoe_board_t *board,
                                          bool is_Xs_turn) {

  // Loop over each case to see if 'X' or 'O' won in each set of 3
  for (uint8_t pass = 0; pass < TWO_WIN_DIRECTIONS; pass++) {
    bool isDraw = true;
    // Loop over each grid to find sets of three in a row on columns and rows
    for (uint8_t row = 0; row < TICTACTOE_BOARD_ROWS; row++) {
      bool xWins = true;
      bool oWins = true;
      for (uint8_t col = 0; col < TICTACTOE_BOARD_COLUMNS; col++) {
        uint8_t internalRow = row;
        uint8_t internalCol = col;

        // Swap from checking columns to checking rows
        if (pass == PASS_TWO_THROUGH_LOOP) {
          internalRow = internalCol;
          internalCol = row;
        }

        xWins &= board->squares[internalRow][internalCol] == MINIMAX_X_SQUARE;
        oWins &= board->squares[internalRow][internalCol] == MINIMAX_O_SQUARE;
        isDraw &=
            board->squares[internalRow][internalCol] != MINIMAX_EMPTY_SQUARE;
      }

      // Return if a win or draw was encountered
      if (xWins && !is_Xs_turn)
        return MINIMAX_X_WINNING_SCORE;
      if (oWins && is_Xs_turn) // else not necessary except to limit computation
                               // time very slightly
        return MINIMAX_O_WINNING_SCORE;
    }
    // if empty, return draw
    if (isDraw)
      return MINIMAX_DRAW_SCORE;
  }

  // Find wins on the diagonals
  for (uint8_t pass = 0; pass < TWO_WIN_DIRECTIONS; pass++) {
    bool xWins = true;
    bool oWins = true;

    // Loop over each diagonal
    for (uint8_t row = 0; row < TICTACTOE_BOARD_ROWS; row++) {
      uint8_t col = row;

      // Change diagonal directions
      if (pass == PASS_TWO_THROUGH_LOOP)
        col = abs(row - MAX_ROW_INDEX);

      xWins &= board->squares[row][col] == MINIMAX_X_SQUARE;
      oWins &= board->squares[row][col] == MINIMAX_O_SQUARE;
    }

    // Return if a win was encountered
    if (xWins && !is_Xs_turn)
      return MINIMAX_X_WINNING_SCORE;
    if (oWins && is_Xs_turn) // else not necessary except to limit computation
                             // time very slightly
      return MINIMAX_O_WINNING_SCORE;
  }

  return MINIMAX_NOT_ENDGAME;
}

// Determine that the game is over by looking at the score.
bool minimax_isGameOver(minimax_score_t score) {
  return score != MINIMAX_NOT_ENDGAME;
}
