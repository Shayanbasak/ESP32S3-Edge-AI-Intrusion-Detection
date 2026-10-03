#include <Arduino.h>
#include "esp_camera.h"
#include "img_converters.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

// =====================================================
// MODEL HEADERS
// =====================================================

#include "edge_ai_human_nonhuman_int8_model_data.h"
#include "activity_int8_model_data.h"

#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"

// =====================================================
// WI-FI + TELEGRAM CONFIGURATION
// =====================================================
// Fill these four values locally. Do NOT share them publicly.

const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

const char* BOT_TOKEN   = "YOUR_TELEGRAM_BOT_TOKEN";
const char* CHAT_ID     = "YOUR_TELEGRAM_CHAT_ID";

// =====================================================
// PIR
// =====================================================

#define PIR_PIN 21

// =====================================================
// CAMERA PIN CONFIGURATION
// ESP32-S3 + OV3660
// =====================================================

#define PWDN_GPIO_NUM     -1
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      15
#define SIOD_GPIO_NUM       4
#define SIOC_GPIO_NUM       5
#define Y9_GPIO_NUM        16
#define Y8_GPIO_NUM        17
#define Y7_GPIO_NUM        18
#define Y6_GPIO_NUM        12
#define Y5_GPIO_NUM        10
#define Y4_GPIO_NUM         8
#define Y3_GPIO_NUM         9
#define Y2_GPIO_NUM        11
#define VSYNC_GPIO_NUM      6
#define HREF_GPIO_NUM       7
#define PCLK_GPIO_NUM      13

// =====================================================
// AI SETTINGS
// =====================================================

#define IMG_WIDTH   96
#define IMG_HEIGHT  96
#define NUM_FRAMES  5
#define HUMAN_THRESHOLD 0.50f

const char* activityLabels[4] = {
    "STANDING",
    "WALKING",
    "SITTING",
    "FALLING"
};

// =====================================================
// TENSOR ARENAS
// =====================================================
// The original working human model used 6 MB.
// The new activity CNN is small, so it gets a separate
// 512 KB arena. If activity AllocateTensors() fails,
// increase ACTIVITY_TENSOR_ARENA_SIZE to 768 KB or 1 MB.

constexpr size_t HUMAN_TENSOR_ARENA_SIZE = 6 * 1024 * 1024;
constexpr size_t ACTIVITY_TENSOR_ARENA_SIZE = 512 * 1024;

uint8_t* human_tensor_arena = nullptr;
uint8_t* activity_tensor_arena = nullptr;

// =====================================================
// HUMAN/NON-HUMAN TFLITE OBJECTS
// =====================================================

const tflite::Model* human_model = nullptr;
tflite::MicroInterpreter* human_interpreter = nullptr;
TfLiteTensor* human_input = nullptr;
TfLiteTensor* human_output = nullptr;

// =====================================================
// ACTIVITY TFLITE OBJECTS
// =====================================================

const tflite::Model* activity_model = nullptr;
tflite::MicroInterpreter* activity_interpreter = nullptr;
TfLiteTensor* activity_input = nullptr;
TfLiteTensor* activity_output = nullptr;

// =====================================================
// EXPERIMENT VARIABLES
// =====================================================

int totalEvents = 0;

// =====================================================
// TELEGRAM IMAGE BUFFER
// =====================================================

uint8_t* bestJpeg = nullptr;
size_t bestJpegLength = 0;
float bestFrameScore = -1.0f;
bool bestFrameHuman = false;

// =====================================================
// CAMERA INITIALIZATION
// =====================================================

bool initCamera()
{
    camera_config_t config;

    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;

    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;

    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;

    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;

    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;

    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_RGB565;
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    config.fb_location = CAMERA_FB_IN_PSRAM;

    esp_err_t err = esp_camera_init(&config);

    if (err != ESP_OK)
    {
        Serial.print("Camera initialization failed. Error: 0x");
        Serial.println(err, HEX);
        return false;
    }

    Serial.println("Camera initialized successfully.");
    return true;
}

// =====================================================
// WIFI CONNECTION
// =====================================================

