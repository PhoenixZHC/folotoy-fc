"""生产网络启动/退出函数的故障注入；不冒充手机或 Wi-Fi 实机测试。"""
from pathlib import Path
import os
import subprocess
import tempfile

ROOT=Path(__file__).resolve().parents[1]
source=(ROOT/'main/gb_web.c').read_text(encoding='utf-8')
network=source[source.index('bool gb_web_start(void)'):]
prefix=r'''
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "gb_web.h"
typedef int esp_err_t;
typedef int esp_netif_t;
typedef void *httpd_handle_t;
typedef void *esp_event_handler_instance_t;
#define ESP_OK 0
#define ESP_FAIL -1
#define ESP_ERR_INVALID_STATE 2
#define ESP_ERR_ESP_NETIF_DHCP_ALREADY_STOPPED 3
#define ESP_LOGI(tag, ...) ((void)(tag))
#define ESP_LOGE(tag, ...) ((void)(tag))
#define WIFI_EVENT 1
#define ESP_EVENT_ANY_ID 0
#define WIFI_STORAGE_RAM 0
#define WIFI_MODE_AP 1
#define WIFI_AUTH_OPEN 0
#define WIFI_IF_AP 1
#define HTTP_GET 0
#define HTTP_POST 1
typedef struct { uint32_t addr; } ip4;
typedef struct { ip4 ip,gw,netmask; } esp_netif_ip_info_t;
#define ESP_IP4TOADDR(a,b,c,d) (((uint32_t)(a)<<24)|((b)<<16)|((c)<<8)|(d))
typedef int wifi_init_config_t;
#define WIFI_INIT_CONFIG_DEFAULT() 0
typedef struct { struct { int channel,max_connection,authmode; struct { bool required; } pmf_cfg; char ssid[33]; size_t ssid_len; } ap; } wifi_config_t;
typedef struct { int stack_size,max_uri_handlers,max_open_sockets,recv_wait_timeout,send_wait_timeout; bool lru_purge_enable; } httpd_config_t;
#define HTTPD_DEFAULT_CONFIG() {0}
typedef struct { const char *uri; int method; void (*handler)(void); } httpd_uri_t;
static const char *TAG="test";
static gb_web_status_t s_status;
static esp_netif_t *s_ap;
static httpd_handle_t s_server;
static bool s_wifi_initialized,s_wifi_started;
static esp_event_handler_instance_t s_ap_event_handler;
static int operations,fail_at,netifs,servers,drivers,handlers,ip_set,dhcp;
static bool stop_fail,deinit_fail;
static int step(void) { return ++operations==fail_at?ESP_FAIL:ESP_OK; }
static void ap_event(void) {}
static void page_handler(void) {}
static void list_handler(void) {}
static void upload_handler(void) {}
static void delete_handler(void) {}
static void rename_handler(void) {}
static int esp_netif_init(void) {return step();}
static int esp_event_loop_create_default(void) {return step();}
static esp_netif_t *esp_netif_create_default_wifi_ap(void) { if(step())return NULL; assert(netifs++==0);return &netifs; }
static int esp_netif_dhcps_stop(esp_netif_t *p) {(void)p;dhcp=0;return step();}
static int esp_netif_set_ip_info(esp_netif_t *p,const esp_netif_ip_info_t *ip) {
    assert(p && !dhcp && ip->ip.addr==0xc0a80401 && ip->gw.addr==0xc0a80401 && ip->netmask.addr==0xffffff00);
    ip_set++;return step();
}
static int esp_netif_dhcps_start(esp_netif_t *p) {assert(p && ip_set);dhcp=1;return step();}
static int esp_wifi_init(const wifi_init_config_t *c) {(void)c; if(step())return -1; assert(drivers++==0);return 0;}
static int esp_wifi_set_storage(int mode) {(void)mode;return step();}
static int esp_event_handler_instance_register(int base,int id,void (*cb)(void),void *arg,esp_event_handler_instance_t *out) {
    (void)base;(void)id;(void)cb;(void)arg;if(step())return -1;assert(handlers++==0);*out=&handlers;return 0;
}
static int esp_wifi_set_mode(int mode) {(void)mode;return step();}
static int esp_wifi_set_config(int iface,const wifi_config_t *c) {assert(iface==1 && c->ap.ssid_len==10 && !memcmp(c->ap.ssid,"folotoy-fc",10) && c->ap.max_connection==2);return step();}
static int esp_wifi_start(void) {return step();}
static int httpd_start(httpd_handle_t *out,const httpd_config_t *c) {
    assert(c->lru_purge_enable && c->recv_wait_timeout<=5 && c->max_open_sockets<=7);
    if(step())return -1;assert(servers++==0);*out=&servers;return 0;
}
static int httpd_register_uri_handler(httpd_handle_t h,const httpd_uri_t *r) {assert(h && r->uri);return step();}
static int httpd_stop(httpd_handle_t h) {assert(h);if(stop_fail)return -1;assert(servers--==1);return 0;}
static int esp_wifi_stop(void) {assert(!servers);return 0;}
static int esp_wifi_deinit(void) {if(deinit_fail)return -1;assert(drivers--==1);return 0;}
static int esp_event_handler_instance_unregister(int base,int id,esp_event_handler_instance_t h) {
    (void)base;(void)id;assert(h && !drivers && handlers--==1);return 0;
}
static void esp_netif_destroy_default_wifi(esp_netif_t *p) {assert(p && !drivers && !handlers && netifs--==1);dhcp=0;}
'''
suffix=r'''
static void clean(void) {
    assert(!netifs && !servers && !drivers && !handlers && !dhcp);
    assert(!s_ap && !s_server && !s_ap_event_handler && !s_wifi_initialized && !s_wifi_started && !s_status.running);
}
int main(void) {
    for(int repeat=0;repeat<20;repeat++) {
        operations=0;fail_at=0;
        assert(gb_web_start() && s_status.running);
        int before=operations;
        assert(gb_web_start() && operations==before);
        gb_web_stop();gb_web_stop();clean();
    }
    int total=operations;
    for(int failure=1;failure<=total;failure++) {
        operations=0;fail_at=failure;
        assert(!gb_web_start());clean();
        fail_at=0;assert(gb_web_start());gb_web_stop();clean();
    }
    assert(gb_web_start());stop_fail=true;gb_web_stop();
    assert(s_server && s_ap && drivers && !s_status.running);
    assert(!gb_web_start());stop_fail=false;
    assert(gb_web_start());gb_web_stop();clean();
    assert(gb_web_start());deinit_fail=true;gb_web_stop();
    assert(!s_server && s_wifi_initialized && s_ap);
    assert(!gb_web_start());deinit_fail=false;
    assert(gb_web_start());gb_web_stop();clean();
    puts("FC_WEB_PASS: lifecycle, partial failures, cleanup retry, fixed AP address");
}
'''
work=Path(tempfile.mkdtemp(prefix='fc-web-',dir=ROOT/'build'))
unit=work/'web.c'; unit.write_text(prefix+network+suffix,encoding='utf-8')
exe=work/('web.exe' if os.name=='nt' else 'web')
subprocess.run([os.getenv('CC','gcc'),'-std=c11','-Wall','-Wextra','-Werror',
                '-Wno-misleading-indentation','-Imain',str(unit),'-o',str(exe)],cwd=ROOT,check=True)
subprocess.run([str(exe)],check=True)
