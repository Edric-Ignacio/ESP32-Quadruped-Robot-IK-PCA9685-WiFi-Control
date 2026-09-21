#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <WiFi.h>
#include <WebServer.h>

const char* ssid     = "*********";
const char* password = "****************";

WebServer server(80);
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);

#define SERVO_MIN 150
#define SERVO_MAX 600

// COORDINATES
float x, y, z;
float xx, yy, zz;

// GAIT SETTINGS
float step_height = 5;
float step_length = 25;
float stepdx      = 1;
float ground      = 60;

// BODY TILT
// +Pitch = front rises, rear lowers | -Pitch = front lowers, rear rises
float pitch = 0;
// +Roll = left rises, right lowers | -Roll = right rises, left lowers
float roll = 0;

float bodyLength = 100;  // front-to-rear attachment distance (mm)
float bodyWidth  = 60;   // left-to-right attachment distance (mm)

// WALK GAIT ANGLES
float thetaH,  thetaT,  thetaK;
float thetaH2, thetaT2, thetaK2;
float thetaH3, thetaT3, thetaK3;
float thetaH4, thetaT4, thetaK4;

// LEFT GAIT ANGLES
float LthetaH,  LthetaT,  LthetaK;
float LthetaH2, LthetaT2, LthetaK2;
float LthetaH3, LthetaT3, LthetaK3;
float LthetaH4, LthetaT4, LthetaK4;

// RIGHT GAIT ANGLES
float RthetaH,  RthetaT,  RthetaK;
float RthetaH2, RthetaT2, RthetaK2;
float RthetaH3, RthetaT3, RthetaK3;
float RthetaH4, RthetaT4, RthetaK4;

// BACKWARD GAIT ANGLES
float BthetaH,  BthetaT,  BthetaK;
float BthetaH2, BthetaT2, BthetaK2;
float BthetaH3, BthetaT3, BthetaK3;
float BthetaH4, BthetaT4, BthetaK4;

// GAIT STATE
bool walkGait     = false;
bool leftGait     = false;
bool rightGait    = false;
bool backwardGait = false;
bool poseGait     = false;
bool standGait    = true;   // stand active by default

int n = 0;
int phase = 0;
int stepsThisPhase = 25;

unsigned long lastTick = 0;
unsigned long tickMs = 10;

// LEG TARGET COORDINATES
float frX, frY, frZ;  // Front Right
float flX, flY, flZ;  // Front Left
float brX, brY, brZ;  // Back Right
float blX, blY, blZ;  // Back Left

