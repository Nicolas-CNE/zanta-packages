#include "fnf.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

void execute_transaction(Package *pkgs, int count) {
    printf("\nexecuting transaction\n...\n");

    mkdir("/var/db/fnf/installed", 0755);
    mkdir("/zanta-rootfs", 0755);

    for (int i = 0; i < count; i++) {
        char pkg_file[256];
        snprintf(pkg_file, sizeof(pkg_file), "/tmp/%s-%s.fnf", pkgs[i].name, pkgs[i].version);

        printf("Installing %s-%s from %s...\n", pkgs[i].name, pkgs[i].version, pkg_file);

        // Extraer el archivo .fnf (tar.zst) en /zanta-rootfs/
        char extract_cmd[512];
        snprintf(extract_cmd, sizeof(extract_cmd), "tar --zstd -xf \"%s\" -C /zanta-rootfs/", pkg_file);
        
        int res = system(extract_cmd);
        if (res != 0) {
            fprintf(stderr, "[fnf] Error (%d) al extraer el paquete %s\n", res, pkgs[i].name);
            continue;
        }

        // Guardar registro de instalación
        char db_entry[256];
        snprintf(db_entry, sizeof(db_entry), "/var/db/fnf/installed/%s.list", pkgs[i].name);
        FILE *fp = fopen(db_entry, "w");
        if (fp) {
            fprintf(fp, "Name: %s\nVersion: %s\nStatus: installed\n", pkgs[i].name, pkgs[i].version);
            fclose(fp);
        }

        remove(pkg_file);
    }
}
