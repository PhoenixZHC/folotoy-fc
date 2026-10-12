"""原创 NES 2.0 / mapper 0 诊断：色条、八键、A 键声音和持久保存标记。"""
from pathlib import Path
import argparse

# 独立 5x7 点阵，不包含商业 ROM、BIOS 或外部图像。
FONT = {
    'A':("01110","10001","10001","11111","10001","10001","10001"),
    'B':("11110","10001","10001","11110","10001","10001","11110"),
    'C':("01111","10000","10000","10000","10000","10000","01111"),
    'D':("11110","10001","10001","10001","10001","10001","11110"),
    'E':("11111","10000","10000","11110","10000","10000","11111"),
    'F':("11111","10000","10000","11110","10000","10000","10000"),
    'G':("01111","10000","10000","10111","10001","10001","01111"),
    'H':("10001","10001","10001","11111","10001","10001","10001"),
    'I':("11111","00100","00100","00100","00100","00100","11111"),
    'K':("10001","10010","10100","11000","10100","10010","10001"),
    'L':("10000","10000","10000","10000","10000","10000","11111"),
    'M':("10001","11011","10101","10101","10001","10001","10001"),
    'N':("10001","11001","10101","10011","10001","10001","10001"),
    'O':("01110","10001","10001","10001","10001","10001","01110"),
    'P':("11110","10001","10001","11110","10000","10000","10000"),
    'R':("11110","10001","10001","11110","10100","10010","10001"),
    'S':("01111","10000","10000","01110","00001","00001","11110"),
    'T':("11111","00100","00100","00100","00100","00100","00100"),
    'U':("10001","10001","10001","10001","10001","10001","01110"),
    'V':("10001","10001","10001","10001","10001","01010","00100"),
    'W':("10001","10001","10001","10101","10101","10101","01010"),
    'X':("10001","10001","01010","00100","01010","10001","10001"),
    'Y':("10001","10001","01010","00100","00100","00100","00100"),
    '0':("01110","10001","10011","10101","11001","10001","01110"),
    '1':("00100","01100","00100","00100","00100","00100","01110"),
    '2':("01110","10001","00001","00010","00100","01000","11111"),
    '3':("11110","00001","00001","01110","00001","00001","11110"),
    '4':("00010","00110","01010","10010","11111","00010","00010"),
    '5':("11111","10000","10000","11110","00001","00001","11110"),
    '6':("01110","10000","10000","11110","10001","10001","01110"),
    '7':("11111","00001","00010","00100","01000","01000","01000"),
    '8':("01110","10001","10001","01110","10001","10001","01110"),
    '9':("01110","10001","10001","01111","00001","00001","01110"),
}

