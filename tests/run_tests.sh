#!/usr/bin/bash

cd ../src/
make clean
make -B
cd ../tests/

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
count=1

for f in queries/*.txt ; do
    echo -e "\n${count}/8"
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
    ../src/sdbsh 127.0.0.1 < "${f}" > "/tmp/smalldb_results"

    echo ">>> tue le serveur"
    killall smalldb

    echo ">>> supprime la BDD temporaire"
    rm "${db}"

    echo ">>> compare les résultats"
    if ! diff -q "/tmp/smalldb_results" "${f%.txt}.result" ; then
        echo -e "\n\033[1;31mErreur avec $f\033[0m"
        diff --color -y  "/tmp/smalldb_results" "${f%.txt}.result"
    else
        echo -e "\033[1;32mOK\033[0m"
    fi
    ((count++))
done

if ! [ -e ../src/smalldbctl ] ; then
    echo "Pas d'exécutable smalldbctl"
    exit 3
fi
