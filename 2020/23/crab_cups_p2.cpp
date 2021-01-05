// https://adventofcode.com/2020/day/23 (part 2)
//
// to build + run: c++ -Wall -Wpedantic -march=native -O3 crab_cups_p2.cpp && ./a.out

#include <cassert>
#include <cstdint>
#include <vector>
#include <iostream>

int main(int argc, char *argv[]) {
    std::string input = "318946572";
    if (2 <= argc) input = argv[1];

    constexpr int32_t TOTAL_MOVES = 10'000'000;
    constexpr int32_t MAX_CUP = 1'000'000;
    std::vector<int32_t> nextCup(MAX_CUP + 1);

    // parse input and prepare nextCup array
    int32_t firstCup = input[0] - '0';  assert(1 <= firstCup && firstCup <= 9);
    int32_t cup = MAX_CUP, maxCup = firstCup;
    for (const char digit : input) {
        int32_t newCup = digit - '0';   assert(1 <= newCup && newCup <= 9);
        nextCup[cup] = newCup;
        cup = newCup;                   assert(0 == nextCup[cup]);      // each digit only once
        maxCup = std::max(maxCup, newCup);
    }
    nextCup[cup] = ++maxCup;            // connect last digit in input to max+1
    while (maxCup < MAX_CUP) {
        nextCup[maxCup] = maxCup + 1;
        ++maxCup;
    }

    int32_t moves = 0, leadsTriplet;
    while (++moves <= TOTAL_MOVES) {
        // read the triplet to be moved, and "cup" will become next firstCup
        const int32_t moveCup1 = nextCup[firstCup];
        const int32_t moveCup2 = nextCup[moveCup1];
        const int32_t moveCup3 = nextCup[moveCup2];
        cup = nextCup[moveCup3];
        // find the cup where the triplet will be reconnected
        leadsTriplet = firstCup;
        do {
            if (0 == --leadsTriplet) leadsTriplet = MAX_CUP;
        } while (moveCup1 == leadsTriplet || moveCup2 == leadsTriplet || moveCup3 == leadsTriplet);
        // relink the firstCup onto cup
        nextCup[firstCup] = cup;
        // relink triplet after leadsTriplet
        nextCup[moveCup3] = nextCup[leadsTriplet];
        nextCup[leadsTriplet] = moveCup1;
        // refresh firstCup
        firstCup = cup;
    }

    int64_t secondCup = nextCup[1];
    int64_t thirdCup = nextCup[secondCup];
    std::cout << "First two cups after 1: " << secondCup << ", " << thirdCup << std::endl;
    std::cout << "Dot product: " << secondCup * thirdCup << std::endl;
}
