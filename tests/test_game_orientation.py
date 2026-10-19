"""执行生产设置、暂停与恢复函数，验证方向隔离、持久化及保存失败。"""
import os, subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
source=(ROOT/'main/game_main.c').read_text(encoding='utf-8')
def extract(signature):
    start=source.index(signature)
    return source[start:source.index('\n}',start)+2]
code=r'''
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "gb_input.h"
#include "fc_video.h"
typedef int esp_err_t;
typedef int nvs_handle_t;
typedef int BaseType_t;
typedef int TickType_t;
typedef int gb_session_t;
typedef struct {int button,event;} button_message_t;
enum {BSP_BTN_UP,BSP_BTN_DOWN,BSP_BTN_OK,BSP_BTN_CLICK,BSP_BTN_LONG};
#define ESP_OK 0
#define NVS_READONLY 0
#define NVS_READWRITE 1
#define pdTRUE 1
#define pdMS_TO_TICKS(x) (x)
#define ESP_LOGE(...) ((void)0)
#define ESP_LOGI(...) ((void)0)
#define GB_LCD_LIGHT 0xffbc
static bool s_sound_enabled=true;
static uint8_t s_default_orientation,s_game_orientation;
static const char *const s_orientation_defaults[]={"默认游戏 竖屏","默认游戏 横屏","默认 横屏翻转"};
static const char *const s_orientation_actions[]={"切换为横屏","切换横屏翻转","切换为竖屏"};
static uint8_t s_volume=60,s_brightness=80;
static int s_buttons,events[32],pos,total,stored=-1,pending=-1,writes,commits;
static bool fail_commit,saw_error;
static uint8_t display_landscape;
static int hints,clears;
static int nvs_open(const char *n,int mode,int *h){(void)n;(void)mode;*h=1;return 0;}
static int nvs_get_u8(int h,const char *key,uint8_t *v){(void)h;if(strcmp(key,"landscape")||stored<0)return 1;*v=stored;return 0;}
static int nvs_set_u8(int h,const char *key,uint8_t v){(void)h;writes++;if(!strcmp(key,"landscape"))pending=v;return 0;}
static int nvs_commit(int h){(void)h;commits++;if(fail_commit)return 1;stored=pending;return 0;}
static void nvs_close(int h){(void)h;}
static pad_sample_t gamepad_snapshot(void){return (pad_sample_t){.connected=true};}
static BaseType_t menu_button_receive(gb_select_pause_t *h,button_message_t *m,TickType_t t){
 (void)h;(void)t;assert(pos<total);*m=(button_message_t){events[pos++],BSP_BTN_CLICK};return 1;
}
static void bsp_display_backlight(int v){(void)v;}
static int64_t esp_timer_get_time(void){return pos*50000;}
static struct {const char *version;} desc={"1.6"};
static void *unused_pointer;
#define esp_app_get_description() (&desc)
static int gb_display_footer(const char *t,uint32_t p){(void)t;(void)p;return 0;}
static const char *esp_err_to_name(int e){(void)e;return "error";}
static void show_ui(const char *title,const char *hint,const char *const *items,size_t n,size_t sel,const char *footer){
 (void)footer;assert(sel<n && n<=6);
 if(!strcmp(title,"设置")){assert(n==5);assert(!strcmp(items[3],s_orientation_defaults[s_default_orientation]));}
 else {assert(n==6);assert(!strcmp(items[5],s_orientation_actions[s_game_orientation]));}
 if(strstr(hint,"保存失败"))saw_error=true;
}
static void gamepad_set_gameplay(bool b){(void)b;}
static bool gb_port_has_battery(gb_session_t *s){(void)s;return false;}
static bool gb_port_needs_save(gb_session_t *s){(void)s;return false;}
static bool gb_saves_write(gb_session_t *s,const uint8_t *hash){(void)s;(void)hash;return true;}
static bool screen_off_loop(void){return true;}
static int fc_display_set_orientation(uint8_t b){display_landscape=b;return 0;}
static int gb_display_clear(int color){assert(color==(s_game_orientation?0:GB_LCD_LIGHT));clears++;return 0;}
static int gb_display_game_hint(const char *t){(void)t;hints++;return 0;}
'''
for signature in ('static void settings_load(', 'static bool settings_save(',
                  'static gb_menu_action_t board_menu_action(', 'static void settings_loop(',
                  'static bool pause_game(', 'static bool prepare_game_screen('):
    code+='\n'+extract(signature)+'\n'
code+=r'''
static void feed(const int *a,int n){memcpy(events,a,n*sizeof(int));total=n;pos=0;}
int main(void){
 (void)s_buttons;(void)unused_pointer;(void)esp_err_to_name;
 settings_load();assert(!s_default_orientation);
 stored=1;settings_load();assert(s_default_orientation);
 stored=2;settings_load();assert(s_default_orientation==2);
 stored=0;settings_load();assert(!s_default_orientation);
 stored=7;settings_load();assert(!s_default_orientation);
 // 设置第五项仍可返回；修改默认方向不改变当前游戏。
 int setting[]={BSP_BTN_DOWN,BSP_BTN_DOWN,BSP_BTN_DOWN,BSP_BTN_OK,BSP_BTN_DOWN,BSP_BTN_OK};
 feed(setting,6);settings_loop();assert(s_default_orientation && stored==1 && !s_game_orientation);
 s_default_orientation=false;settings_load();assert(s_default_orientation);
 fail_commit=true;feed(setting,6);settings_loop();assert(s_default_orientation && stored==1 && saw_error);
 fail_commit=false;
 // 暂停上翻到第六项，切换后回到继续游戏；不写默认配置。
 int before=writes;
 int pause[]={BSP_BTN_UP,BSP_BTN_OK,BSP_BTN_DOWN,BSP_BTN_OK};
 feed(pause,4);assert(pause_game(NULL,NULL,true));
 assert(s_game_orientation && s_default_orientation && writes==before);
 assert(prepare_game_screen() && display_landscape && clears==1 && hints==0);
 feed(pause,4);assert(pause_game(NULL,NULL,true));
 assert(s_game_orientation==2 && s_default_orientation==1 && writes==before);
 assert(prepare_game_screen() && display_landscape==2 && clears==2 && hints==0);
 feed(pause,4);assert(pause_game(NULL,NULL,true));
 assert(!s_game_orientation && s_default_orientation && writes==before);
 assert(prepare_game_screen() && !display_landscape && clears==3 && hints==1);
 feed(setting,6);settings_loop();assert(s_default_orientation==2 && stored==2);
 s_default_orientation=0;settings_load();assert(s_default_orientation==2);
 feed(setting,6);settings_loop();assert(s_default_orientation==0 && stored==0);
 puts("orientation settings, pause and resume PASS");
}
'''
out=ROOT/'.work/orientation-tests'
out.mkdir(parents=True,exist_ok=True)
src=out/'orientation.c';exe=out/'orientation.exe'
src.write_text(code,encoding='utf-8')
subprocess.run([os.getenv('CC','gcc'),'-std=gnu11','-O2','-Wall','-Wextra','-Werror','-Imain',str(src),'main/gb_input.c','-o',str(exe)],cwd=ROOT,check=True)
subprocess.run([str(exe)],cwd=out,check=True)
