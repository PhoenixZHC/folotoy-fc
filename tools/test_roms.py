"""Run accepted local ROMs in isolated host processes; does not redistribute ROMs."""
import argparse, concurrent.futures, json, os, shutil, subprocess, sys
from pathlib import Path
from pack_roms import validate_rom
ROOT=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('directory',type=Path);p.add_argument('--frames',type=int,default=300)
p.add_argument('--draw-every',type=int,choices=(1,2,3),default=2)
args=p.parse_args();out=ROOT/'build/rom-tests';out.mkdir(parents=True,exist_ok=True)
core=ROOT/'components/nofrendo'
sources=[core/'nes'/f'{n}.c' for n in ('nes','cpu','ppu','apu','mem','mmc','input')]
sources += [core/'mappers'/f'map{n}.c' for n in ('000','001','002','003','004','007','023','066')]
exe=out/'rom_runner.exe'
cmd=[os.getenv('CC','gcc'),'-std=gnu11','-O2','-ffunction-sections','-fdata-sections','-Imain','-Icomponents/nofrendo','-Icomponents/nofrendo/nes',
     'tests/rom_runner.c','main/fc_port.c','main/fc_rom.c',str(core/'nofrendo.c'),*map(str,sources),'-Wl,--gc-sections','-lm','-o',str(exe)]
subprocess.run(cmd,cwd=ROOT,check=True)
paths=sorted(args.directory.rglob('*.nes'))
def run(item):
    index,path=item;data=path.read_bytes();row={'name':path.name,'path':str(path.resolve())}
    try: row['mapper']=validate_rom(data)
    except ValueError as e: return dict(row,status='rejected',reason=str(e))
    rom=out/f'{index:04}.nes';rom.write_bytes(data)
    screenshot=out/f'{index:04}.ppm'
    try:
        proc=subprocess.run([str(exe),str(rom),str(args.frames),str(screenshot),str(args.draw_every)],capture_output=True,timeout=30)
        log=proc.stdout.decode('utf-8',errors='replace')+proc.stderr.decode('utf-8',errors='replace')
        (out/f'{index:04}.log').write_text(log,encoding='utf-8')
        if proc.returncode: return dict(row,status='failed',exit_code=proc.returncode,log=f'{index:04}.log')
        payload=next(line[len('ROM_RESULT '):] for line in log.splitlines() if line.startswith('ROM_RESULT '))
        metrics=json.loads(payload)
        return dict(row,status='ran' if metrics['colors']>1 else 'blank_frame',metrics=metrics,screenshot=str(screenshot))
    except (subprocess.TimeoutExpired,StopIteration,ValueError) as e:
        return dict(row,status='failed',reason=str(e))
with concurrent.futures.ThreadPoolExecutor(max_workers=4) as pool:
    results=list(pool.map(run,enumerate(paths)))
summary={s:sum(r['status']==s for r in results) for s in ('ran','rejected','blank_frame','failed')}
(out/'results.json').write_text(json.dumps({'frames':args.frames,'draw_every':args.draw_every,'summary':summary,'results':results},ensure_ascii=False,indent=2),encoding='utf-8')
print(json.dumps(summary),flush=True)
for r in results:
    if r['status'] in ('failed','blank_frame'): print(r['name'],r['status'],r.get('reason',r.get('exit_code','')))
sys.exit(bool(summary['failed']))
