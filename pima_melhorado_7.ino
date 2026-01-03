#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <time.h>

// Configurações Wi-Fi
const char* ssid = "NOME_DA_REDE";
const char* password = "SENHA_DA_REDE";

// IP fixo
IPAddress local_IP(192, 168, 0, 101);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(192, 168, 0, 1);
IPAddress secondaryDNS(8, 8, 4, 4);

// Configuração de Hora (Brasília)
int timezone = -3 * 3600;
int dst = 0;

WiFiServer server(80);

// Variáveis do Medidor
int M, tamanho, escopo, indice;
byte DD1, DD2, DD3, DD4, DD5, und, dez, atv1, atv2, atv3;
unsigned long identidade = 0, atv = 0;

// Variáveis de Armazenamento (EEPROM)
unsigned long atvParcial, atvMensal;
time_t dataParcial, dataMensal;
float precokw; 

// Endereços EEPROM: 0:atvP(4), 4:dataP(4), 8:atvM(4), 12:dataM(4), 16:precokw(4)
#define EEPROM_SIZE 64

void carregarDados() {
  EEPROM.get(0, atvParcial);
  EEPROM.get(4, dataParcial);
  EEPROM.get(8, atvMensal);
  EEPROM.get(12, dataMensal);
  EEPROM.get(16, precokw);
  
  // Proteção: se a memória estiver vazia (FF) ou inválida
  if (atvParcial == 0xFFFFFFFF) atvParcial = 0;
  if (atvMensal == 0xFFFFFFFF) atvMensal = 0;
  if (isnan(precokw) || precokw <= 0) precokw = 1.22; // Valor padrão inicial
}

String formatarData(time_t t) {
  if (t <= 0 || t == 0xFFFFFFFF) return "Sem registro";
  struct tm * timeinfo;
  timeinfo = localtime(&t);
  char buffer[20];
  strftime(buffer, sizeof(buffer), "%d/%m/%Y %H:%M", timeinfo);
  return String(buffer);
}

void setup() {
  Serial.begin(2400);

  WiFi.mode(WIFI_STA);
  if(!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)){
      // Falha no IP fixo
  }

  WiFi.begin(ssid, password);
  int count = 0;
  while(WiFi.status() != WL_CONNECTED && count < 20){
      delay(500);
      count++;
  }

  configTime(timezone, dst, "pool.ntp.org", "time.nist.gov");
  server.begin();

  EEPROM.begin(EEPROM_SIZE);
  carregarDados();
}

void loop() {
  // Mantém conexão Wi-Fi ativa sem travar o código
  static unsigned long lastCheck = 0;
  if (millis() - lastCheck > 10000) {
    if (WiFi.status() != WL_CONNECTED) WiFi.begin(ssid, password);
    lastCheck = millis();
  }

  processarSerial();
  atenderWeb();
}

void processarSerial() {
  while (Serial.available() > 0) {
    yield(); // Alimenta o sistema para evitar desconexão Wi-Fi
    M = Serial.read();
    if (M == 170) { // 0xAA
      if (Serial.read() == 85) { // 0x55
        // Identidade
        DD1 = Serial.read(); dez=DD1>>4; und=DD1<<4; und=und>>4; DD1=(dez*10)+(und);
        DD2 = Serial.read(); dez=DD2>>4; und=DD2<<4; und=und>>4; DD2=(dez*10)+(und);
        DD3 = Serial.read(); dez=DD3>>4; und=DD3<<4; und=und>>4; DD3=(dez*10)+(und);
        DD4 = Serial.read(); dez=DD4>>4; und=DD4<<4; und=und>>4; DD4=(dez*10)+(und);
        DD5 = Serial.read(); dez=DD5>>4; und=DD5<<4; und=und>>4; DD5=(dez*10)+(und);
        identidade = (DD5)+(DD4*100)+(DD3*10000)+(DD2*1000000)+(DD1*100000000);

        tamanho = (Serial.read() - 2);
        escopo = Serial.read();
        indice = Serial.read();

        if (indice == 2) { // Energia Ativa
          atv1 = Serial.read(); dez=atv1>>4; und=atv1<<4; und=und>>4; atv1=(dez*10)+(und);
          atv2 = Serial.read(); dez=atv2>>4; und=atv2<<4; und=und>>4; atv2=(dez*10)+(und);
          atv3 = Serial.read(); dez=atv3>>4; und=atv3<<4; und=und>>4; atv3=(dez*10)+(und);
          atv = (atv3)+(atv2*100)+(atv1*10000);
        }
      }
    }
  }
}

