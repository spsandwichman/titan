#include "titan.h"

u64 zobrist_table[64][PIECE_MAX];
u64 zobrist_en_passant[64];
u64 zobrist_castle_white_queenside;
u64 zobrist_castle_white_kingside;
u64 zobrist_castle_black_queenside;
u64 zobrist_castle_black_kingside;
u64 zobrist_white_to_move;

void zobrist_init() {
    mrand_init_seed(0xDEADBEEFDEADBEEFull);
    
    for_range(square, 0, 64) {
        for_range(piece, 0, PIECE_MAX) {
            zobrist_table[square][piece] = mrand_u64();
        }
        zobrist_en_passant[square] = mrand_u64();
    }
    zobrist_castle_white_queenside = mrand_u64();
    zobrist_castle_white_kingside  = mrand_u64();
    zobrist_castle_black_queenside = mrand_u64();
    zobrist_castle_black_kingside  = mrand_u64();
    zobrist_white_to_move = mrand_u64();
}

u64 zobrist_calculate(Board* b) {
    u64 z = 0;

    for_range(i, 0, 64) {
        u64 i_bit = 1ull << i;
        for_range(piece, 0, PIECE_MAX) {
            if (i_bit & b->bitboards[piece]) z ^= zobrist_table[i][piece];
        }
    }

    if (b->en_passant_square != EN_PASSANT_NONE) z ^= zobrist_en_passant[b->en_passant_square];
    if (b->white_kingside_castle)  z ^= zobrist_castle_white_kingside;
    if (b->white_queenside_castle) z ^= zobrist_castle_white_queenside;
    if (b->black_kingside_castle)  z ^= zobrist_castle_black_kingside;
    if (b->black_queenside_castle) z ^= zobrist_castle_black_queenside;

    return z;
}