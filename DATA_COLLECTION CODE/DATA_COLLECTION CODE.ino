#include <SD.h>
#include <SPI.h>

#define SD_CS_PIN 5
#define MIC_PIN 34
#define LED_PIN 2
#define SAMPLE_RATE 16000
#define RECORD_DURATION 100 // in seconds

File audioFile;

void setup() {
  pinMode(MIC_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200);

  // Initialize SD card
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("SD Card initialization failed!");
    while (1);
  }
  Serial.println("SD Card initialized.");

  // Open file for recording
  char fileName[] = "/recording.wav";
  audioFile = SD.open(fileName, FILE_WRITE);
  if (!audioFile) {
    Serial.println("Failed to open file for writing!");
    while (1);
  }
  Serial.println("File created: recording.wav");

  // Write WAV header placeholder
  writeWavHeader(audioFile, SAMPLE_RATE, SAMPLE_RATE * RECORD_DURATION);
  Serial.println("WAV header written.");
}

void loop() {
  // Start recording
  Serial.println("Starting recording...");
  digitalWrite(LED_PIN, HIGH);

  uint32_t startTime = millis();
  uint32_t samplesWritten = 0;

  while (millis() - startTime < RECORD_DURATION * 10000) {
    int16_t sample = analogRead(MIC_PIN) - 2048; // Center ADC values around 0
    audioFile.write((byte*)&sample, 2);          // Write sample as 16-bit PCM
    samplesWritten++;
  }

  digitalWrite(LED_PIN, LOW);
  Serial.println("Recording finished.");

  // Update WAV header with actual sample count
  audioFile.seek(0);
  writeWavHeader(audioFile, SAMPLE_RATE, samplesWritten);

  audioFile.close();
  Serial.println("File saved and closed.");
  while (1); // Stop further execution
}

void writeWavHeader(File file, uint32_t sampleRate, uint32_t totalSampleCount) {
  uint32_t dataChunkSize = totalSampleCount * 2; // 16-bit samples
  uint32_t fileSize = 36 + dataChunkSize;

  // RIFF header
  file.write((const uint8_t*)"RIFF", 4);
  file.write((byte*)&fileSize, 4);
  file.write((const uint8_t*)"WAVE", 4);

  // fmt subchunk
  file.write((const uint8_t*)"fmt ", 4);
  uint32_t subchunk1Size = 16;
  uint16_t audioFormat = 1; // PCM
  uint16_t numChannels = 1; // Mono
  uint16_t bitsPerSample = 16;
  uint32_t byteRate = sampleRate * numChannels * bitsPerSample / 8;
  uint16_t blockAlign = numChannels * bitsPerSample / 8;

  file.write((byte*)&subchunk1Size, 4);
  file.write((byte*)&audioFormat, 2);
  file.write((byte*)&numChannels, 2);
  file.write((byte*)&sampleRate, 4);
  file.write((byte*)&byteRate, 4);
  file.write((byte*)&blockAlign, 2);
  file.write((byte*)&bitsPerSample, 2);

  // data subchunk
  file.write((const uint8_t*)"data", 4);
  file.write((byte*)&dataChunkSize, 4);
}