bool connectWiFi()
{
    Serial.println();
    Serial.println("======================================");
    Serial.println("CONNECTING TO WI-FI");
    Serial.println("======================================");

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    int attempts = 0;

    while (WiFi.status() != WL_CONNECTED && attempts < 30)
    {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    Serial.println();

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("Wi-Fi connected successfully.");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        Serial.print("Signal RSSI: ");
        Serial.print(WiFi.RSSI());
        Serial.println(" dBm");
        return true;
    }

    Serial.println("ERROR: Wi-Fi connection failed.");
    return false;
}

// =====================================================
// TELEGRAM TEXT MESSAGE
// =====================================================

bool sendTelegramMessage(const String& message)
{
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("Telegram ERROR: Wi-Fi not connected.");
        return false;
    }

    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient https;

    String url =
        "https://api.telegram.org/bot" +
        String(BOT_TOKEN) +
        "/sendMessage";

    if (!https.begin(client, url))
    {
        Serial.println("Telegram HTTPS connection failed.");
        return false;
    }

    https.addHeader(
        "Content-Type",
        "application/x-www-form-urlencoded"
    );

    String body =
        "chat_id=" + String(CHAT_ID) +
        "&text=" + message;

    int httpCode = https.POST(body);

    Serial.print("Telegram message HTTP response: ");
    Serial.println(httpCode);

    if (httpCode > 0)
        Serial.println(https.getString());

    https.end();

    return httpCode >= 200 && httpCode < 300;
}

// =====================================================
// TELEGRAM PHOTO
// =====================================================

bool sendTelegramPhoto(
    uint8_t* jpegData,
    size_t jpegLength,
    const String& caption
)
{
    if (jpegData == nullptr || jpegLength == 0)
    {
        Serial.println("Telegram ERROR: Invalid JPEG.");
        return false;
    }

    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("Telegram ERROR: Wi-Fi not connected.");
        return false;
    }

    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient https;

    String url =
        "https://api.telegram.org/bot" +
        String(BOT_TOKEN) +
        "/sendPhoto";

    Serial.println();
    Serial.println("======================================");
    Serial.println("SENDING IMAGE TO TELEGRAM");
    Serial.println("======================================");

    if (!https.begin(client, url))
    {
        Serial.println("Telegram HTTPS begin failed.");
        return false;
    }

    String boundary = "----ESP32EdgeAISecurityBoundary";

    String head =
        "--" + boundary + "\r\n"
        "Content-Disposition: form-data; name=\"chat_id\"\r\n\r\n" +
        String(CHAT_ID) + "\r\n" +
        "--" + boundary + "\r\n"
        "Content-Disposition: form-data; name=\"caption\"\r\n\r\n" +
        caption + "\r\n" +
        "--" + boundary + "\r\n"
        "Content-Disposition: form-data; name=\"photo\"; filename=\"security_event.jpg\"\r\n"
        "Content-Type: image/jpeg\r\n\r\n";

    String tail = "\r\n--" + boundary + "--\r\n";

    size_t totalLength = head.length() + jpegLength + tail.length();

    https.addHeader(
        "Content-Type",
        "multipart/form-data; boundary=" + boundary
    );

    uint8_t* body = (uint8_t*)ps_malloc(totalLength);

    if (body == nullptr)
    {
        Serial.println("ERROR: Cannot allocate Telegram PSRAM buffer.");
        https.end();
        return false;
    }

    size_t position = 0;

    memcpy(body + position, head.c_str(), head.length());
    position += head.length();

    memcpy(body + position, jpegData, jpegLength);
    position += jpegLength;

    memcpy(body + position, tail.c_str(), tail.length());

    Serial.print("JPEG size: ");
    Serial.print(jpegLength);
    Serial.println(" bytes");

    int httpCode = https.POST(body, totalLength);

    free(body);

    Serial.print("Telegram HTTP response: ");
    Serial.println(httpCode);

    if (httpCode > 0)
    {
        Serial.println("Telegram response:");
        Serial.println(https.getString());
    }

    https.end();

    if (httpCode >= 200 && httpCode < 300)
    {
        Serial.println("TELEGRAM IMAGE SENT SUCCESSFULLY.");
        return true;
    }

    Serial.println("TELEGRAM IMAGE SEND FAILED.");
    return false;
}

