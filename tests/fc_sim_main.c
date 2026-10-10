// 仅在 -DFC_SIM_TEST=ON 的独立构建中编译。无线模块不在这个验收范围内。
#include "gb_port.h"
#include "gb_storage.h"
#include "gb_display.h"
#include "gb_audio.h"
#include "gb_saves.h"
#include "bsp_button.h"
#include "bsp_display.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
static QueueHandle_t buttons;
static uint32_t frame_hash(gb_session_t *session) {
    uint8_t *const *pixels=gb_port_framebuffer(session);
    uint32_t hash=2166136261u;
    for(size_t y=0;y<240;y++) for(size_t x=0;x<272;x++) hash=(hash^pixels[y][x])*16777619u;
    return hash;
}
static void on_button(bsp_btn_t button,bsp_btn_ev_t event,void *context) {
    if(event==BSP_BTN_PRESS) xQueueSend(buttons,&button,0);
}
void app_main(void) {
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(gb_display_init());
    const gb_ui_model_t theme={.title="FC模拟器",.hint="选择游戏",.count=3,.selected=0,
        .items={"魂斗罗","超级马里奥兄弟","坦克大战"},.footer="SIM TEST",.battery_percent=80};
    ESP_ERROR_CHECK(gb_display_ui(&theme));
    ESP_LOGI("fc_sim","FC_SIM_THEME_READY");
    vTaskDelay(pdMS_TO_TICKS(200));
    bsp_display_backlight(75);
    buttons=xQueueCreate(8,sizeof(bsp_btn_t)); configASSERT(buttons);
    ESP_ERROR_CHECK(bsp_button_init(on_button,NULL));
    configASSERT(gb_saves_init()); configASSERT(gb_storage_init());
    ESP_LOGW("fc_sim","SIM_TEST_ONLY: BLE/Wi-Fi excluded; not production acceptance");
    ESP_LOGI("fc_sim","FC_SIM_READY games=%u",(unsigned)gb_storage_count());
    configASSERT(gb_storage_count()>0);
    gb_rom_source_t source; configASSERT(gb_storage_open(0,&source));
    gb_session_t *session=NULL; configASSERT(gb_port_create(&source,&session));
    const gb_rom_entry_t *entry=gb_storage_entry(0);
    configASSERT(gb_saves_load(session,entry->sha256)!=GB_SAVE_ERROR);
    // 在真实异步显示任务运行期间推进跳帧，再进入菜单、退出并重建会话。
    for(int round=0;round<3;round++) {
        configASSERT(gb_port_step_frame(session,true));
        uint32_t hash=frame_hash(session),completed=fc_display_completed();
        ESP_ERROR_CHECK(fc_display_submit(gb_port_framebuffer(session),fc_port_palette(session)));
        for(int skip=0;skip<8;skip++) configASSERT(gb_port_step_frame(session,false));
        configASSERT(frame_hash(session)==hash);
        ESP_ERROR_CHECK(gb_display_ui(&theme));
        ESP_ERROR_CHECK(fc_display_drain());
        configASSERT(fc_display_completed()==completed+1);
        gb_port_destroy(session); session=NULL;
        configASSERT(gb_port_create(&source,&session));
    }
    ESP_LOGI("fc_sim","FC_SIM_ASYNC_LIFECYCLE_OK");
    ESP_ERROR_CHECK(gb_display_clear(0));
    ESP_ERROR_CHECK(gb_display_game_hint("SIM TEST"));
    configASSERT(gb_audio_start(45));
    int64_t deadline=esp_timer_get_time();
    unsigned frames=0;
    for(;;) {
        bsp_btn_t button;gb_port_keys_t keys={0};
        while(xQueueReceive(buttons,&button,0)==pdTRUE) {
            keys.a|=button==BSP_BTN_UP; keys.right|=button==BSP_BTN_DOWN; keys.start|=button==BSP_BTN_OK;
            ESP_LOGI("fc_sim","FC_SIM_BUTTON=%d",(int)button);
        }
        gb_port_set_keys(session,keys);
        configASSERT(gb_port_step_frame(session,true));
        int16_t samples[GB_AUDIO_SAMPLES];
        size_t count=gb_port_audio_frame(session,samples,GB_AUDIO_SAMPLES);
        configASSERT(count && gb_audio_healthy());gb_audio_submit(samples,count);
        bool display_idle=false;
        ESP_ERROR_CHECK(fc_display_idle(&display_idle)); configASSERT(display_idle);
        uint32_t completed=fc_display_completed();
        ESP_ERROR_CHECK(fc_display_submit(gb_port_framebuffer(session),fc_port_palette(session)));
        ESP_ERROR_CHECK(fc_display_drain());
        configASSERT(fc_display_completed()==completed+1);
        frames++;
        if(frames%60==0) ESP_LOGI("fc_sim","FC_SIM_FRAME=%u heap=%lu audio=%lu",frames,(unsigned long)esp_get_free_heap_size(),(unsigned long)gb_audio_output_samples());
        if(frames==120 && gb_port_has_battery(session)) {
            uint8_t saved[8],blank[8]={0},loaded[8];
            configASSERT(gb_port_read_sram(session,0,saved,sizeof(saved)));
            configASSERT(gb_saves_write(session,entry->sha256));
            configASSERT(gb_port_write_sram(session,0,blank,sizeof(blank)));
            configASSERT(gb_saves_load(session,entry->sha256)==GB_SAVE_LOADED);
            configASSERT(gb_port_read_sram(session,0,loaded,sizeof(loaded)));
            configASSERT(!memcmp(saved,loaded,sizeof(saved)));
            ESP_LOGI("fc_sim","FC_SIM_SAVE_RELOAD_OK");
        }
        deadline+=16667;
        int64_t wait=deadline-esp_timer_get_time();
        vTaskDelay(wait>1000 ? pdMS_TO_TICKS(wait/1000)+1 : 1);
    }
}
