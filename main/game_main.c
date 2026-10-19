#include <stdio.h>
#include <stdlib.h>
#include "bsp_button.h"
#include "bsp_battery.h"
#include "bsp_display.h"
#include "bsp_audio.h"
#include "esp_log.h"
#include "esp_app_desc.h"
#include "esp_heap_caps.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "gamepad.h"
#include "gb_display.h"
#include "fc_launch.h"
#include "fc_rom.h"

#include "gb_screen_sleep.h"
#include "gb_audio.h"
#include "gb_audio_pacing.h"
#include "gb_frame_pacing.h"
#include "gb_input.h"
#include "gb_port.h"
#include "gb_saves.h"
#include "gb_storage.h"
#include "gb_web.h"
#include "nvs_flash.h"
#include "nvs.h"

static const char *TAG = "folotoy_fc";
static QueueHandle_t s_buttons;
static bool s_saves_ready;
static uint8_t s_volume = 60;
static uint8_t s_brightness = 80;
static bool s_sound_enabled = true;
static uint8_t s_default_orientation;
static uint8_t s_game_orientation;
static const char *const s_orientation_defaults[] = {"默认游戏 竖屏", "默认游戏 横屏", "默认 横屏翻转"};
static const char *const s_orientation_actions[] = {"切换为横屏", "切换横屏翻转", "切换为竖屏"};
typedef struct { bsp_btn_t button; bsp_btn_ev_t event; } button_message_t;

static void button_callback(bsp_btn_t button, bsp_btn_ev_t event, void *user) {
    (void)user;
    button_message_t msg = {button, event};
    if (s_buttons) xQueueSend(s_buttons, &msg, 0);
}

static int battery_percent(void) { return bsp_battery_soc(); }

static void show_ui(const char *title, const char *hint, const char *const *items,
                    size_t count, size_t selected, const char *footer) {
    gb_ui_model_t model = {.title = title, .hint = hint, .count = count,
                           .selected = selected, .footer = footer,
                           .battery_percent = battery_percent()};
    for (size_t i = 0; i < count && i < GB_UI_MAX_ITEMS; i++) model.items[i] = items[i];
    esp_err_t e = gb_display_ui(&model);
    if (e != ESP_OK) ESP_LOGE(TAG, "UI: %s", esp_err_to_name(e));
}

static void startup_error(const char *reason) {
    const char *items[] = {reason};
    gb_ui_model_t model = {.title = "启动失败", .hint = "请重启设备",
                           .count = 1, .selected = 0, .footer = "",
                           .battery_percent = -1};
    model.items[0] = items[0];
    gb_display_ui(&model);
}

static void settings_load(void) {
    nvs_handle_t handle;
    if (nvs_open("fc_settings", NVS_READONLY, &handle) != ESP_OK) return;
    uint8_t value;
    if (nvs_get_u8(handle, "volume", &value) == ESP_OK && value <= 100) s_volume = value;
    if (nvs_get_u8(handle, "bright", &value) == ESP_OK && value <= 100) s_brightness = value;
    if (nvs_get_u8(handle, "sound", &value) == ESP_OK && value <= 1) s_sound_enabled = value != 0;
    // 保留已有键及0/1含义，2为横屏翻转，升级不会丢失原方向设置。
    if (nvs_get_u8(handle, "landscape", &value) == ESP_OK && value < FC_VIDEO_COUNT) s_default_orientation = value;
    nvs_close(handle);
}

static bool settings_save(void) {
    nvs_handle_t handle;
    esp_err_t e = nvs_open("fc_settings", NVS_READWRITE, &handle);
    if (e != ESP_OK) return false;
    if (e == ESP_OK) e = nvs_set_u8(handle, "volume", s_volume);
    if (e == ESP_OK) e = nvs_set_u8(handle, "bright", s_brightness);
    if (e == ESP_OK) e = nvs_set_u8(handle, "sound", s_sound_enabled ? 1 : 0);
    if (e == ESP_OK) e = nvs_set_u8(handle, "landscape", s_default_orientation);
    if (e == ESP_OK) e = nvs_commit(handle);
    nvs_close(handle);
    if (e != ESP_OK) ESP_LOGE(TAG, "settings save: %s", esp_err_to_name(e));
    return e == ESP_OK;
}

static gb_menu_action_t board_menu_action(button_message_t msg) {
    if (msg.event != BSP_BTN_CLICK) return GB_MENU_NONE;
    if (msg.button == BSP_BTN_UP) return GB_MENU_UP;
    if (msg.button == BSP_BTN_DOWN) return GB_MENU_DOWN;
    if (msg.button == BSP_BTN_OK) return GB_MENU_CONFIRM;
    return GB_MENU_NONE;
}

// Route a one-second Select hold through the existing board long-OK action.
static BaseType_t menu_button_receive(gb_select_pause_t *hold, button_message_t *msg,
                                     TickType_t timeout) {
    BaseType_t received = xQueueReceive(s_buttons, msg, timeout);
    if (gb_select_pause_step(hold, gamepad_snapshot(), esp_timer_get_time())) {
        *msg = (button_message_t){.button=BSP_BTN_OK, .event=BSP_BTN_LONG};
        return pdTRUE;
    }
    return received;
}

