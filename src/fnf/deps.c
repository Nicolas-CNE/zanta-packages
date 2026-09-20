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

// Función personalizada para dividir por '|' respetando campos vacíos
static int split_pipes(char *line, char **fields, int max_fields) {
    int count = 0;
    fields[count++] = line;
    while (*line && count < max_fields) {
        if (*line == '|') {
            *line = '\0';
            fields[count++] = line + 1;
        }
        line++;
    }
    return count;
}

static bool find_package_in_db(const char *name, Package *pkg) {
    FILE *fp = fopen("/var/db/fnf/repo/db.txt", "r");
    if (!fp) return false;

    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\r\n")] = 0;
        if (strlen(line) == 0) continue;

        char *fields[10] = {0};
        int nfields = split_pipes(line, fields, 10);

        if (nfields >= 5 && fields[0] && strcmp(fields[0], name) == 0) {
            memset(pkg, 0, sizeof(Package));
            strncpy(pkg->name, fields[0], MAX_NAME - 1);
            if (nfields > 1 && fields[1]) strncpy(pkg->version, fields[1], MAX_NAME - 1);
            if (nfields > 2 && fields[2]) pkg->download_size = strtoul(fields[2], NULL, 10);
            if (nfields > 3 && fields[3]) pkg->disk_size = strtoul(fields[3], NULL, 10);
            if (nfields > 4 && fields[4]) strncpy(pkg->category, fields[4], 31);

            if (nfields > 5 && fields[5] && strlen(fields[5]) > 0) {
                char *dep_save;
                char *dep = strtok_r(fields[5], ",", &dep_save);
                while (dep && pkg->dep_count < MAX_DEPS) {
                    strncpy(pkg->deps[pkg->dep_count++], dep, MAX_NAME - 1);
                    dep = strtok_r(NULL, ",", &dep_save);
                }
            }

            if (nfields > 6 && fields[6] && strlen(fields[6]) > 0) {
                char *weak_save;
                char *weak = strtok_r(fields[6], ",", &weak_save);
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
