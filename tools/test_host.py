"""Native tests; keep diagnostics separate from shipping firmware."""
import json, os, subprocess, sys, time
from pathlib import Path
from make_diagnostic_rom import make_rom
from make_nes2_diagnostic import make_rom as make_nes2_rom
ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'build'/'host-tests'
OUT.mkdir(parents=True,exist_ok=True)
(OUT/'diagnostic.nes').write_bytes(make_rom())
(OUT/'diagnostic-nes2.nes').write_bytes(make_nes2_rom())
(OUT/'diagnostic-nes2-exponent.nes').write_bytes(make_nes2_rom(True))
CORE=['nes/'+n+'.c' for n in ('nes','cpu','ppu','apu','mem','mmc','input')]
CORE+=['mappers/map'+n+'.c' for n in ('000','001','002','003','004','007','023','066')]
CORE+=['nofrendo.c']
core_sources=['components/nofrendo/'+s for s in CORE]
cases=[
 ('fc_rom',['tests/test_fc_rom.c','main/fc_rom.c'],[]),
 ('fc_nes2_diagnostic',['tests/test_fc_nes2_diagnostic.c','main/fc_port.c','main/fc_rom.c',*core_sources],[str(OUT/'diagnostic-nes2.nes')]),
 ('gamepad_test',['tests/test_gamepad_test.c','main/gamepad_test.c'],[]),
 ('iine_protocol',['tests/test_iine_protocol.c','components/bluepad32/parser/uni_iine_protocol.c'],[str(ROOT/'tests/fixtures/l1241_fw2702_input.txt')]),
 ('bt_le_iine',['tests/test_bt_le_iine.c','components/bluepad32/parser/uni_iine_protocol.c','components/btstack/src/btstack_util.c'],[]),
 ('fc_frame_pacing',['tests/test_fc_frame_pacing.c','main/gb_frame_pacing.c'],[]),
 ('fc_launch',['tests/test_fc_launch.c','main/fc_launch.c'],[]),
 ('fc_saves',['tests/test_fc_saves.c'],[]),
 ('fc_storage',['tests/test_fc_storage.c','main/gb_name.c','main/fc_rom.c'],[]),
 ('fc_core',['tests/test_fc_core.c','main/gb_input.c','main/fc_port.c','main/fc_rom.c',*core_sources],[str(OUT/'diagnostic.nes')]),
 ('fc_frame_memory',['tests/test_fc_frame_memory.c','main/fc_rom.c',*core_sources],[str(OUT/'diagnostic.nes')]),
 ('gb_input',['tests/test_gb_input.c','main/gb_input.c'],[]),
 ('gb_input_buffer',['tests/test_gb_input_buffer.c','main/gb_input.c'],[]),
 ('gb_name',['tests/test_gb_name.c','main/gb_name.c'],[]),
 ('gamepad_discovery',['tests/test_gamepad_discovery.c','main/gamepad_discovery.c'],[]),
 ('bt_le_advertisement',['tests/test_bt_le_advertisement.c','components/bluepad32/bt/uni_bt_le_advertisement.c','main/gamepad_discovery.c'],[]),
 ('bt_le_hid',['tests/test_bt_le_hid.c','components/bluepad32/bt/uni_bt_le_hid.c'],[]),
 ('bt_le_recovery',['tests/test_bt_le_recovery.c','components/bluepad32/bt/uni_bt_le_hid.c','components/btstack/src/btstack_util.c'],[]),
 ('gb_gamepad_hid',['tests/test_gb_gamepad_hid.c','components/bluepad32/parser/uni_hid_parser.c','components/bluepad32/parser/uni_hid_parser_standard.c','components/bluepad32/controller/uni_gamepad.c','components/btstack/src/btstack_hid_parser.c','components/btstack/src/btstack_util.c'],[]),
 ('gb_keyboard_hid',['tests/test_gb_keyboard_hid.c','main/gb_input.c','components/bluepad32/parser/uni_hid_parser.c','components/bluepad32/parser/uni_hid_parser_keyboard.c','components/bluepad32/controller/uni_gamepad.c','components/btstack/src/btstack_hid_parser.c','components/btstack/src/btstack_util.c'],[]),
 ('xiaomi_remote_hid',['tests/test_xiaomi_remote_hid.c','main/gb_input.c','components/bluepad32/parser/uni_hid_parser.c','components/bluepad32/parser/uni_hid_parser_keyboard.c','components/bluepad32/controller/uni_gamepad.c','components/btstack/src/btstack_hid_parser.c','components/btstack/src/btstack_util.c'],[]),
]
results=[]
for name,sources,args in cases:
    exe=OUT/(name+('.exe' if os.name=='nt' else ''))
    extra=[]
    if name in ('bt_le_advertisement','bt_le_hid','bt_le_recovery','gb_gamepad_hid','gb_keyboard_hid','xiaomi_remote_hid','iine_protocol','bt_le_iine'):
        extra+=['-Werror','-Icomponents/bluepad32/include']
    if name in ('bt_le_recovery','gb_gamepad_hid','gb_keyboard_hid','xiaomi_remote_hid','bt_le_iine'):
        extra+=['-Itests/keyboard_stubs','-Icomponents/btstack/src']
    if name in ('bt_le_recovery','bt_le_iine'):
        extra+=['-DCONFIG_BLUEPAD32_MAX_DEVICES=1']
    command=[os.getenv('CC','gcc'),'-std=gnu11','-O2','-Wall','-Wextra','-ffunction-sections','-fdata-sections',*extra,
             '-Itests/stubs','-Imain','-Icomponents/nofrendo','-Icomponents/nofrendo/nes',
             *sources,'-Wl,--gc-sections','-lm','-o',str(exe)]
    with (OUT/(name+'.log')).open('w',encoding='utf-8') as log:
        try:
            subprocess.run(command,cwd=ROOT,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=120)
            runtime=OUT/(name+'-'+str(time.time_ns()))
            runtime.mkdir()
            subprocess.run([str(exe),*args],cwd=runtime,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=120)
            if name=='fc_nes2_diagnostic':
                subprocess.run([str(exe),str(OUT/'diagnostic-nes2-exponent.nes')],cwd=runtime,
                               stdout=log,stderr=subprocess.STDOUT,check=True,timeout=120)
            passed=True
        except (subprocess.SubprocessError,OSError) as e:
            log.write(str(e)); passed=False
    results.append({'name':name,'passed':passed})
    print(name, 'PASS' if passed else 'FAIL',flush=True)
for name in ('fc_web','fc_format_parity','fc_upload_http','home_menu','menu_navigation'):
    with (OUT/(name+'.log')).open('w',encoding='utf-8') as log:
        try:
            subprocess.run([sys.executable,str(ROOT/'tests'/('test_'+name+'.py'))],
                           cwd=ROOT,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=120)
            passed=True
        except (subprocess.SubprocessError,OSError) as e:
            log.write(str(e)); passed=False
    results.append({'name':name,'passed':passed})
    print(name,'PASS' if passed else 'FAIL',flush=True)
summary={'passed':sum(r['passed'] for r in results),'failed':sum(not r['passed'] for r in results),'cases':results}
(OUT/'results.json').write_text(json.dumps(summary,indent=2),encoding='utf-8')
print(json.dumps(summary))
sys.exit(bool(summary['failed']))
