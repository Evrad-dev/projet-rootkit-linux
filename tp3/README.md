# TP3 — Modules noyau Linux

## 1. Objectifs

Étudier le cycle de vie des modules noyau Linux (LKM) et les
mécanismes liés au système de fichiers virtuel (VFS).

## 2. Environnement de travail

- Système : Kali Linux en machine virtuelle
- Noyau utilisé pour le module minimal : 7.1.5+kali-amd64
- Compilation : GCC et GNU Make
- Environnement de test : machine virtuelle isolée

## 3. Modules du TP3

### 3.1. hello_module — Module minimal

Répertoire : `module-minimal/`

Fichiers principaux :
- `hello_module.c` : code source
- `Makefile` : compilation et nettoyage
- `README.md` : documentation du module
- `rapport-tests.md` : résultats des tests
- `compilation.txt` : informations de compilation

Fonctionnalités :
- Initialisation du module
- Journalisation avec `pr_info`
- Nettoyage lors du déchargement

État : compilation, chargement et déchargement vérifiés.

### 3.2. hidemod

Source : `hidemod.c`

Ce module pédagogique étudie des mécanismes internes au noyau,
notamment ftrace et les périphériques misc.

Toute expérimentation doit être réalisée dans une VM isolée,
avec un instantané de restauration.

État : tests fonctionnels à documenter séparément.

### 3.3. chiffrefs

Source : `chiffrefs.c`

Ce module étudie l'enregistrement d'un système de fichiers,
les opérations de lecture et d'écriture et une transformation
de César.

État : tests fonctionnels à documenter séparément.

## 4. Compiler le module minimal

Depuis ce répertoire :

    cd module-minimal
    make clean
    make
    modinfo hello_module.ko

## 5. Tester le cycle de vie du module minimal

Dans une VM de laboratoire :

    insmod hello_module.ko
    lsmod | grep hello_module
    dmesg | tail -n 20
    rmmod hello_module
    lsmod | grep hello_module

Le journal du noyau doit permettre de vérifier les messages
d'initialisation et de nettoyage.

## 6. Compiler les modules principaux

Depuis le répertoire `tp3` :

    make clean
    make

Vérifier ensuite les métadonnées :

    modinfo hidemod.ko
    modinfo chiffrefs.ko

Une compilation réussie ne garantit pas le fonctionnement
correct à l'exécution.

## 7. Sécurité et limites

- Effectuer les essais dans une VM isolée.
- Créer un instantané avant les tests.
- Vérifier les journaux du noyau après chaque essai.
- Ne pas considérer un module comme validé sans tests adaptés.
- La compatibilité dépend de la version et de la configuration
  du noyau ciblé.

## 8. État d'avancement

- [x] Compilation du module minimal
- [x] Chargement du module minimal
- [x] Vérification de son initialisation
- [x] Déchargement du module minimal
- [x] Vérification du nettoyage
- [ ] Documenter les tests fonctionnels de hidemod
- [ ] Documenter les tests fonctionnels de chiffrefs
- [ ] Effectuer la revue finale du TP3
