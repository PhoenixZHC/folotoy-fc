"""上传请求拒绝、中文原因与重试；真实处理函数/存储，隔离 HTTP 和 Flash。"""
from pathlib import Path
import os
import subprocess
import tempfile
ROOT=Path(__file__).resolve().parents[1]
source=(ROOT/'main/gb_web.c').read_text(encoding='utf-8')
handler=source[source.index('static int receive_chunk('):source.index('static esp_err_t rename_handler(')]
prefix=r'''
#include <assert.h>
#include <string.h>
#include <stdio.h>
#define main storage_suite_main
#include "test_fc_storage.c"
#undef main
#define ESP_FAIL -1
#define HTTPD_SOCK_ERR_TIMEOUT -2
typedef struct { int content_len; header_input_t input; } httpd_req_t;
static char response[256],status[64],origin[64],name[64];
static bool close_connection;
static int httpd_req_recv(httpd_req_t *r,char *dst,size_t n) { return header_receive(&r->input,dst,n); }
static size_t httpd_req_get_hdr_value_len(httpd_req_t *r,const char *key) {(void)r;(void)key;return strlen(origin);}
static int httpd_req_get_hdr_value_str(httpd_req_t *r,const char *key,char *dst,size_t n) {(void)r;(void)key;snprintf(dst,n,"%s",origin);return 0;}
static int httpd_resp_set_status(httpd_req_t *r,const char *s) {(void)r;strcpy(status,s);return 0;}
static int httpd_resp_sendstr(httpd_req_t *r,const char *s) {(void)r;snprintf(response,sizeof(response),"%s",s);return 0;}
static int httpd_resp_set_type(httpd_req_t *r,const char *s) {(void)r;assert(strstr(s,"utf-8"));return 0;}
static int httpd_resp_set_hdr(httpd_req_t *r,const char *key,const char *s) {(void)r;assert(!strcmp(key,"Connection")&&!strcmp(s,"close"));close_connection=true;return 0;}
static int httpd_req_get_url_query_str(httpd_req_t *r,char *q,size_t n) {(void)r;snprintf(q,n,"name=test");return 0;}
static int httpd_query_key_value(const char *q,const char *key,char *v,size_t n) {(void)q;(void)key;snprintf(v,n,"%s",name);return 0;}
'''
suffix=r'''
static httpd_req_t request(void) {
    strcpy(origin,"http://192.168.4.1");strcpy(name,"test");
    response[0]=0;strcpy(status,"200 OK");close_connection=false;
    return (httpd_req_t){24592,{{'N','E','S',0x1a,1,1,0,8},0}};
}
int main(void) {
    reset();
    httpd_req_t req=request();req.input.h[8]=0x10;
    assert(upload_handler(&req)==ESP_FAIL);
    assert(!erase_calls && close_connection && !strncmp(status,"400",3));
    assert(!strcmp(response,"不支持此硬件子型号"));
    req=request();
    assert(upload_handler(&req)==ESP_OK && gb_storage_count()==1);
    int before=erase_calls;
    req=request();assert(upload_handler(&req)==ESP_FAIL && erase_calls==before);
    assert(!strcmp(response,"名称无效或重复"));
    req=request();strcpy(origin,"http://example.com");
    assert(upload_handler(&req)==ESP_FAIL && close_connection && erase_calls==before);
    req=request();req.content_len=3;
    assert(upload_handler(&req)==ESP_FAIL && close_connection && erase_calls==before);
    puts("FC_UPLOAD_HTTP_PASS");
}
'''
work=Path(tempfile.mkdtemp(prefix='fc-http-',dir=ROOT/'build'))
unit=work/'http.c';unit.write_text(prefix+handler+suffix,encoding='utf-8')
exe=work/('http.exe' if os.name=='nt' else 'http')
subprocess.run([os.getenv('CC','gcc'),'-std=c11','-Wall','-Wextra','-Werror',
               '-Itests','-Itests/stubs','-Imain',str(unit),'main/gb_name.c','main/fc_rom.c','-o',str(exe)],cwd=ROOT,check=True)
subprocess.run([str(exe)],check=True)
