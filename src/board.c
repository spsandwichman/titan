#include "titan.h"

#define USE_ASCII false

void board_print(Board* b) {
    u64 bit = 1;
    for_range(i, 0, 64) {

        if      (bit & b->bitboards[WHITE | PAWN])   printf(USE_ASCII ? "P" : "♟︎");
        else if (bit & b->bitboards[BLACK | PAWN])   printf(USE_ASCII ? "p" : "♙");
        else if (bit & b->bitboards[WHITE | ROOK])   printf(USE_ASCII ? "R" : "♜");
        else if (bit & b->bitboards[BLACK | ROOK])   printf(USE_ASCII ? "r" : "♖");
        else if (bit & b->bitboards[WHITE | BISHOP]) printf(USE_ASCII ? "B" : "♝");
        else if (bit & b->bitboards[BLACK | BISHOP]) printf(USE_ASCII ? "b" : "♗");
        else if (bit & b->bitboards[WHITE | KNIGHT]) printf(USE_ASCII ? "N" : "♞");
        else if (bit & b->bitboards[BLACK | KNIGHT]) printf(USE_ASCII ? "n" : "♘");
        else if (bit & b->bitboards[WHITE | KING])   printf(USE_ASCII ? "K" : "♚");
        else if (bit & b->bitboards[BLACK | KING])   printf(USE_ASCII ? "k" : "♔");
        else if (bit & b->bitboards[WHITE | QUEEN])  printf(USE_ASCII ? "Q" : "♛");
        else if (bit & b->bitboards[BLACK | QUEEN])  printf(USE_ASCII ? "q" : "♕");
        else { // empty square
            u8 row = i / 8;
            u8 col = i % 8;
            if ((row + col) % 2 == 1) {
                printf(STYLE_Dim);
                printf(".");
                printf(STYLE_Reset);
            } else {
                printf(".");
            }
        }
        printf(" ");

        if (i % 8 == 7) printf("\n");

        bit = bit << 1;
    }
}

void board_print_bitboard(u64 board) {
    for_range(i, 0, 64) {
        u64 bit = 1ull << i;

        if (bit & board) {
            printf("1 ");
        } else {
            printf(". ");
        }
        
        if (i % 8 == 7) printf("\n");
    }
}

void board_print_moveset(Moveset* ms) {
    u8 targets[64] = {0};
    for_range(i, 0, ms->len) {
        targets[ms->moves[i].to]++;
    }

    for_range(i, 0, 64) {

        if (targets[i]) {
            printf("%d ", targets[i]);
        } else {
            printf(". ");
        }
        
        if (i % 8 == 7) printf("\n");
    }
}

Board board_default() {
    Board b = board_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    return b;
}

// assumes a well-formed FEN string
Board board_from_fen(char* fen) {
    Board b = {0};

    // piece configuration
    for (u64 bit = 1; *fen != ' '; fen++) {
        if (*fen == '/') continue;

        switch (*fen) {
        case 'r': b.bitboards[BLACK | ROOK] |= bit; break;
        case 'R': b.bitboards[WHITE | ROOK] |= bit; break;
        case 'p': b.bitboards[BLACK | PAWN] |= bit; break;
        case 'P': b.bitboards[WHITE | PAWN] |= bit; break;
        case 'k': b.bitboards[BLACK | KING] |= bit; break;
        case 'K': b.bitboards[WHITE | KING] |= bit; break;
        case 'q': b.bitboards[BLACK | QUEEN] |= bit; break;
        case 'Q': b.bitboards[WHITE | QUEEN] |= bit; break;
        case 'n': b.bitboards[BLACK | KNIGHT] |= bit; break;
        case 'N': b.bitboards[WHITE | KNIGHT] |= bit; break;
        case 'b': b.bitboards[BLACK | BISHOP] |= bit; break;
        case 'B': b.bitboards[WHITE | BISHOP] |= bit; break;
        case '1': bit = bit << 0; break;
        case '2': bit = bit << 1; break;
        case '3': bit = bit << 2; break;
        case '4': bit = bit << 3; break;
        case '5': bit = bit << 4; break;
        case '6': bit = bit << 5; break;
        case '7': bit = bit << 6; break;
        case '8': bit = bit << 7; break;
        default:
            CRASH("unexpected");
        }

        bit = bit << 1;
    }
    assert(*fen == ' ');
    fen++;

    // color-to-move
    switch (*fen) {
    case 'w': b.white_to_move = true; fen++; break;
    case 'b': b.white_to_move = false; fen++; break;
    default:
        CRASH("unexpected");
    }
    assert(*fen == ' ');
    fen++;

    // castling rights
    while (*fen != ' ') {
        switch (*fen){
        case '-': fen++; break;
        case 'K': b.white_kingside_castle = true; fen++; break;
        case 'Q': b.white_queenside_castle = true; fen++; break;
        case 'k': b.black_kingside_castle = true; fen++; break;
        case 'q': b.black_queenside_castle = true; fen++; break;
        default:
            CRASH("unexpected");
        }
    }
    assert(*fen == ' ');
    fen++;

    // en passant square
    if (*fen == '-') {
        b.en_passant_square = EN_PASSANT_NONE;
        fen++;
    } else {
        b.en_passant_square = util_square_index(fen);
        fen += 2;
    }
    assert(*fen == ' ');
    fen++;

    u16 halfmoves = 0;
    while (*fen != ' ') {
        halfmoves = halfmoves * 10 + (*fen - '0');
        fen++;
    }
    b.halfmove_clock = halfmoves;
    assert(*fen == ' ');
    fen++;

    u32 fullmoves = 0;
    while (*fen != ' ' && *fen != '\0') {
        fullmoves = fullmoves * 10 + (*fen - '0');
        fen++;
    }
    b.fullmoves = fullmoves;

    return b;
}

u64 board_black_bits(Board* b) {
    return 
        b->bitboards[BLACK | PAWN] |
        b->bitboards[BLACK | ROOK] |
        b->bitboards[BLACK | KING] |
        b->bitboards[BLACK | QUEEN] |
        b->bitboards[BLACK | BISHOP] |
        b->bitboards[BLACK | KNIGHT];
}

u64 board_white_bits(Board* b) {
    return 
        b->bitboards[WHITE | PAWN] |
        b->bitboards[WHITE | ROOK] |
        b->bitboards[WHITE | KING] |
        b->bitboards[WHITE | QUEEN] |
        b->bitboards[WHITE | BISHOP] |
        b->bitboards[WHITE | KNIGHT];
}

u64 board_occupied(Board* b) {
    return board_white_bits(b) | board_black_bits(b);
}

u8 forceinline board_piece_at(Board* b, u8 square) {
    return board_piece_at_color(b, square, WHITE);
}

u8 board_piece_at_color(Board* b, u8 square, u8 color) {
    u64 bit = 1ull << square;
    for_range_incl(p, color | PAWN, PIECE_MAX) {
        if (bit & b->bitboards[p]) return p;
    }
    return EMPTY;
}

u8 board_piece_at_mask(Board* b, u8 mask, u8 color) {
    for_range_incl(p, color | PAWN, PIECE_MAX) {
        if (mask & b->bitboards[p]) return p;
    }
    return EMPTY;
}