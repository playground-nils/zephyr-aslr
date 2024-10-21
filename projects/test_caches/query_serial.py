from typing import List
import serial
import time

BAUDRATE = 1500000
PORT = '/dev/ttyUSB0'
CMD =b"""
mmc read $pxefile_addr_r 0x100000 0x11b\n
bootm start $pxefile_addr_r\n
bootm loados\n
bootm go\n
"""


def do_read(serial: serial.Serial, timeout) -> str:
    buffer: List[bytes] = list()
    before: float  = time.time()

    while time.time() - before < timeout :
        if not serial.readable():
            break
        buffer.append(serial.read(1))
        if buffer[-1] == b'\n':
            break

    return b''.join(buffer).decode('utf-8').removesuffix("\n").removesuffix("\r")

def write_csv(res) -> None:
    with open(f"{"caches" if caches else "nocaches"}-{"sctlr.c-off" if sctlr else "sctlr.c-on"}-{time.time()}.csv", "w") as f:
        f.write("size, time(cycles)\n")
        for elem in res:
            f.write(f"{elem}\n")


if __name__ == "__main__":
    res: List[str] = list()
    started: int = -1
    idx: int = 0

    global caches
    global sctlr
    caches: bool = False
    aslr: bool = False

    ser = serial.Serial(PORT, baudrate=BAUDRATE)  # open serial port
    ser.write(CMD)     # write a string

    while True:
        line = do_read(ser, 1)
        print(line)
        if 0 <= started < 2:
            if started == 0:
                caches = line == "y"          
            else:
                sctlr = line == "y"
            started += 1
        elif started >= 2:
            if line == "===END===" :
                break
            idx += 1
            res.append(line)
            continue
        if line == "===START===":
            started = 0
    print(f"has alsr : {caches}")
    write_csv(res)

    ser.close()             # close port

