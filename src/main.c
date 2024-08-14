#include "titan.h"

void titan_init() {
    zobrist_init();
    move_init();
}

#define newline printf("\n")

int main() {
    titan_init();

    // Board b = board_from_fen("4k3/3P4/5N2/4R2B/8/8/8/8 b - - 0 1");
    // Board b = board_from_fen("4k3/3P4/8/4R2B/B7/8/8/8 b - - 0 1");
    // Board b = board_from_fen("4k3/3P4/2P5/7B/8/8/8/8 b - - 0 1");
    // Board b = board_from_fen("4k3/3P1P2/2P1Q3/8/8/8/8/8 b - - 0 1");
    // Board b = board_from_fen("3Bk3/3P1P2/2P1Q3/8/8/8/8/8 b - - 0 1");
    // Board b = board_from_fen("3Nk3/8/8/8/8/8/7K/8 w - - 0 1");
    // Board b = board_from_fen("k7/8/8/8/7P/8/7K/8 w - - 0 1");
    Board b = board_from_fen("8/8/8/1k6/3Pp3/8/8/4KQ2 b - d4 0 1");
    // Board b = board_default(); 
    board_print(&b);

    // printf("checks %d\n", util_count_ones(move_checker_board(&b, true)));

    Moveset ms = {0};
    move_generate_valid(&b, &ms, false);
    newline;
    board_print_moveset(&ms);

    TODO("figure out pinned pieces");
}