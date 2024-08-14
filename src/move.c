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

u64 move_slider_rays[64][64];

u64 gen_slider_ray_between(u8 to, u8 from) {

    if (to == from) return 0;

    int to_col = to % 8;
    int to_row = to / 8;
    int to_l_diag = to_col - to_row;
    int to_r_diag = to_col + to_row;

    int from_col = from % 8;
    int from_row = from / 8;
    int from_l_diag = from_col - from_row;
    int from_r_diag = from_col + from_row;

    u64 ray = 0;

    if (to_col == from_col) {
        // vertical alignment
        u8 min_row = min(to_row, from_row);
        u8 max_row = max(to_row, from_row);

        for_range(i, min_row + 1, max_row) {
            u64 bit = 1ull << (to_col + i*8);
            ray |= bit;
        }
    }
    if (to_row == from_row) {
        // horizontal alignment
        u8 min_col = min(to_col, from_col);
        u8 max_col = max(to_col, from_col);

        for_range(i, min_col + 1, max_col) {
            u64 bit = 1ull << (i + to_row*8);
            ray |= bit;
        }
    }
    if (to_l_diag == from_l_diag) {
        // left diagonal alignment;
        u8 min = min(to, from);
        u8 max = max(to, from);

        for (u8 i = min + 9; i < max; i+=9) {
            u64 bit = 1ull << i;
            ray |= bit;
        }
    }
    if (to_r_diag == from_r_diag) {
        // right diagonal alignment;
        u8 min = min(to, from);
        u8 max = max(to, from);

        for (u8 i = min + 7; i < max; i+=7) {
            u64 bit = 1ull << i;
            ray |= bit;
        }
    }

    return ray;
}

u64 forceinline move_get_slider_ray_between(u8 to, u8 from) {
    return move_slider_rays[to][from];
}

u64 forceinline move_get_rook_slider_ray_between(u8 to, u8 from) {
    int to_col = to % 8;
    int to_row = to / 8;
    int from_col = from % 8;
    int from_row = from / 8;
    if (to_col == from_col || to_row == to_col) {
        return move_slider_rays[to][from];
    }
    return 0;
}

u64 forceinline move_get_bishop_slider_ray_between(u8 to, u8 from) {
    int to_col = to % 8;
    int to_row = to / 8;
    int to_l_diag = to_col - to_row;
    int to_r_diag = to_col + to_row;
    int from_col = from % 8;
    int from_row = from / 8;
    int from_l_diag = from_col - from_row;
    int from_r_diag = from_col + from_row;
    if (to_l_diag == from_l_diag || to_r_diag == from_r_diag) {
        return move_slider_rays[to][from];
    }
    return 0;
}

void move_init_slider_rays() {
    for_range(to, 0, 64) {
        for_range(from, 0, 64) {
            move_slider_rays[to][from] = gen_slider_ray_between(to, from);
        }
    }
}

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
    move_init_slider_rays();
}

void forceinline move_add(Move m, Moveset* ms) {
    ms->moves[ms->len++] = m;
}

