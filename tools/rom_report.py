"""Render captured core frames as PNG and a local, evidence-bounded report."""
import html,json,struct,zlib
from pathlib import Path
def png(rgb,width,height):
    def chunk(tag,data):
        return struct.pack('>I',len(data))+tag+data+struct.pack('>I',zlib.crc32(tag+data)&0xffffffff)
    rows=b''.join(b'\0'+rgb[y*width*3:(y+1)*width*3] for y in range(height))
    return b'\x89PNG\r\n\x1a\n'+chunk(b'IHDR',struct.pack('>IIBBBBB',width,height,8,2,0,0,0))+chunk(b'IDAT',zlib.compress(rows))+chunk(b'IEND',b'')
def main():
    folder=Path('build/rom-tests'); report=json.loads((folder/'results.json').read_text(encoding='utf-8'))
    cards=[]
    for row in report['results']:
        image=''
        if 'screenshot' in row:
            path=Path(row['screenshot']); _,dimensions,_,rgb=path.read_bytes().split(b'\n',3)
            width,height=map(int,dimensions.split()); path.with_suffix('.png').write_bytes(png(rgb,width,height))
            image=f'<img src="{path.stem}.png" width="256" height="240">'
        cards.append('<article><h3>'+html.escape(row['name'])+'</h3>'+image+'<pre>'+html.escape(json.dumps({k:v for k,v in row.items() if k not in ('name','path','screenshot')},ensure_ascii=False,indent=2))+'</pre></article>')
    (folder/'report.html').write_text('<!doctype html><meta charset="utf-8"><title>FC ROM smoke report</title><style>body{font:16px sans-serif;background:#eee}main{display:grid;grid-template-columns:repeat(auto-fill,minmax(290px,1fr));gap:12px}article{background:white;padding:12px;overflow:auto}img{image-rendering:pixelated}pre{white-space:pre-wrap}</style><h1>FC 核心主机烟测</h1><p>固定输入 '+str(report['frames'])+' 帧。运行不等于完整兼容；末帧空白须单独检查。无线和真实硬件性能未覆盖。</p><p>'+html.escape(str(report['summary']))+'</p><main>'+''.join(cards)+'</main>',encoding='utf-8')
if __name__=='__main__':main()
