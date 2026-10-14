# Programmation du noyau Linux — TP1 à TP4

Projet pédagogique consacré à l'étude du noyau Linux, aux appels
système, aux modules chargeables, au système de fichiers et aux
mécanismes de surveillance système avec eBPF.

## Organisation

- `tp1/` : travaux sur l'environnement Linux et le démarrage.
- `tp2/` : appels système `hello` et `uptime`, tests et module de version.
- `tp3/` : modules noyau et étude du système de fichiers.
- `tp3/module-minimal/` : module pédagogique illustrant le cycle de vie
  d'un module noyau Linux.
- `tp4/miniedr/` : sources du mini-EDR et composants eBPF.
- `rendu/` : éléments de documentation du rendu local.

## Environnement

Les compilations dépendent de la version du noyau, de ses en-têtes
et des outils installés sur la machine.

Pour compiler le module minimal :

    cd tp3/module-minimal
    make

Pour nettoyer les fichiers de compilation :

    make clean

## TP4 — miniEDR

Le dossier `tp4/miniedr/` contient les sources et le Makefile.
La compilation nécessite notamment Clang, bpftool, les bibliothèques
libbpf et libelf, ainsi qu'un noyau et un environnement compatibles
avec eBPF et BTF.

Les fichiers générés peuvent être reconstruits avec :

    make

La réussite de la compilation ne garantit pas le fonctionnement
sur toutes les distributions ou versions du noyau.

## Sécurité et limites

Ce dépôt est destiné à l'apprentissage et aux tests autorisés.

Le module `hidemod` étudie des mécanismes de dissimulation :
ne pas le charger sur une machine de production. Toute expérimentation
doit se faire dans une machine virtuelle isolée et restaurable.

Les tests fonctionnels doivent être documentés séparément des tests
de compilation.

## Images et fichiers générés

Les images disque, l'arbre complet des sources du noyau et les
binaires compilés ne sont pas versionnés dans ce dépôt. Ils peuvent
être volumineux, dépendants de l'environnement ou reproductibles
à partir des sources.

## État des travaux

- Module minimal : compilation, chargement, retrait et journaux vérifiés.
- TP3 : compilation documentée ; les tests fonctionnels restent à confirmer.
- TP4 : sources présentes ; la reproductibilité doit être vérifiée dans
  un environnement compatible.
