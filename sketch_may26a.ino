#include <Wire.h>
#include <TFT_eSPI.h>
#include <FastIMU.h>
#include <F_MPU6500.hpp>
#include <WiFi.h>
#include <WebServer.h>

// --- WebServer ---
WebServer server(80);

// --- MPU6500 ---
#define SDA_PIN 8
#define SCL_PIN 9
MPU6500 mpu;
calData calib;
AccelData accel;

// --- TFT ---
TFT_eSPI tft = TFT_eSPI();

// --- Accelerometru (filtrat) ---
float x = 0, y = 0, z = 0;
float filteredX = 0.0, filteredY = 0.0;
const float alpha = 0.1;

// --- 3D ---
const int xOrigin = 120;
const int yOrigin = 120;
const float viewDistance = 75.0;

#define NUM_VERTICES 8
const int cube_vertex[NUM_VERTICES][3] = {
  { -25, -25,  25 }, {  25, -25,  25 },
  {  25,  25,  25 }, { -25,  25,  25 },
  { -25, -25, -25 }, {  25, -25, -25 },
  {  25,  25, -25 }, { -25,  25, -25 }
};
int wireframe[NUM_VERTICES][2];

void setup() {
  Serial.begin(115200);

  Wire.begin(SDA_PIN, SCL_PIN);
  mpu.init(calib);
  Serial.println("MPU6500 initializat.");

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  // Pornim Access Point
  WiFi.softAP("MPU_Cube", "12345678");
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);  // accesează http://192.168.4.1

  // Setăm rutele serverului
  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();
}

void loop() {
  mpu.update();
  mpu.getAccel(&accel);
  server.handleClient();

  // Filtrare accelerometru
  filteredX = alpha * accel.accelX + (1 - alpha) * filteredX;
  filteredY = alpha * accel.accelY + (1 - alpha) * filteredY;

  float ax = constrain(filteredX, -1.0, 1.0);
  float ay = constrain(filteredY, -1.0, 1.0);

  x = asin(ax);
  y = asin(ay);
  z += 0.01;
  if (z > 2 * PI) z -= 2 * PI;

  for (int i = 0; i < NUM_VERTICES; i++) {
    float rotx = cube_vertex[i][2] * sin(y) + cube_vertex[i][0] * cos(y);
    float roty = cube_vertex[i][1];
    float rotz = cube_vertex[i][2] * cos(y) - cube_vertex[i][0] * sin(y);
    float rotxx = rotx;
    float rotyy = roty * cos(x) - rotz * sin(x);
    float rotzz = roty * sin(x) + rotz * cos(x);
    float rotxxx = rotxx * cos(z) - rotyy * sin(z);
    float rotyyy = rotxx * sin(z) + rotyy * cos(z);
    float rotzzz = rotzz;
    rotxxx *= viewDistance / (viewDistance + rotzzz);
    rotyyy *= viewDistance / (viewDistance + rotzzz);
    rotxxx += xOrigin;
    rotyyy += yOrigin;
    wireframe[i][0] = (int)rotxxx;
    wireframe[i][1] = (int)rotyyy;
  }

  draw_wireframe();
  delay(15);
}

void draw_wireframe() {
  tft.fillScreen(TFT_BLACK);

  tft.drawLine(wireframe[0][0], wireframe[0][1], wireframe[1][0], wireframe[1][1], TFT_WHITE);
  tft.drawLine(wireframe[1][0], wireframe[1][1], wireframe[2][0], wireframe[2][1], TFT_WHITE);
  tft.drawLine(wireframe[2][0], wireframe[2][1], wireframe[3][0], wireframe[3][1], TFT_WHITE);
  tft.drawLine(wireframe[3][0], wireframe[3][1], wireframe[0][0], wireframe[0][1], TFT_WHITE);

  tft.drawLine(wireframe[4][0], wireframe[4][1], wireframe[5][0], wireframe[5][1], TFT_WHITE);
  tft.drawLine(wireframe[5][0], wireframe[5][1], wireframe[6][0], wireframe[6][1], TFT_WHITE);
  tft.drawLine(wireframe[6][0], wireframe[6][1], wireframe[7][0], wireframe[7][1], TFT_WHITE);
  tft.drawLine(wireframe[7][0], wireframe[7][1], wireframe[4][0], wireframe[4][1], TFT_WHITE);

  tft.drawLine(wireframe[0][0], wireframe[0][1], wireframe[4][0], wireframe[4][1], TFT_WHITE);
  tft.drawLine(wireframe[1][0], wireframe[1][1], wireframe[5][0], wireframe[5][1], TFT_WHITE);
  tft.drawLine(wireframe[2][0], wireframe[2][1], wireframe[6][0], wireframe[6][1], TFT_WHITE);
  tft.drawLine(wireframe[3][0], wireframe[3][1], wireframe[7][0], wireframe[7][1], TFT_WHITE);
}

void handleRoot() {
  String html = R"rawliteral(
  <!DOCTYPE html><html><head>
  <title>Gyro Ball</title>
  <style>
    body { font-family: sans-serif; text-align: center; }
    #playground {
      width: 320px; height: 320px;
      background: #eee; margin: auto;
      position: relative; border: 2px solid #888;
    }
    #ball {
      width: 20px; height: 20px;
      background: red; border-radius: 50%;
      position: absolute;
      left: 150px; top: 150px;
    }
  </style></head><body>
  <h2>Accelerometer Ball</h2>
  <p id="accval">Loading...</p>
  <div id="playground"><div id="ball"></div></div>

  <script>
    let posX = 150, posY = 150;
    let velX = 0, velY = 0;
    const centerX = 150, centerY = 150;
    const accelScale = 3;
    const damping = 0.9;
    const spring = 0.02;
    const deadzone = 0.05;

    function update() {
      fetch('/data')
        .then(r => r.json())
        .then(data => {
          document.getElementById("accval").innerText = `X=${data.accelX.toFixed(2)}, Y=${data.accelY.toFixed(2)}`;
          const ax = Math.abs(data.accelX) > deadzone ? data.accelX : 0;
          const ay = Math.abs(data.accelY) > deadzone ? data.accelY : 0;

          velX += ax * accelScale;
          velY += ay * accelScale;

          velX += (centerX - posX) * spring;
          velY += (centerY - posY) * spring;

          velX *= damping;
          velY *= damping;

          posX += velX;
          posY += velY;

          posX = Math.max(0, Math.min(300, posX));
          posY = Math.max(0, Math.min(300, posY));

          document.getElementById("ball").style.left = posX + "px";
          document.getElementById("ball").style.top = posY + "px";
        });
    }

    setInterval(update, 50);
  </script>
  </body></html>
  )rawliteral";

  server.send(200, "text/html", html);
}

void handleData() {
  String json = "{";
  json += "\"accelX\":" + String(filteredX, 2) + ",";
  json += "\"accelY\":" + String(filteredY, 2);
  json += "}";
  server.send(200, "application/json", json);
}
