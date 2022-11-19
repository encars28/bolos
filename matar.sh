#!/bin/bash

# Mini script para automatizar la instalación de Matar

function esta_contenido () {
    # Capturo el elemento a comprobar
    elemento="$1"
    shift
    # Capturo la lista
    lista=( "$@" )

    # Aplico un filtrado con grep del elemento sobre la lista y lo almaceno en grp
    grp=$(printf "%s\n" "${lista[@]}" | grep -w "$elemento")

    # Si el elemento está contenido el filtrado habrá dado como resultado el mismo elemento
    # En caso contrario habrá devuelto una cadena vacía
    if [[ $grp == "$elemento" ]]
    then 
        echo true
    else 
        echo false
    fi
}

readarray -t lines < <(ps -f)
letras=( "A" "B" "C" "D" "E" "F" "G" "H" "I" "J" )
declare -a pids

for line in "${lines[@]}"
do
    words=( $line )

    v=$(esta_contenido "${words[7]}" "${letras[@]}")
    if [[ $v == true ]] 
    then
        pids+=( "${words[1]}" )
    fi

done

if [[ ${#pids[@]} -ne 0 ]]
then 
    kill "${pids[@]}"
    echo
    echo "Procesos matados con exito"
    echo
    ps -f
    echo
else
    echo "No hay procesos que matar"
fi
