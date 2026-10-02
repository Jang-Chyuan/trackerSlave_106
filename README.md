# Tracker V2 Slave

由 `C:\Dog_gps\dogLora_Slave_6` 移植至 Heltec Wireless Tracker V2。
預設只啟用供電／GPS 驗證，請依下表逐階燒錄。

| 環境 | 功能 | 通過條件 |
|---|---|---|
| `tracker_v2_gps` | UC6580、電池 ADC；PA、背光、BMI270、網路停用 | NMEA checksumOK 持續增加；戶外 fix=1、age 保持新鮮；電池電壓與電表相符 |
| `tracker_v2_lora` | 加入 SX1262／KCT8103L、原 TDMA | 收到 Master SYNC、mode=SYNCED；TX OK 且 Master 收到正確 TYPE=3 封包 |
| `tracker_v2_tft` | 加入 ST7735S 160×80 TFT | 開機文字與傳送資料可讀；PRG 喚醒；120 秒後只關背光，GPS、LoRa 繼續 |
| `tracker_v2_full` | 加入外接 BMI270、原 WiFi／MQTT／OTA 服務 | BMI270 initialized；約 1000 個有效樣本後產生活動分數，Master 收到 activityValid |

`TX OK` 只代表本機完成傳送，不代表對端已收到。第一階段的 UART initialized 也不代表 GPS 已定位。
TFT 是無讀回驗證的 SPI 顯示器，初始化成功日誌不能取代目視驗收。

## 使用

在此資料夾的 PlatformIO 終端執行：

```powershell
pio run -e tracker_v2_gps
pio run -e tracker_v2_gps -t upload --upload-port COMx
pio device monitor -b 115200 -p COMx
```

將 `COMx` 換成實際序列埠；後續依序將環境名稱換成
`tracker_v2_lora`、`tracker_v2_tft`、`tracker_v2_full`。
全部驗收後可將 `platformio.ini` 的 `default_envs` 改成 `tracker_v2_full`。

既有電池換算單元測試另用 `pio test -c platformio-native.ini`；需先有本機 GCC/G++。
native 使用獨立建置目錄，避免切換設定時清除韌體建置結果。

## 接腳與電源

| 功能 | 設定 |
|---|---|
| GNSS／TFT 共用電源 | GPIO3，HIGH 開啟 |
| UC6580 UART | ESP32 RX=33、TX=34、115200 baud；RX 對接 GNSS TX |
| GNSS Reset / PPS | GPIO35 / GPIO36；本專案不使用 PPS |
| LoRa SPI | SCK=9、MISO=11、MOSI=10、NSS=8 |
| LoRa RESET / BUSY / DIO1 | 12 / 13 / 14 |
| FEM POWER / CSD / CTX | 7 / 4 / 5；DIO2 控制 RF switch |
| SX1262 TCXO | 1.8 V |
| TFT | CS=38、RST=39、DC=40、SCK=41、MOSI=42、背光=21 |
| PRG | GPIO0，按下 LOW |
| 電池 ADC / 啟用 | GPIO1 / GPIO2 HIGH；分壓倍率 4.9，ADC 2.5 dB |
| 外接 BMI270 | 3V3、GND、SDA=15、SCL=16，I²C 位址 0x68 |

BMI270 必須接上外部模組，地址選擇腳須符合 0x68；模組需適當 I²C 上拉。
GPIO36 是 PPS，不能沿用舊板的 IMU 電源控制。GPIO3 同時供應 GNSS 和 TFT，
關螢幕只切 GPIO21 背光，不切 GPIO3。TFT 使用 HSPI，LoRa 使用獨立的預設 FSPI。
外接 1602 LCD 的初始化與 task 已停用，避免舊 SDA=4、SCL=3 設定干擾 PA 與供電。

## 保留的協定與功能

- `Gps2LoraTask.cpp`、`TdmaProtocol.h`、`LoRaPacket.h` 及活動量演算法沿用原始檔案。
- Slave ID **106**、Master ID **9**；Slave 106 沿用原 Slave 6 的 3000 ms TDMA slot。
- 無線參數維持 923 MHz、BW125、SF12、CR4/5、SyncWord 0x12、CRC。
- Tracker V2 官方增益表對 20 dBm 目標給出 SX1262 **6 dBm**；這是名義設定，實際 RF 輸出尚待儀器量測。
- 未啟用／未接 BMI270 時，活動資料保持無效，不產生虛構分數。
- 保留 `oledInit`／`oledTask`／`oledShowTransmission` 介面；內部已換 TFT，方便原傳送任務沿用。
- 第 4 階段保留原網路服務及設定；若要單獨驗證 BMI270，可於該環境 build_flags 加上 `-D TRACKER_NETWORK_ENABLED=0`。

## 實機驗證紀錄

目前只完成軟體移植；請將實際測量值及觀察結果填入，不以編譯代替實機驗收。

| 項目 | 結果 |
|---|---|
| GPIO3 電源、電池 ADC 對電表誤差 | 待實機驗證 |
| 戶外 UC6580 NMEA／定位／衛星數 | 待實機驗證 |
| Master SYNC 接收及對端 TYPE=3 收包 | 待實機驗證 |
| TFT 方向、顏色、兩頁資料及 120 秒關背光 | 待實機驗證 |
| 關背光後 GPS／LoRa 持續工作 | 待實機驗證 |
| BMI270 I²C、活動分數及有效旗標 | 待實機驗證 |
| RF 輸出功率與電池供電穩定性 | 待實機驗證 |

## 來源

- [Heltec Tracker V2](https://heltec.org/project/wireless-tracker-v2/)
- [官方 LoRa 接腳](https://github.com/HelTecAutomation/Heltec_ESP32/blob/55bf1a5fe0ed102c807b1fcb550a1a8ea31d6bd3/src/driver/board-config.h)
- [官方 PA 增益表](https://github.com/HelTecAutomation/Heltec_ESP32/blob/55bf1a5fe0ed102c807b1fcb550a1a8ea31d6bd3/src/driver/sx126x.c)
- [官方 GPS 範例](https://github.com/HelTecAutomation/Heltec_ESP32/blob/55bf1a5fe0ed102c807b1fcb550a1a8ea31d6bd3/examples/GPS/GPSDisplayOnTFT/GPSDisplayOnTFT.ino)
- [Meshtastic Tracker V2 配置](https://github.com/meshtastic/firmware/blob/master/variants/esp32s3/heltec_wireless_tracker_v2/variant.h)

Heltec 函式庫固定在上述 commit，PlatformIO espressif32 固定為 7.0.1。
