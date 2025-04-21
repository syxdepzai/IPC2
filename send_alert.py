import sys
import requests

def send_telegram_alert(message):
    token = "8102212529:AAF4IEZavrizhuJp96fxQouPbUTtaRAWUOM"  # Thay bằng token bot của bạn
    chat_id = 1618512706  # Thay bằng user_id/chat_id của bạn
    url = f"https://api.telegram.org/bot{token}/sendMessage"
    data = {"chat_id": chat_id, "text": message}
    requests.post(url, data=data)

if __name__ == "__main__":
    if len(sys.argv) > 1:
        msg = sys.argv[1]
    else:
        msg = "Cảnh báo: Lưu lượng vượt ngưỡng!"
    send_telegram_alert(msg)