void atenderWeb() {
  WiFiClient client = server.available();
  if (!client) return;

  String request = client.readStringUntil('\r');
  client.flush();

  bool flagSalvar = false;
  time_t agora = time(nullptr);

  // Ações via URL
  if (request.indexOf("?salvarP=1") != -1) {
    atvParcial = atv; dataParcial = agora;
    EEPROM.put(0, atvParcial); EEPROM.put(4, dataParcial);
    flagSalvar = true;
  }
  if (request.indexOf("?salvarM=1") != -1) {
    atvMensal = atv; dataMensal = agora;
    EEPROM.put(8, atvMensal); EEPROM.put(12, dataMensal);
    flagSalvar = true;
  }
  if (request.indexOf("?nPreco=") != -1) {
    int pos = request.indexOf("?nPreco=") + 8;
    String val = request.substring(pos, request.indexOf(" ", pos));
    precokw = val.toFloat();
    EEPROM.put(16, precokw);
    flagSalvar = true;
  }

  if (flagSalvar) {
    EEPROM.commit();
    delay(50); 
  }

  // Resposta HTML
  String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
  s += "<!DOCTYPE html><html><head><meta charset='utf-8' name='viewport' content='width=device-width, initial-scale=1'>";
  s += "<style>body{font-family:sans-serif; text-align:center; background:#f4f4f4; color:#333;}";
  s += ".box{border:1px solid #ddd; padding:15px; margin:10px auto; max-width:400px; border-radius:10px; background:#fff; box-shadow: 2px 2px 10px rgba(0,0,0,0.1);}";
  s += "button, input[type=submit]{padding:10px 20px; border:none; border-radius:5px; cursor:pointer; font-weight:bold;}";
  s += ".btn-p{background:#607d8b; color:white;} .btn-m{background:#4CAF50; color:white;}";
  s += "input[type=number]{padding:8px; width:100px; border:1px solid #ccc; border-radius:5px;}</style></head><body>";

  s += "<h2>Monitor de Energia</h2>";
  s += "<p>ID Medidor: <b>" + String(identidade) + "</b></p>";
  s += "<div class='box' style='background:#2196F3; color:white;'><h3>Leitura Atual: " + String(atv) + " kWh</h3></div>";

  // Configuração de Preço
  s += "<div class='box'><h3>Tarifa Atual: R$ " + String(precokw) + "</h3>";
  s += "<form action='/' method='get'>R$ <input type='number' step='0.01' name='nPreco' value='" + String(precokw) + "'>";
  s += " <input type='submit' value='Alterar'></form></div>";

  // Parcial
  s += "<div class='box'><h3>CONSUMO PARCIAL</h3>";
  s += "<p>Desde: " + formatarData(dataParcial) + " (" + String(atvParcial) + " kWh)</p>";
  s += "<h1>" + String(atv - atvParcial) + " kWh</h1>";
  s += "<p style='color:green; font-size:1.2em;'><b>R$ " + String((atv - atvParcial) * precokw) + "</b></p>";
  s += "<a href='/?salvarP=1'><button class='btn-p'>Resetar Parcial</button></a></div>";

  // Mensal
  s += "<div class='box' style='border-left: 5px solid #4CAF50;'><h3>CONSUMO MENSAL</h3>";
  s += "<p>Iniciado: " + formatarData(dataMensal) + " (" + String(atvMensal) + " kWh)</p>";
  s += "<h1>" + String(atv - atvMensal) + " kWh</h1>";
  s += "<p style='color:green; font-size:1.2em;'><b>R$ " + String((atv - atvMensal) * precokw) + "</b></p>";
  s += "<a href='/?salvarM=1'><button class='btn-m'>Fechar M&ecirc;s</button></a></div>";

  s += "</body></html>";
  client.print(s);
  delay(1);
  client.stop();
}