// =====================================================
// RGB565 -> TFLITE INPUT
// =====================================================

void fillInputFromCamera(
    camera_fb_t* fb,
    TfLiteTensor* targetInput
)
{
    if (!fb || !targetInput)
        return;

    int srcWidth = fb->width;
    int srcHeight = fb->height;
    uint8_t* buffer = fb->buf;

    for (int y = 0; y < IMG_HEIGHT; y++)
    {
        int srcY = (y * srcHeight) / IMG_HEIGHT;

        for (int x = 0; x < IMG_WIDTH; x++)
        {
            int srcX = (x * srcWidth) / IMG_WIDTH;
            int srcIndex = (srcY * srcWidth + srcX) * 2;

            uint16_t pixel =
                ((uint16_t)buffer[srcIndex] << 8) |
                buffer[srcIndex + 1];

            uint8_t r = ((pixel >> 11) & 0x1F) * 255 / 31;
            uint8_t g = ((pixel >> 5) & 0x3F) * 255 / 63;
            uint8_t b = (pixel & 0x1F) * 255 / 31;

            int index = (y * IMG_WIDTH + x) * 3;

            if (targetInput->type == kTfLiteFloat32)
            {
                targetInput->data.f[index + 0] = r / 255.0f;
                targetInput->data.f[index + 1] = g / 255.0f;
                targetInput->data.f[index + 2] = b / 255.0f;
            }
            else if (targetInput->type == kTfLiteInt8)
            {
                // Both current INT8 models use scale=1 and zero_point=-128.
                // Therefore real [0,255] maps to int8 [-128,127].
                targetInput->data.int8[index + 0] = (int8_t)(r - 128);
                targetInput->data.int8[index + 1] = (int8_t)(g - 128);
                targetInput->data.int8[index + 2] = (int8_t)(b - 128);
            }
        }
    }
}

// =====================================================
// HUMAN/NON-HUMAN INFERENCE
// =====================================================

float getHumanProbability()
{
    if (human_interpreter->Invoke() != kTfLiteOk)
    {
        Serial.println("ERROR: Human Invoke() failed.");
        return -1.0f;
    }

    float probability = 0.0f;

    if (human_output->type == kTfLiteFloat32)
    {
        probability = human_output->data.f[0];
    }
    else if (human_output->type == kTfLiteInt8)
    {
        probability =
            (human_output->data.int8[0] - human_output->params.zero_point) *
            human_output->params.scale;
    }
    else
    {
        Serial.println("ERROR: Unsupported human output type.");
        return -1.0f;
    }

    if (probability < 0.0f) probability = 0.0f;
    if (probability > 1.0f) probability = 1.0f;

    return probability;
}

// =====================================================
// ACTIVITY INFERENCE
// =====================================================

int getActivityPrediction(float* bestScoreOut = nullptr)
{
    if (activity_interpreter->Invoke() != kTfLiteOk)
    {
        Serial.println("ERROR: Activity Invoke() failed.");
        return -1;
    }

    int bestClass = 0;
    float bestScore = -1000.0f;

    for (int i = 0; i < 4; i++)
    {
        float score = 0.0f;

        if (activity_output->type == kTfLiteFloat32)
        {
            score = activity_output->data.f[i];
        }
        else if (activity_output->type == kTfLiteInt8)
        {
            score =
                (activity_output->data.int8[i] - activity_output->params.zero_point) *
                activity_output->params.scale;
        }
        else
        {
            Serial.println("ERROR: Unsupported activity output type.");
            return -1;
        }

        Serial.print(activityLabels[i]);
        Serial.print(": ");
        Serial.print(score * 100.0f, 2);
        Serial.println("%");

        if (score > bestScore)
        {
            bestScore = score;
            bestClass = i;
        }
    }

    if (bestScoreOut)
        *bestScoreOut = bestScore;

    Serial.print("Activity prediction: ");
    Serial.println(activityLabels[bestClass]);

    return bestClass;
}

