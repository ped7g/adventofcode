// https://adventofcode.com/2020/day/18 (part 2)
//
// to build + run: c++ -Wall -Wpedantic operation_order_p2.cpp && ./a.out

#include <iostream>
#include <fstream>

static bool error(const char* message) {
    std::cout << "err: " << message << std::endl;
    exit(1);
}

static bool SkipBlanks(std::istream& ins) {
    while (ins.good() && ' ' == ins.peek()) ins.ignore();
    return ins.good();
}

static bool parseExpression(std::istream& ins, int64_t& v);

static bool parseValue(std::istream& ins, int64_t& v) {
    if (!SkipBlanks(ins)) return false;
    int ch = ins.peek();
    if ('(' == ch) {
        ins.ignore();
        if (!parseExpression(ins, v)) return error("syntax in parentheses");
        if (')' != ins.get()) return error("missing ')'");
        return true;
    } else if ('0' <= ch && ch <= '9') {
        ins >> v;
        return true;
    }
    return false;
}

static bool parseExpression(std::istream& ins, int64_t& v) {
    if (!SkipBlanks(ins)) return false;
    if (!parseValue(ins, v)) error("syntax in left value");
    while (SkipBlanks(ins)) {
        if (')' == ins.peek()) return true;
        int opChar = ins.get();
        if ('\n' == opChar) return true;
        int64_t rightValue;
        if ('+' == opChar && parseValue(ins, rightValue)) {
            v += rightValue;
        } else if ('*' == opChar && parseExpression(ins, rightValue)) {
            v *= rightValue;
            return true;
        } else {
            error("unexpected operator");
        }
    }
    return false;
}

int main(int argc, char *argv[]) {
    const char* input_name = "input_t.txt";
        // test: expected results: 231 + 51 + 46 + 1445 + 669060 + 23340 = 694173
    if (2 <= argc) input_name = argv[1];
    std::ifstream input(input_name);
    int64_t line = 0;
    int64_t totalSum = 0, lineValue = 0;
    while (SkipBlanks(input)) {
        ++line;
        if (!parseExpression(input, lineValue)) error("missing '\\n'");
        totalSum += lineValue;
        std::cout << "Line " << line << ",\tvalue: " << lineValue << ",\tsum: " << totalSum << std::endl;
    }
    std::cout << "Total sum: " << totalSum << std::endl;
}
