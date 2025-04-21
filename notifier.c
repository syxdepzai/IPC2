#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#define ALERT_FIFO "/tmp/analyzer_notifier_fifo"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <receiver_email>\n", argv[0]);
        return 1;
    }
    char *receiver_email = argv[1];
    // Tạo FIFO nếu chưa tồn tại
    mkfifo(ALERT_FIFO, 0666);
    char buffer[256];
    while (1) {
        int fd = open(ALERT_FIFO, O_RDONLY);
        if (fd == -1) {
            perror("open ALERT_FIFO");
            sleep(1);
            continue;
        }
        int n = read(fd, buffer, sizeof(buffer)-1);
        if (n > 0) {
            buffer[n] = '\0';
            printf("Notifier nhận cảnh báo: %s\n", buffer);
            // Gọi script gửi email
            char cmd[512];
            snprintf(cmd, sizeof(cmd),
                "python3 send_alert.py 'Cảnh báo mạng' '%s' '%s'", buffer, receiver_email);
            system(cmd);
        }
        close(fd);
    }
    return 0;
}