// =====================================================
// SAVE BEST FRAME AS JPEG
// =====================================================

bool saveBestFrame(
    camera_fb_t* fb,
    float probability
)
{
    if (!fb)
        return false;

    uint8_t* jpgBuffer = nullptr;
    size_t jpgLength = 0;

    bool converted = frame2jpg(
        fb,
        12,
        &jpgBuffer,
        &jpgLength
    );

    if (!converted || jpgBuffer == nullptr || jpgLength == 0)
    {
        Serial.println("JPEG conversion failed.");
        return false;
    }

    if (bestJpeg != nullptr)
    {
        free(bestJpeg);
        bestJpeg = nullptr;
        bestJpegLength = 0;
    }

    bestJpeg = (uint8_t*)ps_malloc(jpgLength);

    if (bestJpeg == nullptr)
    {
        Serial.println("ERROR: Cannot allocate best JPEG.");
        free(jpgBuffer);
        return false;
    }

    memcpy(bestJpeg, jpgBuffer, jpgLength);
    bestJpegLength = jpgLength;
    bestFrameScore = probability;
    bestFrameHuman = probability >= HUMAN_THRESHOLD;

    free(jpgBuffer);
    return true;
}

// =====================================================
// CLEAR BEST FRAME
// =====================================================

void clearBestFrame()
{
    if (bestJpeg != nullptr)
    {
        free(bestJpeg);
        bestJpeg = nullptr;
    }

    bestJpegLength = 0;
    bestFrameScore = -1.0f;
    bestFrameHuman = false;
}

// =====================================================
// FIVE-FRAME HUMAN + ACTIVITY INFERENCE
// =====================================================
// Activity inference is performed only on frames that the
// human model classifies as HUMAN. Activity votes are then
// combined across those human frames.

bool runFiveFrameInference(int& finalActivity, float& activityConfidence)
{
    int humanVotes = 0;
    int nonHumanVotes = 0;

    int activityVotes[4] = {0, 0, 0, 0};
    float activityScoreSum[4] = {0, 0, 0, 0};
    int activityFrameCount = 0;

    finalActivity = -1;
    activityConfidence = 0.0f;

    clearBestFrame();

    Serial.println();
    Serial.println("======================================");
    Serial.println("CAPTURING 5 FRAMES");
    Serial.println("======================================");

    for (int frame = 0; frame < NUM_FRAMES; frame++)
    {
        Serial.print("Frame ");
        Serial.print(frame + 1);
        Serial.println("/5");

        camera_fb_t* fb = esp_camera_fb_get();

        if (!fb)
        {
            Serial.println("Camera capture FAILED.");
            continue;
        }

        // -------------------------------
        // HUMAN / NON-HUMAN
        // -------------------------------

        fillInputFromCamera(fb, human_input);

        float probability = getHumanProbability();

        if (probability < 0.0f)
        {
            esp_camera_fb_return(fb);
            continue;
        }

        bool human = probability >= HUMAN_THRESHOLD;

        Serial.print("Human probability: ");
        Serial.print(probability * 100.0f, 2);
        Serial.println("%");

        if (human)
        {
            Serial.println("Prediction: HUMAN");
            humanVotes++;
        }
        else
        {
            Serial.println("Prediction: NON-HUMAN");
            nonHumanVotes++;
        }

        // -------------------------------
        // Keep strongest image
        // -------------------------------

        bool shouldSave = false;

        if (bestJpeg == nullptr)
        {
            shouldSave = true;
        }
        else if (human && !bestFrameHuman)
        {
            shouldSave = true;
        }
        else if (human == bestFrameHuman && probability > bestFrameScore)
        {
            shouldSave = true;
        }

        if (shouldSave)
        {
            saveBestFrame(fb, probability);
        }

        // -------------------------------
        // ACTIVITY MODEL
        // -------------------------------

        if (human)
        {
            fillInputFromCamera(fb, activity_input);

            float frameActivityScore = 0.0f;
            int activity = getActivityPrediction(&frameActivityScore);

            if (activity >= 0 && activity < 4)
            {
                activityVotes[activity]++;
                activityScoreSum[activity] += frameActivityScore;
                activityFrameCount++;
            }
        }

        esp_camera_fb_return(fb);

        delay(100);
    }

    // =================================================
    // HUMAN MAJORITY VOTE
    // =================================================

    bool finalHuman = humanVotes >= nonHumanVotes;

    Serial.println();
    Serial.println("======================================");
    Serial.println("5-FRAME EDGE AI RESULT");
    Serial.println("======================================");

    Serial.print("HUMAN votes: ");
    Serial.println(humanVotes);

    Serial.print("NON-HUMAN votes: ");
    Serial.println(nonHumanVotes);

    Serial.print("FINAL RESULT: ");
    Serial.println(finalHuman ? "HUMAN" : "NON-HUMAN");

    // =================================================
    // ACTIVITY MAJORITY VOTE
    // =================================================

    if (finalHuman && activityFrameCount > 0)
    {
        int bestActivity = 0;

        for (int i = 1; i < 4; i++)
        {
            if (activityVotes[i] > activityVotes[bestActivity])
            {
                bestActivity = i;
            }
        }

        finalActivity = bestActivity;

        if (activityVotes[bestActivity] > 0)
        {
            activityConfidence =
                activityScoreSum[bestActivity] /
                (float)activityVotes[bestActivity];
        }

        Serial.println();
        Serial.println("======================================");
        Serial.println("ACTIVITY CLASSIFICATION RESULT");
        Serial.println("======================================");

        for (int i = 0; i < 4; i++)
        {
            Serial.print(activityLabels[i]);
            Serial.print(" votes: ");
            Serial.println(activityVotes[i]);
        }

        Serial.print("FINAL ACTIVITY: ");
        Serial.println(activityLabels[finalActivity]);

        Serial.print("ACTIVITY CONFIDENCE: ");
        Serial.print(activityConfidence * 100.0f, 2);
        Serial.println("%");
    }
    else
    {
        Serial.println();
        Serial.println("Activity classification skipped because final result was NON-HUMAN or no valid human activity frame was available.");
    }

    return finalHuman;
}

