

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "settings.h" // paramètres wifi

typedef struct {
  int pin;
  int x, y;
  int time;
  char* lbl;
  } btn ;

char lbl_plus[] = "+";
char lbl_moins[] = "-";
char lbl_auto[] = "auto";
char lbl_numb[] = "auto";

char lbl_onOff[] = "On/Off";
char lbl_onOff5[] = "On/Off5";
char lbl_mode[] = "Mode";

btn boutons[] = {
  {.pin=D2, .x=75, .y=25, .time=100 , .lbl=lbl_plus},
  {.pin=D7, .x=25, .y=25, .time=100 , .lbl=lbl_moins},
  {.pin=D6, .x=50, .y=0, .time=100 , .lbl=lbl_auto},
  {.pin=D5, .x=50, .y=50, .time=100 , .lbl=lbl_numb},
  
  {.pin=D3, .x=50, .y=75, .time=1000, .lbl=lbl_onOff},
  {.pin=D3, .x=150, .y=75, .time=5000, .lbl=lbl_onOff5},
  {.pin=D1, .x=50, .y=100, .time=100 , .lbl=lbl_mode}
};

ESP8266WebServer server ( 80 );

char* homePage = NULL;
int homePageLen = 0;
int homePageAllocated = 0;
int bl_size = 1024 * 16;
void AddHomePage(String str){
  if (homePage == NULL) homePage = (char*)malloc(bl_size);
  int size = str.length();
  if (homePageLen + size + 1 >= homePageAllocated){
    homePageAllocated = bl_size * ( (homePageLen + size + 1) / bl_size + 1 );
    homePage = (char*)realloc(homePage, homePageAllocated);
  }
  strcpy(homePage + homePageLen, str.c_str());
  homePageLen += size;
}

void ServerHome() {
  server.send(200, "text/html", homePage);
  
}

void ServerRequest() {
  int id = server.arg("id").toInt();
  int pin = boutons[id].pin;
  Serial.printf("BOUTON HIGH: %d\n", id);
  digitalWrite(pin, HIGH);
  delay(boutons[id].time);
  digitalWrite(pin, LOW);
  Serial.printf("BOUTON LOW: %d\n", id);
  int state = server.arg("state").toInt();
  server.send(200, "text/plain", "{\"status\" : \"OK\"} ");
}

void WaitWifi() {
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
  }
  Serial.printf("[Wifi] OK\n");
}

void setup() {
  Serial.begin(115200);
  delay(10);
  Serial.setDebugOutput(true);
  WiFi.begin(ssid, password);
  WaitWifi();
  
  String WifiString(WiFi.localIP().toString());
      

  Serial.printf("IP : %s\n", WifiString.c_str());
  int ScaleX = 2, ScaleY = 3;

  int MinX = 100;
  int MinY = 100;
  
  int WIDTH = 120 * ScaleX + MinX;
  int HEIGHT = 120 * ScaleY + MinY;

  AddHomePage((String)"<html><head><style type=\"text/css\">input{ padding: 10px;\n");
  AddHomePage((String)" border-radius: 10px;background-color:#234;color:#FFF;border:1px solid #000;}\n");
  AddHomePage((String)"</style><script type=\"text/javascript\">\n");
  AddHomePage((String)"function btn(i){\n");
  AddHomePage((String)"  var oReq = new XMLHttpRequest();\n");
  AddHomePage((String)"  oReq.open(\"get\", \"/request?id=\"+i);\n");
  AddHomePage((String)"  oReq.send();\n");
  AddHomePage((String)"  }\n");
  AddHomePage((String)"</script><title>Télécommande!</title></head><body>");
  AddHomePage((String)"<div style=\"width:");
  AddHomePage((String)(WIDTH));
  AddHomePage((String)"px;height:");
  AddHomePage((String)(HEIGHT));
  AddHomePage((String)"px; left: 25px; top: 25px; position: absolute;border-radius: 10px; background-color:#000;\"></div>\n");
  int boutons_nbr = sizeof(boutons) / sizeof(boutons[0]);
  for (int i = 0; i < boutons_nbr; i ++){
    AddHomePage((String)"<input type=\"button\" style=\"transform: translate(-50%, -50%);position:absolute;left: ");
    AddHomePage((String)(MinX + boutons[i].x * ScaleX));
    AddHomePage((String)"px;top: ");
    AddHomePage((String)(MinY + boutons[i].y* ScaleY));
    AddHomePage((String)"px;\" value=\"");
    AddHomePage(boutons[i].lbl);
    AddHomePage((String)"\" onclick=\"btn(");
    AddHomePage((String)(i));
    AddHomePage((String)")\">\n");
  }
  AddHomePage((String)"</body></html>");
  
  for (int i = 0; i < boutons_nbr; i ++){
    pinMode(boutons[i].pin, OUTPUT);
    digitalWrite(boutons[i].pin, LOW);
  }
  
  server.on("/", ServerHome);
  server.on("/request", ServerRequest);
  server.begin();
}

void loop() {
  server.handleClient();
}
