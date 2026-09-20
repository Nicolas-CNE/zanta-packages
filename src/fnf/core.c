#include "fnf.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

void execute_transaction(Package *pkgs, int count, const char *explicit_pkg) {
    printf("\nexecuting transaction\n...\n");
    mkdir("/var/db/fnf/installed", 0755);
    mkdir("/zanta-rootfs", 0755);

    for (int i = 0; i < count; i++) {
        char pkg_file[256];
        snprintf(pkg_file, sizeof(pkg_file), "/tmp/%s-%s.fnf", pkgs[i].name, pkgs[i].version);
        printf("Installing %s-%s...\n", pkgs[i].name, pkgs[i].version);

        // 1. Extraer binarios directamente al rootfs
        char extract_cmd[512];
        snprintf(extract_cmd, sizeof(extract_cmd), "tar --zstd -xf \"%s\" -C /zanta-rootfs/", pkg_file);
        if (system(extract_cmd) != 0) {
            fprintf(stderr, "[fnf] Error al extraer %s\n", pkgs[i].name);
            continue;
        }

        // 2. Ejecutar script Post-Install si existe
        if (access("/zanta-rootfs/.fnf-postinstall", F_OK) == 0) {
            printf("  -> Ejecutando script post-instalación para %s...\n", pkgs[i].name);
            system("chroot /zanta-rootfs /bin/sh /.fnf-postinstall 2>/dev/null || sh /zanta-rootfs/.fnf-postinstall 2>/dev/null");
            remove("/zanta-rootfs/.fnf-postinstall");
        }
        remove("/zanta-rootfs/.fnf-info");

        // 3. Crear manifiesto de archivos instalados
        char manifest_cmd[512];
        snprintf(manifest_cmd, sizeof(manifest_cmd), "tar --zstd -tf \"%s\" | grep -v '^\\./\\?\\.fnf-' > /var/db/fnf/installed/%s.manifest", pkg_file, pkgs[i].name);
        system(manifest_cmd);

        // 4. Registrar estado en la DB local
        char db_entry[256];
        snprintf(db_entry, sizeof(db_entry), "/var/db/fnf/installed/%s.list", pkgs[i].name);
        FILE *fp = fopen(db_entry, "w");
        if (fp) {
            const char *reason = (strcmp(pkgs[i].name, explicit_pkg) == 0) ? "manual" : "auto";
            fprintf(fp, "Name: %s\nVersion: %s\nStatus: installed\nReason: %s\nDeps: ", pkgs[i].name, pkgs[i].version, reason);
            for (int d = 0; d < pkgs[i].dep_count; d++) fprintf(fp, "%s,", pkgs[i].deps[d]);
            fprintf(fp, "\n");
            fclose(fp);
        }
        remove(pkg_file);
    }
}

void remove_package(const char *pkg_name) {
    char list_file[256], manifest_file[256];
    snprintf(list_file, sizeof(list_file), "/var/db/fnf/installed/%s.list", pkg_name);
    snprintf(manifest_file, sizeof(manifest_file), "/var/db/fnf/installed/%s.manifest", pkg_name);

    if (access(list_file, F_OK) != 0) {
        printf("Package '%s' is not installed.\n", pkg_name);
        return;
    }

    printf("Removing %s...\n", pkg_name);
    
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "cd /zanta-rootfs && cat \"%s\" | grep -v '/$' | xargs rm -f 2>/dev/null", manifest_file);
    system(cmd);

    remove(list_file);
    remove(manifest_file);
    printf("ok, %s removed.\n", pkg_name);
}

void search_packages(const char *query) {
    printf("Searching for '%s' in repositories...\n\n", query);
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "grep -i '%s' /var/db/fnf/repo/db.txt | awk -F'|' '{print $1 \" - \" $2 \" [\" $5 \"]\"}'", query);
    system(cmd);
}

void list_installed(void) {
    printf("Installed packages:\n");
    system("ls -1 /var/db/fnf/installed/*.list 2>/dev/null | xargs -n1 basename -s .list | column");
}

void autoremove_packages(const char *binary_path) {
    printf("Checking for orphaned dependencies...\n");
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), 
        "for pkg in $(grep -l 'Reason: auto' /var/db/fnf/installed/*.list 2>/dev/null); do "
        "  name=$(basename $pkg .list); "
        "  if ! grep -q \"Deps: .*$name,\" /var/db/fnf/installed/*.list 2>/dev/null; then "
        "    echo \"Found orphan: $name\"; "
        "    %s remove \"$name\"; "
        "  fi; "
        "done", binary_path);
    system(cmd);
}
