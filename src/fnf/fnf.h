#ifndef FNF_H
#define FNF_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define FNF_VERSION "0.1.0-alpha"
#define MAX_DEPS 32
#define MAX_NAME 64

// Estructura de Paquete
typedef struct Package {
    char name[MAX_NAME];
    char version[MAX_NAME];
    size_t download_size;
    size_t disk_size;
    char category[32];   // <-- NUEVO: Almacenará 'core', 'lib' o 'extra'
    char deps[MAX_DEPS][MAX_NAME];
    int dep_count;
    char weak_deps[MAX_DEPS][MAX_NAME];
    int weak_dep_count;
} Package;

typedef struct DAGNode {
    Package pkg;
    struct DAGNode* neighbors[MAX_DEPS];
    int neighbor_count;
    bool visited;
} DAGNode;

DAGNode* create_dag_node(Package pkg);
bool resolve_dependencies(const char *target_pkg, Package *install_list, int *total_pkgs);
size_t get_remote_file_size(const char *url);
// <-- NUEVO: Actualizamos el prototipo para recibir la categoría
bool download_package(const char *pkg_filename, const char *category, const char *dest_path); 
bool sync_repositories(void);
void execute_transaction(Package *pkgs, int count);

#endif
