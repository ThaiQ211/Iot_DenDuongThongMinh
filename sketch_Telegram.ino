#define BLYNK_TEMPLATE_ID "TMPL6qMnR6AEg"
#define BLYNK_TEMPLATE_NAME "He thong den thong minh"
#define BLYNK_AUTH_TOKEN "TDg7eNhssTDpLKq1STm426lQw9gbgA8K"

#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

// Thong tin WiFi
char ssid[] = "Tays";
char pass[] = "11111111";

// Dinh nghia chan
#define CAMBIEN_AS 34
#define CAMBIEN_CD 13
#define NUTNHAN 14
#define LED_DO 17
#define LED_VANG 4
#define LED_XANH 5

// Bien toan cuc
int giatri_as = 0;
int nguong_as = 400;
int trangthai_cd = 0;
int nutnhan_truoc = HIGH;
bool cheDoTuDong = false;
unsigned long thoigian_sangCuoi = 0;
bool denDangSang = false;
unsigned long THOIGIAN_SANG = 7000;

// Hieu ung
int buocHieuUng = 0;
unsigned long thoigianHieuUng = 0;
bool dangChayHieuUng = false;
int loaiHieuUng = 0;

// Bo loc nhieu
int demPhatHien = 0;
int NGUONG_PHAT_HIEN = 3;

// Timer
BlynkTimer timer;

// Dieu khien
bool ghiDeBlynk = false;
bool trangThaiBlynk = false;

// THONG KE
int demChuyenDong = 0;
unsigned long tongThoiGianSang = 0;
unsigned long batDauTinhGio = 0;
bool dangTinhGio = false;
bool daDemChuyenDongNay = false; 

// DO NHAY
int doNhayCamBien = 3;

// === TELEGRAM ===
#define BOT_TOKEN "8286703568:AAGoJ1YiloXFBL8rv5J8KgHPnp-fx1oFiXY"
#define CHAT_ID 6265656836

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);
unsigned long lastCheck = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  client.setInsecure(); // Cho phép ESP32 kết nối Telegram không cần SSL xác thực

  pinMode(CAMBIEN_AS, INPUT);
  pinMode(CAMBIEN_CD, INPUT);
  pinMode(NUTNHAN, INPUT_PULLUP);
  pinMode(LED_DO, OUTPUT);
  pinMode(LED_VANG, OUTPUT);
  pinMode(LED_XANH, OUTPUT);

  tatHetDen();

  Serial.println();
  Serial.println("=== ADVANCED SMART LIGHT SYSTEM (v4 - FINAL LOGIC) ===");
  Serial.println("Connecting to WiFi...");

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  Serial.println("Connected!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  timer.setInterval(1000L, guiDuLieuLenBlynk);
  timer.setInterval(1000L, capNhatThongKe);

  Serial.println("Testing LEDs...");
  digitalWrite(LED_DO, HIGH);
  delay(200);
  digitalWrite(LED_VANG, HIGH);
  delay(200);
  digitalWrite(LED_XANH, HIGH);
  delay(200);
  tatHetDen();

  Serial.println("Waiting 10s for stable connection...");
  for (int i = 0; i < 50; i++) {
    Blynk.run();
    delay(200);
  }

  Serial.println("=== READY ===");
}

// --- PHẦN BLYNK ---
void guiDuLieuLenBlynk() {
  Blynk.virtualWrite(V1, giatri_as);

  String status = "";
  if (ghiDeBlynk) {
    status = trangThaiBlynk ? "ON (Override)" : "OFF (Override)";
  } else if (denDangSang || dangChayHieuUng) {
    status = "LIGHTS ON";
  } else if (cheDoTuDong && giatri_as > nguong_as) {
    status = "WAITING";
  } else {
    status = "OFF";
  }
  Blynk.virtualWrite(V2, status);
  Blynk.virtualWrite(V0, cheDoTuDong ? 1 : 0);
}

void capNhatThongKe() {
  Blynk.virtualWrite(V10, demChuyenDong);
  Blynk.virtualWrite(V11, tongThoiGianSang / 1000);

  Serial.println("--- STATISTICS ---");
  Serial.print("Motions: ");
  Serial.println(demChuyenDong);
  Serial.print("On time: ");
  Serial.print(tongThoiGianSang / 1000);
  Serial.println(" s");
}

BLYNK_WRITE(V0) {
  cheDoTuDong = (param.asInt() == 1);
  if (cheDoTuDong) {
    Serial.println("BLYNK: Auto ON");
    tatHetDen();
    ghiDeBlynk = false;
  } else {
    Serial.println("BLYNK: Manual ON");
  }
}

