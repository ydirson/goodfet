#!/usr/bin/env python3
#
# facedancer-monitor.py

from Facedancer import *

while True:
    try:
        sp = GoodFETSerialPort(timeout=1)
        fd = Facedancer(sp, verbose=3)
    except IOError:
        print(" failed, retrying", file=sys.stderr)
        sp.close()
        continue
    else:
        break

fd.monitor_app.print_info()
fd.monitor_app.list_apps()

res = fd.monitor_app.echo("I am the very model of a modern major general.")

if res == 0:
    print("echo failed")
else:
    print("echo succeeded")