// =====================================================
// SEND SECURITY ALERT
// =====================================================

void sendSecurityAlert(
    bool human,
    int activity,
    float activityConfidence
)
{
    if (bestJpeg == nullptr || bestJpegLength == 0)
    {
        Serial.println("No image available for Telegram.");
        return;
    }

    String caption;

    if (human)
    {
        caption =
            "🚨 HUMAN DETECTED\n\n"
            "Edge AI verification: HUMAN\n"
            "5-frame majority voting: CONFIRMED\n";

        if (activity >= 0 && activity < 4)
        {
            caption += "Activity: ";
            caption += activityLabels[activity];
            caption += "\n";

            caption += "Activity confidence: ";
            caption += String(activityConfidence * 100.0f, 1);
            caption += "%\n";
        }
        else
        {
            caption += "Activity: NOT AVAILABLE\n";
        }
    }
    else
    {
        caption =
            "ℹ️ NON-HUMAN DETECTED\n\n"
            "Edge AI verification: NON-HUMAN\n"
            "5-frame majority voting: CONFIRMED\n"
            "Activity: NOT APPLICABLE\n";
    }

    caption += "Best frame confidence: ";
    caption += String(bestFrameScore * 100.0f, 1);
    caption += "%\n";
    caption += "ESP32-S3 Edge AI Security System";

    sendTelegramPhoto(
        bestJpeg,
        bestJpegLength,
        caption
    );
}

// =====================================================
// INITIALIZE HUMAN/NON-HUMAN AI
// =====================================================

