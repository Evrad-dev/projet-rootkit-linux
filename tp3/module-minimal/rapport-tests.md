# Rapport de test — Module noyau minimal

## Environnement
- Noyau : 7.1.5+kali-amd64
- Compilateur : gcc (Debian 16.2.0-1) 16.2.0
- Date : 2026-10-13T19:10:13+02:00

## Résultats
- Compilation : réussie
- Chargement par insmod : réussi
- Présence vérifiée par lsmod : confirmée
- Initialisation visible dans dmesg : confirmée
- Retrait par rmmod : réussi
- Nettoyage visible dans dmesg : confirmé

## Extraits du journal du noyau
```text
[ 8769.846487] hello_module: loading out-of-tree module taints kernel.
[ 8769.846606] hello_module: module verification failed: signature and/or required key missing - tainting kernel
[ 8769.915015] hello_module: initialisation reussie
[ 8856.480372] hello_module: nettoyage termine
[ 9195.493409] hello_module: initialisation reussie
[ 9220.869049] hello_module: nettoyage termine
[ 9321.851134] hello_module: initialisation reussie
[ 9343.893323] hello_module: nettoyage termine
```

## Limites
- Test réalisé dans une VM Kali.
- Compatibilité validée uniquement pour l'environnement observé.
- Le module est externe au noyau et n'est pas signé par une clé reconnue.
