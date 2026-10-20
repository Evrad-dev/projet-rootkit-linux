# Procédure QEMU — Projet Rootkit Linux

## 1. Prérequis

- Kali Linux
- qemu-img
- qemu-system-x86_64
- Une image QCOW2 contenant un système Linux amorçable

## 2. Tests déjà effectués

- Programme compagnon : 13 tests réussis, 0 échec.
- Création d'une image RAW de test de 64 Mio : OK.
- Conversion RAW vers QCOW2 compressée : OK.
- Compression zlib : confirmée.
- Vérification avec qemu-img check : aucune erreur.
- Protection contre l'écrasement : OK.
- Lancement de QEMU pendant 10 secondes : OK.

Remarque : l'image utilisée pour les tests est vide.
Le démarrage d'un système Linux n'a donc pas encore été validé.

## 3. Commande de démarrage QEMU

À utiliser uniquement avec une image de test amorçable
ou avec une copie de l'image LFS validée par l'équipe.

qemu-system-x86_64 \
  -m 1024 \
  -drive file=IMAGE_TEST.qcow2,format=qcow2,if=virtio \
  -boot c \
  -serial mon:stdio \
  -no-reboot

Adapter la configuration du disque et du firmware
aux caractéristiques du système LFS.

## 4. Checklist de vérification

- [ ] L'image QCOW2 existe.
- [ ] qemu-img info indique le format qcow2.
- [ ] qemu-img check ne signale aucune erreur.
- [ ] QEMU démarre sans erreur d'ouverture du disque.
- [ ] Le chargeur d'amorçage apparaît.
- [ ] Le noyau Linux démarre.
- [ ] Le système LFS atteint une invite de connexion.
- [ ] Les périphériques nécessaires sont présents.
- [ ] Les tests du programme compagnon sont exécutés.
- [ ] Les résultats et blocages sont documentés.

## 5. Limites actuelles

Le démarrage réel du système LFS reste à effectuer
avec l'équipe lorsque l'image sera disponible.

L'intégration du compagnon avec /dev/rootkit
reste à confirmer avec le responsable du module noyau.

Ne jamais modifier, déplacer ou convertir
l'image RAW de travail du coéquipier.