bool initHumanAI()
{
    Serial.println();
    Serial.println("Initializing HUMAN/NON-HUMAN Edge AI...");

    human_model = tflite::GetModel(g_human_nonhuman_int8_model);

    if (human_model->version() != TFLITE_SCHEMA_VERSION)
    {
        Serial.println("ERROR: Human model schema mismatch.");
        return false;
    }

    human_tensor_arena =
        (uint8_t*)ps_malloc(HUMAN_TENSOR_ARENA_SIZE);

    if (!human_tensor_arena)
    {
        Serial.println("ERROR: Human tensor arena allocation failed.");
        return false;
    }

    Serial.println("Human tensor arena allocated.");
    Serial.println("Human tensor arena size: 6 MB");

    static tflite::MicroMutableOpResolver<8> humanResolver;

    humanResolver.AddConv2D();
    humanResolver.AddMaxPool2D();
    humanResolver.AddMean();
    humanResolver.AddFullyConnected();
    humanResolver.AddLogistic();

    static tflite::MicroInterpreter humanStaticInterpreter(
        human_model,
        humanResolver,
        human_tensor_arena,
        HUMAN_TENSOR_ARENA_SIZE
    );

    human_interpreter = &humanStaticInterpreter;

    Serial.println("Calling human AllocateTensors()...");

    if (human_interpreter->AllocateTensors() != kTfLiteOk)
    {
        Serial.println("ERROR: Human AllocateTensors() FAILED.");
        return false;
    }

    human_input = human_interpreter->input(0);
    human_output = human_interpreter->output(0);

    Serial.println("Human AllocateTensors() SUCCESS.");
    Serial.print("Human input shape: ");
    Serial.print(human_input->dims->data[0]);
    Serial.print(" x ");
    Serial.print(human_input->dims->data[1]);
    Serial.print(" x ");
    Serial.print(human_input->dims->data[2]);
    Serial.print(" x ");
    Serial.println(human_input->dims->data[3]);

    Serial.print("Human input type: ");
    Serial.println(human_input->type);

    Serial.print("Human output type: ");
    Serial.println(human_output->type);

    Serial.print("Human output scale: ");
    Serial.println(human_output->params.scale, 6);

    Serial.print("Human output zero point: ");
    Serial.println(human_output->params.zero_point);

    return true;
}

// =====================================================
// INITIALIZE ACTIVITY AI
// =====================================================

bool initActivityAI()
{
    Serial.println();
    Serial.println("Initializing ACTIVITY Edge AI...");

    activity_model = tflite::GetModel(g_activity_int8_model);

    if (activity_model->version() != TFLITE_SCHEMA_VERSION)
    {
        Serial.println("ERROR: Activity model schema mismatch.");
        return false;
    }

    activity_tensor_arena =
        (uint8_t*)ps_malloc(ACTIVITY_TENSOR_ARENA_SIZE);

    if (!activity_tensor_arena)
    {
        Serial.println("ERROR: Activity tensor arena allocation failed.");
        return false;
    }

    Serial.print("Activity tensor arena allocated: ");
    Serial.print(ACTIVITY_TENSOR_ARENA_SIZE / 1024);
    Serial.println(" KB");

    static tflite::MicroMutableOpResolver<8> activityResolver;

    activityResolver.AddConv2D();
    activityResolver.AddMaxPool2D();
    activityResolver.AddMean();
    activityResolver.AddFullyConnected();
    activityResolver.AddSoftmax();

    static tflite::MicroInterpreter activityStaticInterpreter(
        activity_model,
        activityResolver,
        activity_tensor_arena,
        ACTIVITY_TENSOR_ARENA_SIZE
    );

    activity_interpreter = &activityStaticInterpreter;

    Serial.println("Calling activity AllocateTensors()...");

    if (activity_interpreter->AllocateTensors() != kTfLiteOk)
    {
        Serial.println("ERROR: Activity AllocateTensors() FAILED.");
        Serial.println("If this occurs, increase ACTIVITY_TENSOR_ARENA_SIZE to 768 KB or 1 MB.");
        return false;
    }

    activity_input = activity_interpreter->input(0);
    activity_output = activity_interpreter->output(0);

    Serial.println("Activity AllocateTensors() SUCCESS.");

    Serial.print("Activity input shape: ");
    Serial.print(activity_input->dims->data[0]);
    Serial.print(" x ");
    Serial.print(activity_input->dims->data[1]);
    Serial.print(" x ");
    Serial.print(activity_input->dims->data[2]);
    Serial.print(" x ");
    Serial.println(activity_input->dims->data[3]);

    Serial.print("Activity input type: ");
    Serial.println(activity_input->type);

    Serial.print("Activity output elements: ");
    Serial.println(activity_output->dims->data[1]);

    Serial.print("Activity output type: ");
    Serial.println(activity_output->type);

    Serial.print("Activity output scale: ");
    Serial.println(activity_output->params.scale, 6);

    Serial.print("Activity output zero point: ");
    Serial.println(activity_output->params.zero_point);

    return true;
}

