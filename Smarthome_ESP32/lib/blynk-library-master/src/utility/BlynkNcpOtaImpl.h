
// NOTE: This requires ArduinoOTA library

#if   defined(__AVR__) && (FLASHEND >= 0xFFFF)
#include "InternalStorageAVR.h"
#elif defined(ARDUINO_ARCH_STM32)
#include <InternalStorageSTM32.h>
#elif defined(ARDUINO_ARCH_RP2040) && !defined(__MBED__)
#include <InternalStorageRP2.h>
#elif defined(ESP8266) || defined(ESP32)
#include "InternalStorageESP.h"
#elif defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_NRF5)
#include "InternalStorage.h"
#else
  #warning "Blynk.Air: MCU OTA update not implemented for this platform"

  class InternalStorageClass {
  public:
    virtual int open(int length) { return false; }
    virtual size_t write(uint8_t) { return 0; }
    virtual void close() {}
    virtual void clear() {}
    virtual void apply() {}
    virtual long maxSize() { return 0; }
  };
  static InternalStorageClass InternalStorage;
#endif

bool flagApplyOtaUpdate = false;

static struct {
  uint32_t size;
  uint32_t offset;
  uint32_t crc32;
} ota_info;

uint32_t calcCRC32(const void* data, size_t length, uint32_t previousCrc32 = 0)
{
  // NOTE: normal representation for Polynomial is 04C11DB7
  const uint32_t Polynomial = 0xEDB88320; // reversed representation
  uint32_t crc = ~previousCrc32;
  const uint8_t* ptr = (const uint8_t*)data;
  while (length--) {
    crc ^= *ptr++;
    for (unsigned int j = 0; j < 8; j++) {
      crc = (crc >> 1) ^ (-int(crc & 1) & Polynomial);
    }
  }
  return ~crc;
}


bool rpc_client_otaUpdateAvailable_impl(const char* filename, uint32_t filesize, const char* fw_type, const char* fw_ver, const char* fw_build)
{
  const uint32_t maxSize = InternalStorage.maxSize();
  if (!maxSize) {
    BLYNK_LOG("OTA is not supported");
    return false;
  }

  uint32_t percentSize = ((filesize * 100) / maxSize);
  BLYNK_LOG("OTA update: %s size: %d (%d%%), type: %s, version: %s, build: %s",
      filename, filesize, percentSize, fw_type, fw_ver, fw_build);

  if (filesize == 0 || filesize > maxSize) {
    BLYNK_LOG("File size is invalid");
    return false;
  }

  // Prepare for OTA update
  if (InternalStorage.open(filesize)) {
    BLYNK_LOG("Starting OTA");
    ota_info.size   = filesize;
    ota_info.offset = 0;
    ota_info.crc32  = 0;
    // Request the update from NCP
    rpc_blynk_otaUpdateStart(1024);
    return true;
  } else {
    BLYNK_LOG("Starting OTA failed");
  }

  return false;
}

bool rpc_client_otaUpdateWrite_impl(uint32_t offset, buffer_t chunk, uint32_t crc32)
{
  bool crcOK = (calcCRC32(chunk.data, chunk.length) == crc32);

  BLYNK_LOG("OTA chunk @% 6x, size: %d, crc: %08x => %s",
      offset, chunk.length, crc32, crcOK ? "OK" : "fail!");

  if (!crcOK) { return false; }
  if (ota_info.offset != offset) {
    BLYNK_LOG("Offset mismatch");
    return false;
  }

  // Store the update
  const uint8_t* data = chunk.data;
  for (unsigned i = 0; i < chunk.length; i++) {
    if (InternalStorage.write(data[i]) == false) {
      return false;
    }
  }

  // Compute cumulative CRC32
  ota_info.crc32  = calcCRC32(chunk.data, chunk.length, ota_info.crc32);
  ota_info.offset += chunk.length;

  return true;
}

bool rpc_client_otaUpdateFinish_impl()
{
  if (ota_info.offset != ota_info.size) {
    BLYNK_LOG("File size mismatch");
    return false;
  }

  buffer_t digest;
  if (rpc_blynk_otaUpdateGetMD5(&digest)) {
    //LOG_HEX("Expected MD5:    ", digest.data, digest.length);
  }
  if (rpc_blynk_otaUpdateGetSHA256(&digest)) {
    //LOG_HEX("Expected SHA256: ", digest.data, digest.length);
  }

  uint32_t expectedCRC32 = 0;
  if (!rpc_blynk_otaUpdateGetCRC32(&expectedCRC32)) {
    BLYNK_LOG("Cannot get CRC32");
    return false;
  }
  if (expectedCRC32 != ota_info.crc32) {
    BLYNK_LOG("CRC32 check failed (expected: %08x, actual: %08x)",
        expectedCRC32, ota_info.crc32);
    return false;
  }

  BLYNK_LOG("CRC32 verified: %08x", ota_info.crc32);

  // Apply the update and restart
  InternalStorage.close();
  flagApplyOtaUpdate = true;

  return true;
}

void rpc_client_otaUpdateCancel_impl() {
  BLYNK_LOG("OTA canceled");

  InternalStorage.close();
}

void ota_apply_if_needed() {
  if (flagApplyOtaUpdate) {
    flagApplyOtaUpdate = false;

    BLYNK_LOG("Applying the update");
    // Apply the update and restart
    InternalStorage.apply();
  }
}
