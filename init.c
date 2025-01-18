#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <sys/reboot.h>

#define UPDATE_PATH "/secure/ota/update.tar"
#define KEY_PATH "/system/update.key"
#define FIFO_PATH "/secure/init"
#define ZYGOTE_PATH "/system/secbin/zygote"
#define KERNEL_MODULES_PATH "/system/kernel"
#define BUFFER_SIZE 4096

/**
 * TODO: Add GPG verification to this.
 * @author: oddbyte
 */
int verify_signature() {
    return 1;
}

/**
 * Applies a OTA update.
 * @author: oddbyte
 */
void apply_update() {
    char cmd[256];
    system("mount -o remount,rw /");
    snprintf(cmd, sizeof(cmd), "cd / && tar xf %s", UPDATE_PATH);
    system(cmd);
    unlink(UPDATE_PATH);
    sync();
    reboot(RB_AUTOBOOT);
}

/**
 * Loads all kernel modules for DotDot
 * @author: oddbyte
 */
void load_kernel_modules() {
    DIR *dir = opendir(KERNEL_MODULES_PATH);
    if (!dir) return;
    
    struct dirent *entry;
    char cmd[256];
    
    while ((entry = readdir(dir))) {
        if (strstr(entry->d_name, ".ko")) {
            snprintf(cmd, sizeof(cmd), "insmod %s/%s", 
                    KERNEL_MODULES_PATH, entry->d_name);
            system(cmd);
        }
    }
    closedir(dir);
}

/**
 * Allows applications that can write to the FIFO to send shutdown & reboot requests.
 * @author: oddbyte
 */
void handle_command(const char *cmd) {
    if (strcmp(cmd, "shutdown") == 0) {
        sync();
        reboot(RB_POWER_OFF);
    } else if (strcmp(cmd, "reboot") == 0) {
        sync();
        reboot(RB_AUTOBOOT);
    }
}

/**
 * Makes the FIFO file. See handle_command() for more info.
 * @author: oddbyte
 */
void start_command_listener() {
    mkfifo(FIFO_PATH, 0600);
    char buffer[32];
    
    while (1) {
        int fd = open(FIFO_PATH, O_RDONLY);
        if (fd < 0) continue;
        
        memset(buffer, 0, sizeof(buffer));
        read(fd, buffer, sizeof(buffer) - 1);
        close(fd);
        
        handle_command(buffer);
    }
}

/**
 * Main function from Init binary
 * @author: oddbyte
 */
int main() {
    if (access(UPDATE_PATH, F_OK) == 0) {
        if (verify_signature()) {
            apply_update();
        }
    }
    
    pid_t pid = fork();
    if (pid == 0) {
        execl(ZYGOTE_PATH, "zygote", NULL);
        exit(1);
    }
    
    load_kernel_modules();
    start_command_listener();
    
    return 0;
}