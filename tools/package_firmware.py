"""Create and verify an erased 8 MiB image; never writes to a device."""
import argparse, hashlib, json, shutil, re, struct
from pathlib import Path
from verify_firmware import parse_partition_table, verify_firmware_layout
from pack_roms import verify as verify_roms

def package(build, output, roms=None):
    config=json.loads((build/'flasher_args.json').read_text(encoding='utf-8'))
    cache=(build/'CMakeCache.txt').read_text(encoding='utf-8')
    test_build=bool(re.search(r'^FC_SIM_TEST:[^=]+=ON$',cache,re.M))
    if test_build and output.name=='FoloToy-FC-full.bin':
        raise ValueError('Test firmware must have a distinct SIM/diagnostic output name')
    app=(build/'FoloToy-FC.bin').read_bytes()
    elf_digest=hashlib.sha256((build/'FoloToy-FC.elf').read_bytes()).digest()
    if struct.unpack_from('<I',app,32)[0]!=0xabcd5432 or app[176:208]!=elf_digest:
        raise ValueError('Application image and ELF do not match')
    if config['flash_settings']['flash_size']!='8MB': raise ValueError('Expected 8MB flash')
    image=bytearray(b'\xff'*0x800000); ranges=[]; sources={}
    def place(offset, data, name):
        end=offset+len(data)
        if offset<0 or end>len(image) or any(offset<b and a<end for a,b in ranges):
            raise ValueError(f'Overlapping/out-of-range image: {name}')
        image[offset:end]=data; ranges.append((offset,end))
        sources[name]={'offset':offset,'bytes':len(data),'sha256':hashlib.sha256(data).hexdigest()}
    for address, name in config['flash_files'].items():
        place(int(address,0),(build/name).read_bytes(),name)
    table_offset=int(config['partition-table']['offset'],0)
    partitions,_=parse_partition_table(image[table_offset:table_offset+0xc00])
    parts={p.label:p for p in partitions}
    saves=(build/'saves.bin').read_bytes()
    if len(saves)!=parts['saves'].size: raise ValueError('Save image size mismatch')
    place(parts['saves'].offset,saves,'saves.bin')
    if roms:
        data=roms.read_bytes();verify_roms(data)
        if len(data)!=parts['roms'].size: raise ValueError('ROM image size mismatch')
        place(parts['roms'].offset,data,'local-roms.bin')
    verify_firmware_layout(image,build,table_offset,int(config['app']['offset'],0))
    output.parent.mkdir(parents=True,exist_ok=True); output.write_bytes(image)
    if output.read_bytes()!=image: raise ValueError('Merged image readback mismatch')
    for name in ('FoloToy-FC.elf','FoloToy-FC.map','sdkconfig'):
        source=build/name if name!='sdkconfig' else build.parent/name
        if source.exists(): shutil.copy2(source,output.parent/name)
    manifest={'firmware':output.name,'bytes':len(image),'sha256':hashlib.sha256(image).hexdigest(),
              'variant':'simulator-test' if test_build else 'production-entry',
              'contains_local_roms':bool(roms),'flash_offset':0,'images':sources}
    elf=output.parent/'FoloToy-FC.elf'
    manifest['elf_sha256']=hashlib.sha256(elf.read_bytes()).hexdigest()
    output.with_suffix('.json').write_text(json.dumps(manifest,indent=2)+'\n',encoding='utf-8')
    for name in ('README.md','THIRD_PARTY.md'):
        source=build.parent/name
        if source.is_file(): shutil.copy2(source,output.parent/name)
    license_dir=build.parent/'licenses'
    if license_dir.is_dir(): shutil.copytree(license_dir,output.parent/'licenses',dirs_exist_ok=True)
    print(output)
    return manifest

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--build',type=Path,default=Path('build'))
    p.add_argument('--output',type=Path,default=Path('build/release/FoloToy-FC-full.bin'))
    p.add_argument('--roms',type=Path)
    a=p.parse_args();package(a.build.resolve(),a.output.resolve(),a.roms)
