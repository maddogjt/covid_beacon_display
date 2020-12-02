#include <Arduino.h>

#include "oled_display.h"

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <BLEEddystoneURL.h>
#include <BLEEddystoneTLM.h>
#include <BLEBeacon.h>
#include <vector>

struct random_id
{
  uint8_t data[16];

  bool operator==(const random_id &other) const
  {
    return memcmp(data, other.data, sizeof(data)) == 0;
  }
};

struct BeaconData
{
  random_id id_;
  uint32_t firstSeen_;
  uint32_t lastSeen_;
  int32_t maxRssi_;
  int32_t minRssi_;
};

static BLEUUID kCovidTrackingUUID((uint16_t)0xFD6F);
constexpr int kScanTime = 1;           //In seconds
constexpr int kExpirationTime = 30000; // ms
static std::vector<BeaconData> gTrackingData;
static BLEScan *pBLEScan = nullptr;
static size_t gMaxBeacons = 0;

std::vector<BeaconData>::iterator findBeaconData(random_id id)
{
  return std::find_if(gTrackingData.begin(), gTrackingData.end(), [=](BeaconData other) {
    return other.id_ == id;
  });
}

class C19BeaconAdvertiseCallback : public BLEAdvertisedDeviceCallbacks
{
  void onResult(BLEAdvertisedDevice advertisedDevice)
  {
    // I was using a custom patch to the underlying library to add getServiceUUIDCount()
    // Disabling this for public repo
    // const int uuidCount = advertisedDevice.getServiceUUIDCount();

    // bool found = false;
    // if (uuidCount > 0)
    // {
    //   for (int i = 0; i < uuidCount; i++)
    //   {
    //     if (advertisedDevice.getServiceUUID(i).equals(kCovidTrackingUUID))
    //     {
    //       found = true;
    //       break;
    //     }
    //   }
    // }

    bool found = advertisedDevice.haveServiceUUID() && advertisedDevice.getServiceUUID().equals(kCovidTrackingUUID);

    // No covid beacon found, ignore this advertisement
    if (!found)
    {
      return;
    }

    if (advertisedDevice.haveServiceData())
    {
      if (advertisedDevice.getServiceDataUUID().equals(kCovidTrackingUUID))
      {
        Serial.printf("RSSI %d\n", advertisedDevice.getRSSI());
        std::string serviceDataStr = advertisedDevice.getServiceData();
        std::vector<uint8_t> serviceData(serviceDataStr.begin(), serviceDataStr.end());
        int len = serviceData.size();
        Serial.printf("Covid Data len (%d) - ", len);
        for (int i = 0; i < len; i++)
        {
          Serial.printf("%02X", serviceData[i]);
        }
        Serial.println("");

        BeaconData beacon;
        beacon.id_ = *reinterpret_cast<random_id *>(serviceData.data());
        beacon.firstSeen_ = millis();
        beacon.lastSeen_ = beacon.firstSeen_;
        beacon.maxRssi_ = advertisedDevice.getRSSI();
        beacon.minRssi_ = beacon.maxRssi_;

        auto current = findBeaconData(beacon.id_);
        if (current != gTrackingData.end())
        {
          // Serial.printf("refreshing at %d\n", beacon.lastSeen_);
          current->lastSeen_ = beacon.lastSeen_;
          current->maxRssi_ = max(current->maxRssi_, beacon.maxRssi_);
          current->minRssi_ = min(current->minRssi_, beacon.minRssi_);
        }
        else
        {
          // Serial.printf("adding new at %d\n", beacon.lastSeen_);
          gTrackingData.push_back(beacon);
        }
      }
    }
  }
};

void setup()
{
  Serial.begin(115200);
  Serial.println("Scanning...");

  oled_setup();

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new C19BeaconAdvertiseCallback());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99); // less or equal setInterval value
}

void clear_expired_advertisemsnts()
{
  auto now = millis();

  auto iter = gTrackingData.begin();
  while (iter != gTrackingData.end())
  {
    if (now - iter->lastSeen_ > kExpirationTime)
    {
      Serial.printf("expiring at %d\n", now);
      iter = gTrackingData.erase(iter);
    }
    else
    {
      iter++;
    }
  }
};

void loop()
{
  // put your main code here, to run repeatedly:
  BLEScanResults foundDevices = pBLEScan->start(kScanTime, false);
  // Serial.print("Devices found: ");
  // Serial.println(foundDevices.getCount());
  pBLEScan->clearResults(); // delete results fromBLEScan buffer to release memory

  clear_expired_advertisemsnts();
  delay(200);

  gMaxBeacons = max(gMaxBeacons, gTrackingData.size());

  gDisplay.clearDisplay();

  gDisplay.setTextSize(2);
  gDisplay.setTextColor(WHITE);
  gDisplay.setCursor(0, 10);
  // Display static text
  gDisplay.printf("Beacons %d\n", gTrackingData.size());
  gDisplay.printf("Max %d\n", gMaxBeacons);
  gDisplay.display();
}
