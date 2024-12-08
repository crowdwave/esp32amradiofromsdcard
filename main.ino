#include <SD_MMC.h>
#include <FS.h>
#include <soc/rtc.h>
#include "driver/i2s.h"
#include "sample.h"

// Configuration
const bool ENABLE_SAMPLE_PLAYBACK = false;  // Set to true to enable sample.h playback

// Constants
const int BUFFER_SIZE = 1024;  // I2S buffer size
const int CHUNK_SIZE = 32768;  // 32KB chunks
const unsigned long CARRIER_FREQ = 835000;  // Carrier frequency in Hz
const int SAMPLES_PER_CARRIER = 16;  // Number of carrier samples per PCM sample

// PCM file data structure (matches sample.h format)
const signed char* pcm_samples = nullptr;
unsigned int pcm_sampleCount = 0;
unsigned long pcm_sampleRate = 44100;  // Will be updated per file

// Mode selection
enum PlaybackMode {
    SD_PLAYBACK,
    SAMPLE_PLAYBACK
};
PlaybackMode currentMode = SD_PLAYBACK;  // Changed default to SD_PLAYBACK

// I2S Configuration
static const i2s_port_t i2s_num = (i2s_port_t)I2S_NUM_0;
static const i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN),
    .sample_rate = 1000000,
    .bits_per_sample = (i2s_bits_per_sample_t)I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,
    .communication_format = I2S_COMM_FORMAT_STAND_MSB,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 2,
    .dma_buf_len = BUFFER_SIZE
};

// Global variables
File audioFile;
File root;
bool sdInitialized = false;
short buff[BUFFER_SIZE];
int sintab[] = {0, 48, 90, 117, 127, 117, 90, 48, 0, -48, -90, -117, -127, -117, -90, -48};
unsigned long long pos = 0;
unsigned int posLow = 0;
unsigned long long posInc = ((unsigned long long)sampleRate << 32) / 835000;
uint8_t* fileBuffer = nullptr;
size_t fileSize = 0;
size_t bytesRead = 0;

void instructions() {
    Serial.println("------ File Preparation Instructions ------");
    Serial.println("This guide will help you convert your .wav audio files to .pcm format.");
    Serial.println("");
    Serial.println("Supported Sample Rates (must be in filename):");
    Serial.println("- 8000 Hz  (examples: music_8000hz.pcm, voice_8k.pcm)");
    Serial.println("- 11025 Hz (example: music_11025hz.pcm)");
    Serial.println("- 22050 Hz (examples: music_22050hz.pcm, voice_22k.pcm)");
    Serial.println("- 44100 Hz (examples: music_44100hz.pcm, voice_44k.pcm)");
    Serial.println("- 48000 Hz (examples: music_48000hz.pcm, voice_48k.pcm)");
    Serial.println("");
    Serial.println("Batch Conversion (Convert Multiple Files):");
    Serial.println("1. Open a terminal or command prompt.");
    Serial.println("2. Navigate to the folder containing your .wav files.");
    Serial.println("3. Run the following command:");
    Serial.println("   for f in *.wav; do \\");
    Serial.println("     rate=$(ffprobe -v error -select_streams a:0 -show_entries stream=sample_rate -of default=nw=1:nk=1 \"$f\"); \\");
    Serial.println("     ffmpeg -i \"$f\" -f s8 -acodec pcm_s8 -ac 1 \"${f%.*}_${rate}hz.pcm\"; \\");
    Serial.println("   done");
    Serial.println("");
    Serial.println("This will:");
    Serial.println("- Convert each .wav file in the folder to .pcm format");
    Serial.println("- Save the new file with the sample rate in the filename");
    Serial.println("- Examples: 'music_44100hz.pcm', 'voice_22k.pcm'");
    Serial.println("");
    Serial.println("Single File Conversion:");
    Serial.println("If you want to convert a single file, use:");
    Serial.println("   ffmpeg -i input.wav -f s8 -acodec pcm_s8 -ac 1 output_44100hz.pcm");
    Serial.println("");
    Serial.println("Important Notes:");
    Serial.println("- Install ffmpeg and ffprobe before running commands");
    Serial.println("- Sample rate MUST be in the filename (e.g., '_44100hz' or '_44k')");
    Serial.println("- Files without sample rate in name will default to 22050 Hz");
    Serial.println("- Only .pcm files will be played");
    Serial.println("- Hidden files (starting with .) are ignored");
    Serial.println("------------------------------------------");
}

