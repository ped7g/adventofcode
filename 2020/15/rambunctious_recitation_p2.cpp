// https://adventofcode.com/2020/day/15 (part 2)
// unfortunately I don't see any math formula to calculate the sequence without tracking
// previous numbers in millions, which is beyond the available workspace of sjasmplus script
// (the largest virtual device has 4MiB memory IIRC, while the task seems to need about ~120MB)

#include <cstdio>

// constexpr int finalTurn = 2020;     // part 1
constexpr int finalTurn = 30000000; // part 2

constexpr int startSequence[] = {
//     0, 3, 6         // test input
//     3, 1, 2         // test input
    2,20,0,4,1,17   // regular input
};

int numAge[finalTurn + 1] {};

int main() {
    int turn = 0;
    // init sequence
    for (const int forcedNum : startSequence) {
        ++turn;
        printf("Turn %d the number is %d\n", turn, forcedNum);
        numAge[forcedNum] = turn;
    }
    int num = 0, prevTurn = 0;  // number for Turn 4 is zero (any init-sequence number is bespoken first time)
    // speak following numbers until final turn
    while (turn < finalTurn) {
        ++turn;
        if (finalTurn - 5 < turn) printf("Turn %d the number is %d\n", turn, num);
        prevTurn = numAge[num];
        numAge[num] = turn;
        num = prevTurn ? turn - prevTurn : 0;
    }
}
