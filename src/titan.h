#pragma once

#include "orbit.h"
#include "term.h"

#ifdef __x86_64__
    #include <x86intrin.h>
    #ifdef __BMI2__
        #define TITAN_NATIVE
    #endif
#endif

#ifndef NO_LOG
    #define LOG(msg, ...) printf("[TITAN] " msg __VA_OPT__(,) __VA_ARGS__)
    #define LOGP(msg, ...) printf(msg __VA_OPT__(,) __VA_ARGS__)
#else
    #define LOG(...)
    #define LOGP(...)
#endif

#define BOARD_SIZE 64

enum {
    EMPTY = 0,

    PAWN   = 0b00000001,
    ROOK   = 0b00000010,
    BISHOP = 0b00000011,
    KNIGHT = 0b00000100,
    QUEEN  = 0b00000101,
    KING   = 0b00000110,

    WHITE  = 0b00000000,
    BLACK  = 0b00001000,

    PIECE_MAX = (BLACK | KING),

    BITBOARD_MAX = PIECE_MAX,
};

#define EN_PASSANT_NONE 255

typedef struct Board {
    u64 bitboards[BITBOARD_MAX];

    u64 zobrist;
    u8 en_passant_square;
    u16 halfmove_clock;
    u32 fullmoves;

    bool white_to_move : 1;
    bool white_kingside_castle  : 1;
    bool white_queenside_castle : 1;
    bool black_kingside_castle  : 1;
    bool black_queenside_castle : 1;

} Board;

void board_print(Board* b);
void board_print_bits(u64 board);
Board board_default();
Board board_from_fen(char* fen);
u64 board_black_bits(Board* b);
u64 board_white_bits(Board* b);

#define MAX_MOVES 218
typedef struct Move {
    u8 from : 4;
    u8 to   : 4;
} Move;

typedef struct Moveset {
    u16 len;
    Move moves[MAX_MOVES];
} Moveset;

int util_poplsb(u64* i);
int util_log2(u64 i);
u64 util_pext(u64 bits, u64 mask);
u64 util_pdep(u64 bits, u64 mask);
u8    util_square_index(char* square);
char* util_index_square(u8 square);

void mrand_init_seed(u64 seed);
u64  mrand_u64(void);

extern u64 zobrist_table[64][PIECE_MAX];
extern u64 zobrist_en_passant[64];
extern u64 zobrist_castle_white_queenside;
extern u64 zobrist_castle_white_kingside;
extern u64 zobrist_castle_black_queenside;
extern u64 zobrist_castle_black_kingside;
extern u64 zobrist_white_to_move;
void zobrist_init();
u64 zobrist_calculate(Board* b);

void move_init();
u64 move_rook_bitboard(u8 square, u64 enemy, u64 friendly);
u64 move_bishop_bitboard(u8 square, u64 enemy, u64 friendly);
u64 move_king_bitboard(u8 square, u64 friendly);
u64 move_pawn_bitboard(u8 square, bool white, u64 friendly, u64 enemy, u8 en_passant_square);