def make_rom(exponent=False):
    code=bytearray(); labels={}; fixups=[]
    def emit(*values): code.extend(values)
    def label(name): labels[name]=0x8000+len(code)
    def branch(op,name): emit(op,0); fixups.append((len(code)-1,name,True))
    def jump(name): emit(0x4c,0,0); fixups.append((len(code)-2,name,False))
    def write(address,value): emit(0xa9,value,0x8d,address&255,address>>8)
    def ppu(address): write(0x2006,address>>8); write(0x2006,address&255)
    def string(row,col,value):
        assert all(c==' ' or c in FONT for c in value)
        ppu(0x2000+row*32+col)
        for c in value: write(0x2007,ord(c))
    emit(0x78,0xd8,0xa2,0xff,0x9a)
    write(0x2000,0); write(0x2001,0); write(0x4017,0x40)
    emit(0xad,0x02,0x20)
    for n in ('warm1','warm2'):
        label(n); emit(0x2c,0x02,0x20); branch(0x10,n)
    ppu(0x3f00)
    for color in [0x0f,0x16,0x2a,0x30]*8: write(0x2007,color)
    ppu(0x2000)
    emit(0xa2,4,0xa0,0,0xa9,0)
    label('clear'); emit(0x8d,7,0x20,0xc8); branch(0xd0,'clear')
    emit(0xca); branch(0xd0,'clear')
    string(2,5,'FC NES 2.0 TEST'.replace('.',' '))  # 字符只用本文件点阵。
    for row in range(4,8):
        ppu(0x2000+row*32+4)
        for color in (1,2,3):
            for _ in range(8): write(0x2007,color)
    string(9,4,'RED    GREEN   WHITE')
    string(11,4,'A  B  SE ST UP DN LT RT')
    string(15,4,'HOLD A FOR SOUND')
    string(17,4,'SELECT CHANGES MARK')
    string(19,4,'SAVE MARK')
    string(23,4,'EXIT GAME TO SAVE')
    string(25,4,'REOPEN CHECK MARK')
    emit(0xad,0,0x60,0xc9,0xa5); branch(0xf0,'loaded')
    write(0x6000,0xa5); write(0x6001,0)
    string(21,4,'NEW SAVE')
    jump('ready')
    label('loaded'); string(21,4,'LOADED SAVE')
    label('ready')
    write(0,0); write(1,0)
    write(0x4015,1); write(0x4000,0xb0); write(0x4001,0)
    write(0x4002,0xfd); write(0x4003,8)
    write(0x2005,0); write(0x2005,0); write(0x2001,0x0a)
    label('frame'); emit(0x2c,2,0x20); branch(0x10,'frame')
    write(0x4016,1); write(0x4016,0)
    emit(0xa2,8)
    label('read'); emit(0xad,0x16,0x40,0x4a,0x26,0,0xca); branch(0xd0,'read')
    # Select 仅在按下沿改变持久标记。
    emit(0xa5,1,0x49,0xff,0x25,0,0x29,0x20); branch(0xf0,'nochange')
    emit(0xee,1,0x60,0xad,1,0x60,0xc9,10); branch(0x90,'nochange')
    write(0x6001,0)
    label('nochange'); emit(0xa5,0,0x85,1,0x85,2)
    emit(0x29,0x80); branch(0xf0,'silent')
    write(0x4000,0xbf); jump('indicators')
    label('silent'); write(0x4000,0xb0)
    label('indicators'); ppu(0x2000+13*32+4)
    emit(0xa2,8)
    label('indicator'); emit(0x06,2,0xa9,ord('0'),0x69,0,0x8d,7,0x20)
    write(0x2007,ord(' ')); write(0x2007,ord(' '))
    emit(0xca); branch(0xd0,'indicator')
    ppu(0x2000+19*32+15)
    emit(0xad,1,0x60,0x18,0x69,ord('0'),0x8d,7,0x20)
    write(0x2005,0); write(0x2005,0)
    jump('frame')
    for at,name,relative in fixups:
        target=labels[name]
        if relative:
            distance=target-(0x8000+at+1)
            assert -128<=distance<=127,(name,distance)
            code[at]=distance&255
        else: code[at:at+2]=target.to_bytes(2,'little')
    assert len(code)<16378
    prg=bytearray([0xea])*16384; prg[:len(code)]=code
    prg[-6:]=bytes([0,0x80])*3
    chr_data=bytearray(8192)
    chr_data[16:24]=b'\xff'*8
    chr_data[40:48]=b'\xff'*8
    chr_data[48:64]=b'\xff'*16
    for char,rows in FONT.items():
        for y,row in enumerate(rows):
            bits=int(row,2)<<2
            chr_data[ord(char)*16+y]=bits
            chr_data[ord(char)*16+y+8]=bits
    header=bytearray(b'NES\x1a'+bytes([1,1,2,8,0,0,0x70,0,0,0,0,1]))
    if exponent: header[4]=56; header[5]=52; header[9]=255
    return bytes(header+prg+chr_data)

if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output',type=Path,default=Path('build/diagnostic-nes2.nes'))
    parser.add_argument('--exponent',action='store_true')
    args=parser.parse_args()
    args.output.parent.mkdir(parents=True,exist_ok=True)
    args.output.write_bytes(make_rom(args.exponent))
    print(args.output)
