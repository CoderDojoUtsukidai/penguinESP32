#include <WiFiClient.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include <ESPmDNS.h>

int led = 2;
/* change your ssid and password here */
const char* ssid     = "penguin1";
const char* password = "penguin1";

AsyncWebServer server(80);

// Motor A
const int motor1Pin1 = 27;
const int motor2Pin1 = 26;//14;

const int motor1Pin2 = 14;//26;
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
              "<form action=FD><input type=submit value=FORWARD></form>"
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

void serverSend(AsyncWebServerRequest *request, int status_code, const String& content_type, const String& content) {
  AsyncWebServerResponse *response = request->beginResponse(status_code, content_type, content);
  response->addHeader("Access-Control-Allow-Origin", "*");
  request->send(response);
}

/* this callback will be invoked when get servo rotation request */
void handleServo(AsyncWebServerRequest *request) {
  //Serial.println(server.argName(0));
  int newAngle = atoi(request->getParam(0)->value().c_str());
  servoGo(oldAngle, newAngle);
  oldAngle = newAngle;
  serverSend(request, 200, "text/html", "ok");
}

void handleNotFound(AsyncWebServerRequest *request) {
  String message = "File Not Found\n\n";
  serverSend(request, 404, "text/plain", message);
}

/* プロトタイプ宣言*/
void handleRoot(AsyncWebServerRequest *);
void handle_stop(AsyncWebServerRequest *);
void handle_forward(AsyncWebServerRequest *);
void handle_turn_left(AsyncWebServerRequest *);
void handle_turn_right(AsyncWebServerRequest *);
void handle_back(AsyncWebServerRequest *);
void ST_ACT();
void FD_ACT();
void TL_ACT();
void TR_ACT();
void BK_ACT();

void setup(void) {

  Serial.begin(115200);
  Serial.println("");

  // Configure static IP
  IPAddress ip(172,24,1,2);
  IPAddress gateway(172,24,1,1);
  IPAddress subnet(255, 255, 255, 0);
  IPAddress primaryDNS(8, 8, 8, 8); //optional
  IPAddress secondaryDNS(8, 8, 4, 4); //optional

  if (WiFi.config(ip, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("Configured IP");
  }
  else {
    Serial.println("STA Failed to configure");
  }
  
  // Wait for connection
  WiFi.begin(ssid, password);
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

  server.on("/", HTTP_GET, handleRoot);
  server.on("/ang", HTTP_GET, handleServo);

  server.onNotFound(handleNotFound);

  /* 各アクション時のハンドル設定*/
  server.on("/ST", HTTP_GET, handle_stop);
  server.on("/FD", HTTP_GET, handle_forward);
  server.on("/TL", HTTP_GET, handle_turn_left);
  server.on("/TR", HTTP_GET, handle_turn_right);
  server.on("/BK", HTTP_GET, handle_back);

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

  server.begin();
}

void loop(void) {
}

/*------ ここから下は全て新しく追加するプログラムです----------*/
/* 各ハンドルの内容記述*/
/* ハンドル初期状態の記述*/
void handleRoot(AsyncWebServerRequest *request) {
  serverSend(request, 200, "text/html", form);
}
/* ストップハンドルの記述*/
void handle_stop(AsyncWebServerRequest *request) {
  Serial.println("STOP");
  digitalWrite(led, LOW);
  ST_ACT();
  serverSend(request, 200, "text/html", form);
}
/* 前進ハンドルの記述*/
void handle_forward(AsyncWebServerRequest *request) {
  Serial.println("FORWARD");
  digitalWrite(led, HIGH);
  FD_ACT();
  serverSend(request, 200, "text/html", form);
}
/* 左折ハンドルの記述*/
void handle_turn_left(AsyncWebServerRequest *request) {
  Serial.println("LEFT");
  TL_ACT();
  serverSend(request, 200, "text/html", form);
}
/* 右折ハンドルの記述*/
void handle_turn_right(AsyncWebServerRequest *request) {
  Serial.println("RIGHT");
  TR_ACT();
  serverSend(request, 200, "text/html", form);
}
/* バックハンドルの記述*/
void handle_back(AsyncWebServerRequest *request) {
  Serial.println("BACK");
  BK_ACT();
  serverSend(request, 200, "text/html", form);
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
  ST_ACT();
  digitalWrite(led, HIGH);
  digitalWrite(motor1Pin1, HIGH);
  digitalWrite(motor1Pin2, HIGH);
//  digitalWrite(motor2Pin1, LOW);
//  digitalWrite(motor2Pin2, LOW);
}
/* 左折の関数*/
void TL_ACT() {
  ST_ACT();
//  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, HIGH);
//  digitalWrite(motor2Pin1, LOW);
//  digitalWrite(motor2Pin2, LOW);
}
/* 右折の関数*/
void TR_ACT() {
  ST_ACT();
  digitalWrite(motor1Pin1, HIGH);
//  digitalWrite(motor1Pin2, LOW);
//  digitalWrite(motor2Pin1, HIGH);
//  digitalWrite(motor2Pin2, LOW);
}
/* バックの関数*/
void BK_ACT() {
  ST_ACT();
//  digitalWrite(motor1Pin1, LOW);
//  digitalWrite(motor1Pin2, HIGH);
  digitalWrite(motor2Pin1, HIGH);
  digitalWrite(motor2Pin2, HIGH);
}
