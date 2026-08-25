#include <eek_inferencing.h>
#include <ESP_I2S.h>
#include <string.h>
#define EI_CLASSIFIER_TFLITE_ENABLE_ESP_NN 1

I2SClass I2S;

const uint32_t WINDOW_SIZE = EI_CLASSIFIER_RAW_SAMPLE_COUNT;
const uint32_t RING_SIZE = WINDOW_SIZE * 2;

static int16_t *ringBuffer = nullptr;
static int16_t *snapshotBuffer = nullptr;
static volatile uint32_t writeIndex = 0;

void captureTask(void *pvParameters)
{
    for (;;)
    {
        int sample = I2S.read();
        if (sample == -1) continue;
        ringBuffer[writeIndex % RING_SIZE] = (int16_t)sample;
        writeIndex++;
    }
}

static int get_signal_data(size_t offset, size_t length, float *out_ptr)
{
    numpy::int16_to_float(snapshotBuffer + offset, out_ptr, length);
    return 0;
}

void setup()
{
    Serial.begin(115200);
    while (!Serial);
    delay(500);

    ringBuffer = (int16_t *)ps_malloc(RING_SIZE * sizeof(int16_t));
    snapshotBuffer = (int16_t *)ps_malloc(WINDOW_SIZE * sizeof(int16_t));
    if (!ringBuffer || !snapshotBuffer) {
        Serial.println("ERROR: buffer allocation failed.");
        while (1);
    }

    I2S.setPinsPdmRx(42, 41);
    if (!I2S.begin(I2S_MODE_PDM_RX, EI_CLASSIFIER_FREQUENCY, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO)) {
        Serial.println("ERROR: I2S failed.");
        while (1);
    }

    xTaskCreatePinnedToCore(captureTask, "capture", 4096, NULL, 1, NULL, 0);

    while (writeIndex < WINDOW_SIZE) { delay(10); }
}

void loop()
{
    uint32_t endIdx = writeIndex;
    for (uint32_t i = 0; i < WINDOW_SIZE; i++) {
        uint32_t ringPos = (endIdx - WINDOW_SIZE + i) % RING_SIZE;
        snapshotBuffer[i] = ringBuffer[ringPos];
    }

    signal_t signal;
    signal.total_length = WINDOW_SIZE;
    signal.get_data = get_signal_data;

    ei_impulse_result_t result = { 0 };
    EI_IMPULSE_ERROR res = run_classifier(&signal, &result, false);

    if (res != EI_IMPULSE_OK) {
        Serial.println("CLASSIFIER FAILED.");
        delay(200);
        return;
    }

    for (size_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
        if (strcmp(result.classification[i].label, "trigger") == 0 && result.classification[i].value > 0.5) {
            Serial.println("TRIGGERED!");
        }
    }

    delay(200);
}