// =====================================================
// SETUP
// =====================================================

void setup()
{
    Serial.begin(115200);
    delay(2000);

    Serial.println();
    Serial.println("======================================");
    Serial.println("ESP32-S3 EDGE AI SMART SECURITY");
    Serial.println("HUMAN + ACTIVITY + TELEGRAM");
    Serial.println("======================================");

    pinMode(PIR_PIN, INPUT);

    Serial.println("PIR GPIO: 21");

    Serial.print("PSRAM: ");

    if (psramFound())
    {
        Serial.println("FOUND");
        Serial.print("PSRAM size: ");
        Serial.print(ESP.getPsramSize() / 1024 / 1024);
        Serial.println(" MB");
    }
    else
    {
        Serial.println("NOT FOUND");
        while (true) delay(1000);
    }

    // Wi-Fi
    if (!connectWiFi())
    {
        Serial.println("WARNING: Telegram disabled until Wi-Fi is available.");
    }

    // Camera
    Serial.println();
    Serial.println("Initializing camera...");

    if (!initCamera())
    {
        Serial.println("STOP: Camera initialization failed.");
        while (true) delay(1000);
    }

    // Human model
    if (!initHumanAI())
    {
        Serial.println("STOP: Human AI initialization failed.");
        while (true) delay(1000);
    }

    // Activity model
    if (!initActivityAI())
    {
        Serial.println("STOP: Activity AI initialization failed.");
        while (true) delay(1000);
    }

    // Telegram startup test
    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println();
        Serial.println("Sending Telegram startup message...");

        sendTelegramMessage(
            "ESP32-S3 Edge AI Security System is online. Human + Activity AI ready."
        );
    }

    Serial.println();
    Serial.println("======================================");
    Serial.println("SYSTEM READY");
    Serial.println("======================================");
    Serial.println();
    Serial.println("Waiting for PIR motion...");
}

// =====================================================
// MAIN LOOP
// =====================================================

void loop()
{
    if (digitalRead(PIR_PIN) == HIGH)
    {
        totalEvents++;

        Serial.println();
        Serial.println();
        Serial.println("######################################");
        Serial.print("SECURITY EVENT #");
        Serial.println(totalEvents);
        Serial.println("PIR MOTION DETECTED");
        Serial.println("######################################");

        // Give the person/object a moment to enter view.
        delay(500);

        int finalActivity = -1;
        float activityConfidence = 0.0f;

        bool human = runFiveFrameInference(
            finalActivity,
            activityConfidence
        );

        Serial.println();
        Serial.println("======================================");
        Serial.println("FINAL SECURITY DECISION");
        Serial.println("======================================");

        if (human)
        {
            Serial.println("HUMAN DETECTED");

            if (finalActivity >= 0 && finalActivity < 4)
            {
                Serial.print("ACTIVITY: ");
                Serial.println(activityLabels[finalActivity]);
            }
        }
        else
        {
            Serial.println("NON-HUMAN DETECTED");
        }

        // Wi-Fi recovery if necessary.
        if (WiFi.status() != WL_CONNECTED)
        {
            Serial.println("Wi-Fi disconnected. Attempting reconnection...");
            connectWiFi();
        }

        if (WiFi.status() == WL_CONNECTED)
        {
            sendSecurityAlert(
                human,
                finalActivity,
                activityConfidence
            );
        }
        else
        {
            Serial.println("Telegram skipped: no Wi-Fi.");
        }

        clearBestFrame();

        Serial.println();
        Serial.println("Waiting for PIR reset...");

        while (digitalRead(PIR_PIN) == HIGH)
        {
            delay(100);
        }

        delay(2000);

        Serial.println("Ready for next event.");
    }

    delay(100);
}
