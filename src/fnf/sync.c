#include "fnf.h"
#include <stdlib.h>

#define DB_URL "https://raw.githubusercontent.com/Nicolas-CNE/zanta-packages/main/repo/db.txt"

bool sync_repositories(void) {
    printf("[fnf] Sincronizando bases de datos de repositorios Zanta Linux...\n");
    system("mkdir -p /var/db/fnf/repo");
    
    int res = system("wget -q -O /var/db/fnf/repo/db.txt \"" DB_URL "\"");
    if (res == 0) {
        printf("Sincronización completada con éxito.\n");
        return true;
    } else {
        fprintf(stderr, "Error al intentar sincronizar el repositorio remoto.\n");
        return false;
    }
}