BLYNK_WRITE(V3) {
  int value = param.asInt();
  if (value == 1) {
    ghiDeBlynk = true;
    trangThaiBlynk = true;
    dangChayHieuUng = false;
    digitalWrite(LED_DO, HIGH);
    digitalWrite(LED_VANG, HIGH);
    digitalWrite(LED_XANH, HIGH);
    denDangSang = true;
    batDauTinhThoiGian();
    Serial.println("BLYNK: FORCE ON");
  } else {
    if (ghiDeBlynk) {
      ghiDeBlynk = false;
      trangThaiBlynk = false;
      tatHetDen();
      Serial.println("BLYNK: Override OFF");
    }
  }
}

BLYNK_WRITE(V5) {
  THOIGIAN_SANG = param.asInt() * 1000;
  Serial.print("Duration: ");
  Serial.print(param.asInt());
  Serial.println("s");
}

BLYNK_WRITE(V6) {
  nguong_as = param.asInt();
  Serial.print("Threshold: ");
  Serial.println(nguong_as);
}

BLYNK_WRITE(V7) {
  loaiHieuUng = param.asInt();
  String effectName[] = {"Wave", "Blink", "Fade", "Rainbow"};
  Serial.print("Effect selected: ");
  Serial.println(effectName[loaiHieuUng]);
}

BLYNK_WRITE(V8) {
  doNhayCamBien = param.asInt();
  NGUONG_PHAT_HIEN = doNhayCamBien;
  String level[] = {"", "Very High", "", "Normal", "", "Low"};
  Serial.print("Sensitivity: ");
  Serial.println(level[doNhayCamBien]);
}

BLYNK_WRITE(V12) {
  if (param.asInt() == 1) {
    demChuyenDong = 0;
    tongThoiGianSang = 0;
    Serial.println("Statistics RESET");
    capNhatThongKe();
    Blynk.virtualWrite(V12, 0);
  }
}

// --- CAM BIEN ---
bool kiemTraChuyenDong() {
  trangthai_cd = digitalRead(CAMBIEN_CD);

  if (trangthai_cd == HIGH) {
    if (!daDemChuyenDongNay) {
      demPhatHien++;
      if (demPhatHien >= NGUONG_PHAT_HIEN) {
        demPhatHien = 0;
        demChuyenDong++;
        Blynk.virtualWrite(V4, 255);
        daDemChuyenDongNay = true;
        return true;
      }
    }
  } else {
    if (demPhatHien > 0) demPhatHien--;
    daDemChuyenDongNay = false;
    Blynk.virtualWrite(V4, 0);
  }
  return false;
}

// --- HIEU UNG ---
void batHieuUng() {
  dangChayHieuUng = true;
  buocHieuUng = 0;
  thoigianHieuUng = millis();
  batDauTinhThoiGian();

  String effectName[] = {"WAVE", "BLINK", "FADE", "RAINBOW"};
  Serial.print(">>> EFFECT: ");
  Serial.println(effectName[loaiHieuUng]);
}

void xuLyHieuUng() {
  if (!dangChayHieuUng) return;

  unsigned long now = millis();
  unsigned long elapsed = now - thoigianHieuUng;

  switch (loaiHieuUng) {
    case 0: // WAVE
      if (buocHieuUng == 0) {
        digitalWrite(LED_XANH, HIGH);
        if (elapsed >= 800) { buocHieuUng = 1; thoigianHieuUng = now; }
      } else if (buocHieuUng == 1) {
        digitalWrite(LED_VANG, HIGH);
        if (elapsed >= 800) { buocHieuUng = 2; thoigianHieuUng = now; }
      } else {
        digitalWrite(LED_DO, HIGH);
        ketThucHieuUng();
      }
      break;
    case 1: // BLINK
      if (buocHieuUng < 6) {
        if (elapsed >= 200) {
          bool state = (buocHieuUng % 2 == 0);
          digitalWrite(LED_DO, state);
          digitalWrite(LED_VANG, state);
          digitalWrite(LED_XANH, state);
          buocHieuUng++;
          thoigianHieuUng = now;
        }
      } else {
        digitalWrite(LED_DO, HIGH);
        digitalWrite(LED_VANG, HIGH);
        digitalWrite(LED_XANH, HIGH);
        ketThucHieuUng();
      }
      break;
    case 2: // FADE
      if (buocHieuUng == 0) {
        digitalWrite(LED_XANH, HIGH);
        if (elapsed >= 500) { buocHieuUng = 1; thoigianHieuUng = now; }
      } else if (buocHieuUng == 1) {
        digitalWrite(LED_XANH, HIGH);
        digitalWrite(LED_VANG, HIGH);
        if (elapsed >= 500) { buocHieuUng = 2; thoigianHieuUng = now; }
      } else {
        digitalWrite(LED_DO, HIGH);
        digitalWrite(LED_VANG, HIGH);
        digitalWrite(LED_XANH, HIGH);
        ketThucHieuUng();
      }
      break;
    case 3: // RAINBOW
      if (buocHieuUng == 0) {
        digitalWrite(LED_DO, HIGH);
        if (elapsed >= 400) { digitalWrite(LED_DO, LOW); buocHieuUng = 1; thoigianHieuUng = now; }
      } else if (buocHieuUng == 1) {
        digitalWrite(LED_VANG, HIGH);
        if (elapsed >= 400) { digitalWrite(LED_VANG, LOW); buocHieuUng = 2; thoigianHieuUng = now; }
      } else if (buocHieuUng == 2) {
        digitalWrite(LED_XANH, HIGH);
        if (elapsed >= 400) { digitalWrite(LED_XANH, LOW); buocHieuUng = 3; thoigianHieuUng = now; }
      } else {
        digitalWrite(LED_DO, HIGH);
        digitalWrite(LED_VANG, HIGH);
        digitalWrite(LED_XANH, HIGH);
        ketThucHieuUng();
      }
      break;
  }
}

