#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ini.h" // You can grab ini.h and ini.c from GitHub (benhoyt/inih)

typedef struct {
    const char* target_group;
} configuration;

static int handler(void* user, const char* section, const char* name, const char* value) {
    configuration* pconfig = (configuration*)user;

    // Check if the current section matches the group we want to launch
    if (strcmp(section, pconfig->target_group) == 0) {
        printf("Launching %s: %s\n", name, value);
        
        pid_t pid = fork();
        if (pid == 0) {
            // Child process
            char *args[] = {(char*)value, NULL};
            execv(args[0], args);
            perror("execv failed");
            exit(1);
        }
    }
    return 1;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <group_name>\n", argv[0]);
        return 1;
    }

    configuration config;
    config.target_group = argv[1];

    if (ini_parse("apps.ini", handler, &config) < 0) {
        printf("Can't load 'apps.ini'\n");
        return 1;
    }

    return 0;
}
