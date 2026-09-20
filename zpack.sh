#!/bin/sh
set -e

if [ -z "$1" ]; then
    echo "Uso: ./zpack.sh <ruta_a_la_receta>"
    exit 1
fi

RECIPE_DIR=$(realpath "$1")
if [ ! -f "$RECIPE_DIR/zpkg" ]; then
    echo "Error: No existe el archivo zpkg en $RECIPE_DIR"
    exit 1
fi

. "$RECIPE_DIR/zpkg"

BUILD_DIR="/tmp/zbuild/$PKG_NAME-$PKG_VER"
export PKG_DEST="/tmp/zbuild-dest/$PKG_NAME-$PKG_VER"

rm -rf "$BUILD_DIR" "$PKG_DEST"
mkdir -p "$BUILD_DIR" "$PKG_DEST"

echo "[*] Empaquetando $PKG_NAME v$PKG_VER ($PKG_CAT)..."
cd "$BUILD_DIR"
package_binary

echo "[*] Generando manifiesto .fnf-info..."
cat << MANIFEST > "$PKG_DEST/.fnf-info"
name=$PKG_NAME
version=$PKG_VER
arch=x86_64
description=$PKG_DESC
category=$PKG_CAT
MANIFEST

# Generar script post-install si la variable POST_INSTALL existe
if [ -n "$POST_INSTALL" ]; then
    echo "[*] Generando script post-install (.fnf-postinstall)..."
    cat << EOF > "$PKG_DEST/.fnf-postinstall"
#!/bin/sh
$POST_INSTALL
EOF
    chmod +x "$PKG_DEST/.fnf-postinstall"
fi

mkdir -p "/root/zanta-packages/packages/$PKG_CAT"

echo "[*] Comprimiendo paquete .fnf..."
cd "$PKG_DEST"
tar --zstd -cf "/root/zanta-packages/packages/$PKG_CAT/$PKG_NAME-$PKG_VER.fnf" .

echo "[✔] Listo: packages/$PKG_CAT/$PKG_NAME-$PKG_VER.fnf"
