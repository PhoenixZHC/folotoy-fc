"""Pack owned classic iNES ROMs into a first-install FC container (no device writes)."""
import argparse, hashlib, struct
from pathlib import Path
MAX_IMAGE=0x400000
DATA_END=MAX_IMAGE-8192
ENTRY=struct.Struct('<32sII32s')

def aligned(value): return (value+4095)&~4095

def validate_rom(data):
    if len(data)<16 or data[:4]!=b'NES\x1a': raise ValueError('Not an iNES ROM')
    p,c,flags,high,ram=data[4:9]
    mapper=(flags>>4)|(high&0xf0)
    if not p or high&15 or any(data[9:16]) or ram>1:
        raise ValueError('Only classic iNES NTSC with <=8 KiB PRG RAM is supported')
    shapes={0: p<=2 and c<=1,1:p<=16 and c<=16,2:2<=p<=16 and c==0,
            3:p<=2 and 1<=c<=4,4:2<=p<=32 and c<=32,
            7:2<=p<=16 and p%2==0 and c==0,23:2<=p<=32 and 1<=c<=32,
            66:2<=p<=8 and p%2==0 and 1<=c<=4}
    if not shapes.get(mapper,False): raise ValueError(f'Unsupported mapper or bank layout: {mapper}')
    if len(data)!=16+(512 if flags&4 else 0)+p*16384+c*8192:
        raise ValueError('ROM length does not match header')
    return mapper

def encode_name(name):
    encoded=name.encode('utf-8')
    if not 1<=len(encoded)<=31 or any(ord(c)<32 or 0x7f<=ord(c)<0xa0 or c in '/\\' or ord(c)>0xffff or 0x2000<=ord(c)<=0x200f or ord(c) in (0x2028,0x2029,0xfeff) for c in name):
        raise ValueError('Invalid name: use 1-31 UTF-8 bytes without control characters or slashes')
    return encoded

def pack(paths,names=None):
    if not 1<=len(paths)<=16: raise ValueError('Provide 1-16 ROMs')
    if names is not None and len(names)!=len(paths): raise ValueError('Provide one --name per ROM')
    image=bytearray(b'\xff'*MAX_IMAGE); entries=[]; seen=set(); at=4096
    for index,path in enumerate(paths):
        data=Path(path).read_bytes(); validate_rom(data)
        name=encode_name(names[index] if names is not None else Path(path).stem)
        if name in seen: raise ValueError('Duplicate game name')
        seen.add(name)
        if at+aligned(len(data))>DATA_END: raise ValueError('ROM partition is full')
        image[at:at+len(data)]=data
        entries.append(ENTRY.pack(name,at,len(data),hashlib.sha256(data).digest()))
        at+=aligned(len(data))
    directory=struct.pack('<4sHH',b'FFCR',1,len(entries))+b''.join(entries)
    image[:len(directory)]=directory
    return bytes(image)

def verify(image):
    if len(image)!=MAX_IMAGE or image[:4]!=b'FFCR': raise ValueError('Invalid container')
    version,count=struct.unpack_from('<HH',image,4)
    if version!=1 or not 1<=count<=16: raise ValueError('Invalid directory')
    names=[]; ranges=[]
    for i in range(count):
        raw,at,size,digest=ENTRY.unpack_from(image,8+i*ENTRY.size)
        if b'\0' not in raw: raise ValueError('Unterminated name')
        name=raw.split(b'\0',1)[0].decode('utf-8'); encode_name(name)
        end=at+aligned(size)
        if at<4096 or at%4096 or end>DATA_END or size<16400: raise ValueError('Invalid bounds')
        if name in names or any(at<b and a<end for a,b in ranges): raise ValueError('Overlapping entries')
        data=image[at:at+size]; validate_rom(data)
        if hashlib.sha256(data).digest()!=digest: raise ValueError('Hash mismatch')
        names.append(name); ranges.append((at,end))
    return names

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('roms',nargs='*',type=Path); p.add_argument('--output',type=Path); p.add_argument('--verify',type=Path)
    p.add_argument('--name',action='append',help='Short display name, one per ROM in input order')
    args=p.parse_args()
    if args.verify:
        if args.roms or args.output or args.name: p.error('--verify is standalone')
        print(verify(args.verify.read_bytes()))
    else:
        if not args.output: p.error('--output required')
        image=pack(args.roms,args.name); verify(image)
        args.output.parent.mkdir(parents=True,exist_ok=True); args.output.write_bytes(image)
        print(args.output)