void ketThucHieuUng() {
  dangChayHieuUng = false;
  denDangSang = true;
  thoigian_sangCuoi = millis();
  Serial.println(">>> EFFECT DONE <<<");
}

void batDauTinhThoiGian() {
  if (!dangTinhGio) {
    dangTinhGio = true;
    batDauTinhGio = millis();
  }
}

void ketThucTinhThoiGian() {
  if (dangTinhGio) {
    tongThoiGianSang += (millis() - batDauTinhGio);
    dangTinhGio = false;
  }
}

void tatHetDen() {
  digitalWrite(LED_DO, LOW);
  digitalWrite(LED_VANG, LOW);
  digitalWrite(LED_XANH, LOW);
  denDangSang = false;
  dangChayHieuUng = false;
  buocHieuUng = 0;
  ketThucTinhThoiGian();
  Serial.println("ALL OFF");
}

// --- TELEGRAM STATUS ---
String getStatus() {
  String status = "";
  status += cheDoTuDong ? "Mode: AUTO\n" : "Mode: MANUAL\n";

  if (cheDoTuDong) {
    if (dangChayHieuUng) status += "Lights: EFFECT RUNNING\n";
    else if (denDangSang) status += "Lights: ON\n";
    else status += "Lights: OFF\n";
  } else {
    if (digitalRead(LED_DO) || digitalRead(LED_VANG) || digitalRead(LED_XANH))
      status += "Lights: ON\n";
    else
      status += "Lights: OFF\n";
  }
  return status;
}

// --- LOOP ---
void loop() {
  Blynk.run();
  timer.run();

  // ----- CHECK TELEGRAM -----
  if (millis() - lastCheck > 1000) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      for (int i = 0; i < numNewMessages; i++) {
        String text = bot.messages[i].text;
        String chat_id = String(bot.messages[i].chat_id);

        if (text == "/status") {
          bot.sendMessage(chat_id, getStatus(), "");
        }
      }
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastCheck = millis();
  }

  // ----- 1. Kiem tra Ghi de -----
  if (ghiDeBlynk) return;

  // ----- 2. Doc tat ca cam bien 1 lan -----
  giatri_as = analogRead(CAMBIEN_AS);
  bool coChuyenDong = kiemTraChuyenDong();
  int nut = digitalRead(NUTNHAN);

  // ----- 3. Xu ly Nhan nut vat ly -----
  if (nut == LOW && nutnhan_truoc == HIGH) {
    cheDoTuDong = !cheDoTuDong;
    tatHetDen();
    demPhatHien = 0;
    ghiDeBlynk = false;
    Blynk.virtualWrite(V0, cheDoTuDong ? 1 : 0);
    Serial.println(cheDoTuDong ? "BUTTON: Auto" : "BUTTON: Manual");
    delay(300);
  }
  nutnhan_truoc = nut;

  // ----- 4. Logic dieu khien chinh -----
  if (!cheDoTuDong) {
    if (giatri_as > nguong_as) {
      digitalWrite(LED_DO, HIGH);
      digitalWrite(LED_VANG, HIGH);
      digitalWrite(LED_XANH, HIGH);
      batDauTinhThoiGian();
    } else {
      digitalWrite(LED_DO, LOW);
      digitalWrite(LED_VANG, LOW);
      digitalWrite(LED_XANH, LOW);
      ketThucTinhThoiGian();
    }
  } else {
    if (giatri_as > nguong_as) {
      if (coChuyenDong && !denDangSang && !dangChayHieuUng) {
        Serial.println(">>> MOTION DETECTED <<<");
        batHieuUng();
      }
      if (denDangSang && coChuyenDong) {
        thoigian_sangCuoi = millis();
        Serial.println("Extended");
      }
      if (denDangSang && (millis() - thoigian_sangCuoi > THOIGIAN_SANG)) {
        Serial.println("Timeout");
        tatHetDen();
      }
    } else {
      if (denDangSang || dangChayHieuUng) {
        tatHetDen();
      }
    }
  }

  // ----- 5. Xu ly hieu ung -----
  xuLyHieuUng();
}
