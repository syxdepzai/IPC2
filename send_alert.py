import smtplib
from email.mime.text import MIMEText
import sys
import os

def send_mail(subject, body, to_email):
    # Đọc thông tin email và mật khẩu từ biến môi trường để bảo mật hơn
    from_email = os.environ.get('ALERT_EMAIL', 'theanh010104@gmail.com')
    password = os.environ.get('ALERT_EMAIL_PASSWORD', 'jijl zdfs rgju tidr')
    msg = MIMEText(body)
    msg['Subject'] = subject
    msg['From'] = from_email
    msg['To'] = to_email

    s = smtplib.SMTP_SSL('smtp.gmail.com', 465)
    s.login(from_email, password)
    s.sendmail(msg['From'], [msg['To']], msg.as_string())
    s.quit()

if __name__ == "__main__":
    if len(sys.argv) != 4:
        print("Usage: python3 send_alert.py <subject> <body> <to_email>")
        sys.exit(1)
    subject = sys.argv[1]
    body = sys.argv[2]
    to_email = sys.argv[3]
    send_mail(subject, body, to_email)
