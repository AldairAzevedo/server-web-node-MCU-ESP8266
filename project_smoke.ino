//Programa: NodeMCU e Q2 - Controle e Monitoramento IoT Fumaça e Gás Inflamável
//Autor: Aldair Azevedo

#include <ESP8266WiFi.h>        // Importa a Biblioteca ESP8266WiFi
#include <PubSubClient.h>       // Importa a Biblioteca PubSubClient
#include <ESP8266HTTPClient.h>  //Importa a lib HTTP
#include <ArduinoJson.h>        // Importa Lib JSON
#include <string.h>             //Importar lib de String

//defines - mapeamento de pinos do NodeMCU
#define D0 16
#define D1 5
#define D2 4
#define D3 0
#define D4 2
#define D5 14
#define D6 12
#define D7 13
#define D8 15
#define D9 3
#define D10 1

//int Gas_analog = A0;    // used for ESP8266
//int Gas_digital = D1;   // used for ESP8266

int Gas_analog = 0;   // used for ESP32
int Gas_digital = 5;  // used for ESP32


// WIFI
const char* SSID = "Redmi Note 9";          // SSID / nome da rede WI-FI que deseja se conectar
const char* PASSWORD = "44559900";  // Senha da rede WI-FI que deseja se conectar

//Variáveis e objetos globais
WiFiClient espClient;    // Cria o objeto espClient
char EstadoSaida = '0';  //variável que armazena o estado atual da saída

int id = 'd74cc88b-e7cf-4a5d-b1ed-1a06ba9d26a9';
String status = "SUCCESS";
HTTPClient http;  // Cria o objeto espClient

//Prototypes
void initSerial();
void initWiFi();
void reconectWiFi();
void VerificaConexaoWiFI(void);
void InitOutput(void);

/* 
 *  Implementações das funções
 */
void setup() {
  //inicializações:
  InitOutput();
  initSerial();
  initWiFi();
  pinMode(Gas_digital, INPUT);
  PostToApi();
}

//Função: inicializa comunicação serial com baudrate 115200 (para fins de monitorar no terminal serial
//        o que está acontecendo.
//Parâmetros: nenhum
//Retorno: nenhum
void initSerial() {
  Serial.begin(115200);
}

//Função: inicializa e conecta-se na rede WI-FI desejada
//Parâmetros: nenhum
//Retorno: nenhum
void initWiFi() {
  delay(10);
  Serial.println("------Conexao WI-FI-------");
  Serial.print("Conectando-se na rede: ");
  Serial.println(SSID);
  Serial.println("Aguarde...");

  reconectWiFi();
}

//Função: reconecta-se ao WiFi
//Parâmetros: nenhum
//Retorno: nenhum
void reconectWiFi() {
  //se já está conectado a rede WI-FI, nada é feito.
  //Caso contrário, são efetuadas tentativas de conexão
  if (WiFi.status() == WL_CONNECTED)
    return;

  WiFi.begin(SSID, PASSWORD);  // Conecta na rede WI-FI

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Conectado com sucesso na rede ");
  Serial.print(SSID);
  Serial.println("IP obtido: ");
  Serial.println(WiFi.localIP());
}

//Função: verifica o estado das conexões WiFI
//        Em caso de desconexão (qualquer uma das duas), a conexão
//        é refeita.
//Parâmetros: nenhum
//Retorno: nenhum
void VerificaConexaoWiFI(void) {
  reconectWiFi();  //se não há conexão com o WiFI, a conexão é refeita
}

// Post para a API
void PostToApi() {
  StaticJsonBuffer<300> JSONbuffer;
  JsonObject& JSONencoder = JSONbuffer.createObject();
  JSONencoder["userId"] = "2fc050e5-068b-4280-8ded-cd2e4cc7051c";
  JSONencoder["status"] = status;
  char JSONmessageBuffer[300];
  JSONencoder.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
  //Serial.println(JSONmessageBuffer);

  http.begin(espClient, "http://apissmokee.herokuapp.com/status");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("auth-key", "My_authentication_key");

  int httpCode = http.POST(JSONmessageBuffer);
  String payload = http.getString();
  http.end();

  Serial.print("HTTP POST:");
  Serial.print("---------");
  Serial.print(httpCode);
  Serial.print("---------");
  Serial.print(payload);
  Serial.print("---------");
}

//Função: inicializa o output em nível lógico baixo
//Parâmetros: nenhum
//Retorno: nenhum
void InitOutput(void) {
  //IMPORTANTE: o Led já contido na placa é acionado com lógica invertida (ou seja,
  //enviar HIGH para o output faz o Led apagar / enviar LOW faz o Led acender)
  pinMode(D0, OUTPUT);
  digitalWrite(D0, LOW);
}


//programa principal
void loop() {
  //garante funcionamento das conexões WiFi
  VerificaConexaoWiFI();

  int gassensorAnalog = analogRead(Gas_analog);
  int gassensorDigital = digitalRead(Gas_digital);
  String ultimoStatus;

  Serial.print("Gas Sensor: ");
  Serial.print(gassensorAnalog);
  Serial.print("\t");
  Serial.print("Gas Class: ");
  Serial.print(gassensorDigital);
  Serial.print("\t");
  Serial.print("\t");

  if (gassensorDigital != 0 && gassensorAnalog < 120) {
    status = "SUCCESS";
    PostToApi();
    Serial.println("Sem possibilidades de fogo 0% - " + status);
  } else if (gassensorDigital != 0 && gassensorAnalog >= 120 && gassensorAnalog < 150) {
    status = "INFO";
    PostToApi();
    Serial.println("Deteccao de gas 20% - " + status);
  } else if (gassensorDigital != 0 && gassensorAnalog >= 150 && gassensorAnalog < 200) {
    status = "WARNING";
    PostToApi();
    Serial.println("Média deteccao de gas 60% - " + status);
  } else {
    status = "DANGER";
    PostToApi();
    Serial.println("Alta deteccao de gas 100% - " + status);
  }

  delay(1000);
}