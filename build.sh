gcc -o main main.c words.c words.h -O3
if [ "$1" == "-run" ]; then
    ./main
fi
