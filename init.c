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
#include <signal.h>


/**
 * Some pre-processer variables here
 * @author: oddbyte
 */

#define UPDATE_PATH "/secure/ota/update.tar"
#define KEY_PATH "/system/update.key"
#define FIFO_PATH "/secure/init"
#define ZYGOTE_PATH "/system/secbin/zygote"
#define KERNEL_MODULES_PATH "/system/kernel"
#define BUFFER_SIZE 4096

/**
 * Other variables:
 * @author: oddbyte
 */

// Stuff here I guess

/* 
   --------------------------------------------------------
   --------------------- CODE START -----------------------
   --------------------------------------------------------
*/

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
 * Main function from Init binary
 * @author: oddbyte
 */
int main() {
    // Check for OTA files
    if (access(UPDATE_PATH, 0) == 0) {
        if (verify_signature()) {
            apply_update();
        }
    }

    load_kernel_modules();
    
    // Start Zygote
    execl(ZYGOTE_PATH, "zygote", NULL);
    
    return 0;
}