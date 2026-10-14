# TP2 — Appels système et module noyau

## Objectifs

Ce TP porte sur l'ajout et l'utilisation d'appels système dans un noyau
Linux modifié, ainsi que sur la compilation et les tests d'un module noyau.

## Organisation

- `hello/test_hello.c` : programme de test de l'appel système `hello`.
- `uptime/test_uptime.c` : programme de test de l'appel système `uptime`.
- `version-login_x/version.c` : source du module de version.
- `version-login_x/version.h` : déclarations partagées.
- `version-login_x/test.c` : programme de test.
- `version-login_x/Makefile` : instructions de compilation du module.

## Prérequis

- Un compilateur C, tel que GCC.
- Les en-têtes correspondant au noyau ciblé pour compiler le module.
- Un noyau contenant les appels système personnalisés pour exécuter
  correctement les tests `hello` et `uptime`.

La disponibilité de ces appels système dépend de la configuration et de
la compilation du noyau utilisé.

## Compilation des programmes de test

Depuis la racine du projet :

    gcc -Wall -Wextra -o tp2/hello/test_hello tp2/hello/test_hello.c
    gcc -Wall -Wextra -o tp2/uptime/test_uptime tp2/uptime/test_uptime.c

Exécuter ces programmes uniquement dans l'environnement prévu pour le TP.
La compilation seule ne garantit pas que les appels système personnalisés
soient disponibles.

## Compilation du module version

Depuis `tp2/version-login_x/`, consulter le Makefile et vérifier que
`/lib/modules/$(uname -r)/build` pointe vers les en-têtes du noyau ciblé.

    make

Pour supprimer les fichiers générés :

    make clean

## Tests et résultats

Consigner les commandes exécutées, la version du noyau, les résultats
observés et les éventuelles erreurs dans le rapport de TP.

Ne pas charger un module compilé pour une autre version du noyau.
Effectuer les essais dans une machine virtuelle de laboratoire.

## Limites

Les résultats dépendent du noyau compilé pour le TP, de ses appels système
et de la configuration de l'environnement. Les tests doivent être réalisés
et documentés avant de déclarer le TP entièrement validé.
