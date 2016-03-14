inline constexpr uint64_t getSinglePosition(int x, int y) {
    return (0x8000000000000000ull >> (8 * y)) >> x;
}

inline constexpr uint64_t NORTH(uint64_t x) { return (x << 8); }
inline constexpr uint64_t SOUTH(uint64_t x) { return (x >> 8); }
inline constexpr uint64_t EAST(uint64_t x) { return ((x & 0xfefefefefefefefeull) >> 1); }
inline constexpr uint64_t WEST(uint64_t x) { return ((x & 0x7f7f7f7f7f7f7f7full) << 1); }
inline constexpr uint64_t NOREAST(uint64_t x) { return NORTH(EAST(x)); }
inline constexpr uint64_t SOUEAST(uint64_t x) { return SOUTH(EAST(x)); }
inline constexpr uint64_t NORWEST(uint64_t x) { return NORTH(WEST(x)); }
inline constexpr uint64_t SOUWEST(uint64_t x) { return SOUTH(WEST(x)); }

// A bitboard for each diagonal on the board.
uint64_t diagonals[] = {
    9223372036854775808ull,
    4647714815446351872ull,
    2323998145211531264ull,
    1161999622361579520ull,
    580999813328273408ull,
    290499906672525312ull,
    145249953336295424ull,
    72624976668147840ull,
    283691315109952ull,
    1108169199648ull,
    4328785936ull,
    16909320ull,
    66052ull,
    258ull,
    1ull
};

// A bitboard for every top-right-to-bottom-left diagonal on the board
uint64_t anti_diagonals[] = {
    72057594037927936ull,
    144396663052566528ull,
    288794425616760832ull,
    577588855528488960ull,
    1155177711073755136ull,
    2310355422147575808ull,
    4620710844295151872ull,
    9241421688590303745ull,
    36099303471055874ull,
    141012904183812ull,
    550831656968ull,
    2151686160ull,
    8405024ull,
    32832ull,
    128ull
};

//A bitboard for every row
uint64_t rows[] = {
    18374686479671623680ull,
    71776119061217280ull,
    280375465082880ull,
    1095216660480ull,
    4278190080ull,
    16711680ull,
    65280ull,
    255ull
};

uint64_t columns[] = {
    9259542123273814144ull,
    4629771061636907072ull,
    2314885530818453536ull,
    1157442765409226768ull,
    578721382704613384ull,
    289360691352306692ull,
    144680345676153346ull,
    72340172838076673ull
};

// A bitboard with all four corners marked
uint64_t all_corners = getSinglePosition(0, 0)
                     | getSinglePosition(0, 7)
                     | getSinglePosition(7, 0)
                     | getSinglePosition(7, 7);

// A modified matrix from the old one,
// This matrix only characterizes the specific utility of each piece
// Thus, most pieces in the middle are "0" since they are useless.
// However, corners are VERY useful, and therefore weighted 50.
// This is a good metric that approximates the "corner heuristic" for
// other engines. 
int utilityMatrix[8][8] =
{{50,-2, 2, 2, 2, 2,-2, 50},
 {-2,-9, 0, 0, 0, 0,-9,-2 },
 { 2, 1, 0, 0, 0, 1, 0, 2 },
 { 2, 1, 0, 0, 0, 0, 0, 2 },
 { 2, 1, 0, 0, 0, 0, 0, 2 },
 { 2, 1, 1, 0, 0, 1, 0, 2 },
 {-2,-9, 0, 0, 0, 0,-9,-2 },
 {50,-2, 2, 2, 2, 2,-2, 50}};
