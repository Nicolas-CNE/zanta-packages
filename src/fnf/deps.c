#include "fnf.h"

DAGNode* create_dag_node(Package pkg) {
    DAGNode *node = malloc(sizeof(DAGNode));
    if (!node) return NULL;
    node->pkg = pkg;
    node->neighbor_count = 0;
    node->visited = false;
    return node;
}

static bool is_already_added(Package *list, int count, const char *name) {
    for (int i = 0; i < count; i++) {
        if (strcmp(list[i].name, name) == 0) return true;
    }
    return false;
}

static bool find_package_in_db(const char *name, Package *pkg) {
    FILE *fp = fopen("/var/db/fnf/repo/db.txt", "r");
    if (!fp) return false;

    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\r\n")] = 0;
        if (strlen(line) == 0) continue;

        char *saveptr;
        char *p_name = strtok_r(line, "|", &saveptr);
        char *p_ver = strtok_r(NULL, "|", &saveptr);
        char *p_dsize = strtok_r(NULL, "|", &saveptr);
        char *p_ksize = strtok_r(NULL, "|", &saveptr);
        char *p_cat = strtok_r(NULL, "|", &saveptr);  // <-- EXTRAEMOS CATEGORÍA
        char *p_deps = strtok_r(NULL, "|", &saveptr);
        char *p_weaks = strtok_r(NULL, "|", &saveptr);

        if (p_name && strcmp(p_name, name) == 0) {
            memset(pkg, 0, sizeof(Package));
            strncpy(pkg->name, p_name, MAX_NAME - 1);
            if (p_ver) strncpy(pkg->version, p_ver, MAX_NAME - 1);
            if (p_dsize) pkg->download_size = strtoul(p_dsize, NULL, 10);
            if (p_ksize) pkg->disk_size = strtoul(p_ksize, NULL, 10);
            if (p_cat) strncpy(pkg->category, p_cat, 31); // <-- GUARDAMOS CATEGORÍA

            if (p_deps && strlen(p_deps) > 0) {
                char *dep_save;
                char *dep = strtok_r(p_deps, ",", &dep_save);
                while (dep && pkg->dep_count < MAX_DEPS) {
                    strncpy(pkg->deps[pkg->dep_count++], dep, MAX_NAME - 1);
                    dep = strtok_r(NULL, ",", &dep_save);
                }
            }

            if (p_weaks && strlen(p_weaks) > 0) {
                char *weak_save;
                char *weak = strtok_r(p_weaks, ",", &weak_save);
                while (weak && pkg->weak_dep_count < MAX_DEPS) {
                    strncpy(pkg->weak_deps[pkg->weak_dep_count++], weak, MAX_NAME - 1);
                    weak = strtok_r(NULL, ",", &weak_save);
                }
            }

            fclose(fp);
            return true;
        }
    }

    fclose(fp);
    return false;
}

bool resolve_dependencies(const char *target_pkg, Package *install_list, int *total_pkgs) {
    Package target;
    if (!find_package_in_db(target_pkg, &target)) return false;

    for (int i = 0; i < target.dep_count; i++) {
        if (!is_already_added(install_list, *total_pkgs, target.deps[i])) {
            resolve_dependencies(target.deps[i], install_list, total_pkgs);
        }
    }

    for (int i = 0; i < target.weak_dep_count; i++) {
        if (!is_already_added(install_list, *total_pkgs, target.weak_deps[i])) {
            resolve_dependencies(target.weak_deps[i], install_list, total_pkgs);
        }
    }

    if (!is_already_added(install_list, *total_pkgs, target.name)) {
        install_list[(*total_pkgs)++] = target;
    }

    return true;
}
