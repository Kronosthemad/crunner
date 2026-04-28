#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "ini.h" // You can grab ini.h and ini.c from GitHub (benhoyt/inih)
#include <stdbool.h>
#include <sched.h>

#define CONFIG_SUBDIR "crunner"
#define CONFIG_FILE "apps.ini"

static char* get_config_path(void) {
    static char path[1024];
    char* xdg_config_home = getenv("XDG_CONFIG_HOME");
    char* xdg_config_dirs = getenv("XDG_CONFIG_DIRS");
    char home[1024];

    if (!getcwd(home, sizeof(home))) {
        return NULL;
    }

    struct stat st;

    if (xdg_config_home) {
        snprintf(path, sizeof(path), "%s/%s/%s", xdg_config_home, CONFIG_SUBDIR, CONFIG_FILE);
        if (stat(path, &st) == 0) {
            return path;
        }
    } else {
        char* home_env = getenv("HOME");
        if (home_env) {
            snprintf(path, sizeof(path), "%s/.config/%s/%s", home_env, CONFIG_SUBDIR, CONFIG_FILE);
            if (stat(path, &st) == 0) {
                return path;
            }
        }
    }

    if (xdg_config_dirs) {
        char dirs[1024];
        strncpy(dirs, xdg_config_dirs, sizeof(dirs) - 1);
        dirs[sizeof(dirs) - 1] = '\0';
        char* token = strtok(dirs, ":");
        while (token) {
            snprintf(path, sizeof(path), "%s/%s/%s", token, CONFIG_SUBDIR, CONFIG_FILE);
            if (stat(path, &st) == 0) {
                return path;
            }
            token = strtok(NULL, ":");
        }
    }

    snprintf(path, sizeof(path), "%s/%s", home, CONFIG_FILE);
    if (stat(path, &st) == 0) {
        return path;
    }

    return NULL;
}

bool is_running(const char* name) {
    char command[256];
    // Use pgrep -x to look for an exact match of the process name
    snprintf(command, sizeof(command), "pgrep -x %s > /dev/null", name);
    
    int result = system(command);
    return (result == 0); // system returns 0 if pgrep found a match
}

typedef struct {
    const char* target_group;
} configuration;

static int handler(void* user, const char* section, const char* name, const char* value) {
    configuration* pconfig = (configuration*)user;

    // Check if the current section matches the group we want to launch
    	if (is_running(value)) {
    		printf("%s is already running. Skipping.\n", value);
	} else {
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
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <group_name>\n", argv[0]);
        return 1;
    }

    configuration config;
    config.target_group = argv[1];

    char* config_path = get_config_path();
    if (!config_path) {
        printf("Can't find config file. Checked locations:\n");
        char* xdg_config_home = getenv("XDG_CONFIG_HOME");
        char* home = getenv("HOME");
        char* xdg_config_dirs = getenv("XDG_CONFIG_DIRS");
        if (xdg_config_home) {
            printf("  %s/%s/%s\n", xdg_config_home, CONFIG_SUBDIR, CONFIG_FILE);
        } else if (home) {
            printf("  %s/.config/%s/%s\n", home, CONFIG_SUBDIR, CONFIG_FILE);
        }
        if (xdg_config_dirs) {
            printf("  %s/%s/%s\n", xdg_config_dirs, CONFIG_SUBDIR, CONFIG_FILE);
        }
        printf("  ./%s (current directory)\n", CONFIG_FILE);
        return 1;
    }

    printf("Using config: %s\n", config_path);

    if (ini_parse(config_path, handler, &config) < 0) {
        printf("Can't load config: %s\n", config_path);
        return 1;
    }

    return 0;
}
