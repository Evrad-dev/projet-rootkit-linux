# Module noyau minimal — Mission 1

## Objectif
Illustrer le cycle de vie dun module noyau Linux : initialisation, journalisation et nettoyage.

## Fichiers
- hello_module.c : code source du module.
- Makefile : compilation et nettoyage.

## Prérequis
- Linux ou une VM de test isolée.
- GCC, GNU Make et les en-têtes du noyau ciblé.

## Compilation
Exécuter make dans ce répertoire.

## Vérification
Utiliser modinfo hello_module.ko pour consulter les métadonnées.

## Tests
Dans une VM isolée uniquement, tester insmod puis rmmod et consulter dmesg.

## Nettoyage
Exécuter make clean.

## Limites
La compilation seule ne prouve pas le bon fonctionnement. Documenter les versions et les résultats réellement observés.