// WEB PAGE
const char PAGE[] PROGMEM = R"HTML(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Quadruped Control</title>
<style>
body{font-family:system-ui,sans-serif;background:#111;color:#eee;text-align:center;padding:30px 16px;margin:0;}
h1{font-size:20px;}
button{font-size:18px;padding:16px 28px;margin:6px;border:none;border-radius:10px;color:#fff;cursor:pointer;}
.go{background:#2e7d32;}
.stop{background:#c62828;}
.neutral{background:#37474f;}
label{display:block;margin-top:20px;font-size:14px;color:#aaa;}
input[type=range]{width:80%;max-width:320px;}
.value{color:#8bc34a;font-weight:bold;}
.control{margin-top:20px;}
#status{margin-top:24px;font-size:18px;color:#8bc34a;}
</style>
</head>
<body>
<h1>Quadruped Control</h1>

<button class="go" onclick="go('/walk')">WALK</button>
<button class="go" onclick="go('/left')">LEFT</button>
<button class="go" onclick="go('/right')">RIGHT</button>
<button class="go" onclick="go('/backward')">BACKWARD</button>
<br>
<button class="neutral" onclick="go('/pose')">POSE</button>
<button class="stop" onclick="go('/stop')">STOP</button>
<button class="neutral" onclick="go('/stand')">STAND</button>

<div class="control">
<label>Step length: <span class="value" id="lv">25</span> mm</label>
<input type="range" min="5" max="50" value="25"
  oninput="document.getElementById('lv').innerText=this.value;go('/set?len='+this.value)">
</div>

<div class="control">
<label>Step height: <span class="value" id="hv">5</span> mm</label>
<input type="range" min="1" max="20" value="5"
  oninput="document.getElementById('hv').innerText=this.value;go('/set?ht='+this.value)">
</div>

<div class="control">
<label>Ground height: <span class="value" id="gv">60</span> mm</label>
<input type="range" min="30" max="100" value="60"
  oninput="document.getElementById('gv').innerText=this.value;go('/set?gr='+this.value)">
</div>

<div class="control">
<label>Speed: <span class="value" id="sv">10</span> ms per increment</label>
<input type="range" min="2" max="40" value="10"
  oninput="document.getElementById('sv').innerText=this.value;go('/set?tick='+this.value)">
</div>

<div class="control">
<label>Pitch: <span class="value" id="pv">0</span> &deg;</label>
<input type="range" min="-15" max="15" value="0" step="1"
  oninput="document.getElementById('pv').innerText=this.value;go('/set?pitch='+this.value)">
</div>

<div class="control">
<label>Roll: <span class="value" id="rv">0</span> &deg;</label>
<input type="range" min="-15" max="15" value="0" step="1"
  oninput="document.getElementById('rv').innerText=this.value;go('/set?roll='+this.value)">
</div>

<div id="status">connecting...</div>

<script>
function go(u){
  fetch(u).then(r=>r.text())
  .then(t=>{document.getElementById('status').innerText=t;})
  .catch(()=>{document.getElementById('status').innerText='offline';});
}
setInterval(()=>{go('/status');},1500);
go('/status');
</script>
</body>
</html>
)HTML";

// STATUS TEXT
String statusText() {
    if (walkGait)     return "WALK GAIT";
    if (leftGait)     return "LEFT GAIT";
    if (rightGait)    return "RIGHT GAIT";
    if (backwardGait) return "BACKWARD GAIT";
    if (poseGait)     return "POSE";
    if (standGait)    return "STANDING";
    return "STOPPED";
}

// APPLY BODY PITCH AND ROLL
void applyTilt(
    float &frX, float &frY, float &frZ,
    float &flX, float &flY, float &flZ,
    float &brX, float &brY, float &brZ,
    float &blX, float &blY, float &blZ
) {
    float pitchRad = pitch * PI / 180.0;
    float rollRad  = roll  * PI / 180.0;

    // PITCH: +Pitch -> front up, rear down
    float frontPitch =  tan(pitchRad) * (bodyLength / 2.0);
    float rearPitch  = -tan(pitchRad) * (bodyLength / 2.0);
    frZ += frontPitch;  flZ += frontPitch;
    brZ += rearPitch;   blZ += rearPitch;

    // ROLL: +Roll -> left up, right down
    float leftRoll  =  tan(rollRad) * (bodyWidth / 2.0);
    float rightRoll = -tan(rollRad) * (bodyWidth / 2.0);
    frZ += rightRoll;  brZ += rightRoll;   // right side
    flZ += leftRoll;   blZ += leftRoll;    // left side
}

// STAND (live: re-reads ground/pitch/roll each call)
void standStill() {
    frX = 0; frY = 10; frZ = ground;
    flX = 0; flY = 10; flZ = ground;
    brX = 0; brY = 10; brZ = ground;
    blX = 0; blY = 10; blZ = ground;

    applyTilt(
        frX,frY,frZ,
        flX,flY,flZ,
        brX,brY,brZ,
        blX,blY,blZ
    );

    move   (0,1,2,    frX,frY,frZ, &thetaH, &thetaT, &thetaK );
    movee  (4,5,6,    flX,flY,flZ, &thetaH2,&thetaT2,&thetaK2);
    moveee (8,9,10,   brX,brY,brZ, &thetaH3,&thetaT3,&thetaK3);
    moveeee(12,13,14, blX,blY,blZ, &thetaH4,&thetaT4,&thetaK4);
}

// POSE
void poseStill() {
    move   (0,1,2,    0,10,60+40, &thetaH, &thetaT, &thetaK );
    movee  (4,5,6,    0,10,60+40, &thetaH2,&thetaT2,&thetaK2);
    moveee (8,9,10,   0,10,30, &thetaH3,&thetaT3,&thetaK3);
    moveeee(12,13,14, 0,10,30, &thetaH4,&thetaT4,&thetaK4);
}

// GAIT BASE
void gaitBase(float &x, float &y, float &z, float &xx, float &yy, float &zz) {
    if (phase == 0) {
        x  = -step_length / 2 + stepdx * n;
        y  = 10;
        z  = ground - step_height - step_height * cos(x / step_length * PI);
        xx =  step_length / 2 - stepdx * n;
        yy = 10;
        zz = ground;
    } else {
        x  =  step_length / 2 - stepdx * n;
        y  = 10;
        z  = ground;
        xx = -step_length / 2 + stepdx * n;
        yy = 10;
        zz = ground - step_height - step_height * cos(x / step_length * PI);
    }
}

// WALK
void gaitTickWalk() {
    gaitBase(x,y,z, xx,yy,zz);
    frX = x;  frY = y;  frZ = z;
    flX = xx; flY = yy; flZ = zz;
    brX = xx; brY = yy; brZ = zz;
    blX = x;  blY = y;  blZ = z;

    applyTilt(frX,frY,frZ, flX,flY,flZ, brX,brY,brZ, blX,blY,blZ);

    move   (0,1,2,    frX,frY,frZ, &thetaH, &thetaT, &thetaK );
    movee  (4,5,6,    flX,flY,flZ, &thetaH2,&thetaT2,&thetaK2);
    moveee (8,9,10,   brX,brY,brZ, &thetaH3,&thetaT3,&thetaK3);
    moveeee(12,13,14, blX,blY,blZ, &thetaH4,&thetaT4,&thetaK4);

    n++;
    if (n > stepsThisPhase) { n = 0; phase = 1 - phase; }
}

// LEFT
void gaitTickLeft() {
    gaitBase(x,y,z, xx,yy,zz);
    frX =  x;  frY = y;  frZ = z;
    flX = -xx; flY = yy; flZ = zz;
    brX =  xx; brY = yy; brZ = zz;
    blX = -x;  blY = y;  blZ = z;

    applyTilt(frX,frY,frZ, flX,flY,flZ, brX,brY,brZ, blX,blY,blZ);

    move   (0,1,2,    frX,frY,frZ, &LthetaH, &LthetaT, &LthetaK );
    movee  (4,5,6,    flX,flY,flZ, &LthetaH2,&LthetaT2,&LthetaK2);
    moveee (8,9,10,   brX,brY,brZ, &LthetaH3,&LthetaT3,&LthetaK3);
    moveeee(12,13,14, blX,blY,blZ, &LthetaH4,&LthetaT4,&LthetaK4);

    n++;
    if (n > stepsThisPhase) { n = 0; phase = 1 - phase; }
}

// RIGHT
void gaitTickRight() {
    gaitBase(x,y,z, xx,yy,zz);
    frX = -x;  frY = y;  frZ = z;
    flX =  xx; flY = yy; flZ = zz;
    brX = -xx; brY = yy; brZ = zz;
    blX =  x;  blY = y;  blZ = z;

    applyTilt(frX,frY,frZ, flX,flY,flZ, brX,brY,brZ, blX,blY,blZ);

    move   (0,1,2,    frX,frY,frZ, &RthetaH, &RthetaT, &RthetaK );
    movee  (4,5,6,    flX,flY,flZ, &RthetaH2,&RthetaT2,&RthetaK2);
    moveee (8,9,10,   brX,brY,brZ, &RthetaH3,&RthetaT3,&RthetaK3);
    moveeee(12,13,14, blX,blY,blZ, &RthetaH4,&RthetaT4,&RthetaK4);

    n++;
    if (n > stepsThisPhase) { n = 0; phase = 1 - phase; }
}

// BACKWARD
void gaitTickBackward() {
    gaitBase(x,y,z, xx,yy,zz);
    frX = -x;  frY = y;  frZ = z;
    flX = -xx; flY = yy; flZ = zz;
    brX = -xx; brY = yy; brZ = zz;
    blX = -x;  blY = y;  blZ = z;

    applyTilt(frX,frY,frZ, flX,flY,flZ, brX,brY,brZ, blX,blY,blZ);

    move   (0,1,2,    frX,frY,frZ, &BthetaH, &BthetaT, &BthetaK );
    movee  (4,5,6,    flX,flY,flZ, &BthetaH2,&BthetaT2,&BthetaK2);
    moveee (8,9,10,   brX,brY,brZ, &BthetaH3,&BthetaT3,&BthetaK3);
    moveeee(12,13,14, blX,blY,blZ, &BthetaH4,&BthetaT4,&BthetaK4);

    n++;
    if (n > stepsThisPhase) { n = 0; phase = 1 - phase; }
}

// Helper: clear every gait flag
void clearGaits() {
    walkGait = leftGait = rightGait = backwardGait = poseGait = standGait = false;
}

// SETUP
void setup() {
    Serial.begin(115200);

    Wire.begin();
    pwm.begin();
    pwm.setPWMFreq(50);

    stepsThisPhase = (int)(step_length / stepdx);

    standStill();

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(400);
        Serial.print(".");
    }
    Serial.print("\nOpen: http://");
    Serial.println(WiFi.localIP());

    // MAIN PAGE
    server.on("/", []() {
        server.send_P(200, "text/html", PAGE);
    });

    // WALK
    server.on("/walk", []() {
        clearGaits(); walkGait = true;
        n = 0; phase = 0; lastTick = millis();
        server.send(200, "text/plain", "WALK GAIT");
    });

    // LEFT
    server.on("/left", []() {
        clearGaits(); leftGait = true;
        n = 0; phase = 0; lastTick = millis();
        server.send(200, "text/plain", "LEFT GAIT");
    });

    // RIGHT
    server.on("/right", []() {
        clearGaits(); rightGait = true;
        n = 0; phase = 0; lastTick = millis();
        server.send(200, "text/plain", "RIGHT GAIT");
    });

    // BACKWARD
    server.on("/backward", []() {
        clearGaits(); backwardGait = true;
        n = 0; phase = 0; lastTick = millis();
        server.send(200, "text/plain", "BACKWARD GAIT");
    });

    // POSE
    server.on("/pose", []() {
        clearGaits(); poseGait = true;
        n = 0; phase = 0;
        poseStill();
        server.send(200, "text/plain", "POSE");
    });

    // STOP
    server.on("/stop", []() {
        clearGaits();
        server.send(200, "text/plain", "STOPPED");
    });

    // STAND (now a live state)
    server.on("/stand", []() {
        clearGaits(); standGait = true;
        n = 0; phase = 0; lastTick = millis();
        standStill();
        server.send(200, "text/plain", "STANDING");
    });

    // SETTINGS
    server.on("/set", []() {
        if (server.hasArg("len")) {
            step_length = constrain(server.arg("len").toFloat(), 5, 50);
            stepsThisPhase = (int)(step_length / stepdx);
        }
        if (server.hasArg("ht"))    step_height = constrain(server.arg("ht").toFloat(), 1, 20);
        if (server.hasArg("tick"))  tickMs      = constrain(server.arg("tick").toInt(), 2, 40);
        if (server.hasArg("gr"))    ground      = constrain(server.arg("gr").toFloat(), 20, 120);
        if (server.hasArg("pitch")) pitch       = constrain(server.arg("pitch").toFloat(), -15, 15);
        if (server.hasArg("roll"))  roll        = constrain(server.arg("roll").toFloat(), -15, 15);
        server.send(200, "text/plain", statusText());
    });

    // STATUS
    server.on("/status", []() {
        server.send(200, "text/plain", statusText());
    });

    server.begin();
}

// MAIN LOOP
void loop() {
    server.handleClient();

    if (millis() - lastTick >= tickMs) {
        lastTick = millis();

        if (walkGait)     gaitTickWalk();
        if (leftGait)     gaitTickLeft();
        if (rightGait)    gaitTickRight();
        if (backwardGait) gaitTickBackward();
        if (poseGait)     poseStill();
        if (standGait)    standStill();   // live stand: re-applies ground/pitch/roll
    }
}
