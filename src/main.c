#include "titan.h"

void titan_init() {
    zobrist_init();
    move_init();
}

int main() {
    titan_init();

    Board b = board_default();
    board_print(&b);
}