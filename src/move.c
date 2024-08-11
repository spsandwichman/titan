#include "titan.h"

u64 move_bitboards_knight[64];
u64 move_bitboards_king[64];

//  assume 'occupied' are all enemy pieces, so they can be captured.
//  move_bitboards_rook[square][pext(occupied, move_bitmasks_rook[square])]
u64 move_bitboards_rook[64][1 << 14];
u64 move_bitmasks_rook[64];

//  assume 'occupied' are all enemy pieces, so they can be captured.
//  move_bitboards_bishop[square][pext(occupied, move_bitmasks_bishop[square])]
u64 move_bitboards_bishop[64][1 << 13];
u64 move_bitmasks_bishop[64];

//  assume 'occupied' are all enemy pieces, so they can be captured.
u64 gen_rook_move_bitboard(u8 square, u64 occupied) {
    u8 col = square % 8;
    u8 row = square / 8;

    u8 up = row;
    u8 down = 7 - up;
    u8 left = col;
    u8 right = 7 - left;

    u64 moves = 0;

    for_range_incl(i, 1, up) {
        u64 bit = (1ull << square);
        bit = bit >> i*8;
        moves |= bit;
        if (bit & occupied) break;
    }
    for_range_incl(i, 1, down) {
        u64 bit = (1ull << square);
        bit = bit << i*8;
        moves |= bit;
        if (bit & occupied) break;
    }
    for_range_incl(i, 1, left) {
        u64 bit = (1ull << square);
        bit = bit >> i;
        moves |= bit;
        if (bit & occupied) break;
    }
    for_range_incl(i, 1, right) {
        u64 bit = (1ull << square);
        bit = bit << i;
        moves |= bit;
        if (bit & occupied) break;
    }
    return moves;
}

//  assume 'occupied' are all enemy pieces, so they can be captured.
u64 gen_bishop_move_bitboard(u8 square, u64 occupied) {
    u8 col = square % 8;
    u8 row = square / 8;

    u8 up = row;
    u8 down = 7 - up;
    u8 left = col;
    u8 right = 7 - left;

    u64 moves = 0;

    for_range_incl(i, 1, min(up, left)) {
        u64 bit = 1ull << square;
        bit = bit >> 9*i;
        moves |= bit;
        if (bit & occupied) break;
    }
    for_range_incl(i, 1, min(down, left)) {
        u64 bit = 1ull << square;
        bit = bit << 7*i;
        moves |= bit;
        if (bit & occupied) break;
    }
    for_range_incl(i, 1, min(up, right)) {
        u64 bit = 1ull << square;
        bit = bit >> 7*i;
        moves |= bit;
        if (bit & occupied) break;
    }
    for_range_incl(i, 1, min(down, right)) {
        u64 bit = 1ull << square;
        bit = bit << 9*i;
        moves |= bit;
        if (bit & occupied) break;
    }

    return moves;
}

void move_init_bitboards_bishop() {
    for_range(index, 0, 64) {
        u8 index_col = index % 8;
        u8 index_row = index / 8;

        u64 bishop = 1ull << index;

        u64 left  = 0x8040201008040201ull; // left diagonal bitboard
        u64 right = 0x0102040810204080ull; // right diagonal bitboard

        int left_distance = (int)index_col - (int)index_row;

        if (left_distance > 0) left = left >> (left_distance * 8);
        else                   left = left << (-left_distance * 8);

        int right_distance = 7 - (int)index_col - (int)index_row;

        if (right_distance > 0) right = right >> (right_distance * 8);
        else                    right = right << -(right_distance * 8);
        
        u64 mask = ~bishop & (right | left);
        move_bitmasks_bishop[index] = mask;
    }

    for_range(square, 0, 64) {
        for_range(config, 0, 1 << 13) {
            u64 occupied = util_pdep(config, move_bitmasks_bishop[square]);
            move_bitboards_bishop[square][config] = gen_bishop_move_bitboard(square, occupied);
        }
    }
}

