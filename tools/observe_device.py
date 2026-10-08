"""Bounded serial observation of an explicitly selected port; no reset by default."""
import argparse,time
from pathlib import Path
import serial

p=argparse.ArgumentParser(description=__doc__)
p.add_argument('--port',required=True);p.add_argument('--seconds',type=float,default=20)
p.add_argument('--output',type=Path,required=True)
p.add_argument('--stop-file',type=Path,help='Stop early when this local marker exists')
a=p.parse_args()
if not 0<a.seconds<=600:p.error('--seconds must be in (0, 600]')
connection=serial.Serial(port=None,baudrate=115200,timeout=0.2)
connection.dtr=False;connection.rts=False;connection.port=a.port
connection.open()
a.output.parent.mkdir(parents=True,exist_ok=True)
try:
    deadline=time.monotonic()+a.seconds
    with a.output.open('wb') as log:
        while time.monotonic()<deadline:
            if a.stop_file and a.stop_file.exists():break
            data=connection.read(max(connection.in_waiting,1))
            if data:log.write(data);log.flush()
finally:connection.close()
print(f'Captured {a.output.stat().st_size} bytes; port closed: {a.port}')
