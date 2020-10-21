import watchdog.events
import watchdog.observers
import time
import pathlib
import smtplib

#To run on local host port is set to 1025 and smtp_server to "localhost". To run local SMTP server use "python -m smtpd -c DebuggingServer -n localhost:1025" in command prompt
#login is not required while running with local SMTP server



curPath=pathlib.Path().absolute()

port = 1025  # For SSL
smtp_server = "localhost"
sender_email = "my@gmail.com"  # Enter your address
receiver_email = "receiver@gmail.com"  # Enter receiver address
#password = input("Type your password and press enter: ")

def sendmail(msg):
    with smtplib.SMTP(smtp_server, port) as server:
        server.sendmail(sender_email, receiver_email, msg)


class Handler(watchdog.events.PatternMatchingEventHandler):
    def __init__(self):
        watchdog.events.PatternMatchingEventHandler.__init__(self, patterns=['*.img'],
                                                             ignore_directories=True, case_sensitive=False)

    def on_created(self, event):
        print("Watchdog received created event - % s." % event.src_path)
        message=("""
        
        Subject: New file Received in system
        
        New file: % s"""""% event.src_path)
        sendmail(message)
        
        # Event is created, you can process it now

   # def on_modified(self, event):
   #     print("Watchdog received modified event - % s." % event.src_path)
        # Event is modified, you can process it now


if __name__ == "__main__":
    src_path = curPath
    event_handler = Handler()
    observer = watchdog.observers.Observer()
    observer.schedule(event_handler, path=src_path, recursive=True)
    observer.start()
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        observer.stop()
    observer.join()
