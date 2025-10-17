import processing.serial.*;
import java.awt.event.MouseEvent;

// Serial
Serial myPort;
String selectedPort = "";
boolean isConnected = false;

// Số lượng cảm biến
int NUM_SENSORS = 3;
int[] iAngle = new int[NUM_SENSORS]; 
int[] iDistance = new int[NUM_SENSORS];

// Biến vẽ
PFont font;
float radarScale = 1.3; //Hệ số phóng to radar
String[] availablePorts;
boolean showPortList = false;
PImage logo;
PFont fontCustom;

void setup() {
  size(1200, 680);
  smooth();
  logo = loadImage("logo.png");

  font = createFont("Arial", 16);
  textFont(font);
  
  fontCustom = createFont("Times New Roman Bold", 70); 

  // Lấy danh sách cổng serial
  availablePorts = Serial.list();
}

void draw() {
  //background(0);
  fill(98, 245, 31);  
  // Nếu chưa kết nối thì hiển thị menu chọn cổng
  if (!isConnected) {
    drawSelectPortMenu();
    return;
  }
  image(logo, 20, 60, 200, 200);
  textFont(fontCustom);
  text("Đồ án", 340, 90+40);
  text("Đo Lường", 405, 180+40);
  stroke(98, 245, 31);
  line(0, 0, width, 0);
  line(0, height-1, width, height-1);
  line(0+1, 0, 0+1, height);
  line(width-1, 0, width-1, height);
  line(0, height/2, width, height/2);
  line(width/2, 0, width/2, height);
  // Nếu đã kết nối: hiển thị radar
  for (int i = 0; i < NUM_SENSORS; i++) {
    pushMatrix();
    if (i == 0) {
      // Sensor1 ở giữa trên
      translate(width / 2 + width / 4, height / 3 + 75);
    } else if (i == 1) {
      // Sensor2 ở dưới bên trái
      translate(width / 2 - width / 4, height - 40);
    } else if (i == 2) {
      // Sensor3 ở dưới bên phải
      translate(width / 2 + width / 4, height - 40);
    }
    
    // simulating motion blur and slow fade of the moving line
    noStroke();
    fill(0, 7); 
    rect(0, 0, width*2, height*2);
    fill(98, 245, 31);
    drawRadar(i);
    drawLine(i);
    drawObject(i);
    drawText(i);
    popMatrix();
  }
}

void drawSelectPortMenu() {
  background(50);
  fill(255);
  textAlign(CENTER, CENTER);
  textSize(22);
  text("⚙️  Chọn cổng Serial để kết nối", width/2, 80);
  
  textSize(16);
  fill(98, 245, 31);
  rectMode(CENTER);

  // Hiển thị các cổng serial
  for (int i = 0; i < availablePorts.length; i++) {
    float y = 150 + i * 40;
    rect(width/2, y, 300, 30, 8);
    fill(255);
    text(availablePorts[i], width/2, y);
    fill(98, 245, 31);
  }

  if (availablePorts.length == 0) {
    fill(255, 100, 100);
    text("⚠️  Không tìm thấy cổng Serial nào!", width/2, 200);
  }
}

void mousePressed() {
  // Nếu chưa kết nối, kiểm tra xem user click vào cổng nào
  if (!isConnected) {
    for (int i = 0; i < availablePorts.length; i++) {
      float y = 150 + i * 40;
      if (mouseY > y - 15 && mouseY < y + 15 && mouseX > width/2 - 150 && mouseX < width/2 + 150) {
        selectedPort = availablePorts[i];
        println("Đang kết nối tới: " + selectedPort);
        try {
          myPort = new Serial(this, selectedPort, 115200);
          myPort.bufferUntil('.');
          isConnected = true;
          println("✅ Kết nối thành công!");
        } catch (Exception e) {
          println("❌ Lỗi kết nối: " + e.getMessage());
        }
      }
    }
  }
}

void serialEvent(Serial myPort) {
  String raw = myPort.readStringUntil('.');
  if (raw == null) return;
  raw = trim(raw);
  if (raw.length() == 0) return;
  if (raw.charAt(raw.length()-1) == '.') raw = raw.substring(0, raw.length()-1);

  // Format: "ang1,dist1;ang2,dist2;ang3,dist3."
  String[] sensors = split(raw, ';');
  for (int i = 0; i < min(sensors.length, NUM_SENSORS); i++) {
    String[] vals = split(sensors[i], ',');
    if (vals.length == 2) {
      try {
        iAngle[i] = int(trim(vals[0]));
        iDistance[i] = int(trim(vals[1]));
      } catch(Exception e) {
        println("Parse error sensor " + i + ": " + sensors[i]);
      }
    }
  }
}

void drawRadar(int idx) {
  noFill();
  strokeWeight(2);
  stroke(98,245,31);
  
  float radarSize = (width / 2.5 - 60) * radarScale;
  float r = radarSize / 2;
  
  // Vẽ 4 vòng cung
  for (int k = 1; k <= 4; k++) {
    arc(0, 0, r * 2 * k / 4, r * 2 * k / 4, PI, TWO_PI);
  }
  
  // Vẽ các tia chia
  for (int ang = 0; ang <= 180; ang += 30) {
    strokeWeight(3);
    float x = (r + 10) * cos(radians(ang));
    float y = -(r + 10) * sin(radians(ang));
    line(0, 0, x, y);
    textSize(14);
    text(ang + "°", x, y);
  }
  
  line(-r, 0, r, 0);
}

void drawLine(int idx) {
  strokeWeight(3);
  stroke(30,250,60);
  float radarSize = (width / 2.5 - 60) * radarScale;
  float r = radarSize / 2;
  
  int angleVal = constrain(iAngle[idx], 0, 180);
  float theta = radians(angleVal);
  float x = r * cos(theta);
  float y = -r * sin(theta);
  line(0, 0, x, y);
}

void drawObject(int idx) {
  strokeWeight(3);
  stroke(255,10,10);
  float radarSize = (width / 2.5 - 60) * radarScale;
  float r = radarSize / 2;
  int flag = 0;
  
  switch (idx) {
    case 0:
      flag = 30;
      break;
    case 1:
      flag = 60;
      break;
    case 2:
      flag = 90;
    default:
      break;
  }
  
  int d = max(iDistance[idx], -1);
  if (d >= 0 && d < 90) {
    float pixs = map(d, 0, flag, 0, r);
    pixs = constrain(pixs, 0, r);
    float theta = radians(constrain(iAngle[idx], 0, 180));
    float ox = pixs * cos(theta);
    float oy = -pixs * sin(theta);
    float rx = r * cos(theta);
    float ry = -r * sin(theta);
    line(ox, oy, rx, ry);
    point(ox, oy);
  }
}

void drawText(int idx) {
  fill(98,245,31);
  textSize(16);
  textAlign(CENTER, BOTTOM);

  float radarSize = (width / 2.5 - 60) * radarScale;
  float r = radarSize / 2;

  int ang = iAngle[idx];
  int dist = iDistance[idx];
  int flag1 = 0;
  
  switch (idx) {
    case 0:
      flag1 = 30;
      break;
    case 1:
      flag1 = 60;
      break;
    case 2:
      flag1 = 90;
    default:
      break;
  }
  String status = (dist > flag1 || dist < 0) ? "Out" : "In";

  float tx = -r + 250;
  float ty = -r + 300;

  String info = "Sensor " + nf(idx + 1, 1)
            + " | Angle: " + nf(ang, 3)
            + "° | Distance: " + nf(dist, 4);

  text(info, tx, ty);
  text("| " + status, tx + 190, ty);
}
