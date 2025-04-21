#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#define CTRL_FIFO "/tmp/controller_fifo"

int main() {
    mkfifo(CTRL_FIFO, 0666);
    char cmd[128];
    while (1) {
        printf("Nhập lệnh (START/STOP/SETTHRESHOLD <value>/QUIT): ");
        if (fgets(cmd, sizeof(cmd), stdin) == NULL) break;
        int fd = open(CTRL_FIFO, O_WRONLY);
        if (fd != -1) {
            write(fd, cmd, strlen(cmd));
            close(fd);
        }
        if (strncmp(cmd, "QUIT", 4) == 0) break;
    }
    return 0;
}
