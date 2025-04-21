# Network monitor bằng C ứng dụng IPC (Inter Process Communication)

## Installation
```bash
git clone https://github.com/syxdepzai/IPC2.git
cd IPC2
sudo apt-get update
sudo apt-get install build-essential libcurl4-openssl-dev libpcap-dev
pip3 install python-telegram-bot
```

## Build project
```bash
make setup
make telegram_bot
```

## Run các tiến trình
Mở nhiều terminal và chạy tuần tự:
```bash
./analyzer
./logger
./monitor {PID_analyzer}
./controller
```

**Hoặc điều khiển từ xa qua Telegram: thông qua bot Telegram**

## Cảnh báo qua Telegram 
- Khi vượt ngưỡng, analyzer sẽ gửi cảnh báo trực tiếp về Telegram.
- **Toàn bộ cảnh báo đều gửi qua Telegram Bot.**

### Hướng dẫn tạo Telegram Bot và lấy chat_id
1. **Tạo bot mới**
   - Mở Telegram, tìm "@BotFather" và gửi lệnh `/newbot`.
   - Đặt tên và username cho bot, bạn sẽ nhận được **token** dạng `123456:ABC-DEF1234ghIkl-zyx57W2v1u123ew11`.
2. **Lấy chat_id của bạn**
   - Tìm bot vừa tạo, nhấn "Start" để bắt đầu chat với bot.
   - Truy cập:
     ```
     https://api.telegram.org/bot<TOKEN>/getUpdates
     ```
     (thay <TOKEN> bằng token của bạn)
   - Gửi một tin nhắn bất kỳ cho bot trên Telegram, reload link trên, tìm trường `"chat":{"id":...}` chính là chat_id của bạn.
3. **Cấu hình vào mã nguồn**
   - Mở file `analyzer.c`, tìm dòng:
     ```c
     const char *token = "..."; // Thay bằng token bot của bạn
     const char *chat_id = "..."; // Thay bằng chat_id của bạn
     ```
   - Thay đúng token và chat_id bạn vừa lấy.

---

## Các lệnh controller hỗ trợ
- **START**: Bật lại chức năng ghi log và gửi cảnh báo của analyzer.
- **STOP**: Tạm dừng ghi log và gửi cảnh báo.
- **SETTHRESHOLD <giá trị>**: Đổi ngưỡng cảnh báo (bytes/second).
- **QUIT**: Dừng analyzer và giải phóng tài nguyên.

## Lưu ý
- Đường dẫn FIFO `/tmp/...` chỉ phù hợp với Linux.
- Token Telegram, chat_id nên để ở biến môi trường/file cấu hình để bảo mật nếu dùng cho sản phẩm thực tế.
- Nếu gặp lỗi build, kiểm tra lại các thư viện phụ thuộc đã cài đủ chưa.


