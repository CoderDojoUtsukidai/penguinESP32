#include <WiFiClient.h>
#include <ESP32WebServer.h>
#include <WiFi.h>
#include <ESPmDNS.h>

int led = 2;
/* change your ssid and password here */

const char* ssid     = "xxxxx";
const char* password = "zzzzz";

ESP32WebServer server(80);

// Motor A
const int motor1Pin1 = 27;
const int motor1Pin2 = 26;
const int motor2Pin1 = 14;
const int motor2Pin2 = 16;

const int servoPin = 12;
int oldAngle = 0;
int PWM_WIDTH = 20000;

String form = "<!DOCTYPE html><html><head><meta name=viewport content=width=100><meta charset='utf-8'>"
              "<link href='https://code.jquery.com/ui/1.10.4/themes/ui-lightness/jquery-ui.css' rel='stylesheet'>"
              "<script src='https://code.jquery.com/jquery-1.10.2.js'></script>"
              "<script src='https://code.jquery.com/ui/1.10.4/jquery-ui.js'></script>"
              "<script>"
              "$(function() {"
              "    $('#sliVal').html('Angle: 0');"
              "    $('#slider').slider({"
              "        orientation:'vertical',value:0,min: 0,max: 180,step: 5"
              "    });"
              "    $('#slider').slider().bind({"
              "        slidestop: function(e,ui){"
              "            $('#res').css('background','red');"
              "            $('#sliVal').html('Angle: '+ui.value);"
              "            $.get('/ang?val=' + ui.value, function(d, s){"
              "                $('#res').css('background','green');"
              "                $('#res').html(s);"
              "            }); "
              "        }"
              "    });"
              "});"
              "</script>"
              "</head><body>"
              "<H1>Penguin Esp32 Controller</H1>"
              "<div id='slider'></div></br>"
              "<div id='sliVal'></div>"
              "<div id='res'></div></br>"
              "<div id='slider'></div></br>"
              "<div id='sliVal'></div>"
              "<div id='res'></div>"
              "<hr>"
              "<form action=ST><input type=submit value=STOP></form>"
              "<form action=FD><input type=submit value=FORWADR></form>"
              "<form action=TL><input type=submit value=LEFT></form>"
              "<form action=TR><input type=submit value=RIGH></form>"
              "<form action=BK><input type=submit value=BACK></form>"
              "</body></html>";

/* this function map from angle to pulse width */
int servoPulse(int angleDegrees)
{
  int pulseWidth = map(angleDegrees, 0, 180, 600, 2400);
  return pulseWidth;
}
/* this function check the rotation angle
  and trigger pulse accordingly*/
void servoGo(int oldAngle, int newAngle)
{
  int pulseWidth;
  if (oldAngle == newAngle) {
    return;
  } else if (oldAngle < newAngle) {
    /* clockwise processing */
    for (int i = oldAngle; i <= newAngle; i++) {
      /* convert angle to pulse width us*/
      pulseWidth = servoPulse(i);
      /* trigger HIGH pulse */
      digitalWrite(servoPin, HIGH);
      /* use delayMicroseconds to delay for pulseWidth */
      delayMicroseconds(pulseWidth);
      /* trigger LOW pulse */
      digitalWrite(servoPin, LOW);
      /* use delayMicroseconds to delay
        for rest time (20000 - pulseWidth) */
      delayMicroseconds(PWM_WIDTH - pulseWidth);
    }
  } else if (oldAngle > newAngle) {
    /* anti-clockwise processing */
    for (int i = oldAngle; i >= newAngle; i--) {
      pulseWidth = servoPulse(i);
      digitalWrite(servoPin, HIGH);
      delayMicroseconds(pulseWidth);
      digitalWrite(servoPin, LOW);
      delayMicroseconds(PWM_WIDTH - pulseWidth);
    }
  }
}
/* this callback will be invoked when get servo rotation request */
void handleServo() {
  //Serial.println(server.argName(0));
  int newAngle = server.arg(0).toInt();
  servoGo(oldAngle, newAngle);
  oldAngle = newAngle;
  server.send(200, "text/html", "ok");
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  server.send(404, "text/plain", message);
}

