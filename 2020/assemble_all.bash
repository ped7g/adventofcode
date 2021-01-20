#!/usr/bin/env bash

echo -e "This script will try to build all asm sources (even the unfinished/incorrect ones) for all days, launching background sjasmplus processes!"
echo -e "This script will also hide all output of the assembling except errors/warnings, this is just exercising the build-ability of the sources!"
echo -e "\n  \033[91m!!! this may take \033[93mHOURS/DAYS\033[91m to finish in case of some tasks !!!"
echo -e "\n  \033[96mmake sure you know how to get rid of unwanted processes (killall, htop, ...)\n\033[91m"
read -p "Are you sure? (y/n) " -n 1 -r
echo -e "\033[0m"
[[ ! $REPLY == "y" ]] && exit 0

for daydir in *; do
    if [[ -d $daydir ]]; then
        (
            cd $daydir
            for asm in *.asm; do
                echo -e "\033[93m!day $daydir: \033[92m!!Assembling: $asm\033[91m"
                sjasmplus --nologo --msg=war $asm 2>&1 | grep -E "^[^>][a-zA-Z0-9_.]+"
            done
        ) &
    fi
done
