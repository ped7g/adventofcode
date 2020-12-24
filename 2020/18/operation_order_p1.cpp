// https://adventofcode.com/2020/day/18 (part 1)
//
// I gave up on the sjasmplus solution, as it requires 64b math and I got bored with that.
// The provided "operation_order_p1.asm" is example implementation,
// but limited only to 32bit math. (works only for test input)
//
// to build + run: c++ -Wall -Wpedantic operation_order_p1.cpp && ./a.out

#include <iostream>
#include <fstream>

static bool SkipBlanks(std::istream& ins) {
    while (ins.good() && ' ' == ins.peek()) ins.ignore();
    return ins.good();
}

static int64_t parseExpression(std::istream& ins) {
    int64_t res = 0, nextValue;
    bool isMul = false;
    while (SkipBlanks(ins)) {
        int firstChar = ins.peek();
        if ('\n' == firstChar || ')' == firstChar) return res;
        if ('(' == firstChar) {
            ins.ignore();
            nextValue = parseExpression(ins);
            if (')' != ins.get()) {
                std::cout << "err: missing ')'" << std::endl;
                exit(1);
            }
            res = isMul ? res * nextValue : res + nextValue;
        } else if ('0' <= firstChar && firstChar <= '9') {
            ins >> nextValue;
            res = isMul ? res * nextValue : res + nextValue;
        } else if ('+' == firstChar || '*' == firstChar) {
            isMul = ('*' == ins.get());
        } else {
            std::cout << "err: unexpected character" << std::endl;
            exit(1);
        }
    }
    std::cout << "err: incomplete expression" << std::endl;
    exit(1);
}

int main(int argc, char *argv[]) {
    const char* input_name = "input_t.txt"; // test: expected results: 71 + 51 + 26 + 437 + 12240 + 13632 = 26457
    if (2 <= argc) input_name = argv[1];
    std::ifstream input(input_name);
    int64_t line = 0;
    int64_t totalSum = 0;
    while (SkipBlanks(input)) {
        ++line;
        int64_t value = parseExpression(input);
        if ('\n' != input.get()) {
            std::cout << "err: missing '\\n'" << std::endl;
            exit(1);
        }
        totalSum += value;
        std::cout << "Line " << line << ",\tvalue: " << value << ",\tsum: " << totalSum << std::endl;
    }
    std::cout << "Total sum: " << totalSum << std::endl;
}
