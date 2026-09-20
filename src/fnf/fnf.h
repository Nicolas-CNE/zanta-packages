#ifndef FNF_H
#define FNF_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define FNF_VERSION "0.3.0-clean"
#define MAX_DEPS 32
#define MAX_NAME 64

typedef struct Package {
    char name[MAX_NAME];
    char version[MAX_NAME];
    size_t download_size;
    size_t disk_size;
    char category[32];
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

void execute_transaction(Package *pkgs, int count, const char *explicit_pkg);
void remove_package(const char *pkg_name);
void search_packages(const char *query);
void list_installed(void);
void autoremove_packages(const char *binary_path);

DAGNode* create_dag_node(Package pkg);
bool resolve_dependencies(const char *target_pkg, Package *install_list, int *total_pkgs);
size_t get_remote_file_size(const char *url);
bool download_package(const char *pkg_filename, const char *category, const char *dest_path); 
bool sync_repositories(void);

#endif
