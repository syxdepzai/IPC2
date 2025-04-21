#include "common.h"
#include <stdbool.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#define CTRL_FIFO "/tmp/controller_fifo"

volatile sig_atomic_t got_signal = 0; // Cờ báo hiệu nhận được signal

void sigusr1_handler(int sig) {
    got_signal = 1;
    // Chú ý: Không nên làm gì phức tạp (như printf) trong signal handler
    // Chỉ nên đặt cờ hiệu.
}

int shmid;
int semid;
int fifo_fd = -1;
struct net_stats *shared_data = NULL;

// Hàm dọn dẹp tài nguyên IPC
void cleanup_ipc() {
    printf("Analyzer cleaning up IPC resources...\n");
    if (shared_data != NULL && shared_data != (void *)-1) {
        shmdt(shared_data);
    }
    if (shmid != -1) {
        shmctl(shmid, IPC_RMID, NULL); // Xóa Shared Memory
    }
    if (semid != -1) {
        semctl(semid, 0, IPC_RMID);    // Xóa Semaphore set
    }
     if (fifo_fd != -1) {
        close(fifo_fd);
    }
    unlink(FIFO_PATH); // Xóa file Named Pipe
}

// Hàm xử lý tín hiệu завершения (SIGINT, SIGTERM)
void term_handler(int sig) {
    printf("\nAnalyzer received signal %d, shutting down...\n", sig);
    cleanup_ipc();
    exit(EXIT_SUCCESS);
}