void listDir(File dir, int numTabs = 0) {
    while (true) {
        File entry = dir.openNextFile();
        if (!entry) {
            break;
        }
        String filename = String(entry.name());
        // Skip hidden files and show only .pcm files
        if (!filename.startsWith(".") && 
            (!entry.isDirectory() && filename.endsWith(".pcm"))) {
            for (uint8_t i = 0; i < numTabs; i++) {
                Serial.print('\t');
            }
            Serial.print(entry.name());
            Serial.print("\t\t");
            Serial.println(entry.size(), DEC);
        }
        entry.close();
    }
}

unsigned long getSampleRateFromFilename(const char* filename) {
    String fname = String(filename);
    // Look for common sample rates in filename
    if (fname.indexOf("8000") >= 0) return 8000;
    if (fname.indexOf("8k") >= 0) return 8000;
    if (fname.indexOf("11025") >= 0) return 11025;
    if (fname.indexOf("22050") >= 0) return 22050;
    if (fname.indexOf("22k") >= 0) return 22050;
    if (fname.indexOf("44100") >= 0) return 44100;
    if (fname.indexOf("44k") >= 0) return 44100;
    if (fname.indexOf("48000") >= 0) return 48000;
    if (fname.indexOf("48k") >= 0) return 48000;
    
    return 22050;  // Default if not found
}

void loadPCMFile() {
    if (!sdInitialized) {
        if (!SD_MMC.begin()) {
            Serial.println("SD_MMC initialization failed in 4-bit mode, trying 1-bit mode...");
            if (!SD_MMC.begin("/sdcard", true)) {
                Serial.println("SD_MMC initialization failed!");
                return;
            }
        }
        root = SD_MMC.open("/");
        if (!root) {
            Serial.println("Failed to open root directory!");
            return;
        }
        sdInitialized = true;
    }

    // If we have a file open but haven't finished reading it
    if (audioFile && bytesRead < fileSize) {
        size_t remaining = fileSize - bytesRead;
        size_t toRead = min(remaining, (size_t)CHUNK_SIZE);
        
        size_t bytesThisRead = audioFile.read(fileBuffer, toRead);
        if (bytesThisRead > 0) {
            pcm_samples = (const signed char*)fileBuffer;
            pcm_sampleCount = bytesThisRead;
            bytesRead += bytesThisRead;
            Serial.printf("Loaded next chunk: %d bytes, total read: %d/%d\n", 
                bytesThisRead, bytesRead, fileSize);
            return;
        }
    }

    // If we get here, we need to open a new file
    if (audioFile) audioFile.close();
    if (fileBuffer) {
        free(fileBuffer);
        fileBuffer = nullptr;
    }

    audioFile = root.openNextFile();
    while (audioFile) {
        String filename = String(audioFile.name());
        if (!audioFile.isDirectory() && !filename.startsWith(".") && filename.endsWith(".pcm")) {
            fileSize = audioFile.size();
            bytesRead = 0;
            
            fileBuffer = (uint8_t*)malloc(CHUNK_SIZE);
            if (fileBuffer) {
                size_t toRead = min(fileSize, (size_t)CHUNK_SIZE);
                size_t bytesThisRead = audioFile.read(fileBuffer, toRead);
                if (bytesThisRead > 0) {
                    pcm_samples = (const signed char*)fileBuffer;
                    pcm_sampleCount = bytesThisRead;
                    bytesRead = bytesThisRead;
                    pcm_sampleRate = getSampleRateFromFilename(filename.c_str());
                    posInc = ((unsigned long long)pcm_sampleRate << 32) / 835000;
                    Serial.printf("Opened PCM file: %s, size: %d, first chunk: %d bytes\n", 
                        filename.c_str(), fileSize, bytesThisRead);
                    return;
                }
                free(fileBuffer);
                fileBuffer = nullptr;
            }
        }
        audioFile.close();
        audioFile = root.openNextFile();
    }
    root.rewindDirectory();
}

