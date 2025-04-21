import os
from telegram.ext import ApplicationBuilder, MessageHandler, ContextTypes, filters

CTRL_FIFO = "/tmp/controller_fifo"
TELEGRAM_TOKEN = "8102212529:AAF4IEZavrizhJp96fxQouPbUTtaRAWU0M"  # <-- Thay bằng token bot của bạn
# Để bảo mật, chỉ cho phép user_id sau được điều khiển (thay bằng ID Telegram của bạn)
ALLOWED_USER_ID = 1618512706

def send_to_fifo(cmd):
    try:
        with open(CTRL_FIFO, "w") as fifo:
            fifo.write(cmd.strip() + "\n")
        return True
    except Exception as e:
        print("Error writing to FIFO:", e)
        return False

async def handle_command(update, context: ContextTypes.DEFAULT_TYPE):
    user_id = update.message.from_user.id
    if user_id != ALLOWED_USER_ID:
        await update.message.reply_text("Bạn không có quyền điều khiển hệ thống này!")
        return
    cmd = update.message.text.strip().upper()
    if cmd.startswith("SETTHRESHOLD"):
        parts = cmd.split()
        if len(parts) == 2 and parts[1].isdigit():
            success = send_to_fifo(cmd)
            await update.message.reply_text("Đã gửi lệnh: " + cmd if success else "Gửi lệnh thất bại!")
        else:
            await update.message.reply_text("Cú pháp đúng: SETTHRESHOLD <giá trị>")
    elif cmd in ["START", "STOP", "QUIT"]:
        success = send_to_fifo(cmd)
        await update.message.reply_text("Đã gửi lệnh: " + cmd if success else "Gửi lệnh thất bại!")
    else:
        await update.message.reply_text("Chỉ hỗ trợ: START, STOP, SETTHRESHOLD <giá trị>, QUIT")

if __name__ == '__main__':
    import sys
    import asyncio
    if sys.platform == "win32":
        asyncio.set_event_loop_policy(asyncio.WindowsSelectorEventLoopPolicy())
    app = ApplicationBuilder().token(TELEGRAM_TOKEN).build()
    app.add_handler(MessageHandler(filters.TEXT & (~filters.COMMAND), handle_command))
    print("Telegram bot is running...")
    app.run_polling()