int main() {
    shmid = -1; // Khởi tạo để cleanup có thể kiểm tra
    semid = -1;

    // 1. Đăng ký signal handler cho SIGUSR1 (từ Monitor) và tín hiệu kết thúc
    struct sigaction sa_usr1, sa_term;

    sa_usr1.sa_handler = sigusr1_handler;
    sigemptyset(&sa_usr1.sa_mask);
    sa_usr1.sa_flags = SA_RESTART; // Khởi động lại system call bị ngắt bởi signal (nếu có thể)
    if (sigaction(SIGUSR1, &sa_usr1, NULL) == -1) {
        perror("sigaction(SIGUSR1)");
        exit(EXIT_FAILURE);
    }

    sa_term.sa_handler = term_handler;
    sigemptyset(&sa_term.sa_mask);
    sa_term.sa_flags = 0;
    if (sigaction(SIGINT, &sa_term, NULL) == -1 || sigaction(SIGTERM, &sa_term, NULL) == -1) {
         perror("sigaction(SIGINT/SIGTERM)");
         exit(EXIT_FAILURE);
    }


    // 2. Tạo Shared Memory
    shmid = shmget(SHM_KEY, sizeof(struct net_stats), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget (analyzer)");
        exit(EXIT_FAILURE);
    }
    shared_data = (struct net_stats *)shmat(shmid, NULL, 0);
    if (shared_data == (void *)-1) {
        perror("shmat (analyzer)");
        shmctl(shmid, IPC_RMID, NULL); // Dọn dẹp shm nếu shmat lỗi
        exit(EXIT_FAILURE);
    }
    // Khởi tạo dữ liệu trong shared memory (tùy chọn)
    memset(shared_data, 0, sizeof(struct net_stats));

    // 3. Tạo và khởi tạo Semaphore
    semid = semget(SEM_KEY, 1, IPC_CREAT | 0666);
    if (semid == -1) {
        perror("semget (analyzer)");
        cleanup_ipc(); // Dọn dẹp cả shm
        exit(EXIT_FAILURE);
    }
    union semun arg;
    arg.val = 1; // Khởi tạo giá trị semaphore là 1 (cho phép 1 tiến trình truy cập)
    if (semctl(semid, 0, SETVAL, arg) == -1) {
        perror("semctl SETVAL (analyzer)");
        cleanup_ipc(); // Dọn dẹp shm và sem
        exit(EXIT_FAILURE);
    }

    // 4. Tạo Named Pipe (FIFO)
    if (mkfifo(FIFO_PATH, 0666) == -1) {
        if (errno != EEXIST) { // Bỏ qua nếu file đã tồn tại
            perror("mkfifo (analyzer)");
            cleanup_ipc();
            exit(EXIT_FAILURE);
        }
    }

    // 5. Mở FIFO ở chế độ ghi (blocking cho đến khi Logger mở để đọc)
    printf("Analyzer (PID: %d) waiting for Logger to connect to FIFO %s...\n", getpid(), FIFO_PATH);
    fifo_fd = open(FIFO_PATH, O_WRONLY);
    if (fifo_fd == -1) {
        perror("open FIFO for writing (analyzer)");
        cleanup_ipc();
        exit(EXIT_FAILURE);
    }
     printf("Analyzer connected to Logger via FIFO.\n");

    // Tạo FIFO điều khiển
    if (mkfifo(CTRL_FIFO, 0666) == -1) {
        if (errno != EEXIST) { // Bỏ qua nếu file đã tồn tại
            perror("mkfifo (analyzer) for CTRL_FIFO");
            cleanup_ipc();
            exit(EXIT_FAILURE);
        }
    }

    printf("Analyzer (PID: %d) started. Waiting for signals...\n", getpid());
    printf("Run monitors like: ./monitor %d\n", getpid());

    struct net_stats last_stats = {0}; // Lưu trạng thái trước đó để so sánh
    long long rx_byte_threshold = 200; // Ví dụ: ngưỡng 1MB/s (cần tính rate)

    int logging_enabled = 1;
    int alert_threshold = 1000000; // ví dụ giá trị mặc định

    // 6. Vòng lặp chính: chờ tín hiệu và xử lý
    while (1) {
        // --- Đọc lệnh từ controller FIFO ---
        int ctrl_fd = open(CTRL_FIFO, O_RDONLY | O_NONBLOCK);
        if (ctrl_fd != -1) {
            char ctrl_buf[128];
            int ctrl_bytes = read(ctrl_fd, ctrl_buf, sizeof(ctrl_buf)-1);
            if (ctrl_bytes > 0) {
                ctrl_buf[ctrl_bytes] = '\0';
                printf("Analyzer nhận lệnh: %s\n", ctrl_buf);
                if (strncmp(ctrl_buf, "STOP", 4) == 0) {
                    logging_enabled = 0;
                    printf("Analyzer: STOP logging!\n");
                } else if (strncmp(ctrl_buf, "START", 5) == 0) {
                    logging_enabled = 1;
                    printf("Analyzer: START logging!\n");
                } else if (strncmp(ctrl_buf, "SETTHRESHOLD", 12) == 0) {
                    int new_threshold = atoi(ctrl_buf + 13);
                    alert_threshold = new_threshold;
                    printf("Analyzer: Đã đổi alert threshold thành %d\n", alert_threshold);
                } else if (strncmp(ctrl_buf, "QUIT", 4) == 0) {
                    printf("Analyzer: Nhận lệnh QUIT, thoát chương trình!\n");
                    close(ctrl_fd);
                    break;
                }
            }
            close(ctrl_fd);
        }
        // --- Kết thúc đọc lệnh ---

        // Chờ tín hiệu một cách hiệu quả
        pause(); // Tạm dừng tiến trình cho đến khi có tín hiệu

        if (got_signal) {
            got_signal = 0; // Reset cờ hiệu

            struct net_stats current_stats;

            // 7. Đồng bộ: Đợi semaphore
             if (semaphore_op(semid, -1) == -1) continue; // Bỏ qua nếu không đợi được


            // 8. Đọc dữ liệu từ Shared Memory
            memcpy(&current_stats, shared_data, sizeof(struct net_stats));

            // 9. Đồng bộ: Giải phóng semaphore
            if (semaphore_op(semid, 1) == -1) {
                // Log lỗi nhưng vẫn tiếp tục phân tích dữ liệu đã đọc
                 fprintf(stderr, "Analyzer: Failed to signal semaphore after reading.\n");
            }

            // 10. Phân tích dữ liệu (ví dụ đơn giản)
            printf("Analyzer received data: RX bytes=%lld, TX bytes=%lld at %ld\n",
                   current_stats.rx_bytes, current_stats.tx_bytes, current_stats.timestamp);

             // Tính toán sự thay đổi so với lần đọc trước
             if (last_stats.timestamp != 0 && current_stats.timestamp > last_stats.timestamp) {
                 time_t time_diff = current_stats.timestamp - last_stats.timestamp;
                 long long rx_diff = current_stats.rx_bytes - last_stats.rx_bytes;
                 //long long tx_diff = current_stats.tx_bytes - last_stats.tx_bytes;

                 if (time_diff > 0) {
                      long long rx_rate = rx_diff / time_diff; // Bytes per second
                      printf("Analyzer calculated RX rate: %lld B/s\n", rx_rate);

                      // Kiểm tra ngưỡng
                      if (rx_rate > alert_threshold) {
                          char alert_msg[256];
                          snprintf(alert_msg, sizeof(alert_msg),
                                   "ALERT: High RX rate detected on %s: %lld B/s (Threshold: %lld B/s)",
                                   NET_INTERFACE, rx_rate, alert_threshold);
                          printf("Analyzer: Sending alert: %s\n", alert_msg);

                          // 11. Gửi cảnh báo đến Logger qua Pipe
                          if (write(fifo_fd, alert_msg, strlen(alert_msg) + 1) == -1) { // +1 để gửi cả null terminator
                              perror("write to FIFO (analyzer)");
                              // Logger có thể đã chết, cân nhắc xử lý (ví dụ: thử mở lại FIFO)
                          }

                          // === Gửi cảnh báo đến Notifier qua FIFO riêng ===
                          int alert_fd = open("/tmp/analyzer_notifier_fifo", O_WRONLY | O_NONBLOCK);
                          if (alert_fd != -1) {
                              write(alert_fd, alert_msg, strlen(alert_msg) + 1);
                              close(alert_fd);
                          } else {
                              perror("open ALERT_FIFO for notifier");
                          }
                      }
                 }
             }

             // Cập nhật trạng thái cuối cùng
            memcpy(&last_stats, &current_stats, sizeof(struct net_stats));

        }
    }

    // Đoạn mã này thường không đạt được do vòng lặp vô hạn,
    // việc dọn dẹp được xử lý bởi term_handler
    cleanup_ipc();
    return 0;
}
