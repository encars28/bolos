#!/bin/bash
# Script para ejecutar el programa

# for ((i=1; i<=100; i++))
# do
    gcc bolos.c -o bolos
    ./bolos

    readarray -t lines < <(ps -f)

    for line in "${lines[@]}"
    do
        words=( $line )
        if [[ ${words[7]} == "A" ]]
        then
            pid=${words[1]}
            break
        fi
    done

    kill -SIGTERM "$pid"
# done