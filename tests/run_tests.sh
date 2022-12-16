#!/bin/bash

# On se déplace dans le dossier contenant le script
# pour pouvoir lancer le script depuis n'importe où
# (le terminal qui a lancé le script n'est pas influencé
# par ce déplacement).
cd "$(dirname "$0")" || exit 255

if ! [ -e ../src/smalldb ] ; then
    echo "Pas d'exécutable smalldb"
    exit 2
elif pidof -q smalldb ; then
    killall smalldb
fi
if ! [ -e ../src/sdbsh ] ; then
    echo "Pas d'exécutable sdbsh"
    exit 2
fi

# Dossier pour les résultats
# (On n'a pas l'accès en écriture à /tmp au NO)
mkdir -p ~/tmp

# Compteur d'échecs
declare -i fails=0

# Largeur du terminal
cols="$(tput cols)"

# Entête de comparaison côte à côte
(( half=(cols-1)/2 ))
header="$(printf "%-${half}s|\tAttendu:" "Obtenu:")"

for f in queries/*.txt ; do
    echo -e "\n>>> copie la BDD"
    db="data/$(date -Ins).bin"
    cp data/test_db.bin "${db}"

    echo ">>> lance le serveur"
    ../src/smalldb "${db}" &
    sleep 0.5
    if ! pidof -q smalldb ; then
        echo "Impossible de lancer le serveur"
        exit 1
    fi

    echo ">>> lance le client"
    ../src/sdbsh 127.0.0.1 < "${f}" > "${HOME}/tmp/smalldb_results"

    echo ">>> tue le serveur"
    killall smalldb

    echo ">>> supprime la BDD temporaire"
    rm "${db}"

    echo ">>> compare les résultats"
    if ! diff -q "${HOME}/tmp/smalldb_results" "${f%.txt}.result" > /dev/null ; then
        fails+=1
        echo -e "\n\033[1;31mErreur avec ${f}\033[0m"
        sed 's/[^a-zA-Z ()0-9-]/ /g' "${HOME}/tmp/smalldb_results" > "${HOME}/tmp/smalldb_results.ns"
        if diff -q -w "${HOME}/tmp/smalldb_results.ns" "${f%.txt}.result" > /dev/null ; then
            # Erreur dans les caractères spéciaux
            echo -e "\033[1;32m>>> Obtenu:\033[0m"
            cat -v "${HOME}/tmp/smalldb_results" | sed -e 'y/ /_/' -e 's/\t/>---/g' -e 's/$/¶/'
            echo -e "\033[1;32m>>> Attendu:\033[0m"
            cat -v "${f%.txt}.result" | sed -e 'y/ /_/' -e 's/\t/>---/g' -e 's/$/¶/'
            echo -e "" \
                "----------------------\n" \
                "| _   : espace       |\n" \
                "| >---: tab          |\n" \
                "| ¶   : fin de ligne |\n" \
                "----------------------"
        else
            # Autre erreur
            echo "${header}"
            diff --color -y  -W "${cols}" "${HOME}/tmp/smalldb_results" "${f%.txt}.result"
        fi
    else
        echo -e "\033[1;32mOK\033[0m"
    fi
done

# Vous pouvez ajouter vos tests ici

echo "${fails} échecs."

if ! [ -e ../src/smalldbctl ] ; then
    echo "Pas d'exécutable smalldbctl"
    exit 3
fi