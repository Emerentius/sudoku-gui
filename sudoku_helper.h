#pragma once

#include "sudoku_ffi/src/sudoku_ffi/sudoku.h"

// ideally, the things in this file will either be expanded into
// more powerful C++ solutions or added to the sudoku_ffi bindings
// possibly a combination of both

auto row(int cell) -> int {
    return cell / 9;
}

auto col(int cell) -> int {
    return cell % 9;
}


auto band(int cell) -> int {
    return cell / 27;
}

auto stack(int cell) -> int {
    return col(cell) / 3;
}

auto block(int cell) -> int {
    return band(cell) * 3 + stack(cell);
}

auto house_type(int house) -> HouseType {
    assert(house < 27);
    if (house < 9) {
        return HouseType::Row;
    } else if (house < 18) {
        return HouseType::Col;
    } else {
        return HouseType::Block;
    }
}

// internal iteration because C++ iterators are crap
template<typename F>
auto foreach_cell_in_row(int row, F f) -> void {
    auto first_cell = row * 9;
    for (int cell = first_cell; cell < first_cell + 9; cell++) {
        f(cell);
    }
}

template<typename F>
auto foreach_cell_in_col(int col, F f) -> void {
    auto first_cell = col;
    for (int cell = first_cell; cell < 81; cell += 9) {
        f(cell);
    }
}

template<typename F>
auto foreach_cell_in_block(int block, F f) -> void {
    auto first_row = block / 3 * 3;
    auto first_col = block % 3 * 3;
    for (int row = first_row; row < first_row + 3; row++) {
        for (int col = first_col; col < first_col + 3; col++) {
            auto cell = row * 9 + col;
            f(cell);
        }
    }
}

template<typename F>
auto foreach_cell_in_house(int house, F f) -> void {
    switch (house_type(house)) {
        case HouseType::Row:   foreach_cell_in_row(house, f);        break;
        case HouseType::Col:   foreach_cell_in_col(house - 9, f);    break;
        case HouseType::Block: foreach_cell_in_block(house - 18, f); break;
    }
}

auto house_of_cell(int cell, HouseType type) -> int {
    switch (type) {
        case HouseType::Row:   return row(cell);
        case HouseType::Col:   return col(cell) + 9;
        case HouseType::Block: return block(cell) + 18;
    }
}

auto row_cell_at_position(int row, int position) -> int {
    return row * 9 + position;
}

auto col_cell_at_position(int col, int position) -> int {
    return 9 * position + col;
}

auto block_cell_at_position(int block, int position) -> int {
    auto band = block / 3;
    auto stack = block % 3;
    auto row_in_block = position / 3;
    auto col_in_block = position % 3;
    auto row = 3 * band + row_in_block;
    auto col = 3 * stack + col_in_block;
    return 9 * row + col;
}

auto cell_at_position(int house, int position) -> int {
    switch (house_type(house)) {
        case HouseType::Row: return row_cell_at_position(house, position);
        case HouseType::Col: return col_cell_at_position(house - 9, position);
        case HouseType::Block: return block_cell_at_position(house - 18, position);
    }
}
