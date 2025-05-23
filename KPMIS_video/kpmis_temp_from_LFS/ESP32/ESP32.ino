#include <WiFi.h>
#include <WebServer.h>

/* Установите здесь свои SSID и пароль */
const char* ssid = "КПМИС";  
const char* password = "12345678";  

/* Настройки IP адреса */
IPAddress local_ip(192,168,2,1);
IPAddress gateway(192,168,2,1);
IPAddress subnet(255,255,255,0);
WebServer server(80);

String esp = "";//Строка для считвания строки
int Data_temp = 0;//Строка для LFS датчика
char c; //Считываемый символ

void setup() {
  Serial.begin(115200);//Сериал для общения с AtMega
  //Инциируем wifi-модуль
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(100);
  //Обработчик для начальной странице
  server.on("/", handle_OnConnect);
  server.onNotFound(handle_NotFound);
  //Запускаем Web-Server
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  ReadAtMega();
  server.handleClient();
}

void handle_OnConnect() {
  server.send(200, "text/html", SendHTML(Data_temp)); 
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

void ReadAtMega(){
  //Считываем информацию с AtMega
  while(Serial.available()){
    c = Serial.read(); //Записываем символм
    esp += c; //Закидываем в строку
    delay(50);
  }
  delay(500);
  if(esp.length() > 0) Data_temp = esp.toInt(); //Если строка сформировалось тогда записываем информацию в другую переменную
  esp = ""; //Очищаем строку для нового чтения
}

String SendHTML(int Data_temp){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<meta http-equiv=\"Content-type\" content=\"text/html; charset=utf-8\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>Данные с LFS датчика</title>\n";
  ptr += "<meta http-equiv=\"refresh\" content=\"1\">\n"; // Страница обновляется каждые 3 секунд
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr +=".button {display: block;width: 80px;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr +=".button-on {background-color: #3498db;}\n";
  ptr +=".button-on:active {background-color: #2980b9;}\n";
  ptr +=".button-off {background-color: #34495e;}\n";
  ptr +=".button-off:active {background-color: #2c3e50;}\n";
  ptr +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<h1>ESP32. Данные с мультидатчика</h1>\n";
  ptr +="<p>Температура: <span style=\"color: red;\"> ";
  ptr +=(int)Data_temp/10;
  ptr +="°C</p>";
  if (Data_temp/10<25)ptr +="<p>Вода <span style=\"color: blue;\">холодная";
  else ptr +="<p>Вода <span style=\"color: red;\">горячая";
  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}
