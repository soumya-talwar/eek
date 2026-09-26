#include <eek_inferencing.h>
#include <ESP_I2S.h>
#include <string.h>
#include "scream.h"
#define EI_CLASSIFIER_TFLITE_ENABLE_ESP_NN 1

I2SClass I2S;                  
I2SClass I2SOut(I2S_NUM_1);

#define I2S_BCLK_PIN   2   // D1
#define I2S_LRC_PIN    1   // D0
#define I2S_DOUT_PIN   3   // D2

const uint32_t WINDOW_SIZE = EI_CLASSIFIER_RAW_SAMPLE_COUNT;
const uint32_t RING_SIZE = WINDOW_SIZE * 2;

static int16_t *ringBuffer = nullptr;
static int16_t *snapshotBuffer = nullptr;
static volatile uint32_t writeIndex = 0;

const uint32_t SCREAM_SAMPLE_RATE = 44100;
const size_t SCREAM_LENGTH = sizeof(scream) / sizeof(scream[0]);
bool audioOutReady = false;

int16_t *stereoScream = nullptr;
size_t stereoScreamBytes = 0;

unsigned long lastTriggerTime = 0;
const unsigned long TRIGGER_COOLDOWN_MS = 1000;

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

void buildStereoScream()
{
    size_t sampleCount = sizeof(scream) / sizeof(scream[0]);
    stereoScream = (int16_t *)ps_malloc(sampleCount * 2 * sizeof(int16_t));
    if (!stereoScream) {
        Serial.println("ERROR: stereoScream allocation failed.");
        return;
    }
    for (size_t i = 0; i < sampleCount; i++) {
        int16_t s = (int16_t)pgm_read_word(&scream[i]);
        stereoScream[i * 2]     = s; // Left
        stereoScream[i * 2 + 1] = s; // Right
    }
    stereoScreamBytes = sampleCount * 2 * sizeof(int16_t);
}

void playScream()
{
    if (!audioOutReady || !stereoScream) return;
    Serial.println("Playing scream...");
    size_t written = I2SOut.write((const uint8_t*)stereoScream, stereoScreamBytes);
    Serial.print("Bytes written: ");
    Serial.print(written);
    Serial.print(" / ");
    Serial.println(stereoScreamBytes);
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

    buildStereoScream();

    I2S.setPinsPdmRx(42, 41);
    if (!I2S.begin(I2S_MODE_PDM_RX, EI_CLASSIFIER_FREQUENCY, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO)) {
        Serial.println("ERROR: I2S mic failed.");
        while (1);
    }

    I2SOut.setPins(I2S_BCLK_PIN, I2S_LRC_PIN, I2S_DOUT_PIN);
    audioOutReady = I2SOut.begin(I2S_MODE_STD, SCREAM_SAMPLE_RATE, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO);
    if (!audioOutReady) {
        Serial.println("ERROR: I2S output (amp) failed to start.");
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
            if (millis() - lastTriggerTime > TRIGGER_COOLDOWN_MS) {
                playScream();
                lastTriggerTime = millis();
            }
        }
    }

    delay(200);
}