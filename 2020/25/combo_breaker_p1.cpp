// https://adventofcode.com/2020/day/25 (part 1)
//
// The assembly part did break the secret-loop numbers for me (in ~6min run)
//
// But generating encryption key has proven to be even more of chore, so I gave up
// and wrote this C++ version instead, where 64b math is easy (and which runs
// under 0.2s, contrary to expected ~25 days of lazy implemented sjasmplus script).
//
// to build + run:
//  c++ -std=c++17 -Wall -Wpedantic -march=native -O3 combo_breaker_p1.cpp && ./a.out

#include <iostream>

int main() {
#if 1
    // real input - expected key: 11576351
    //              expected CARD_SK: 13207740
    //              expected DOOR_SK: 8229037
    constexpr int64_t CARD_PK = 2069194;
    constexpr int64_t DOOR_PK = 16426071;
#else
    // test input - expected key: 14897079
    //              expected CARD_SK: 8
    //              expected DOOR_SK: 11
    constexpr int64_t CARD_PK = 5764801;
    constexpr int64_t DOOR_PK = 17807724;
#endif
    constexpr int64_t KEY_MOD = 20201227;

    int64_t card_sk = ~0;
    int64_t door_sk = ~0;
    // search for secret-loop values first
    int64_t loop = 0, value = 1;
    while (~0 == (card_sk | door_sk)) {
        value = (value * 7) % KEY_MOD;
        ++loop;
        if (CARD_PK == value) card_sk = loop;
        if (DOOR_PK == value) door_sk = loop;
    }
    std::cout << "Card secret loops: " << card_sk << std::endl;
    std::cout << "Door secret loops: " << door_sk << std::endl;

    int64_t key = 1;
    int loopCounter = card_sk;
    while (loopCounter--) key = (key * DOOR_PK) % KEY_MOD;
    std::cout << "Encryption key: " << key << std::endl;
}
