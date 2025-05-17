# Tab5-Display_espidf

ESP32-P4（M5Stack系）用 ディスプレイスピードテストプロジェクト


![image](https://github.com/user-attachments/assets/4998d911-944f-4b15-9ad8-856d1c2704cc)

## 概要
- ESP32-P4開発ボード上でM5Unified/M5GFXライブラリを利用し、複数の画面サイズ・回転で描画速度を自動計測・表示します。
- テスト結果はシリアルログおよび画面に出力されます。

## 特徴
- M5Unified/M5GFXの最新developブランチ対応
- 複数の解像度・回転角度で自動テスト
- ESP-IDF 5.4系でビルド
- シリアル出力による詳細な計測ログ

## ビルド・書き込み方法（ESP-IDF）
```sh
. $HOME/esp/esp-idf/export.sh
idf.py fullclean
idf.py build
idf.py -p /dev/ttyACM0 flash
idf.py -p /dev/ttyACM0 monitor
```

## プロジェクト構成
- `main/main.cpp` : メインロジック（ディスプレイテスト）
- `main/idf_component.yml` : M5Unified/M5GFX依存設定
- `sdkconfig` : ESP-IDFプロジェクト設定
- `platformio.ini` : PlatformIO用設定（参考）

## 依存ライブラリ
- [M5Unified (develop)](https://github.com/m5stack/M5Unified)
- [M5GFX (develop)](https://github.com/m5stack/M5GFX)

## 注意事項
- ESP-IDFのバージョンは5.4.x系で動作確認済み
- シリアルポートは `/dev/ttyACM0` 例。環境により変更してください

## ライセンス
MIT License（各ライブラリのライセンスも参照）

---
ご質問や不具合報告はIssueまたはPull Requestでお願いします。
