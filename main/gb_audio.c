#include "gb_audio.h"
#include "gb_audio_pacing.h"
#include "bsp_audio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include <string.h>

typedef struct {
    bool stop;
    uint16_t output_count;
    int16_t samples[GB_AUDIO_SAMPLES];
} audio_block_t;

// APU 和输出统一为 16 kHz，使用 codec 支持的时钟配置且无需逐块重采样。
// FC 核心每帧交替提交 266/267 个样本，不重复推进 APU 或重采样。

static const char *TAG = "gb_audio";
static QueueHandle_t s_queue;
static SemaphoreHandle_t s_stopped;
static TaskHandle_t s_task;
// 音频队列和工作栈常驻，避免游戏占用堆后再申请大块连续内存。
static StaticQueue_t s_queue_control;
static uint8_t s_queue_storage[8 * sizeof(audio_block_t)];
static StaticSemaphore_t s_stopped_control;
static StaticTask_t s_task_control;
static StackType_t s_task_stack[5120 / sizeof(StackType_t)];
static bool s_running;
static volatile bool s_failed;
static volatile bool s_stop_requested;
static uint32_t s_dropped;
static volatile uint32_t s_output_samples;
static volatile uint32_t s_waits;
static volatile uint32_t s_max_wait_us;
static uint64_t s_submitted_samples;
static int16_t s_output[GB_AUDIO_MAX_OUTPUT_SAMPLES * 2];

static void audio_task(void *arg) {
    (void)arg;
    audio_block_t block;
    while (true) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    // 启动时先积攒四帧，让较慢游戏也有足够的音频缓冲。
    while (uxQueueMessagesWaiting(s_queue) < 4 && !s_stop_requested) vTaskDelay(1);
    while (!s_stop_requested) {
        size_t count = 0;
        bool stop = false;
        for (int i = 0; i < 2; i++) {
            int64_t waiting_since = esp_timer_get_time();
            if (xQueueReceive(s_queue, &block, portMAX_DELAY) != pdTRUE || block.stop) {
                stop = true;
                break;
            }
            uint32_t waited = (uint32_t)(esp_timer_get_time() - waiting_since);
            if (waited > 5000) s_waits++;
            if (waited > s_max_wait_us) s_max_wait_us = waited;
            memcpy(s_output + count, block.samples, block.output_count * sizeof(int16_t));
            count += block.output_count;
        }
        if (stop) break;
        if (bsp_audio_write(s_output, count * sizeof(s_output[0])) != ESP_OK) {
            ESP_LOGE(TAG, "PCM output failed");
            s_failed = true;
            break;
        }
        s_output_samples += count;
    }
    bsp_audio_set_volume(0);
    ESP_LOGI(TAG, "audio stopped, stack free=%u", (unsigned)uxTaskGetStackHighWaterMark(NULL));
    xSemaphoreGive(s_stopped);
    }
}

bool gb_audio_prepare(void) {
    if (s_running) return true;
    esp_err_t e = bsp_audio_init_playback();
    if (e == ESP_OK) e = bsp_audio_set_format(GB_AUDIO_OUTPUT_RATE, 16, 1);
    if (e != ESP_OK) {
        ESP_LOGE(TAG, "codec init failed: %s", esp_err_to_name(e));
        return false;
    }
    bsp_audio_set_volume(0);
    return true;
}

bool gb_audio_start(uint8_t volume) {
    if (s_running || !gb_audio_prepare()) return false;
    if (!s_queue) {
        s_queue = xQueueCreateStatic(8, sizeof(audio_block_t), s_queue_storage, &s_queue_control);
        s_stopped = xSemaphoreCreateBinaryStatic(&s_stopped_control);
    }
    xQueueReset(s_queue);
    s_failed = false;
    s_stop_requested = false;
    s_dropped = 0;
    s_output_samples = 0;
    s_waits = 0;
    s_max_wait_us = 0;
    s_submitted_samples = 0;
    bsp_audio_set_volume(volume);
    if (!s_task) {
        s_task = xTaskCreateStatic(audio_task, "gb_audio", sizeof(s_task_stack), NULL, 4,
                                   s_task_stack, &s_task_control);
    }
    s_running = true;
    xTaskNotifyGive(s_task);
    return true;
}

void gb_audio_submit(const int16_t *samples, size_t count) {
    if (!s_running || !samples || !count || count > GB_AUDIO_SAMPLES || s_failed) return;
    audio_block_t block = {.stop = false};
    block.output_count = (uint16_t)count;
    s_submitted_samples += count;
    memcpy(block.samples, samples, count * sizeof(int16_t));
    if (xQueueSend(s_queue, &block, 0) != pdTRUE) {
        audio_block_t stale;
        if (xQueueReceive(s_queue, &stale, 0) == pdTRUE) s_dropped++;
        if (xQueueSend(s_queue, &block, 0) != pdTRUE) s_dropped++;
    }
}

bool gb_audio_healthy(void) { return !s_failed; }
uint32_t gb_audio_dropped(void) { return s_dropped; }
uint32_t gb_audio_output_samples(void) { return s_output_samples; }
uint32_t gb_audio_requested_samples(void) {
    return (uint32_t)s_submitted_samples;
}
uint32_t gb_audio_waits(void) { return s_waits; }
uint32_t gb_audio_max_wait_us(void) { return s_max_wait_us; }

bool gb_audio_stop(void) {
    if (!s_running) return true;
    s_stop_requested = true;
    xQueueReset(s_queue);
    audio_block_t block = {.stop = true};
    xQueueSend(s_queue, &block, 0);
    if (xSemaphoreTake(s_stopped, pdMS_TO_TICKS(2000)) != pdTRUE) {
        ESP_LOGE(TAG, "audio task did not stop");
        return false;
    }
    s_running = false;
    return !s_failed;
}
