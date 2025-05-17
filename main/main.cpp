#include <M5Unified.h>

// テスト設定
struct ScreenSize {
  int width;
  int height;
  const char* name;
};

const ScreenSize SCREEN_SIZES[] = {
    {160, 160, "160x160"}, {320, 320, "320x320"},   {640, 640, "640x640"},
    {720, 720, "720x720"}, {720, 1280, "720x1280"}, {1280, 720, "1280x720"},
};
const int NUM_SIZES = 6;
const int NUM_ROTATIONS = 4;

// キャンバスとテスト変数
M5Canvas canvas(&M5.Display);
unsigned long startTime, endTime, duration;
float totalTime = 0;
int frameCount = 0;
int totalFrames = 20;
int sizeIndex = 0;
int rotMode = 0;
bool testDone = false;
bool inMaxTest = false;  // 最大サイズテスト中かどうかを追跡するフラグ
ScreenSize maxSize = {0, 0, "Maximum"};

// タッチ検出変数
unsigned long lastTouchTime = 0;
const unsigned long touchDelay = 500;  // デバウンス遅延（ミリ秒）

// 結果の保存
float results[6][4] = {0};
bool tested[6][4] = {false};
float maxResults[4] = {0};
bool maxTested[4] = {false};

// 最終結果の表示
void showResults() {
  M5.Display.setRotation(1);
  canvas.deleteSprite();
  canvas.createSprite(M5.Display.width(), M5.Display.height());
  canvas.fillScreen(TFT_BLACK);
  canvas.setTextSize(1);
  canvas.setTextColor(TFT_WHITE);
  canvas.setCursor(10, 10);
  canvas.println("Display Speed Test Results");
  canvas.println("====================");

  // 標準サイズ
  for (int i = 0; i < NUM_SIZES; i++) {
    canvas.println("");
    canvas.print("Size: ");
    canvas.println(SCREEN_SIZES[i].name);
    canvas.println("--------------------");

    for (int r = 0; r < NUM_ROTATIONS; r++) {
      if (tested[i][r]) {
        canvas.print("Rot ");
        canvas.print(r);
        canvas.print(": ");
        canvas.print(results[i][r], 2);
        canvas.println(" ms/frame");
      }
    }

    // 最速の回転を見つける
    float bestTime = 999999;
    int bestRot = -1;
    for (int r = 0; r < NUM_ROTATIONS; r++) {
      if (tested[i][r] && results[i][r] < bestTime) {
        bestTime = results[i][r];
        bestRot = r;
      }
    }

    if (bestRot >= 0) {
      canvas.print("Best: Rotation ");
      canvas.println(bestRot);
    }
  }

  // 最大サイズ
  canvas.println("\nMAXIMUM SIZE TEST");
  canvas.print("Size: ");
  canvas.print(maxSize.width);
  canvas.print("x");
  canvas.println(maxSize.height);
  canvas.println("--------------------");

  for (int r = 0; r < NUM_ROTATIONS; r++) {
    if (maxTested[r]) {
      canvas.print("Rot ");
      canvas.print(r);
      canvas.print(": ");
      canvas.print(maxResults[r], 2);
      canvas.println(" ms/frame");
    }
  }

  canvas.println("\nTouch screen to retest");

  // Center on screen
  int x = (M5.Display.width() - canvas.width()) / 2;
  int y = (M5.Display.height() - canvas.height()) / 2;
  canvas.pushSprite(x, y);

  // シリアルに出力
  ESP_LOGI("TEST", "%s", "\n===== TEST RESULTS =====");
  for (int i = 0; i < NUM_SIZES; i++) {
    ESP_LOGI("TEST", "%s", "\nSize: ");
    ESP_LOGI("TEST", "%s", SCREEN_SIZES[i].name);
    for (int r = 0; r < NUM_ROTATIONS; r++) {
      if (tested[i][r]) {
        ESP_LOGI("TEST", "%s", "Rotation ");
        ESP_LOGI("TEST", "Rotation %d", r);
        ESP_LOGI("TEST", "%s", ": ");
        ESP_LOGI("TEST", "%f ms/frame", results[i][r]);
        ESP_LOGI("TEST", "%s", " ms/frame");
      }
    }
  }

  // 最大サイズの結果を出力
  ESP_LOGI("TEST", "%s", "\nMAXIMUM SIZE TEST");
  ESP_LOGI("TEST", "%s", "Size: ");
  ESP_LOGI("TEST", "%d", maxSize.width);
  ESP_LOGI("TEST", "%s", "x");
  ESP_LOGI("TEST", "%d", maxSize.height);

  for (int r = 0; r < NUM_ROTATIONS; r++) {
    if (maxTested[r]) {
      ESP_LOGI("TEST", "%s", "Rotation ");
      ESP_LOGI("TEST", "Rotation %d", r);
      ESP_LOGI("TEST", "%s", ": ");
      ESP_LOGI("TEST", "%f ms/frame", maxResults[r]);
      ESP_LOGI("TEST", "%s", " ms/frame");
    }
  }
}

