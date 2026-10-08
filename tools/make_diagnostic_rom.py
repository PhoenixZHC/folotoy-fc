"""Generate our own mapper-0 diagnostic; no third-party game or BIOS bytes."""
from pathlib import Path
import sys

def make_rom(battery=True):
    code=bytearray()
    def emit(*values): code.extend(values)
    def write(address,value): emit(0xa9,value,0x8d,address&255,address>>8)
    emit(0x78,0xd8,0xa2,0xff,0x9a) # SEI, CLD, LDX, TXS
    write(0x2000,0); write(0x2001,0)
    emit(0xad,0x02,0x20) # Reset PPU address latch.
    write(0x2006,0x3f); write(0x2006,0)
    for color in [0x0f,0x16,0x2a,0x30]*8: write(0x2007,color)
    write(0x2006,0x20); write(0x2006,0)
    emit(0xa2,0x04,0xa0,0x00,0xa9,0x00)
    # 1024 name/attribute bytes, tile zero.
    emit(0x8d,0x07,0x20,0xc8,0xd0,0xfa,0xca,0xd0,0xf7)
    write(0x2005,0); write(0x2005,0); write(0x2000,0); write(0x2001,0x0a)
    write(0x4015,1); write(0x4000,0xbf); write(0x4001,0); write(0x4002,0xfd); write(0x4003,0x08)
    loop=0x8000+len(code)
    write(0x4016,1); write(0x4016,0)
    emit(0xad,0x16,0x40,0x29,1,0x8d,0x00,0x60) # A button -> SRAM[0]
    emit(0xee,0x01,0x60) # CPU heartbeat in SRAM[1].
    emit(0x4c,loop&255,loop>>8)
    prg=bytearray([0xea])*16384
    prg[:len(code)]=code
    prg[-6:]=bytes([0,0x80,0,0x80,0,0x80])
    chr_data=bytearray(8192)
    chr_data[:8]=bytes([0xaa])*8
    return b'NES\x1a'+bytes([1,1,2 if battery else 0,0])+bytes(8)+prg+chr_data

if __name__=='__main__':
    path=Path(sys.argv[1] if len(sys.argv)>1 else 'build/diagnostic.nes')
    path.parent.mkdir(parents=True,exist_ok=True)
    path.write_bytes(make_rom())
    print(path)
