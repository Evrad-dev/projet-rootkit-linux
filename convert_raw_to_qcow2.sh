#!/usr/bin/env bash
set -euo pipefail

if [ "$#" -ne 2 ]; then
    echo "Usage: $0 source.raw destination.qcow2" >&2
    exit 1
fi

SOURCE="$1"
DESTINATION="$2"

if ! command -v qemu-img >/dev/null 2>&1; then
    echo "Erreur : qemu-img est introuvable." >&2
    exit 1
fi

if [ ! -f "$SOURCE" ] || [ -L "$SOURCE" ]; then
    echo "Erreur : source absente, non régulière ou lien symbolique." >&2
    exit 1
fi

if [ -e "$DESTINATION" ] || [ -L "$DESTINATION" ]; then
    echo "Erreur : la destination existe déjà. Aucun écrasement." >&2
    exit 1
fi

DEST_DIR=$(dirname -- "$DESTINATION")

if [ ! -d "$DEST_DIR" ]; then
    echo "Erreur : dossier de destination introuvable." >&2
    exit 1
fi

FORMAT=$(qemu-img info -f raw --output=json "$SOURCE" |
    python3 -c 'import json,sys; print(json.load(sys.stdin)["format"])')

if [ "$FORMAT" != "raw" ]; then
    echo "Erreur : format RAW attendu." >&2
    exit 1
fi

TEMP=$(mktemp "$DEST_DIR/.conversion.XXXXXXXX.qcow2")

cleanup() {
    rm -f -- "$TEMP"
}
trap cleanup EXIT

echo "Conversion RAW vers QCOW2 compressée..."
qemu-img convert -f raw -O qcow2 -c "$SOURCE" "$TEMP"

echo "Vérification du résultat..."
qemu-img check "$TEMP"

# Crée le nom final sans écraser un fichier déjà présent.
if ! ln -- "$TEMP" "$DESTINATION"; then
    echo "Erreur : destination déjà occupée ou publication impossible." >&2
    exit 1
fi

echo "Conversion terminée avec succès : $DESTINATION"
