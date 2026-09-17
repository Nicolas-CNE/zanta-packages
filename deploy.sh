#!/bin/sh

# Configuración
REPO_DIR="/root/zanta-packages"
COMMIT_MSG="${1:-initial commit}"

cd "$REPO_DIR" || { echo "Error: No se puede acceder a $REPO_DIR"; exit 1; }

echo "[*] Generando índice del repositorio (db.txt)..."
mkdir -p repo
> repo/db.txt

# Recorrer todos los paquetes en las subcarpetas
for pkg in packages/*/*.fnf; do
    [ -e "$pkg" ] || continue
    
    # Extraer el nombre de la categoría (la carpeta contenedora)
    category=$(basename "$(dirname "$pkg")")
    
    filename=$(basename "$pkg" .fnf)
    version="${filename##*-}"
    name="${filename%-*}"
    
    dsize=$(wc -c < "$pkg")
    ksize=$((dsize * 4))
    
    # Escribir en db.txt con la categoría incluida
    echo "${name}|${version}|${dsize}|${ksize}|${category}||" >> repo/db.txt
    
    echo "  -> Indexado: $name v$version ($dsize bytes) en [$category]"
done

echo "[*] Preparando archivos para Git..."
# Guardar absolutamente todo el proyecto, incluyendo src/
git add -A

echo "[*] Creando commit..."
git commit -m "$COMMIT_MSG"

echo "[*] Subiendo al repositorio remoto..."
git push origin main

echo "[✔] Despliegue completado."
