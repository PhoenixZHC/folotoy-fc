"""Exercise the actual ATT Find Information branch with synthetic peer replies.

Only transport/event side effects are stubbed. Generated files stay in .work.
"""
import os
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]
source = (ROOT / 'components/btstack/src/ble/gatt_client.c').read_text(encoding='utf-8')
start = source.index('        case ATT_FIND_INFORMATION_REPLY: {')
end = source.index('        case ATT_WRITE_RESPONSE:', start)
branch = source[start:end]
query_start = source.index('static bool is_query_done(')
query_end = source.index('\n}', query_start) + 2
out = ROOT / '.work/gatt-ccc-tests'
out.mkdir(parents=True, exist_ok=True)
prefix = r'''
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "ble/gatt_client.h"
#include "btstack_util.h"
#define log_info(...) ((void)0)
static unsigned completions;
static uint8_t completed_status;
void gatt_client_handle_transaction_complete(gatt_client_t *client, uint8_t status) {
    client->state = P_READY;
    completions++;
    completed_status = status;
}
static void report_gatt_all_characteristic_descriptors(gatt_client_t *c, uint8_t *p, uint16_t n, uint8_t s) {
    (void)c; (void)p; (void)n; (void)s;
    assert(!"CCC reply must not enter generic descriptor path");
}
static void trigger_next_characteristic_descriptor_query(gatt_client_t *c, uint16_t h) {
    (void)c; (void)h;
    assert(!"CCC reply must not enter generic descriptor path");
}
'''
suffix = r'''
static gatt_client_t make_client(void) {
    gatt_client_t c;
    memset(&c, 0, sizeof(c));
    c.state = P_W4_FIND_CLIENT_CHARACTERISTIC_CONFIGURATION_QUERY_RESULT;
    c.start_group_handle = 0x17;
    c.end_group_handle = 0x19;
    return c;
}
int main(void) {
    // CCC arrives before the end of the descriptor range: do not overwrite WRITE.
    uint8_t ccc[] = {ATT_FIND_INFORMATION_REPLY, 1, 0x18, 0, 0x02, 0x29};
    gatt_client_t c = make_client();
    receive(&c, ccc, sizeof(ccc));
    assert(c.state == P_W2_WRITE_CLIENT_CHARACTERISTIC_CONFIGURATION);
    assert(c.client_characteristic_configuration_handle == 0x18);
    assert(completions == 0);

    // No CCC yet: advance, then correctly process CCC on the next reply.
    uint8_t value[] = {ATT_FIND_INFORMATION_REPLY, 1, 0x17, 0, 0x4d, 0x2a};
    c = make_client();
    receive(&c, value, sizeof(value));
    assert(c.state == P_W2_SEND_FIND_CLIENT_CHARACTERISTIC_CONFIGURATION_QUERY);
    assert(c.start_group_handle == 0x18);
    c.state = P_W4_FIND_CLIENT_CHARACTERISTIC_CONFIGURATION_QUERY_RESULT;
    receive(&c, ccc, sizeof(ccc));
    assert(c.state == P_W2_WRITE_CLIENT_CHARACTERISTIC_CONFIGURATION);

    // A normal single reply with CCC and Report Reference still schedules WRITE.
    uint8_t both[] = {ATT_FIND_INFORMATION_REPLY, 1, 0x18, 0, 0x02, 0x29, 0x19, 0, 0x08, 0x29};
    c = make_client();
    receive(&c, both, sizeof(both));
    assert(c.state == P_W2_WRITE_CLIENT_CHARACTERISTIC_CONFIGURATION);
    assert(c.client_characteristic_configuration_handle == 0x18);

    // Reaching the end without CCC must report failure, never wait indefinitely.
    uint8_t no_ccc[] = {ATT_FIND_INFORMATION_REPLY, 1, 0x19, 0, 0x08, 0x29};
    c = make_client();
    receive(&c, no_ccc, sizeof(no_ccc));
    assert(c.state == P_READY && completions == 1);
    assert(completed_status == ATT_ERROR_ATTRIBUTE_NOT_FOUND);

    // A vendor UUID at the end is not a standard CCC.
    uint8_t vendor[20] = {ATT_FIND_INFORMATION_REPLY, 2, 0x19, 0};
    c = make_client();
    receive(&c, vendor, sizeof(vendor));
    assert(c.state == P_READY && completions == 2);
    assert(completed_status == ATT_ERROR_ATTRIBUTE_NOT_FOUND);
    puts("5 CCC discovery scenarios passed");
    return 0;
}
'''
generated = out / 'ccc_discovery.c'
generated.write_text(prefix + source[query_start:query_end] +
                     '\nstatic void receive(gatt_client_t *gatt_client, uint8_t *packet, uint16_t size) {\n'
                     'switch (packet[0]) {\n' + branch + 'default: assert(0);\n}}\n' + suffix,
                     encoding='utf-8')
exe = out / ('ccc_discovery.exe' if os.name == 'nt' else 'ccc_discovery')
subprocess.run([os.getenv('CC', 'gcc'), '-std=gnu11', '-Wall', '-Wextra', '-Werror',
                '-ffunction-sections', '-fdata-sections', '-Itests/keyboard_stubs',
                '-Icomponents/btstack/src', str(generated), 'components/btstack/src/btstack_util.c',
                '-Wl,--gc-sections', '-o', str(exe)], cwd=ROOT, check=True, timeout=60)
subprocess.run([str(exe)], cwd=out, check=True, timeout=10)