void switchPlaybackMode() {
    if (currentMode == SD_PLAYBACK) {
        if (ENABLE_SAMPLE_PLAYBACK) {
            currentMode = SAMPLE_PLAYBACK;
            Serial.println("Switching to sample.h playback");
            pos = 0;
            posLow = 0;
            posInc = ((unsigned long long)sampleRate << 32) / 835000;
        } else {
            loadPCMFile();  // Just load the next file instead
        }
    } else {
        currentMode = SD_PLAYBACK;
        Serial.println("Switching to PCM file playback");
        pos = 0;
        posLow = 0;
        loadPCMFile();
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("\nInitializing...");
    
    // Display instructions
    instructions();
    
    if (!SD_MMC.begin()) {
        Serial.println("SD_MMC initialization failed in 4-bit mode, trying 1-bit mode...");
        if (!SD_MMC.begin("/sdcard", true)) {
            Serial.println("SD_MMC initialization failed!");
            return;
        }
    }
    Serial.println("SD_MMC initialized successfully.");
    
    // List PCM files
    Serial.println("\nAvailable PCM Files:");
    Serial.println("--------------------------------");
    Serial.println("Filename\t\tSize (bytes)");
    Serial.println("--------------------------------");
    root = SD_MMC.open("/");
    listDir(root);
    Serial.println("--------------------------------\n");
    root.rewindDirectory();

    setCpuFrequencyMhz(240);
    i2s_driver_install(i2s_num, &i2s_config, 0, NULL);
    i2s_set_pin(i2s_num, NULL);
    i2s_set_dac_mode(I2S_DAC_CHANNEL_RIGHT_EN);
    i2s_set_sample_rates(i2s_num, 1000000);

    SET_PERI_REG_BITS(I2S_CLKM_CONF_REG(0), I2S_CLKM_DIV_A_V, 1, I2S_CLKM_DIV_A_S);
    SET_PERI_REG_BITS(I2S_CLKM_CONF_REG(0), I2S_CLKM_DIV_B_V, 1, I2S_CLKM_DIV_B_S);
    SET_PERI_REG_BITS(I2S_CLKM_CONF_REG(0), I2S_CLKM_DIV_NUM_V, 2, I2S_CLKM_DIV_NUM_S);
    SET_PERI_REG_BITS(I2S_SAMPLE_RATE_CONF_REG(0), I2S_TX_BCK_DIV_NUM_V, 2, I2S_TX_BCK_DIV_NUM_S);
}

void loop() {
    const signed char* currentSamples = (currentMode == SAMPLE_PLAYBACK) ? samples : pcm_samples;
    const unsigned int currentSampleCount = (currentMode == SAMPLE_PLAYBACK) ? sampleCount : pcm_sampleCount;

    if (currentSamples == nullptr || currentSampleCount == 0) {
        switchPlaybackMode();
        return;
    }

    for (int i = 0; i < BUFFER_SIZE; i += 16) {
        if (posLow >= currentSampleCount) {
            if (currentMode == SD_PLAYBACK && bytesRead < fileSize) {
                // If we're playing a file and there's more to read, load next chunk
                pos = 0;
                posLow = 0;
                loadPCMFile();
            } else {
                // Only switch modes if we're done with the whole file
                switchPlaybackMode();
            }
            return;
        }
        
        int s = currentSamples[posLow] + 128;
        
        for (int j = 0; j < 16; j += 4) {
            buff[i + j + 1] = (sintab[j + 0] * s + 0x8000);
            buff[i + j + 0] = (sintab[j + 1] * s + 0x8000);
            buff[i + j + 3] = (sintab[j + 2] * s + 0x8000);
            buff[i + j + 2] = (sintab[j + 3] * s + 0x8000);
        }
        
        pos += posInc;
        posLow = pos >> 32;
        if (posLow >= currentSampleCount) {
            if (currentMode == SD_PLAYBACK && bytesRead < fileSize) {
                // If we're playing a file and there's more to read, load next chunk
                pos = 0;
                posLow = 0;
                loadPCMFile();
            } else {
                // Only switch modes if we're done with the whole file
                switchPlaybackMode();
            }
            return;
        }
    }

    size_t bytes_written;
    i2s_write(i2s_num, (char*)buff, sizeof(buff), &bytes_written, portMAX_DELAY);
}