static void gamepad_test_loop(void) {
    gb_select_pause_t select_back;
    gb_select_pause_init(&select_back, gamepad_snapshot());
    xQueueReset(s_buttons);
    int battery = battery_percent();
    int64_t battery_refreshed = esp_timer_get_time();
    while (true) {
        button_message_t msg;
        while (menu_button_receive(&select_back, &msg, 0) == pdTRUE) {
            if (msg.button == BSP_BTN_OK && msg.event == BSP_BTN_LONG) {
                xQueueReset(s_buttons);
                return;
            }
        }
        gamepad_test_state_t state = gamepad_test_snapshot();
        int64_t now = esp_timer_get_time();
        if (now - battery_refreshed >= 5000000) {
            battery = battery_percent();
            battery_refreshed = now;
        }
        esp_err_t e = gb_display_gamepad_test(&state, (uint32_t)(now/1000), battery);
        if (e != ESP_OK) {
            ESP_LOGE(TAG, "gamepad test display: %s", esp_err_to_name(e));
            return;
        }
        // B/Start remain display-only; a one-second Select hold returns.
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

static void settings_loop(void) {
    gb_select_pause_t select_back;
    gb_select_pause_init(&select_back, gamepad_snapshot());
    const char *version = esp_app_get_description()->version;
    size_t selected = 0;
    bool editing = false, dirty = true, save_failed = false;
    gb_menu_input_t pad_input;
    gb_menu_input_init(&pad_input, gamepad_snapshot());
    int64_t refreshed = 0;
    int64_t footer_started = esp_timer_get_time(), footer_refreshed = 0;
    while (true) {
        button_message_t msg;
        bool board_event = menu_button_receive(&select_back, &msg, pdMS_TO_TICKS(50)) == pdTRUE;
        gb_menu_action_t pad_action = gb_menu_input_step(&pad_input, gamepad_snapshot());
        gb_menu_action_t action = board_event ? board_menu_action(msg) : GB_MENU_NONE;
        if (board_event && msg.event == BSP_BTN_LONG && msg.button == BSP_BTN_OK)
            action = GB_MENU_BACK;
        if (action == GB_MENU_NONE) action = pad_action;
        if (action == GB_MENU_BACK) {
            if (editing) { editing = false; settings_save(); }
            else return;
            dirty = true;
        } else if (action != GB_MENU_NONE) {
            if (editing) {
                if (action == GB_MENU_UP || action == GB_MENU_DOWN) {
                    int change = action == GB_MENU_UP ? 10 : -10;
                    if (selected == 0) {
                        int value = (int)s_volume + change;
                        s_volume = value < 0 ? 0 : value > 100 ? 100 : value;
                    } else if (selected == 1) {
                        int value = (int)s_brightness + change;
                        s_brightness = value < 10 ? 10 : value > 100 ? 100 : value;
                        bsp_display_backlight(s_brightness);
                    }
                } else if (action == GB_MENU_CONFIRM) {
                    editing = false;
                    settings_save();
                }
            } else if (action == GB_MENU_UP) selected = (selected + 4) % 5;
            else if (action == GB_MENU_DOWN) selected = (selected + 1) % 5;
            else if (action == GB_MENU_CONFIRM) {
                if (selected == 4) return;
                if (selected == 2) { s_sound_enabled = !s_sound_enabled; settings_save(); }
                else if (selected == 3) {
                    uint8_t previous = s_default_orientation;
                    s_default_orientation = (s_default_orientation + 1) % FC_VIDEO_COUNT;
                    save_failed = !settings_save();
                    if (save_failed) {
                        s_default_orientation = previous;
                        ESP_LOGE(TAG, "default orientation save failed");
                    }
                    else ESP_LOGI(TAG, "default game orientation: %u", s_default_orientation);
                }
                else editing = true;
            }
            dirty = true;
        }
        int64_t now = esp_timer_get_time();
        const char *footer = editing ? "上/下调整 OK保存 长按OK/Select结束"
                                     : "上/下选择 OK确认 长按OK/Select返回";
        if (dirty || now - refreshed > 15000000) {
            char vol[32], bright[32], sound[32], version_hint[64];
            snprintf(vol, sizeof(vol), "音量 %u%%", s_volume);
            snprintf(bright, sizeof(bright), "亮度 %u%%", s_brightness);
            snprintf(sound, sizeof(sound), "声音 %s", s_sound_enabled ? "开" : "关");
            snprintf(version_hint, sizeof(version_hint), "版本 %s%s", version,
                     editing ? " 调整中" : "");
            if (save_failed) snprintf(version_hint, sizeof(version_hint), "方向保存失败 请重试");
            const char *items[] = {vol, bright, sound,
                s_orientation_defaults[s_default_orientation], "返回"};
            show_ui("设置", version_hint,
                    items, 5, selected, footer);
            refreshed = now;
            if (dirty) footer_started = now;
            dirty = false;
        }
        if (now - footer_refreshed >= 100000) {
            esp_err_t e = gb_display_footer(footer, (uint32_t)((now - footer_started) / 40000));
            if (e != ESP_OK) ESP_LOGE(TAG, "settings footer: %s", esp_err_to_name(e));
            footer_refreshed = now;
        }
    }
}

static size_t home_loop(void) {
    const char *items[] = {"连接手柄", "游戏管理", "手柄测试", "设置"};
    size_t selected = 0;
    bool dirty = true;
    int64_t refreshed = 0;
    gb_menu_input_t pad_input;
    gb_menu_input_init(&pad_input, gamepad_snapshot());
    while (true) {
        button_message_t msg;
        bool board_event = xQueueReceive(s_buttons, &msg, pdMS_TO_TICKS(50)) == pdTRUE;
        pad_sample_t sample = gamepad_snapshot();
        if (sample.connected != pad_input.connected) dirty = true;
        gb_menu_action_t pad_action = gb_menu_input_step(&pad_input, sample);
        gb_menu_action_t action = board_event ? board_menu_action(msg) : GB_MENU_NONE;
        if (action == GB_MENU_NONE) action = pad_action;
        if (action == GB_MENU_UP || action == GB_MENU_DOWN) {
            selected = (selected + (action == GB_MENU_UP ? 3 : 1)) % 4;
            dirty = true;
        } else if (action == GB_MENU_CONFIRM) return selected;
        int64_t now = esp_timer_get_time();
        if (dirty || now - refreshed > 15000000) {
            items[0] = sample.connected ? "游戏列表" : "连接手柄";
            show_ui("FC模拟器", sample.connected ? "输入设备已连接" : "请选择操作",
                    items, 4, selected, "上/下选择 A/OK确认");
            refreshed = now;
            dirty = false;
        }
    }
}

static void log_management_heap(const char *stage) {
    ESP_LOGI(TAG, "WIFI_DIAG %s free=%u largest=%u minimum=%u", stage,
             (unsigned)heap_caps_get_free_size(MALLOC_CAP_8BIT),
             (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_8BIT),
             (unsigned)heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT));
}

static void management_loop(void) {
    gb_select_pause_t select_back;
    gb_select_pause_init(&select_back, gamepad_snapshot());
    log_management_heap("before management");
    gamepad_set_scanning(false);
    vTaskDelay(pdMS_TO_TICKS(100));
    if (s_saves_ready && gb_storage_catalog_trusted() &&
        !gb_saves_prune_orphans()) ESP_LOGE(TAG, "orphan save cleanup failed");
    if (!gb_web_start()) {
        gamepad_set_scanning(true);
        const char *items[] = {"无线网络开启失败", "返回"};
        show_ui("游戏管理", "内存或网络不可用", items, 2, 1,
                "OK 返回");
        button_message_t msg;
        while (true) {
            if (menu_button_receive(&select_back, &msg, pdMS_TO_TICKS(50)) == pdTRUE &&
                msg.button == BSP_BTN_OK) return;
        }
    }
    log_management_heap("web started");
    gb_web_status_t status = gb_web_status();
    // 清除打开此页时残留的 OK 事件；仅长按才退出。
    xQueueReset(s_buttons);
    int64_t refreshed = 0;
    while (true) {
        button_message_t msg;
        if (menu_button_receive(&select_back, &msg, pdMS_TO_TICKS(50)) == pdTRUE &&
            msg.button == BSP_BTN_OK && msg.event == BSP_BTN_LONG) {
            ESP_LOGI(TAG, "Leaving game management: OK long press");
            break;
        }
        int64_t now = esp_timer_get_time();
        if (!refreshed || now - refreshed > 2000000) {
            char ssid[38], games[38], space[38];
            snprintf(ssid, sizeof(ssid), "WiFi %s", status.ssid);
            snprintf(games, sizeof(games), "游戏数量 %u", (unsigned)gb_storage_count());
            snprintf(space, sizeof(space), "剩余空间 %uKB",
                     (unsigned)(gb_storage_available() / 1024));
            const char *items[] = {ssid, "192.168.4.1", games, space};
            gb_ui_model_t model = {.title = "游戏管理", .hint = "无互联网也保持连接",
                                   .count = 4, .selected = 4, .footer = "长按OK/Select 返回",
                                   .battery_percent = battery_percent(), .compact = true};
            for (size_t i = 0; i < 4; i++) model.items[i] = items[i];
            esp_err_t draw_error = gb_display_ui(&model);
            if (draw_error != ESP_OK)
                ESP_LOGE(TAG, "management UI: %s", esp_err_to_name(draw_error));
            refreshed = now;
        }
    }
    log_management_heap("before web stop");
    gb_web_stop();
    log_management_heap("after web stop");
    gamepad_set_scanning(true);
    log_management_heap("after scanning resume");
}

static void show_error(const char *reason) {
    ESP_LOGE(TAG, "%s", reason);
    const char *items[] = {reason, "返回"};
    show_ui("发生错误", "请查看提示", items, 2, 1, "OK 返回");
    button_message_t msg;
    do { xQueueReceive(s_buttons, &msg, portMAX_DELAY); }
    while (msg.button != BSP_BTN_OK || msg.event != BSP_BTN_CLICK);
}

static bool screen_off_loop(void) {
    if (!gb_audio_stop()) return false;
    esp_err_t e = bsp_audio_sleep();
    if (e != ESP_OK) { bsp_audio_wake(); return false; }
    e = gb_display_screen_on(false);
    if (e != ESP_OK) { bsp_audio_wake(); return false; }
    bsp_display_backlight(0);
    ESP_LOGI(TAG, "screen off: emulator paused, audio suspended");
    gb_screen_sleep_t state = {0};
    bool lit = false;
    while (state.phase != 3) {
        int mv = bsp_button_read_mv();
        pad_sample_t pad = gamepad_snapshot();
        bool pressed = (mv >= 0 && mv < 1900) || (pad.connected && pad.any_button);
        gb_screen_sleep_step(&state, pressed, (uint32_t)(esp_timer_get_time() / 1000));
        button_message_t ignored;
        while (xQueueReceive(s_buttons, &ignored, 0) == pdTRUE) { }
        if (state.phase >= 2 && !lit) {
            e = gb_display_screen_on(true);
            bsp_display_backlight(s_brightness);
            lit = true;
            if (e != ESP_OK) break;
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
    esp_err_t audio = bsp_audio_wake();
    ESP_LOGI(TAG, "screen wake: display=%s audio=%s", esp_err_to_name(e), esp_err_to_name(audio));
    return e == ESP_OK && audio == ESP_OK;
}

static bool pause_game(gb_session_t *session, const uint8_t hash[32], bool connected) {
    gb_select_pause_t select_back;
    gb_select_pause_init(&select_back, gamepad_snapshot());
    gamepad_set_gameplay(false);
    bool battery = gb_port_has_battery(session);
    const char *items[] = {"继续游戏", battery ? "同步电池存档" : "无电池存档",
                           "设置", "息屏", "退出游戏",
                           s_orientation_actions[s_game_orientation]};
    const char *hint = battery ? "游戏内存档，重进后读取" : "此游戏无电池存档";
    if (!gb_saves_write(session, hash)) hint = "同步失败，进度仅在内存";
    size_t selected = 0;
    show_ui(connected ? "游戏暂停" : "输入设备断开", hint, items, 6, selected,
            "上/下选择 OK确认");
    button_message_t msg;
    gb_menu_input_t pad_input;
    gb_menu_input_init(&pad_input, gamepad_snapshot());
    while (true) {
        bool board_event = menu_button_receive(&select_back, &msg, pdMS_TO_TICKS(50)) == pdTRUE;
        pad_sample_t pad = gamepad_snapshot();
        gb_menu_action_t pad_action = gb_menu_input_step(&pad_input, pad);
        gb_menu_action_t action = board_event ? board_menu_action(msg) : GB_MENU_NONE;
        bool redraw = board_event;
        if (board_event && msg.button == BSP_BTN_OK && msg.event == BSP_BTN_LONG) {
            if (gb_saves_write(session, hash)) return false;
            hint = "同步失败，进度仅在内存";
        } else {
            if (action == GB_MENU_NONE) action = pad_action;
            if (action != GB_MENU_NONE) redraw = true;
            if (action == GB_MENU_UP) selected = (selected + 5) % 6;
            else if (action == GB_MENU_DOWN) selected = (selected + 1) % 6;
            else if (action == GB_MENU_BACK) {
                if (pad.connected) return true;
                hint = "请重新连接输入设备";
            } else if (action == GB_MENU_CONFIRM) {
                if (selected == 0) {
                    if (pad.connected) return true;
                    hint = "请重新连接输入设备";
                } else if (selected == 1) {
                    if (!battery) hint = "此游戏无电池存档";
                    else if (!gb_port_needs_save(session)) hint = "没有新存档";
                    else hint = gb_saves_write(session, hash)
                                    ? "同步完成，重进后读取" : "同步失败，进度仅在内存";
                } else if (selected == 2) {
                    settings_loop();
                    gb_select_pause_init(&select_back, gamepad_snapshot());
                    gb_menu_input_init(&pad_input, gamepad_snapshot());
                    hint = battery ? "游戏内存档，重进后读取" : "此游戏无电池存档";
                } else if (selected == 3) {
                    if (!screen_off_loop()) hint = "息屏恢复失败，请重试";
                    else if (gamepad_snapshot().connected) return true;
                    else hint = "请重新连接输入设备";
                    gb_select_pause_init(&select_back, gamepad_snapshot());
                    gb_menu_input_init(&pad_input, gamepad_snapshot());
                } else if (selected == 5) {
                    s_game_orientation = (s_game_orientation + 1) % FC_VIDEO_COUNT;
                    items[5] = s_orientation_actions[s_game_orientation];
                    hint = "继续游戏后生效";
                    ESP_LOGI(TAG, "game orientation: %u", s_game_orientation);
                } else {
                    if (gb_saves_write(session, hash)) return false;
                    hint = "同步失败，进度仅在内存";
                }
            }
        }
        bool now_connected = gamepad_snapshot().connected;
        redraw |= now_connected != connected;
        connected = now_connected;
        if (!redraw) continue;
        show_ui(connected ? "游戏暂停" : "输入设备断开",
                selected == 3 ? "任意按键唤醒" : hint, items, 6, selected,
                "上/下选择 OK确认");
    }
}

static gb_port_keys_t port_keys(gb_keys_t k) {
    return (gb_port_keys_t){.right=k.right,.left=k.left,.up=k.up,.down=k.down,
                            .a=k.a,.b=k.b,.select=k.select,.start=k.start};
}

typedef struct {
    bool pause;
    bool start;
    bool select;
} game_button_actions_t;

static game_button_actions_t poll_game_buttons(void) {
    button_message_t msg;
    game_button_actions_t actions = {0};
    while (xQueueReceive(s_buttons, &msg, 0) == pdTRUE) {
        if (msg.button == BSP_BTN_OK && msg.event == BSP_BTN_LONG)
            actions.pause = true;
        else if (msg.event == BSP_BTN_CLICK) {
            if (msg.button == BSP_BTN_OK) actions.start = true;
            else if (msg.button == BSP_BTN_UP) actions.select = true;
        }
    }
    return actions;
}

static bool pairing_loop(void) {
    gb_select_pause_t select_back;
    gb_select_pause_init(&select_back, gamepad_snapshot());
    gamepad_begin_discovery();
    size_t selected = 0;
    uint32_t shown_revision = UINT32_MAX;
    bool dirty = true;
    int64_t selected_at_us = 0;
    bool connection_slow = false;
    while (!gamepad_snapshot().connected) {
        gamepad_discovery_t discovery = gamepad_discovery_snapshot();
        if (discovery.count && selected >= discovery.count) selected = discovery.count - 1;
        button_message_t msg;
        while (menu_button_receive(&select_back, &msg, 0) == pdTRUE) {
            if (msg.button == BSP_BTN_OK && msg.event == BSP_BTN_LONG) {
                if (!discovery.selected) return false;
                gamepad_clear_selection();
                discovery = gamepad_discovery_snapshot();
                selected = 0;
                selected_at_us = 0;
                connection_slow = false;
                dirty = true;
            } else if (discovery.iine_confirmation_pending &&
                       msg.button == BSP_BTN_OK && msg.event == BSP_BTN_CLICK) {
                if (gamepad_confirm_iine()) {
                    selected_at_us = esp_timer_get_time();
                    connection_slow = false;
                    dirty = true;
                }
            } else if (!discovery.selected && msg.event == BSP_BTN_CLICK && discovery.count) {
                if (msg.button == BSP_BTN_UP)
                    selected = (selected + discovery.count - 1) % discovery.count;
                else if (msg.button == BSP_BTN_DOWN)
                    selected = (selected + 1) % discovery.count;
                else if (msg.button == BSP_BTN_OK && gamepad_select_candidate(selected)) {
                    selected_at_us = esp_timer_get_time();
                    connection_slow = false;
                }
                dirty = true;
            }
        }
        discovery = gamepad_discovery_snapshot();
        if (discovery.selected && selected_at_us == 0) selected_at_us = esp_timer_get_time();
        if (discovery.selected && !connection_slow &&
            !discovery.passkey_visible && esp_timer_get_time() - selected_at_us >= 25000000) {
            connection_slow = true;
            dirty = true;
        }
        if (discovery.revision != shown_revision) dirty = true;
        if (dirty) {
            const char *items[GB_UI_MAX_ITEMS] = {0};
            char choices[GB_UI_MAX_ITEMS][38] = {{0}};
            size_t count = 0, selected_item = 0;
            const char *hint = "键盘或手柄开启配对";
            if (discovery.selected) {
                if (discovery.iine_confirmation_pending) {
                    snprintf(choices[0], sizeof(choices[0]), "L1241 FW %02X.%02X",
                             (unsigned)(discovery.iine_firmware >> 8),
                             (unsigned)(discovery.iine_firmware & 0xff));
                    items[0] = choices[0];
                    items[1] = "版本未验证";
                    items[2] = "OK 尝试连接";
                    count = 3;
                    hint = "长按OK/Select 取消";
                } else if (discovery.passkey_visible) {
                    snprintf(choices[0], sizeof(choices[0]), "配对码 %06lu",
                             (unsigned long)discovery.passkey);
                    items[0] = choices[0];
                    items[1] = "键盘输入后按Enter";
                    count = 2;
                    hint = "长按OK/Select 取消";
                } else {
                    items[0] = discovery.pairing_failed ? "配对失败 请重试" :
                               connection_slow ? "连接较慢 请重试" : "连接中...";
                    count = 1;
                    hint = "长按OK/Select 重试";
                }
            } else if (!discovery.count) {
                items[0] = "扫描中...";
                count = 1;
            } else {
                size_t first = selected > 3 ? selected - 3 : 0;
                selected_item = selected - first;
                for (size_t i = first; i < discovery.count && count < GB_UI_MAX_ITEMS; i++) {
                    const gamepad_candidate_t *candidate = &discovery.candidates[i];
                    char short_name[19];
                    gamepad_copy_name(short_name, sizeof(short_name), candidate->name[0] ? candidate->name :
                                      candidate->type_pending ? "BLE HID" :
                                      candidate->keyboard ? "BLE Keyboard" : "手柄");
                    snprintf(choices[count], sizeof(choices[count]), "%s %02X:%02X", short_name,
                             candidate->address[4], candidate->address[5]);
                    items[count] = choices[count];
                    count++;
                }
            }
            show_ui("配对键盘或手柄", hint, items, count, selected_item,
                    "上/下 选择 OK 连接");
            shown_revision = discovery.revision;
            dirty = false;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    return true;
}

static bool prepare_game_screen(void) {
    esp_err_t e = fc_display_set_orientation(s_game_orientation);
    if (e == ESP_OK) e = gb_display_clear(s_game_orientation ? 0x0000 : GB_LCD_LIGHT);
    if (e == ESP_OK && !s_game_orientation) e = gb_display_game_hint("长按OK/Select 暂停");
    if (e != ESP_OK) ESP_LOGE(TAG, "game screen: %s", esp_err_to_name(e));
    return e == ESP_OK;
}



static bool play_fc_launch(void) {
    esp_err_t e=gb_display_clear(GB_LCD_LIGHT);
    int64_t started=esp_timer_get_time();
    while(e==ESP_OK) {
        uint32_t elapsed=(uint32_t)((esp_timer_get_time()-started)/1000);
        e=fc_display_launch_frame(elapsed);
        if(elapsed>=FC_LAUNCH_MS) break;
        vTaskDelay(pdMS_TO_TICKS(40));
    }
    return e==ESP_OK;
}

static void destroy_game_session(gb_session_t *session) {
    gamepad_set_gameplay(false);
    esp_err_t e=fc_display_drain();
    if(e!=ESP_OK) ESP_LOGE(TAG,"display drain: %s",esp_err_to_name(e));
    gb_port_destroy(session);
}

static void run_game(size_t index) {
    const gb_rom_entry_t *entry = gb_storage_entry(index);
    if (!entry) return;
    s_game_orientation = s_default_orientation;
    const char *loading[] = {entry->name};
    show_ui("加载游戏", "正在检查游戏", loading, 1, 0, "请稍候");
    gb_rom_source_t rom;
    if (!gb_storage_open(index, &rom)) { show_error("游戏文件校验失败"); return; }
    // I2S DMA/codec 需要先于核心分块分配，尤其是首次直接启动 CHR RAM 游戏。
    if (!gb_audio_prepare() && s_sound_enabled && s_volume > 0) {
        show_error("声音初始化失败"); return;
    }
    gb_session_t *session = NULL;
    fc_rom_info_t format_info;
    fc_rom_error_t format_error = fc_rom_validate(rom.mapped, rom.size, rom.size, &format_info);
    if (format_error != FC_ROM_OK) { show_error(fc_rom_error_string(format_error)); return; }
    if (!gb_port_create(&rom, &session)) { show_error("游戏不支持/内存不足"); return; }
    ESP_LOGI(TAG, "ROM loaded: %.31s", entry->name);
    if (gb_port_has_battery(session)) {
        if (!s_saves_ready) {
            destroy_game_session(session); show_error("存档空间不可用"); return;
        }
        if (gb_saves_load(session, entry->sha256) == GB_SAVE_ERROR) {
            destroy_game_session(session); show_error("存档副本损坏"); return;
        }
    }
    if (!gamepad_snapshot().connected && !pause_game(session, entry->sha256, false)) {
        destroy_game_session(session); return;
    }
    if(!play_fc_launch()) {
        destroy_game_session(session);show_error("启动动画显示失败");return;
    }
    // 动画期间的板载按键不传入游戏，手柄仍需松开启动键。
    xQueueReset(s_buttons);
    bool audio_enabled = s_sound_enabled && s_volume > 0;
    if (audio_enabled && !gb_audio_start(s_volume)) {
        destroy_game_session(session); show_error("声音启动失败"); return;
    }
    if (!prepare_game_screen()) {
        if (audio_enabled) gb_audio_stop();
        destroy_game_session(session); show_error("屏幕传输失败"); return;
    }
    gamepad_set_gameplay(true);
    int64_t next = esp_timer_get_time();
    int64_t last_save = next;
    int64_t last_report = next;
    int64_t last_idle_yield = next;
    uint32_t report_idle_yields = 0;
    uint32_t frames = 0;
    uint32_t report_frames = 0;
    uint32_t last_display_count = fc_display_completed();
    uint32_t dropped_frames = 0;
    uint64_t report_core_us = 0, report_audio_us = 0;
    uint64_t report_draw_us = 0, report_skip_us = 0, report_lcd_us = 0;
    uint32_t report_draw_frames = 0, report_skip_frames = 0;
    uint32_t report_key_changes = 0, report_multi_keys = 0;
    uint8_t previous_keys = 0;


    gb_frame_pacing_t pacing;
    gb_frame_pacing_init(&pacing);
    gb_select_pause_t select_pause;
    gb_select_pause_init(&select_pause, gamepad_snapshot());
    while (true) {
        game_button_actions_t board = poll_game_buttons();
        pad_sample_t pad = gamepad_frame_snapshot();
        bool select_held = gb_select_pause_step(&select_pause, gamepad_snapshot(),
                                               esp_timer_get_time());
        if (board.pause || select_held || !pad.connected) {
            if (select_held) ESP_LOGI(TAG, "Pause requested: Select long press");
            gb_port_set_keys(session, (gb_port_keys_t){0});
            if (audio_enabled && !gb_audio_stop()) {
                destroy_game_session(session); show_error("声音停止失败"); return;
            }
            if (!pause_game(session, entry->sha256, pad.connected)) break;
            audio_enabled = s_sound_enabled && s_volume > 0;
            if (audio_enabled && !gb_audio_start(s_volume)) {
                destroy_game_session(session); show_error("声音启动失败"); return;
            }
            if (!prepare_game_screen()) {
                if (audio_enabled) gb_audio_stop();
                destroy_game_session(session); show_error("屏幕传输失败"); return;
            }
            gamepad_set_gameplay(true);
            gb_select_pause_init(&select_pause, gamepad_snapshot());
            next = esp_timer_get_time();
            last_report = next;
            last_display_count = fc_display_completed();
            last_idle_yield = next;
            report_idle_yields = 0;
            report_frames = 0;
            dropped_frames = 0;
            report_core_us = report_audio_us = 0;
            report_draw_us = report_skip_us = report_lcd_us = 0;
            report_draw_frames = report_skip_frames = 0;
            report_key_changes = report_multi_keys = 0;
            previous_keys = 0;


            continue;
        }
        gb_keys_t keys = gb_input_map(pad);
        keys.start |= board.start;
        keys.select |= board.select;
        gb_port_set_keys(session, port_keys(keys));
        uint8_t key_bits = (uint8_t)((pad.right ? 1 : 0) | (pad.left ? 2 : 0) |
                           (pad.up ? 4 : 0) | (pad.down ? 8 : 0) |
                           (pad.a ? 16 : 0) | (pad.b ? 32 : 0) |
                           (pad.view ? 64 : 0) | (pad.menu ? 128 : 0));
        if (key_bits != previous_keys) report_key_changes++;
        if (__builtin_popcount((unsigned)key_bits) >= 2) report_multi_keys++;
        previous_keys = key_bits;
        bool display_idle=false;
        if(fc_display_idle(&display_idle)!=ESP_OK) {
            if(audio_enabled) gb_audio_stop();
            destroy_game_session(session); show_error("屏幕传输失败"); return;
        }
        bool draw_due = gb_frame_pacing_draw_due(&pacing, esp_timer_get_time(), next);
        bool draw = display_idle && draw_due;
        if(draw) gb_frame_pacing_mark_drawn(&pacing);
        if(draw_due && !display_idle) dropped_frames++;
        int64_t frame_started = esp_timer_get_time();
        if (!gb_port_step_frame(session, draw)) {
            if (audio_enabled) gb_audio_stop();
            destroy_game_session(session); show_error("游戏读取失败"); return;
        }
        int64_t core_done = esp_timer_get_time();
        report_core_us += (uint64_t)(core_done - frame_started);
        if (draw) { report_draw_us += (uint64_t)(core_done - frame_started); report_draw_frames++; }
        else { report_skip_us += (uint64_t)(core_done - frame_started); report_skip_frames++; }
        if (audio_enabled) {
            int16_t samples[GB_AUDIO_SAMPLES];
            size_t produced = gb_port_audio_frame(session, samples, GB_AUDIO_SAMPLES);
            if (!produced || !gb_audio_healthy()) {
                gb_audio_stop(); destroy_game_session(session); show_error("声音输出失败"); return;
            }
            gb_audio_submit(samples, produced);
        }
        int64_t audio_done = esp_timer_get_time();
        report_audio_us += (uint64_t)(audio_done - core_done);
        if (draw) {
            esp_err_t display_error = fc_display_submit(gb_port_framebuffer(session), fc_port_palette(session));
            report_lcd_us += (uint64_t)(esp_timer_get_time()-audio_done);
            if (display_error != ESP_OK) {
                if (audio_enabled) gb_audio_stop();
                destroy_game_session(session); show_error("屏幕传输失败"); return;
            }
        }
        gb_frame_pacing_record(&pacing, (uint32_t)(esp_timer_get_time() - frame_started));
        frames++;
        report_frames++;
        int64_t now = esp_timer_get_time();
        if (now - last_report >= 10000000LL) {
            uint32_t completed=fc_display_completed();
            ESP_LOGI(TAG, "display_fps=%.1f lcd_submit_us=%lu rendered=%lu pcm_produced=%lu pcm_written=%lu",
                     (double)(completed-last_display_count)*1000000.0/(double)(now-last_report),
                     (unsigned long)(report_lcd_us/(report_draw_frames?report_draw_frames:1)),
                     (unsigned long)report_draw_frames,
                     (unsigned long)gb_audio_requested_samples(),
                     (unsigned long)gb_audio_output_samples());
            last_display_count=completed;
            ESP_LOGI(TAG, "emu_fps=%.1f draw_every=%u core_us=%lu draw_us=%lu skip_us=%lu audio_us=%lu key_changes=%lu multi_frames=%lu display_drops=%lu audio_drops=%lu audio_waits=%lu max_audio_wait_us=%lu free_heap=%lu min_heap=%lu largest_block=%lu idle_yields=%lu",
                     (double)report_frames * 1000000.0 / (double)(now - last_report),
                     (unsigned)pacing.draw_interval,
                     (unsigned long)(report_core_us / report_frames),
                     (unsigned long)(report_draw_us / (report_draw_frames ? report_draw_frames : 1)),
                     (unsigned long)(report_skip_us / (report_skip_frames ? report_skip_frames : 1)),
                     (unsigned long)(report_audio_us / report_frames),
                     (unsigned long)report_key_changes,
                     (unsigned long)report_multi_keys,
                     (unsigned long)dropped_frames,
                     (unsigned long)gb_audio_dropped(),
                     (unsigned long)gb_audio_waits(),
                     (unsigned long)gb_audio_max_wait_us(),
                     (unsigned long)esp_get_free_heap_size(),
                     (unsigned long)esp_get_minimum_free_heap_size(),
                     (unsigned long)heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL),
                     (unsigned long)report_idle_yields);
            last_report = now;
            report_frames = 0;
            dropped_frames = 0;
            report_core_us = report_audio_us = 0;
            report_draw_us = report_skip_us = report_lcd_us = 0;
            report_draw_frames = report_skip_frames = 0;
            report_key_changes = report_multi_keys = 0;
            report_idle_yields = 0;
        }
        if (now - last_save >= 30000000LL && gb_port_has_battery(session)) {
            if (!gb_saves_write(session, entry->sha256)) {
                ESP_LOGE(TAG, "automatic save failed; pausing with session in RAM");
                if (audio_enabled) gb_audio_stop();
                if (!pause_game(session, entry->sha256, true)) break;
                audio_enabled = s_sound_enabled && s_volume > 0;
                if (audio_enabled && !gb_audio_start(s_volume)) {
                    destroy_game_session(session); show_error("声音启动失败"); return;
                }
                if (!prepare_game_screen()) {
                    if (audio_enabled) gb_audio_stop();
                    destroy_game_session(session); show_error("屏幕传输失败"); return;
                }
                gamepad_set_gameplay(true);
                gb_select_pause_init(&select_pause, gamepad_snapshot());
                next = esp_timer_get_time();


                last_report = next;
                last_display_count = fc_display_completed();
                report_frames = 0;
                dropped_frames = 0;
                report_core_us = report_audio_us = 0;
                report_draw_us = report_skip_us = report_lcd_us = 0;
                report_draw_frames = report_skip_frames = 0;
                report_key_changes = report_multi_keys = 0;
            }
            last_save = esp_timer_get_time();
        }
        now = esp_timer_get_time();
        next = gb_frame_pacing_next(next, now);
        // taskYIELD cannot schedule the lower-priority IDLE task. Even when
        // every frame is late, periodically block so IDLE can run its watchdog.
        if (now - last_idle_yield >= 250000) {
            vTaskDelay(1);
            now = esp_timer_get_time();
            last_idle_yield = now;
            report_idle_yields++;
        }
        while (next - now > (int64_t)portTICK_PERIOD_MS * 1000 + 1500) {
            vTaskDelay(1);
            now = esp_timer_get_time();
            last_idle_yield = now;
            report_idle_yields++;
        }
        while (next > now) {
            taskYIELD();
            now = esp_timer_get_time();
        }
    }
    if (audio_enabled && !gb_audio_stop()) ESP_LOGE(TAG, "audio stop failed");
    destroy_game_session(session);
}

static void menu_loop(void) {
    xQueueReset(s_buttons);
    pad_sample_t entry = gamepad_snapshot();
    gb_select_pause_t select_back;
    gb_select_pause_init(&select_back, entry);
    gb_menu_input_t pad_input;
    gb_menu_input_init(&pad_input, entry);
    size_t selected = 0;
    bool dirty = true;
#ifdef FC_INPUT_TRACE
    ESP_LOGI(TAG, "game_list_enter connected=%u held_a=%u", (unsigned)entry.connected, (unsigned)entry.a);
#endif
    while (true) {
        size_t count = gb_storage_count();
        if (!count) {
            const char *items[] = {"没有游戏"};
            show_ui("游戏列表", "请到游戏管理上传", items, 1, 0, "长按OK/Select 返回");
            button_message_t msg;
            while (true) {
                if (!gamepad_snapshot().connected) return;
                if (menu_button_receive(&select_back, &msg, pdMS_TO_TICKS(50)) == pdTRUE &&
                    msg.button == BSP_BTN_OK && msg.event == BSP_BTN_LONG) return;
            }
        }
        button_message_t msg;
        bool board_event = menu_button_receive(&select_back, &msg, 0) == pdTRUE;
        pad_sample_t pad = gamepad_snapshot();
        if (!pad.connected) return;
        gb_menu_action_t pad_action = gb_menu_input_step(&pad_input, pad);
        gb_menu_action_t action = board_event ? board_menu_action(msg) : GB_MENU_NONE;
        if (board_event && msg.event == BSP_BTN_LONG && msg.button == BSP_BTN_OK)
            action = GB_MENU_BACK;
        if (action == GB_MENU_NONE && pad_action != GB_MENU_BACK) action = pad_action;
        if (action == GB_MENU_BACK) return;
        if (action == GB_MENU_UP || action == GB_MENU_DOWN) {
            selected = (selected + (action == GB_MENU_UP ? count - 1 : 1)) % count;
            dirty = true;
        } else if (action == GB_MENU_CONFIRM) {
            run_game(selected);
            xQueueReset(s_buttons);
            pad_sample_t resumed = gamepad_snapshot();
            gb_select_pause_init(&select_back, resumed);
            gb_menu_input_init(&pad_input, resumed);
            dirty = true;
        }
        if (dirty) {
            const char *items[GB_UI_MAX_ITEMS] = {0};
            char names[GB_UI_MAX_ITEMS][38] = {{0}};
            size_t first = selected > 3 ? selected - 3 : 0;
            for (size_t i = 0; i < GB_UI_MAX_ITEMS && first + i < count; i++) {
                const gb_rom_entry_t *entry = gb_storage_entry(first + i);
                snprintf(names[i], sizeof(names[i]), "%.31s", entry->name);
                items[i] = names[i];
            }
            size_t shown = count - first;
            if (shown > GB_UI_MAX_ITEMS) shown = GB_UI_MAX_ITEMS;
            show_ui("游戏列表", "请选择游戏", items, shown, selected - first,
                    "上/下 OK开始 长按OK/Select返回");
            dirty = false;
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

static void input_entry_loop(void) {
    if (gamepad_snapshot().connected || pairing_loop()) menu_loop();
}

void app_main(void) {
    esp_err_t e = nvs_flash_init();
    if (e != ESP_OK) { ESP_LOGE(TAG, "NVS: %s", esp_err_to_name(e)); return; }
    e = gb_display_init();
    if (e != ESP_OK) { ESP_LOGE(TAG, "display: %s", esp_err_to_name(e)); return; }
    s_buttons = xQueueCreate(16, sizeof(button_message_t));
    if (!s_buttons) { startup_error("按键队列内存不足"); return; }
    e = bsp_button_init(button_callback, NULL);
    if (e != ESP_OK) { startup_error("按键初始化失败"); return; }
    settings_load();
    bsp_display_backlight(s_brightness);
    xQueueReset(s_buttons);
    e = gamepad_start();
    if (e != ESP_OK) { startup_error("蓝牙初始化失败"); return; }
    s_saves_ready = gb_saves_init();
    if (bsp_battery_init() != ESP_OK) ESP_LOGW(TAG, "battery unavailable");
    if (!s_saves_ready) ESP_LOGE(TAG, "save partition unavailable; battery games disabled");
    if (!gb_storage_init()) ESP_LOGW(TAG, "ROM container unavailable or invalid");
    else if (s_saves_ready && gb_storage_catalog_trusted() &&
             !gb_saves_prune_orphans()) ESP_LOGE(TAG, "orphan save cleanup failed");
    ESP_LOGI(TAG, "FC_READY version=%s games=%u", esp_app_get_description()->version, (unsigned)gb_storage_count());
#ifdef FC_WIFI_MEMORY_TEST
    // 仅专用回归构建启用；不上传、删除 ROM，也不读写游戏存档。
    gamepad_set_scanning(false);
    vTaskDelay(pdMS_TO_TICKS(100));
    bool first_web = gb_web_start();
    vTaskDelay(pdMS_TO_TICKS(2000));
    gb_web_stop();
    gamepad_set_scanning(true);
    vTaskDelay(pdMS_TO_TICKS(1000));
    ESP_LOGI(TAG, "WIFI_TEST cold_web=%s", first_web ? "PASS" : "FAIL");
    for (int round = 0; round < 4; round++) {
        for (size_t position = 0; position < gb_storage_count(); position++) {
            size_t i = gb_storage_count() - 1 - position;
            gb_rom_source_t rom;
            gb_session_t *session = NULL;
            bool ok = gb_storage_open(i, &rom) && gb_audio_prepare() && gb_port_create(&rom, &session);
            if (ok) {
                bool launch = fc_display_launch_frame(0) == ESP_OK &&
                              fc_display_launch_frame(FC_LAUNCH_MS) == ESP_OK;
                bool audio = gb_audio_start(0);
                ok = audio && launch;
                for (int frame = 0; frame < 8; frame++) {
                    gb_port_step_frame(session, true);
                    int16_t samples[267];
                    size_t count = gb_port_audio_frame(session, samples, 267);
                    if (audio) gb_audio_submit(samples, count);
                }
                if (fc_display_frame(gb_port_framebuffer(session), fc_port_palette(session)) != ESP_OK) ok = false;
                vTaskDelay(pdMS_TO_TICKS(50));
                if (audio && !gb_audio_stop()) ok = false;
                gb_port_destroy(session);
            }
            ESP_LOGI(TAG, "WIFI_TEST round=%d game=%u result=%s", round, (unsigned)i, ok ? "PASS" : "FAIL");
        }
        if (round == 3) break;
        gamepad_set_scanning(false);
        vTaskDelay(pdMS_TO_TICKS(100));
        bool started = gb_web_start();
        vTaskDelay(pdMS_TO_TICKS(2000));
        gb_web_stop();
        gamepad_set_scanning(true);
        vTaskDelay(pdMS_TO_TICKS(1000));
        ESP_LOGI(TAG, "WIFI_TEST cycle=%d web=%s", round, started ? "PASS" : "FAIL");
        log_management_heap("automated cycle complete");
    }
    ESP_LOGI(TAG, "WIFI_TEST complete");
#endif
    while (true) {
        size_t choice = home_loop();
        if (choice == 0) input_entry_loop();
        else if (choice == 1) management_loop();
        else if (choice == 2) gamepad_test_loop();
        else if (choice == 3) settings_loop();
    }
}
