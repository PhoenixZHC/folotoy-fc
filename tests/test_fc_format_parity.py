"""Python 打包器与固件 C 解析器用同一输入交叉检查，产物只写 build。"""
from pathlib import Path
import ctypes
import os
import random
import subprocess
import sys
import tempfile

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
from pack_roms import validate_rom,pack,verify
from make_nes2_diagnostic import make_rom

work=Path(tempfile.mkdtemp(prefix='fc-formats-',dir=ROOT/'build'))
lib=work/('parser.dll' if os.name=='nt' else 'parser.so')
subprocess.run([os.getenv('CC','gcc'),'-std=c11','-shared','-fPIC','-Imain',
                'main/fc_rom.c','-o',str(lib)],cwd=ROOT,check=True)
parse=ctypes.CDLL(str(lib)).fc_rom_validate
parse.argtypes=[ctypes.c_void_p,ctypes.c_size_t,ctypes.c_size_t,ctypes.c_void_p]
parse.restype=ctypes.c_int
rng=random.Random(20261012)
base=make_rom()
cases=[base,make_rom(True)]
for mapper,p,c in [(0,1,1),(1,16,0),(2,16,0),(3,2,4),(4,96,32),(7,16,0),(23,32,32),(66,8,4)]:
    h=bytearray(base[:16]);h[4]=p;h[5]=c;h[6]=(mapper&15)<<4;h[7]=8|(mapper&0xf0);h[10]=0;h[11]=0 if c else 7
    cases.append(bytes(h)+bytes(p*16384+c*8192))
for _ in range(800):
    data=bytearray(base)
    for _ in range(rng.randrange(1,4)): data[rng.randrange(4,16)]=rng.randrange(256)
    if rng.randrange(4)==0: data=data[:rng.randrange(16,len(data))]
    cases.append(bytes(data))
for i,data in enumerate(cases):
    buffer=ctypes.create_string_buffer(data); info=ctypes.create_string_buffer(128)
    accepted=parse(buffer,len(data),len(data),info)==0
    try: validate_rom(data); packed=True
    except ValueError: packed=False
    assert accepted==packed,(i,data[:16].hex(),accepted,packed)
for exponent in (False,True):
    path=work/('diagnostic-exp.nes' if exponent else 'diagnostic.nes')
    path.write_bytes(make_rom(exponent))
    assert verify(pack([path]))==[path.stem]
print(f"FC_FORMAT_PARITY_PASS: {len(cases)} headers, two diagnostic containers")