// テストをスキップすべきかチェック
// テストをスキップすべきかチェック
bool shouldSkip() {
  // テスト対象のサイズを取得
  int w = SCREEN_SIZES[sizeIndex].width;
  int h = SCREEN_SIZES[sizeIndex].height;

  // 特定のサイズと回転の組み合わせをスキップ
  // Size: 720x1280のときは、Rotation 1とRotation 3をスキップ
  if (w == 720 && h == 1280 && (rotMode == 1 || rotMode == 3)) {
    ESP_LOGI("TEST", "Skipping size %dx%d with rotation %d (as requested)\n", w, h,
                  rotMode);
    return true;
  }

  // Size: 1280x720のときは、Rotation 0とRotation 2をスキップ
  if (w == 1280 && h == 720 && (rotMode == 0 || rotMode == 2)) {
    ESP_LOGI("TEST", "Skipping size %dx%d with rotation %d (as requested)\n", w, h,
                  rotMode);
    return true;
  }

  // 実際のディスプレイサイズより大きい場合はスキップ
  //if (w > M5.Display.width() || h > M5.Display.height()) {
  //  ESP_LOGI("TEST", "Skipping size %dx%d (exceeds display size %dx%d)\n", w, h,
  //                M5.Display.width(), M5.Display.height());
  //  return true;
  //}

  return false;
}

// 次のテストを設定
void setupNextTest() {
  // 次の有効なテストを見つける
  while (true) {
    // 現在のテストをスキップすべきかチェック
    if (!shouldSkip()) break;

    // 次の設定に移動
    rotMode++;
    if (rotMode >= NUM_ROTATIONS) {
      rotMode = 0;

      if (!inMaxTest) {
        sizeIndex++;

        if (sizeIndex >= NUM_SIZES) {
          // Start maximum size test
          inMaxTest = true;
          ESP_LOGI("TEST", "%s", "Starting maximum size test");
        }
      } else {
        // All tests complete, including max size test
        testDone = true;
        showResults();
        return;
      }
    }
  }

  // スプライトを準備
  canvas.deleteSprite();
  int width, height;

  if (inMaxTest) {
    width = maxSize.width;
    height = maxSize.height;
  } else {
    width = SCREEN_SIZES[sizeIndex].width;
    height = SCREEN_SIZES[sizeIndex].height;
  }

  canvas.createSprite(width, height);
  M5.Display.setRotation(rotMode);

  frameCount = 0;
  totalTime = 0;

  if (!inMaxTest) {
    ESP_LOGI("TEST", "Testing: %s, Rotation %d", SCREEN_SIZES[sizeIndex].name, rotMode);
  } else {
    ESP_LOGI("TEST", "Testing: %dx%d, Rotation %d", maxSize.width, maxSize.height, rotMode);
  }

}

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  

  // 最大サイズを保存
  maxSize.width = M5.Display.width();
  maxSize.height = M5.Display.height();

  ESP_LOGI("TEST", "%s", "Starting display speed test...");
  setupNextTest();
}

// 画面がタッチされているかチェック
bool isTouched() {
  if (M5.Touch.getCount()) {
    // タッチのデバウンス処理
    if (lgfx::v1::millis() - lastTouchTime > touchDelay) {
      lastTouchTime = lgfx::v1::millis();
      return true;
    }
  }
  return false;
}

