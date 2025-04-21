import os
from telegram.ext import Updater, MessageHandler, Filters

CTRL_FIFO = "/tmp/controller_fifo"
TELEGRAM_TOKEN = "YOUR_TELEGRAM_BOT_TOKEN"  # <-- Thay bằng token bot của bạn
# Để bảo mật, chỉ cho phép user_id sau được điều khiển (thay bằng ID Telegram của bạn)
ALLOWED_USER_ID = 123456789

def send_to_fifo(cmd):
    try:
        with open(CTRL_FIFO, "w") as fifo:
            fifo.write(cmd.strip() + "\n")
        return True
    except Exception as e:
        print("Error writing to FIFO:", e)
        return False

def handle_command(update, context):
    user_id = update.message.from_user.id
    if user_id != ALLOWED_USER_ID:
        update.message.reply_text("Bạn không có quyền điều khiển hệ thống này!")
        return
    cmd = update.message.text.strip().upper()
    if cmd.startswith("SETTHRESHOLD"):
        parts = cmd.split()
        if len(parts) == 2 and parts[1].isdigit():
            success = send_to_fifo(cmd)
            update.message.reply_text("Đã gửi lệnh: " + cmd if success else "Gửi lệnh thất bại!")
        else:
            update.message.reply_text("Cú pháp đúng: SETTHRESHOLD <giá trị>")
    elif cmd in ["START", "STOP", "QUIT"]:
        success = send_to_fifo(cmd)
        update.message.reply_text("Đã gửi lệnh: " + cmd if success else "Gửi lệnh thất bại!")
    else:
        update.message.reply_text("Chỉ hỗ trợ: START, STOP, SETTHRESHOLD <giá trị>, QUIT")

def main():
    updater = Updater(TELEGRAM_TOKEN, use_context=True)
    dp = updater.dispatcher
    dp.add_handler(MessageHandler(Filters.text & ~Filters.command, handle_command))
    updater.start_polling()
    print("Telegram bot is running...")
    updater.idle()

if __name__ == '__main__':
    main()
