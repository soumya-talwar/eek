#include <soumya-talwar-project-1_inferencing.h>
#include <ESP_I2S.h>
#define EI_CLASSIFIER_TFLITE_ENABLE_ESP_NN 1

I2SClass I2S;

const uint32_t SAMPLE_COUNT = EI_CLASSIFIER_RAW_SAMPLE_COUNT;

static int16_t *audioBuffer = nullptr;

static int get_signal_data(
    size_t offset,
    size_t length,
    float *out_ptr
)
{
    numpy::int16_to_float(
        audioBuffer + offset,
        out_ptr,
        length
    );
    return 0;
}

void setup()
{
    Serial.begin(115200);
    while (!Serial);
    delay(500);
    audioBuffer = (int16_t *)ps_malloc(
        SAMPLE_COUNT * sizeof(int16_t)
    );
    if (audioBuffer == nullptr)
    {
        Serial.println("ERROR: Audio buffer allocation failed.");
        while (1);
    }
    I2S.setPinsPdmRx(42, 41);
    if (!I2S.begin(
        I2S_MODE_PDM_RX,
        EI_CLASSIFIER_FREQUENCY,
        I2S_DATA_BIT_WIDTH_16BIT,
        I2S_SLOT_MODE_MONO
    ))
    {
        Serial.println("ERROR: I2S failed.");
        while (1);
    }
}

void loop()
{
    for (uint32_t i = 0; i < SAMPLE_COUNT; i++)
    {
        int sample = I2S.read();

        while (sample == -1)
        {
            sample = I2S.read();
        }

        audioBuffer[i] = (int16_t)sample;
    }
    signal_t signal;
    signal.total_length = SAMPLE_COUNT;
    signal.get_data = get_signal_data;
    ei_impulse_result_t result = { 0 };
    unsigned long startTime = millis();
    EI_IMPULSE_ERROR res = run_classifier(
        &signal,
        &result,
        false
    );
    unsigned long elapsed = millis() - startTime;
    if (res != EI_IMPULSE_OK)
    {
        Serial.println("CLASSIFIER FAILED.");
        delay(2000);
        return;
    }
    for (size_t i = 0;
         i < EI_CLASSIFIER_LABEL_COUNT;
         i++)
    {
        if (result.classification[i].label == "marriage" && result.classification[i].value > 0.3)
            Serial.println("MARRIAGE!");
        
    }
    delay(500);
}