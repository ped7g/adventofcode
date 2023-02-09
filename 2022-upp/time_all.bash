#!/usr/bin/env bash
[[ ! -d build/ ]] && echo -e "\033[91mbuild directory missing\033[0m" && exit 12
rm build/__out__.log
time \
for f in build/*; do
	basef=`basename $f`
	echo -e "\n# $basef\n" >> build/__out__.log
	echo -e -n "\n# $basef" >&2
	time $f $basef/input.txt >> build/__out__.log
done
