// 模拟总量足够但最大连续块受限，并验证分块分配中途失败后的清理。
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
static void *owned[128];
static int calls, fail_at, live;
static void *limited_calloc(size_t n, size_t bytes) {
    calls++;
    if (calls == fail_at || (bytes && n > 8192 / bytes)) return NULL;
    void *p = calloc(n, bytes);
    if (p) { for (int i=0;i<128;i++) if (!owned[i]) { owned[i]=p; live++; break; } }
    return p;
}
static void tracked_free(void *p) {
    if (p) for (int i=0;i<128;i++) if (owned[i]==p) { owned[i]=NULL; live--; break; }
    free(p);
}
#define calloc limited_calloc
#define free tracked_free
#include "../main/fc_port.c"
#undef calloc
#undef free
int main(int argc, char **argv) {
    assert(argc==2);
    FILE *f=fopen(argv[1],"rb"); assert(f);
    fseek(f,0,SEEK_END); long size=ftell(f); rewind(f);
    uint8_t *rom=malloc(size); assert(rom);
    assert(fread(rom,1,size,f)==(size_t)size); fclose(f);
    gb_rom_source_t source={.mapped=rom,.size=(uint32_t)size};
    for (int failure=1;failure<=65;failure++) {
        calls=0; fail_at=failure;
        gb_session_t *s=NULL;
        bool ok=gb_port_create(&source,&s);
        if (calls>=failure) { assert(!ok); assert(!s); }
        else { assert(ok); gb_port_destroy(s); }
        assert(live==0);
        // 每次失败后必须能重新启动；任何单次 calloc 超过 8 KiB 都会失败。
        calls=0; fail_at=0;
        assert(gb_port_create(&source,&s));
        for(int frame=0;frame<120;frame++) assert(gb_port_step_frame(s,true));
        uint8_t *const *rows=gb_port_framebuffer(s);
        for(int y=0;y<240;y++) {
            assert(rows[y]);
            for(int x=8;x<264;x++) assert((rows[y][x]&63)==(rows[y%60][x]&63));
        }
        gb_port_destroy(s); assert(live==0);
    }
    free(rom);
    puts("fragmented frame allocation and failure cleanup PASS");
    return 0;
}
