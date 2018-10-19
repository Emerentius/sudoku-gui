#pragma once

#include <variant>
#include <bitset>
#include <cstdint>

struct Clue {
    uint8_t digit;
};

struct Entry {
    uint8_t digit;
};

using CellCandidates = std::bitset<9>;

// kind of duplicates CellState, but as a std::variant instead of a tagged union
// too dangerous to rely on a tagged union everywhere
// it also differentiates clues and mere entries
// TODO: find better name
using CellWidgetState = std::variant<Clue, Entry, CellCandidates>;
