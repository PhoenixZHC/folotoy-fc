"""Read local ROM headers; report admission separately from runtime compatibility."""
from pathlib import Path
import collections, hashlib, json, sys
from pack_roms import validate_rom
root=Path(sys.argv[1])
rows=[]
for path in sorted(root.rglob('*')):
    if path.suffix.lower()!='.nes': continue
    data=path.read_bytes()
    mapper=((data[6]>>4)|(data[7]&0xf0)) if len(data)>=16 else None
    try: validate_rom(data); accepted=True; reason=''
    except ValueError as e: accepted=False; reason=str(e)
    rows.append({'path':str(path.resolve()),'name':path.name,'bytes':len(data),'mapper':mapper,
                 'accepted':accepted,'reason':reason,'sha256':hashlib.sha256(data).hexdigest()})
out=Path('build/rom-audit.json'); out.parent.mkdir(exist_ok=True)
out.write_text(json.dumps(rows,ensure_ascii=False,indent=2),encoding='utf-8')
print('Total:',len(rows),'Accepted headers:',sum(r['accepted'] for r in rows))
print('Mappers:',collections.Counter(r['mapper'] for r in rows))
print('Rejected:',collections.Counter(r['reason'] for r in rows if not r['accepted']))
for r in rows:
    if any(k in r['name'] for k in ('超级马里奥','魂斗罗','坦克','冒险岛','双截龙','松鼠')):
        print(r['name'],'mapper',r['mapper'],r['accepted'],r['reason'])
