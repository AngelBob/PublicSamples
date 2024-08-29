#pragma once

#include <bitset>
#include <list>
#include <memory>
#include <vector>

typedef enum _BLOCK_TYPE
{
    BLOCK_TYPE_MIN = 0,
    BLOCK_TYPE_BOX = 0,
    BLOCK_TYPE_ROW = 1,
    BLOCK_TYPE_COL = 2,
    BLOCK_TYPE_MAX = 3,
} BLOCK_TYPE;

typedef enum _SEARCH_STRATEGY
{
    STRATEGY_HIDDEN_SINGLE = 0,
    STRATEGY_POINTING_PAIR,
    STRATEGY_NAKED_PAIR,
    STRATEGY_HIDDEN_PAIR,
    STRATEGY_NAKED_TRIPLET,
    STRATEGY_HIDDEN_TRIPLET,
    STRATEGY_X_WING,
    STRATEGY_XY_WING,
    STRATEGY_SWORDFISH,
    STRATEGY_END
} SEARCH_STRATEGY;

typedef enum _ANNOTATION_BITS
{
    ANNOTATIONS_BASIC    = 0,
    ANNOTATIONS_EXTENDED = 1
} ANNOTATION_BITS;

typedef struct _PROGRAM_ARGS
{
    std::bitset<8> annotations;
    uint32_t       test_strategy;
    std::string    input_file;
    bool           is_test;
} PROGRAM_ARGS;

class cell;
class block;

using cell_array_t = std::vector<std::shared_ptr<cell>>;
using block_array_t = std::vector<std::shared_ptr<block>>;

using cell_value_t = uint8_t;