/* プロトタイプ宣言*/
void handleRoot();
void handle_stop();
void handle_forward();
void handle_turn_left();
void handle_turn_right();
void handle_back();
void ST_ACT();
void FD_ACT();
void TL_ACT();
void TR_ACT();
void BK_ACT();

void setup(void) {

  Serial.begin(115200);
  WiFi.begin(ssid, password);
  //  WiFi.begin(ssid2, password2);
  Serial.println("");
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);
  server.on("/ang", handleServo);

  server.onNotFound(handleNotFound);

  /* 各アクション時のハンドル設定*/
  server.on("/ST", handle_stop);
  server.on("/FD", handle_forward);
  server.on("/TL", handle_turn_left);
  server.on("/TR", handle_turn_right);
  server.on("/BK", handle_back);

  server.begin();
  pinMode(servoPin, OUTPUT);
  Serial.println("HTTP server started");

  pinMode(motor1Pin1, OUTPUT); // 出力ピンの設定
  pinMode(motor1Pin2, OUTPUT);
  pinMode(motor2Pin1, OUTPUT);
  pinMode(motor2Pin2, OUTPUT);
  digitalWrite(motor1Pin1, LOW); // 最初はストップから
  digitalWrite(motor1Pin2, LOW);
  digitalWrite(motor2Pin1, LOW);
  digitalWrite(motor2Pin2, LOW);
  Serial.println("STOP");
  pinMode(led, OUTPUT);
  Serial.println("led => 2");
  digitalWrite(led, HIGH);
}

void loop(void) {
  server.handleClient();
}

/*------ ここから下は全て新しく追加するプログラムです----------*/
/* 各ハンドルの内容記述*/
/* ハンドル初期状態の記述*/
void handleRoot() {
  server.send(200, "text/html", form);
}
/* ストップハンドルの記述*/
void handle_stop() {
  Serial.println("STOP");
  digitalWrite(led, LOW);
  ST_ACT();
  server.send(200, "text/html", form);
}
/* 前進ハンドルの記述*/
void handle_forward() {
  Serial.println("FORWARD");
  digitalWrite(led, HIGH);
  FD_ACT();
  server.send(200, "text/html", form);
}
/* 左折ハンドルの記述*/
void handle_turn_left() {
  Serial.println("LEFT");
  TL_ACT();
  server.send(200, "text/html", form);
}
/* 右折ハンドルの記述*/
void handle_turn_right() {
  Serial.println("RIGHT");
  TR_ACT();
  server.send(200, "text/html", form);
}
/* バックハンドルの記述*/
void handle_back() {
  Serial.println("BACK");
  BK_ACT();
  server.send(200, "text/html", form);
}
/*-------------- 各ハンドル記述内の関数内容------------------*/
/* ストップの関数*/
void ST_ACT() {
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, LOW);
  digitalWrite(motor2Pin1, LOW);
  digitalWrite(motor2Pin2, LOW);
}
/* 前進の関数*/
void FD_ACT() {
  digitalWrite(motor1Pin1, HIGH);
  digitalWrite(motor1Pin2, LOW);
  digitalWrite(motor2Pin1, HIGH);
  digitalWrite(motor2Pin2, LOW);
}
/* 左折の関数*/
void TL_ACT() {
  digitalWrite(motor1Pin1, HIGH);
  digitalWrite(motor1Pin2, LOW);
  digitalWrite(motor2Pin1, LOW);
  digitalWrite(motor2Pin2, LOW);
}
/* 右折の関数*/
void TR_ACT() {
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, LOW);
  digitalWrite(motor2Pin1, HIGH);
  digitalWrite(motor2Pin2, LOW);
}
/* バックの関数*/
void BK_ACT() {
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, HIGH);
  digitalWrite(motor2Pin1, LOW);
  digitalWrite(motor2Pin2, HIGH);
}