void loop() {
  M5.update();

  // テスト完了の処理
  if (testDone) {
    // テスト再開のためのタッチをチェック
    if (isTouched()) {
      testDone = false;
      sizeIndex = 0;
      rotMode = 0;
      inMaxTest = false;

      // テスト記録をリセット
      for (int i = 0; i < NUM_SIZES; i++) {
        for (int r = 0; r < NUM_ROTATIONS; r++) {
          tested[i][r] = false;
        }
      }

      for (int r = 0; r < NUM_ROTATIONS; r++) {
        maxTested[r] = false;
      }

      setupNextTest();
    }
    return;
  }

  // 描画テスト開始
  startTime = lgfx::v1::micros();

  // テストコンテンツを描画
  canvas.fillScreen(TFT_BLACK);

  // 画面サイズに基づいて図形を描画
  int area = canvas.width() * canvas.height();
  int numRects = sqrt(area) / 10;

  for (int i = 0; i < numRects; i++) {
    int x = i * (canvas.width() / numRects);
    int y = i * (canvas.height() / numRects);
    int w = canvas.width() / numRects;
    int h = canvas.height() / numRects;
    // より派手な色を生成
    uint16_t color;
    // 正弦波と余弦波を使って鮮やかな色を生成
    uint8_t r = 128 + 127 * sin(i * 0.4);
    uint8_t g = 128 + 127 * sin(i * 0.4 + 2.0);
    uint8_t b = 128 + 127 * sin(i * 0.4 + 4.0);

    // インデックスに基づいて色の強度を変化させる
    if (i % 3 == 0) {
      // 高彩度の色
      color = M5.Lcd.color565(r, g, b);
    } else if (i % 3 == 1) {
      // 青みがかった色
      color = M5.Lcd.color565(r / 2, g, b);
    } else {
      // 赤みがかった色
      color = M5.Lcd.color565(r, g / 2, b / 2);
    }

    canvas.fillRect(x, y, w, h, color);
  }

  // テスト情報を表示
  canvas.setTextSize(1);
  canvas.setTextColor(TFT_WHITE, TFT_BLACK);
  canvas.setCursor(10, 10);

  if (!inMaxTest) {
    canvas.print(SCREEN_SIZES[sizeIndex].name);
  } else {
    canvas.print(maxSize.width);
    canvas.print("x");
    canvas.print(maxSize.height);
  }

  canvas.print(" Rot:");
  canvas.print(rotMode);
  canvas.print(" Frame:");
  canvas.print(frameCount);
  canvas.print("/");
  canvas.println(totalFrames);

  // Center on screen
  int x = (M5.Display.width() - canvas.width()) / 2;
  int y = (M5.Display.height() - canvas.height()) / 2;
  canvas.pushSprite(x, y);

  // 時間を測定
  endTime = lgfx::v1::micros();
  duration = (endTime - startTime) / 1000.0;
  totalTime += duration;
  frameCount++;

  // すべてのフレームが完了したとき
  if (frameCount >= totalFrames) {
    float avgTime = totalTime / totalFrames;

    // 結果を保存
    if (!inMaxTest) {
      results[sizeIndex][rotMode] = avgTime;
      tested[sizeIndex][rotMode] = true;
    } else {
      maxResults[rotMode] = avgTime;
      maxTested[rotMode] = true;
    }

    ESP_LOGI("TEST", "%s", "Average: ");
    ESP_LOGI("TEST", "Average: %f ms/frame", avgTime);
    ESP_LOGI("TEST", "%s", " ms/frame");

    // 次のテストに移動
    rotMode++;
    if (rotMode >= NUM_ROTATIONS) {
      rotMode = 0;

      if (!inMaxTest) {
        sizeIndex++;

        if (sizeIndex >= NUM_SIZES) {
          // Start maximum size test
          inMaxTest = true;
          ESP_LOGI("TEST", "%s", "Starting maximum size test");
        }
      } else {
        // All tests complete, including max size test
        testDone = true;
        showResults();
        return;
      }
    }

    setupNextTest();
  }

  M5.delay(1);
}

#if defined(ESP_PLATFORM) && !defined(ARDUINO)
extern "C" {
int app_main(int, char**) {
  setup();
  for (;;) {
    loop();
  }
  return 0;
}
}
#endif