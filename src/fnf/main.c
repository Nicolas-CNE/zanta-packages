#include "fnf.h"
#include <time.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("fnf v%s - Zanta Linux Package Manager\n", FNF_VERSION);
        printf("Commands: install, remove, reinstall, search, list, autoremove, sync\n");
        return 1;
    }

    bool time_spent = false;
    int arg_idx = 1;
    
    if (strcmp(argv[1], "--timespent") == 0) {
        time_spent = true;
        arg_idx = 2;
        if (argc < 3) return 1;
    }

    const char *command = argv[arg_idx];
    const char *pkg_name = (argc > arg_idx + 1) ? argv[arg_idx + 1] : NULL;
    struct timespec start, end;

    if (strcmp(command, "sync") == 0) {
        sync_repositories();
        return 0;
    } else if (strcmp(command, "search") == 0 && pkg_name) {
        search_packages(pkg_name);
        return 0;
    } else if (strcmp(command, "list") == 0) {
        list_installed();
        return 0;
    } else if (strcmp(command, "autoremove") == 0) {
        autoremove_packages(argv[0]);
        return 0;
    } else if (strcmp(command, "remove") == 0 && pkg_name) {
        remove_package(pkg_name);
        return 0;
    } else if ((strcmp(command, "install") == 0 || strcmp(command, "reinstall") == 0) && pkg_name) {
        
        if (strcmp(command, "reinstall") == 0) {
            printf("Reinstalling %s...\n", pkg_name);
            remove_package(pkg_name);
        }

        printf("resolving dependencies for %s... ok\n\n", pkg_name);
        Package install_list[32];
        int pkg_count = 0;

        if (!resolve_dependencies(pkg_name, install_list, &pkg_count)) {
            printf("Package %s not found in repositories.\n", pkg_name);
            return 1;
        }

        size_t total_dl = 0, total_disk = 0;
        int act_deps = 0, act_weaks = 0;

        for (int i = 0; i < pkg_count; i++) {
            total_dl += install_list[i].download_size;
            total_disk += install_list[i].disk_size;
            if (strcmp(install_list[i].name, pkg_name) != 0) {
                if (install_list[i].weak_dep_count == 0) act_deps++;
                else act_weaks++;
            }
        }

        printf("Package to install: %s\n", pkg_name);
        
        if (act_deps > 0) {
            printf("dependencies: ");
            for (int i = 0; i < pkg_count; i++)
                if (install_list[i].weak_dep_count == 0 && strcmp(install_list[i].name, pkg_name) != 0) 
                    printf("%s ", install_list[i].name);
            printf("\n");
        }
        if (act_weaks > 0) {
            printf("weak dependencies: ");
            for (int i = 0; i < pkg_count; i++)
                if (install_list[i].weak_dep_count > 0 && strcmp(install_list[i].name, pkg_name) != 0) 
                    printf("%s ", install_list[i].name);
            printf("\n");
        }

        printf("\ntotal transaction size: %.2f MB\n", (double)(total_dl + total_disk) / 1024 / 1024);
        printf("total size to download: %.2f MB\n", (double)total_dl / 1024 / 1024);
        printf("total size to use on disk: %.2f MB\n\n", (double)total_disk / 1024 / 1024);

        char ans[10];
        printf("Proceed with transaction? [Y/n] ");
        if (fgets(ans, sizeof(ans), stdin) != NULL) {
            if (ans[0] == 'n' || ans[0] == 'N') {
                printf("Transaction cancelled.\n");
                return 0;
            }
        }

        if (time_spent) clock_gettime(CLOCK_MONOTONIC, &start);

        printf("downloading packages...\n");
        for (int i = 0; i < pkg_count; i++) {
            char pkg_file[256];
            snprintf(pkg_file, sizeof(pkg_file), "%s-%s.fnf", install_list[i].name, install_list[i].version);
            download_package(pkg_file, install_list[i].category, "/tmp");
        }

        execute_transaction(install_list, pkg_count, pkg_name);

        if (time_spent) {
            clock_gettime(CLOCK_MONOTONIC, &end);
            double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
            printf("\nok, %d packages installed in %.3fs\n", pkg_count, elapsed);
        } else {
            printf("\nok, %d packages installed.\n", pkg_count);
        }
        return 0;
    }

    printf("Error: Invalid command or missing package name.\n");
    return 1;
}
