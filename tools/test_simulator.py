"""Automate the isolated test firmware in PassportSim, retaining raw receipts."""
import argparse,json,os,subprocess,sys,hashlib,shutil
from pathlib import Path
from make_diagnostic_rom import make_rom
from pack_roms import pack
from package_firmware import package
ROOT=Path(__file__).resolve().parents[1]

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('simulator',type=Path)
    args=p.parse_args();cli=args.simulator.resolve()
    out=ROOT/'build/sim-tests';out.mkdir(parents=True,exist_ok=True)
    rom=out/'diagnostic.nes';rom.write_bytes(make_rom())
    container=out/'diagnostic-roms.bin';container.write_bytes(pack([rom]))
    image=ROOT/'build-sim/release/FoloToy-FC-diagnostic.bin'
    package(ROOT/'build-sim',image,container)
    home=ROOT/'build/sim-state';config=home/'config/corpus.toml';config.parent.mkdir(parents=True,exist_ok=True)
    corpus='fc-auto-'+hashlib.sha256(image.read_bytes()).hexdigest()[:12]
    existing=config.read_text(encoding='utf-8') if config.exists() else ''
    if '['+corpus+']' not in existing:
        with config.open('a',encoding='utf-8') as f:
            f.write('\n['+corpus+']\n'+f'bin = "{image.as_posix()}"\nelf = "{(ROOT/"build-sim/FoloToy-FC.elf").as_posix()}"\n')
    env=dict(os.environ,PASSPORTSIM_HOME=str(home));receipts=[];passed=0
    def call(label,*command):
        proc=subprocess.run([str(cli),'--output','json',*command],capture_output=True,env=env,timeout=60)
        (out/(label+'.json')).write_bytes(proc.stdout)
        data=json.loads(proc.stdout)
        receipts.append({'command':label,'exit_code':proc.returncode})
        # v1.0.1 返回 result=pass 的 run 命令仍可能退出 10；保留该差异，严格检查 JSON。
        if proc.returncode and not(proc.returncode==10 and data.get('result')=='pass'):
            raise RuntimeError(f'{label}: exit={proc.returncode}, {data.get("message",data.get("status"))}')
        if data.get('code') or data.get('receipt',{}).get('hle',{}).get('tripwires_hit'):
            raise RuntimeError(f'{label}: simulator fault')
        return data
    def artifact(data,section=None):
        value=data[section] if section else data
        matches=list((home/'data/artifacts').glob('**/'+instance+'/'+value['path']))
        receipt=next(a for a in data['artifacts'] if a['path']==value['path'])
        matches=[path for path in matches if path.stat().st_size==receipt['bytes']
                 and hashlib.sha256(path.read_bytes()).hexdigest()==receipt['sha256']]
        # CLI 重启后会复用 p1；历史同名文件必须按当前回执哈希校验。
        # 多个相同哈希的副本具有相同内容，不按时间猜测。
        if not matches: raise RuntimeError('Missing simulator artifact matching receipt hash')
        path=matches[0];target=out/path.name;shutil.copy2(path,target);return target.name
    instance=None
    try:
        start=call('start','start',corpus,'--boot','none');instance=start['instance']
        expected_elf=hashlib.sha256((ROOT/'build-sim/FoloToy-FC.elf').read_bytes()).hexdigest()
        assert start['receipt']['binding']['app_elf_sha256']==expected_elf,'Simulator loaded a stale ELF'
        theme=call('theme-ready','run','serial:/FC_SIM_THEME_READY/','--instance',instance,'--timeout','5s','--wall-budget-ms','15000')
        assert theme['status']=='matched'
        theme_screen=call('theme-screen','screenshot','--instance',instance)
        assert theme_screen['w']==240 and theme_screen['h']==320 and theme_screen['panel']['display_on']
        theme_path=artifact(theme_screen);passed+=1
        lifecycle=call('async-lifecycle','run','serial:/FC_SIM_ASYNC_LIFECYCLE_OK/','--instance',instance,'--timeout','10s','--wall-budget-ms','20000')
        assert lifecycle['status']=='matched';passed+=1
        run=call('save-reload','run','serial:/FC_SIM_SAVE_RELOAD_OK/','--instance',instance,'--timeout','15s','--wall-budget-ms','25000')
        assert run['status']=='matched';passed+=1
        screen=call('screen','screenshot','--instance',instance)
        assert screen['w']==240 and screen['h']==320 and screen['panel']['display_on']
        assert not screen['panel']['sleep_in'];screen_path=artifact(screen);passed+=1
        audio=call('audio','audio_capture','--instance',instance,'--duration-ms','500','--wav')
        assert 425<audio['analysis']['fundamental_hz']<455 and audio['analysis']['peak']>100
        wav_path=artifact(audio,'wav');passed+=1
        call('button','input','ok','click','--instance',instance,'--then-run','300ms')
        serial=call('serial','serial','read','--instance',instance,'--cursor','0')
        assert 'FC_SIM_BUTTON=2' in json.dumps(serial);passed+=1
        report={'passed':passed,'failed':0,'scope':'Test entry only; BLE/Wi-Fi and real hardware excluded',
                'firmware_sha256':hashlib.sha256(image.read_bytes()).hexdigest(),
                'screen':screen_path,'audio':wav_path,'commands':receipts,
                'theme_screen':theme_path,
                'timing_lint':screen['receipt'].get('timing_lint',[]),
                'unmodeled_first_touch':screen['receipt'].get('unmodeled_first_touch',[])}
    except Exception as error:
        report={'passed':passed,'failed':1,'error':str(error),'commands':receipts}
    finally:
        if instance:
            try:call('stop','stop',instance)
            except Exception as error: print('Simulator cleanup:',error,file=sys.stderr)
    (out/'results.json').write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf-8')
    print(json.dumps(report,ensure_ascii=False));return bool(report['failed'])
if __name__=='__main__':sys.exit(main())
