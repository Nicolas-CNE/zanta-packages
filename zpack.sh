#!/bin/sh

if [ -z "$1" ]; then
    echo "Uso: ./zpack.sh <ruta_a_la_receta>"
    exit 1
fi

RECIPE_DIR=$(realpath "$1")
. "$RECIPE_DIR/zpkg"

# Directorios de trabajo
BUILD_DIR="/tmp/zbuild/$PKG_NAME-$PKG_VER"
export PKG_DEST="/tmp/zbuild-dest/$PKG_NAME-$PKG_VER"

rm -rf "$BUILD_DIR" "$PKG_DEST"
mkdir -p "$BUILD_DIR" "$PKG_DEST"

echo "[*] Ejecutando receta para $PKG_NAME v$PKG_VER..."
cd "$BUILD_DIR"
package_binary

echo "[*] Generando manifiesto .fnf-info..."
cat << MANIFEST > "$PKG_DEST/.fnf-info"
name=$PKG_NAME
version=$PKG_VER
arch=x86_64
description=$PKG_DESC
MANIFEST

# Asegurar que exista la carpeta de la categoría
mkdir -p "/root/zanta-packages/packages/$PKG_CAT"

echo "[*] Empaquetando .fnf..."
cd "$PKG_DEST"
tar --zstd -cf "/root/zanta-packages/packages/$PKG_CAT/$PKG_NAME-$PKG_VER.fnf" .

echo "[✔] Listo: packages/$PKG_CAT/$PKG_NAME-$PKG_VER.fnf"
