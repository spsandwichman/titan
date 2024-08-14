#include "titan.h"

#ifndef TITAN_NATIVE
// TODO: write fallback functions for non x86-64 BMI2 platforms
#error "TITAN currently relies on x86-64's BMI2 extension for fast bitboard manipulation."
#endif

int util_count_ones(u64 i) {
    return __builtin_popcountll(i);
}

int util_poplsb(u64* i) {    
    if (*i == 0) return -1;
    
    int index = __builtin_ctzll(*i);
    *i &= *i - 1;
    return index;
}

int util_lsb(u64 i) {
    if (i == 0) return -1;
    return __builtin_ctzll(i);
}

int util_log2(u64 i) {
    return __builtin_ctzll(i);
}

u64 util_pext(u64 bits, u64 mask) {
    return _pext_u64(bits, mask);
}

u64 util_pdep(u64 bits, u64 mask) {
    return _pdep_u64(bits, mask);
}

u8 util_square_index(char* square) {
    u8 sq = *square - 'a';
    square++;
    sq += (7 - (*square - '1')) * 8;
    return sq;
}

char* util_index_square(u8 square) {
    static char* squares[64] = {
        "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
        "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
        "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
        "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
        "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
        "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
        "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
        "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1"
    };
    return squares[square];
}

// returns piece type if slider, otherwise return false
u8 util_is_slider(u8 piece) {
    u8 type = piece & 0b00000111;
    switch (type) {
    case ROOK:
    case QUEEN:
    case BISHOP: return type;
    default: return EMPTY;
    }
}