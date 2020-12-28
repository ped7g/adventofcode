# Advent of code vs sjasmplus script

The [Advent of Code](https://adventofcode.com/) webpage provides small programming puzzles.

The [sjasmplus](https://github.com/z00m128/sjasmplus) is Z80 CPU (and similar: R800, Z80N, i8080, LR35902) assembler, having medium-strong macro/script language.

While the assembler script is by no means the most optimal weapon of choice, I decided to try to solve the AoC puzzles in it. For some fun, but mostly because as sjasmplus contributor, I did find this idea being good exercise of the script implementation, and an opportunity to collect feedback and new ideas, about the script language use.

Toward end (day 19 at the time of writing this text) I lost lot of motivation, as there's very little new ideas in terms of script-use, and most of the tasks require 64bit math, while sjasmplus has only 32 bit expression evaluator, so it become a chore to write the big-math routines handling larger values.

I'm not sure how many more days I will add, but I'm setting this repository to public, as it's 28.12.2020, so whoever was undergoing the AoC this year as some kind of competition, should be already done with it.

Please treat this kindly as collection of how-to-abuse-sjasmplus-script examples, although with most of the Z80 assembler projects you should in the first place never even need this level of scripting. But if you eventually need to resolve somewhat more advanced computing task, you can use this as source of ideas and script-patterns (probably go backward from latest days to meet the most improved variants of some similar macros).

(also don't forget the sjasmplus has Lua scripting engine embedded, which is very likely better/simpler choice for this kind of tasks, but I'm intentionally avoiding Lua scripts in these solutions to exercise "our" sjasmplus-script implementation)
