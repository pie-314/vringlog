#!/usr/bin/env python3
import time
from threading import Thread

DEV = "/dev/vringlog"

def producer():
    logs = [
        "System Alert: CPU temp spiking!\n",
        "Network Warning: Dropped packet on eth0\n",
        "Auth Success: User 'admin' logged in\n",
        "Database Info: Flush complete\n",
        "System Alert: Out of bounds read blocked\n" # 5th log triggers buffer wall
    ]

    for log in logs:
        try:
            with open(DEV, "w") as f:
                f.write(log)
                print(f"[TX] {log.strip()}")
        except OSError as e:
            print(f"[TX] Dropped -> '{log.strip()}' ({e.strerror})")
        time.sleep(0.1)

def consumer():
    time.sleep(0.3) # Allow buffer pipeline to prime

    for _ in range(6):
        with open(DEV, "r") as f:
            data = f.read()
            if data:
                print(f"[RX] {data.strip()}")
            else:
                print("[RX] Pipeline dry")
        time.sleep(0.2)

if __name__ == "__main__":
    t1 = Thread(target=producer)
    t2 = Thread(target=consumer)

    t1.start()
    t2.start()

    t1.join()
    t2.join()
