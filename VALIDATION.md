# 移植驗證紀錄

日期：2026-09-26～2026-09-27

## 軟體檢查

- 編譯環境：PlatformIO espressif32 7.0.1、Arduino ESP32 2.0.17、RadioLib 7.7.1。
- 最終四環境全部編譯成功，詳見 `validation/build-final.log` 末尾摘要。
  `tracker_v2_gps`、`tracker_v2_lora`、`tracker_v2_tft`、`tracker_v2_full` 均為 SUCCESS。
- 完整功能環境 RAM 使用 48,208 bytes（14.7%）；程式 Flash 使用 886,845 bytes（26.5% 的 app partition）。
- `preserved-source-hashes.json` 記錄仍與原專案 SHA-256 一致的封包及活動量來源；身份設定與 TDMA 主機註解／ID 映射已依需求修改。
- Slave ID=106、Master ID=9；Slave 106 沿用原 Slave 6 的 3000 ms TDMA slot。
- 電池換算 native 測試已嘗試；本機缺少 `gcc`／`g++`，未執行成功，詳見 `validation/test-native.log`。

## 實機狀態

尚未燒錄或宣稱任何硬體驗收通過。偵測到 COM12 的 ESP32 USB 裝置，
但未確認為目標 Wireless Tracker V2，故未改寫該裝置。
請依 README 的四個累進階段接線、燒錄與記錄結果。

實機驗證前，接妥相應频段的 LoRa 天線；GPS 定位請在戶外測試。
20 dBm 為官方增益表推算的名義目標，尚未量測實際 RF 輸出。
