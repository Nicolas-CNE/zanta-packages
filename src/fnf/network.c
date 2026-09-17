#include "fnf.h"
#include <unistd.h>

#define REPO_URL "https://raw.githubusercontent.com/Nicolas-CNE/zanta-packages/main/packages"

size_t get_remote_file_size(const char *url) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "curl -sI %s | grep -i Content-Length | tail -n1 | awk '{print $2}' | tr -d '\r'", url);
    FILE *fp = popen(cmd, "r");
    if (!fp) return 0;
    size_t size = 0;
    fscanf(fp, "%zu", &size);
    pclose(fp);
    return size;
}

// <-- NUEVO: Recibimos const char *category
bool download_package(const char *pkg_filename, const char *category, const char *dest_path) {
    char local_file[512];
    snprintf(local_file, sizeof(local_file), "%s/%s", dest_path, pkg_filename);
    
    if (access(local_file, F_OK) == 0) {
        printf("downloading %s...\n", pkg_filename);
        printf("[========================================>] 100%% (cached)\n");
        return true;
    }

    printf("downloading %s...\n", pkg_filename);
    char cmd[1024];
    
    // <-- NUEVO: Intercalamos %s (category) en la URL
    snprintf(cmd, sizeof(cmd), "wget -O \"%s\" \"%s/%s/%s\"", local_file, REPO_URL, category, pkg_filename);
    
    int res = system(cmd);
    return (res == 0);
}