void move_init_bitboards_rook() {
    for_range(index, 0, 64) {
        u8 index_col = index % 8;
        u8 index_row = index / 8;

        u64 rook = 1ull << index;

        u64 col = 0x0101010101010101ull;
        u64 row = 0x00000000000000FFull;
    
        u64 mask = ~rook & ((col << index_col) | (row << (index_row*8)));

        move_bitmasks_rook[index] = mask;
    }

    for_range(square, 0, 64) {
        for_range(config, 0, 1 << 14) {
            u64 occupied = util_pdep(config, move_bitmasks_rook[square]);
            move_bitboards_rook[square][config] = gen_rook_move_bitboard(square, occupied);
        }
    }
}

void move_init_bitboards_knight() {
    for_range(i, 0, 64) {
        u8 col = i % 8;

        u64 knight = 1ull << i;

        u64 moves = 0;
        if (col != 0) {
            moves |= knight >> 17;
            moves |= knight << 15;
        }
        if (col != 7) {
            moves |= knight >> 15;
            moves |= knight << 17;
        }
        if (col <= 5) {
            moves |= knight >> 6;
            moves |= knight << 10;
        }
        if (col >= 2) {
            moves |= knight << 6;
            moves |= knight >> 10;
        }

        move_bitboards_knight[i] = moves;
    }
}

void move_init_bitboards_king() {
    for_range(i, 0, 64) {
        u8 col = i % 8;

        u64 king = 1ull << i;
        u64 moves = 0;

        moves |= king << 8;
        moves |= king >> 8;

        if (col != 7) {
            moves |= king << 1;
            moves |= king << 9;
            moves |= king >> 7;
        }
        if (col != 0) {
            moves |= king >> 1;
            moves |= king >> 9;
            moves |= king << 7;
        }

        move_bitboards_king[i] = moves;
    }
}

void move_init() {
    move_init_bitboards_king();
    move_init_bitboards_knight();
    move_init_bitboards_rook();
    move_init_bitboards_bishop();

    printf("%llu\n", sizeof(move_bitboards_bishop) + sizeof(move_bitboards_rook));
}

u64 move_rook_bitboard(u8 square, u64 enemy, u64 friendly) {
    u64 occupied = enemy | friendly;
    u64 index = util_pext(occupied, move_bitmasks_rook[square]);
    return move_bitboards_rook[square][index] & ~friendly;
}

u64 move_bishop_bitboard(u8 square, u64 enemy, u64 friendly) {
    u64 occupied = enemy | friendly;
    u64 index = util_pext(occupied, move_bitmasks_bishop[square]);
    return move_bitboards_bishop[square][index] & ~friendly;
}

u64 move_king_bitboard(u8 square, u64 friendly) {
    return move_bitboards_king[square] & ~friendly;
}

u64 move_pawn_bitboard(u8 square, bool white, u64 friendly, u64 enemy, u8 en_passant_square) {

    u8 col = square % 8;
    u8 row = square / 8;

    u64 pawn = 1ull << square;

    u64 moves = 0;

    // forward move
    if (white) {
        moves |= pawn >> 8;
        if (row == 6) moves |= pawn >> 16;
    } else {
        moves |= pawn << 8;
        if (row == 1) moves |= pawn << 16;

    }
    moves = moves & ~(enemy | friendly);

    // diagonal captures & en passant
    if (white) {
        u64 left_diagonal = ((pawn >> 8) >> 1);
        u64 right_diagonal = ((pawn >> 8) << 1);
        moves |= (left_diagonal | right_diagonal) & enemy;
        if (col != 7 && en_passant_square == square + 1) {
            moves |= right_diagonal;
        }
        if (col != 0 && en_passant_square == square - 1) {
            moves |= left_diagonal;
        }
    } else {
        u64 left_diagonal = ((pawn << 8) >> 1);
        u64 right_diagonal = ((pawn << 8) << 1);
        moves |= (left_diagonal | right_diagonal) & enemy;
        if (col != 7 && en_passant_square == square + 1) {
            moves |= right_diagonal;
        }
        if (col != 0 && en_passant_square == square - 1) {
            moves |= left_diagonal;
        }
    }

    return moves;
}