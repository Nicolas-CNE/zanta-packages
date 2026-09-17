#include "fnf.h"
#include <time.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("fnf v%s - Zanta Linux Package Manager\nUso: fnf <command> [pkg]\n", FNF_VERSION);
        return 1;
    }

    if (strcmp(argv[1], "sync") == 0) {
        sync_repositories();
        return 0;
    }

    if (strcmp(argv[1], "install") == 0) {
        if (argc < 3) {
            printf("Error: Especifica un paquete para instalar.\n");
            return 1;
        }

        const char *pkg_name = argv[2];
        printf("resolving dependencies for %s... ok\n\n", pkg_name);

        Package install_list[16];
        int pkg_count = 0;

        if (!resolve_dependencies(pkg_name, install_list, &pkg_count)) {
            printf("Package %s not found.\n", pkg_name);
            return 1;
        }

        size_t total_download = 0;
        size_t total_disk = 0;

        printf("Package to install: %s\n", pkg_name);
        printf("dependencies: ");
        for (int i = 0; i < pkg_count; i++) {
            if (install_list[i].weak_dep_count == 0 && strcmp(install_list[i].name, pkg_name) != 0) 
                printf("%s ", install_list[i].name);
        }
        printf("\nweak dependencies: ");
        for (int i = 0; i < pkg_count; i++) {
            if (install_list[i].weak_dep_count > 0 && strcmp(install_list[i].name, pkg_name) != 0) 
                printf("%s ", install_list[i].name);
        }
        printf("\n\n");

        for (int i = 0; i < pkg_count; i++) {
            total_download += install_list[i].download_size;
            total_disk += install_list[i].disk_size;
        }

        printf("total transaction size: %.2f MB\n", (double)(total_download + total_disk) / 1024 / 1024);
        printf("total size to download: %.2f MB\n", (double)total_download / 1024 / 1024);
        printf("total size to use on disk: %.2f MB\n\n", (double)total_disk / 1024 / 1024);

        printf("downloading packages...\n");
        for (int i = 0; i < pkg_count; i++) {
            char pkg_filename[256];
            snprintf(pkg_filename, sizeof(pkg_filename), "%s-%s.fnf", install_list[i].name, install_list[i].version);
            // <-- NUEVO: Ahora pasamos la categoría a la función de red
            download_package(pkg_filename, install_list[i].category, "/tmp");
        }

        clock_t start = clock();
        execute_transaction(install_list, pkg_count);
        clock_t end = clock();

        double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
        printf("\nok, %d packages installed in %.2fs\n", pkg_count, elapsed);
        return 0;
    }

    return 0;
}