void move_add_promotion_variants(Move m, Moveset* ms) {
    m.special = SPECIAL_PROMOTE_QUEEN; move_add(m, ms);
    m.special = SPECIAL_PROMOTE_ROOK; move_add(m, ms);
    m.special = SPECIAL_PROMOTE_BISHOP; move_add(m, ms);
    m.special = SPECIAL_PROMOTE_KNIGHT; move_add(m, ms);
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

u64 move_knight_bitboard(u8 square, u64 friendly) {
    return move_bitboards_knight[square] & ~friendly;
}

u64 move_king_bitboard(u8 square, u64 friendly) {
    return move_bitboards_king[square] & ~friendly;
}

u64 move_pawn_bitboard_captures(u8 square, bool white, u64 enemy) {

    u8 col = square % 8;
    u8 row = square / 8;

    u64 pawn = 1ull << square;

    u64 moves = 0;

    if (white) {
        u64 left_diagonal = ((pawn >> 8) >> 1);
        u64 right_diagonal = ((pawn >> 8) << 1);
        moves |= (left_diagonal | right_diagonal) & enemy;
    } else {
        u64 left_diagonal = ((pawn << 8) >> 1);
        u64 right_diagonal = ((pawn << 8) << 1);
        moves |= (left_diagonal | right_diagonal) & enemy;
    }

    return moves;
}

u64 move_pawn_bitboard_en_passant(u8 square, bool white, u8 en_passant_square) {

    u8 col = square % 8;
    u8 row = square / 8;

    u64 pawn = 1ull << square;

    u64 moves = 0;

    if (white) {
        u64 left_diagonal = ((pawn >> 8) >> 1);
        u64 right_diagonal = ((pawn >> 8) << 1);
        if (col != 7 && en_passant_square == square + 1) {
            moves |= right_diagonal;
        }
        if (col != 0 && en_passant_square == square - 1) {
            moves |= left_diagonal;
        }
    } else {
        u64 left_diagonal = ((pawn << 8) >> 1);
        u64 right_diagonal = ((pawn << 8) << 1);
        if (col != 7 && en_passant_square == square + 1) {
            moves |= right_diagonal;
        }
        if (col != 0 && en_passant_square == square - 1) {
            moves |= left_diagonal;
        }
    }

    return moves;
}

u64 move_pawn_bitboard_advances(u8 square, bool white, u64 friendly, u64 enemy) {

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

    return moves;
}

u64 move_king_danger_squares(Board* b, bool white_to_move) {
    // basically, which squares are being attacked if the 
    // friendly king doesn't exist.

    u8 enemy_color = white_to_move ? BLACK : WHITE;
    u8 friendly_color = !white_to_move ? BLACK : WHITE;

    u64 friendly = 
        b->bitboards[friendly_color | PAWN] |
        b->bitboards[friendly_color | ROOK] |
        b->bitboards[friendly_color | KNIGHT] |
        b->bitboards[friendly_color | BISHOP] |
        b->bitboards[friendly_color | QUEEN];

    u64 enemy = 
        b->bitboards[enemy_color | PAWN] |
        b->bitboards[enemy_color | ROOK] |
        b->bitboards[enemy_color | KING] |
        b->bitboards[enemy_color | KNIGHT] |
        b->bitboards[enemy_color | BISHOP] |
        b->bitboards[enemy_color | QUEEN];
    
    u64 attacked = 0;

    u64 enemy_pawns = b->bitboards[enemy_color | PAWN];
    while (enemy_pawns != 0) {
        u8 square = util_poplsb(&enemy_pawns);
        attacked |= move_pawn_bitboard_captures(
            square, !white_to_move, BOARD_FULL
        );
    }

    u64 enemy_rooks = b->bitboards[enemy_color | ROOK];
    while (enemy_rooks != 0) {
        u8 square = util_poplsb(&enemy_rooks);
        attacked |= move_rook_bitboard(square, friendly, enemy);
    }

    u64 enemy_bishops = b->bitboards[enemy_color | BISHOP];
    while (enemy_bishops != 0) {
        u8 square = util_poplsb(&enemy_bishops);
        attacked |= move_bishop_bitboard(square, friendly, enemy);
    }

    u64 enemy_knights = b->bitboards[enemy_color | KNIGHT];
    while (enemy_knights != 0) {
        u8 square = util_poplsb(&enemy_knights);
        attacked |= move_knight_bitboard(square, enemy);
    }

    u64 enemy_queens = b->bitboards[enemy_color | QUEEN];
    while (enemy_queens != 0) {
        u8 square = util_poplsb(&enemy_queens);
        attacked |= move_bishop_bitboard(square, friendly, enemy);
        attacked |= move_rook_bitboard(square, friendly, enemy);
    }

    return attacked;
}

// if 'white', check if white king is in check
u64 move_checker_board(Board* b, bool check_white) {
    u64 checkers = 0;

    u8 enemy_color = check_white ? BLACK : WHITE;
    u8 friendly_color = !check_white ? BLACK : WHITE;

    u8 king_square = util_lsb(b->bitboards[friendly_color | KING]);

    u64 enemy = check_white ? board_black_bits(b) : board_white_bits(b);
    u64 friendly = !check_white ? board_black_bits(b) : board_white_bits(b);

    u64 attacked_by_knight = 
        b->bitboards[enemy_color | KNIGHT] & 
        move_knight_bitboard(king_square, friendly);
    checkers |= attacked_by_knight;

    u64 rook_attacks = move_rook_bitboard(king_square, enemy, friendly);
    u64 attacked_by_rook = 
        b->bitboards[enemy_color | ROOK] &
        rook_attacks;
    checkers |= attacked_by_rook;

    u64 bishop_attacks = move_bishop_bitboard(king_square, enemy, friendly);
    u64 attacked_by_bishop = 
        b->bitboards[enemy_color | BISHOP] &
        bishop_attacks;
    checkers |= attacked_by_bishop;

    u64 attacked_by_queen =
        b->bitboards[enemy_color | QUEEN] &
        (rook_attacks | bishop_attacks);
    checkers |= attacked_by_queen;

    u64 attacked_by_pawn =
        b->bitboards[enemy_color | PAWN] &
        move_pawn_bitboard_captures(king_square, check_white, enemy);
    checkers |= attacked_by_pawn;

    return checkers;
}

void generate_simple_moves(Board* b, u8 from, u64 moveboard, bool white_move, u64 enemy_board, Moveset* ms) {
    u8 enemy_color = white_move ? BLACK : WHITE;
    
    while (moveboard != 0) {
        u8 target = util_poplsb(&moveboard);

        bool capture = EMPTY;
        if ((1ull << target) & enemy_board) {
            // detect what kind of piece we're capturing
            capture = board_piece_at_color(b, target, enemy_color);
        }

        Move m = {
            .to = target,
            .from = from,
            .special = SPECIAL_NONE,
            .capture = capture,
        };
        move_add(m, ms);
    }
}

// returns true if the current player is in checkmate.
bool move_generate_valid(Board* b, Moveset* ms, bool only_captures) {

    u64 pinned_move_masks[64];

    u64 pinned_pieces = 0;

    u64 check_board = move_checker_board(b, b->white_to_move);
    int check_count = util_count_ones(check_board);

    u8  friendly_color = b->white_to_move ? WHITE : BLACK;
    u64 friendly_board = b->white_to_move ? board_white_bits(b) : board_black_bits(b);
    u8  enemy_color = !b->white_to_move ? WHITE : BLACK;
    u64 enemy_board = !b->white_to_move ? board_white_bits(b) : board_black_bits(b);
    u64 occupied_board = friendly_board | enemy_board;

    u64 push_mask = BOARD_FULL;
    u64 capture_mask = BOARD_FULL;

    if (only_captures) {
        push_mask = 0; // dont generate moves that dont capture
        capture_mask = enemy_board; // restrict capture board to enemy pieces
        // ^ needed for certain edge cases like en passant
    }

    if (check_count < 2) {
        u8 king_square = util_lsb(b->bitboards[friendly_color | KING]);

        // check rook pins first
        u64 enemy_rooks = b->bitboards[enemy_color | ROOK];
        while (enemy_rooks) {
            u8 enemy_rook_square = util_poplsb(&enemy_rooks);

            u64 ray = move_get_rook_slider_ray_between(king_square, enemy_rook_square);

            // not pinning anything if the slider ray doesn't exist
            if (!ray) continue;
            // only care if there's one piece in between the two squares
            if (util_count_ones(ray & occupied_board) != 1) continue;
            // only care if that piece is a friendly piece
            if ((ray & friendly_board) == 0) continue;

            u64 move_mask = ray | (1ull << enemy_rook_square);
            pinned_move_masks[util_lsb(ray & occupied_board)] = move_mask; // set move mask for pinned piece
            pinned_pieces |= ray & occupied_board;

            printf("%s pinning %s to %s\n", util_index_square(enemy_rook_square), util_index_square(util_lsb(ray & occupied_board)), util_index_square(king_square));
        }

        u64 enemy_bishops = b->bitboards[enemy_color | ROOK];
        while (enemy_bishops) {
            u8 enemy_bishop_square = util_poplsb(&enemy_bishops);

            u64 ray = move_get_bishop_slider_ray_between(king_square, enemy_bishop_square);

            // not pinning anything if the slider ray doesn't exist
            if (!ray) continue;
            // only care if there's one piece in between the two squares
            if (util_count_ones(ray & occupied_board) != 1) continue;
            // only care if that piece is a friendly piece
            if ((ray & friendly_board) == 0) continue;

            u64 move_mask = ray | (1ull << enemy_bishop_square);
            pinned_move_masks[util_lsb(ray & occupied_board)] = move_mask; // set move mask for pinned piece
            pinned_pieces |= ray & occupied_board;

            printf("%s pinning %s to %s\n", util_index_square(enemy_bishop_square), util_index_square(util_lsb(ray & occupied_board)), util_index_square(king_square));
        }

        // TODO("calculate absolute pin masks");
    }

    if (check_count == 1) {
        // we can only capture the checking piece
        // OR move in between the checking piece and the king
        // (if the checking piece is a slider)
        capture_mask = check_board;
        u8 king_square = util_lsb(b->bitboards[friendly_color | KING]);
        u8 check_square = util_lsb(check_board);

        // returns 0 if a slider ray does not exist between these squares
        push_mask &= move_get_slider_ray_between(check_square, king_square);

    } else {
        // check count is 2 or greater
        // we can ONLY move the king
        u8 king_square = util_lsb(b->bitboards[friendly_color | KING]);
        
        u64 king_moves = move_king_bitboard(king_square, friendly_board);
        king_moves = king_moves & ~move_king_danger_squares(b, b->white_to_move);

        if (king_moves != 0) generate_simple_moves(b, 
            king_square, king_moves, b->white_to_move, 
            enemy_board, ms);
        return ms->len == 0;
    }

    // generate rook moves
    u64 rooks = b->bitboards[friendly_color | ROOK];
    while (rooks) {
        u8 rook_square = util_poplsb(&rooks);
        u64 rook_moves = move_rook_bitboard(rook_square, enemy_board, friendly_board);
        rook_moves &= push_mask | capture_mask;
        // if pinned, restrict movement
        if ((1ull << rook_square) & pinned_pieces) rook_moves &= pinned_move_masks[rook_square];
        if (rook_moves) generate_simple_moves(b,
            rook_square, rook_moves, b->white_to_move, 
            enemy_board, ms);
    }

    // generate bishop moves
    u64 bishops = b->bitboards[friendly_color | BISHOP];
    while (bishops) {
        u8 bishop_square = util_poplsb(&bishops);
        u64 bishop_moves = move_bishop_bitboard(bishop_square, enemy_board, friendly_board);
        bishop_moves &= push_mask | capture_mask;
        // if pinned, restrict movement
        if ((1ull << bishop_square) & pinned_pieces) bishop_moves &= pinned_move_masks[bishop_square];
        if (bishop_moves) generate_simple_moves(b,
            bishop_square, bishop_moves, b->white_to_move, 
            enemy_board, ms);
    }

    // generate queen moves
    u64 queens = b->bitboards[friendly_color | QUEEN];
    while (queens) {
        u8 queen_square = util_poplsb(&queens);
        u64 queen_moves = move_bishop_bitboard(queen_square, enemy_board, friendly_board)
                        | move_rook_bitboard(queen_square, enemy_board, friendly_board);
        queen_moves &= push_mask | capture_mask;
        // if pinned, restrict movement
        if ((1ull << queen_square) & pinned_pieces) queen_moves &= pinned_move_masks[queen_square];
        if (queen_moves) generate_simple_moves(b,
            queen_square, queen_moves, b->white_to_move, 
            enemy_board, ms);
    }

    u64 knights = b->bitboards[friendly_color | KNIGHT];
    while (knights) {
        u8 knight_square = util_poplsb(&knights);
        u64 knight_moves = move_knight_bitboard(knight_square, friendly_board);
        knight_moves &= push_mask | capture_mask;
        // if pinned, it cant move anywhere lmao
        if ((1ull << knight_square) & pinned_pieces) continue;
        if (knight_moves) generate_simple_moves(b,
            knight_square, knight_moves, b->white_to_move, 
            enemy_board, ms);
    }

    u64 pawns = b->bitboards[friendly_color | PAWN];
    while (pawns) {
        u8 pawn_square = util_poplsb(&pawns);
        u64 pawn_advances = move_pawn_bitboard_advances(
            pawn_square, b->white_to_move, friendly_board, enemy_board);
        pawn_advances &= push_mask;
        if ((1ull << pawn_square) & pinned_pieces) pawn_advances &= pinned_move_masks[pawn_square];
        while (pawn_advances) {
            u8 target = util_poplsb(&pawn_advances);
            u8 row = target / 8;
            if (row == 0 || row == 8) {
                move_add_promotion_variants((Move){
                    .capture = 0,
                    .from = pawn_square,
                    .to = target,
                }, ms);
            } else {
                u8 special = SPECIAL_NONE;
                if (abs(target - pawn_square) == 16) special = SPECIAL_DOUBLE_ADVANCE;
                move_add((Move){
                    .capture = 0,
                    .from = pawn_square,
                    .to = target,
                    .special = special,
                }, ms);
            }
        }

        u64 pawn_captures = move_pawn_bitboard_captures(
            pawn_square, b->white_to_move, enemy_board
        );
        pawn_captures &= capture_mask;
        if ((1ull << pawn_square) & pinned_pieces) pawn_captures &= pinned_move_masks[pawn_square];
        while (pawn_captures) {
            u8 target = util_poplsb(&pawn_captures);
            u8 row = target / 8;
            if (row == 0 || row == 8) {
                move_add_promotion_variants((Move){
                    .capture = board_piece_at_color(b, target, enemy_color),
                    .from = pawn_square,
                    .to = target,
                }, ms);
            } else {
                move_add((Move){
                    .capture = board_piece_at_color(b, target, enemy_color),
                    .from = pawn_square,
                    .to = target,
                    .special = SPECIAL_NONE,
                }, ms);
            }
        }

        if (b->en_passant_square != EN_PASSANT_NONE) {
            u64 en_passant_bit = 1ull << b->en_passant_square;
            u64 pawn_en_passant_target = move_pawn_bitboard_en_passant(
                pawn_square, b->white_to_move, b->en_passant_square
            ); 
            if (!((1ull << pawn_square) & pinned_pieces) && ((en_passant_bit & capture_mask) || (pawn_en_passant_target & push_mask))) {
                u8 target = util_lsb(pawn_en_passant_target);
                move_add((Move){
                    .capture = board_piece_at_color(b, target, enemy_color),
                    .from = pawn_square,
                    .to = target,
                    .special = SPECIAL_EN_PASSANT,
                }, ms);
            }
        }
    }

    u64 king = b->bitboards[friendly_color | KING];
    if (king) {
        u8 king_square = util_lsb(king);
        u64 king_moves = move_king_bitboard(king_square, friendly_board);

        king_moves &= push_mask | capture_mask;

        if (king_moves) king_moves &= ~move_king_danger_squares(b, b->white_to_move);
        generate_simple_moves(b, 
            king_square, king_moves, b->white_to_move, enemy_board, ms
        );
    }

    // generate castles
    if (check_count == 0 && b->white_to_move) {
        bool white_can_kingside = b->white_kingside_castle && (
            (b->bitboards[WHITE | KING] & (1ull << 60)) &&
            (b->bitboards[WHITE | ROOK] & (1ull << 63)) &&
            !((board_occupied(b) & (0b011ull) << 61)));
        if (white_can_kingside) {
            TODO("add kingside castle");
        }
    }

    return check_count != 0 && ms->len == 0;
}