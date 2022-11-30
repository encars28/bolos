#!/bin/bash
# Script para ejecutar el programa

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