# Network monitor bằng c ứng dụng IPC (Inter Process Communication)

# Installation
```bash
git clone https://github.com/syxdepzai/IPC2.git
```
# Usage
Use make to compile 
```bash
make setup
```
Run 
```bash
./analyzer
./logger
./monitor {PID of analyzer} 
./notifier {receiver email}
./controller
```
## Các lệnh controller hỗ trợ

- **START**  
  Bật lại chức năng ghi log và gửi cảnh báo của analyzer (nếu trước đó đã STOP).
  ```
  START
  ```

- **STOP**  
  Tạm dừng chức năng ghi log và gửi cảnh báo của analyzer (analyzer vẫn chạy, chỉ không gửi alert/log).
  ```
  STOP
  ```

- **SETTHRESHOLD <giá trị>**  
  Đổi ngưỡng cảnh báo (bytes/second) cho analyzer.
  ```
  SETTHRESHOLD 50000
  ```
  Sau lệnh này, analyzer sẽ chỉ gửi cảnh báo nếu vượt qua ngưỡng mới.

- **QUIT**  
  Tắt hoàn toàn analyzer, giải phóng tài nguyên IPC.
  ```
  QUIT
  ```
