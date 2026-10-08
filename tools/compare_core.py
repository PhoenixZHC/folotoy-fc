"""Compare local pre-optimization core against current core using the 8 selected ROMs."""
import argparse,array,json,math,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('baseline',type=Path);args=p.parse_args()
out=ROOT/'build/perf-compare';out.mkdir(parents=True,exist_ok=True)
executables=[]
for label,core in [('before',args.baseline.resolve()),('after',ROOT/'components/nofrendo')]:
    sources=[core/'nes'/f'{n}.c' for n in ('nes','cpu','ppu','apu','mem','mmc','input')]
    sources += [core/'mappers'/f'map{n}.c' for n in ('000','001','002','003','004','007','023','066')]
    exe=out/f'{label}.exe'
    subprocess.run(['gcc','-std=gnu11','-O2','-ffunction-sections','-fdata-sections','-Imain',f'-I{core}',f'-I{core/"nes"}',
        'tests/rom_compare_runner.c','main/fc_port.c','main/fc_rom.c',str(core/'nofrendo.c'),*map(str,sources),'-Wl,--gc-sections','-lm','-o',str(exe)],cwd=ROOT,check=True)
    executables.append((label,exe))
games=json.loads((ROOT/'build/preloaded/selection-8genres.json').read_text(encoding='utf-8'))['games']
results=[]
for index,game in enumerate(games):
    rom=out/f'{index}.nes';rom.write_bytes((ROOT/'FC中文游戏全集'/game['source']).read_bytes())
    row={'game':game['title']};pcm=[]
    for label,exe in executables:
        audio=out/f'{index}-{label}.pcm'
        run=subprocess.run([str(exe),str(rom),str(audio)],capture_output=True,timeout=45,check=True)
        line=next(x for x in run.stdout.decode(errors='replace').splitlines() if x.startswith('COMPARE_RESULT '))
        row[label]=json.loads(line.removeprefix('COMPARE_RESULT '))
        data=array.array('h');data.frombytes(audio.read_bytes());pcm.append(data)
    row['video_equal']=row['before']['video_hash']==row['after']['video_hash']
    row['sample_count_equal']=len(pcm[0])==len(pcm[1])==960000
    row['audio_equal_samples']=sum(a==b for a,b in zip(*pcm))
    row['audio_rms']=[math.sqrt(sum(x*x for x in data)/len(data)) for data in pcm]
    row['audio_peak']=[max(map(abs,data)) for data in pcm]
    results.append(row);print(json.dumps(row,ensure_ascii=False),flush=True)
(out/'results.json').write_text(json.dumps(results,ensure_ascii=False,indent=2),encoding='utf-8')
raise SystemExit(any(not r['video_equal'] or not r['sample_count_equal'] for r in results))
