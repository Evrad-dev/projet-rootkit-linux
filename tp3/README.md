# TP3 — Modules noyau Linux

## Objectifs
Étudier le cycle de vie des modules noyau Linux et les mécanismes
du VFS (Virtual File System).

## Modules

### hidemod
- Source : hidemod.c
- Binaire compilé : hidemod.ko
- Mécanismes étudiés : LKM, ftrace, périphérique misc.
- Attention : module pédagogique de dissimulation, à examiner
  uniquement dans un environnement de laboratoire isolé.

### chiffrefs
- Source : chiffrefs.c
- Binaire compilé : chiffrefs.ko
- Mécanismes étudiés : enregistrement d'un système de fichiers,
  opérations de lecture et d'écriture, transformation de César.

## Compilation

Depuis le répertoire tp3 :

    make clean
    make

## Vérification des binaires

    modinfo chiffrefs.ko
    modinfo hidemod.ko

La compilation ne garantit pas le bon fonctionnement à l'exécution.
Les tests fonctionnels doivent être effectués dans une VM isolée,
avec un instantané de restauration.

## État actuel
- Compilation des deux modules : réussie.
- Tests fonctionnels : à effectuer.
- Revue de sécurité et gestion des erreurs : à poursuivre.
