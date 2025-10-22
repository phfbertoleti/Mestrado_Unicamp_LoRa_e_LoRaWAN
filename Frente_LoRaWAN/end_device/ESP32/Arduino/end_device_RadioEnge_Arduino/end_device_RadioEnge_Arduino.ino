/* Programa: end device básico para comunicação com gateway LoRaWAN.
 *           Software desenvolvido para ser gravado em um ESP32-WROOM32
 *           e usar como módulo LoRaWAN o Módulo LoRaWAN RadioEnge
 *           (https://www.radioenge.com.br/produto/modulo-lorawan/)
 * 
 * Data: Outubro/2025
 *  Autor: Lucas Fernandes (lucasfernandes94lf@gmail.com)
 */
#include <Arduino.h>

#define LORA_TX 22
#define LORA_RX 23

HardwareSerial LoRaSerial(1);

// Credenciais ABP (no formato aceito pelo módulo LoRaWAN RadioEnge)
const char* DEVADDR = "26:0D:43:45";
const char* NWKSKEY = "D9:B2:0A:74:27:CF:C2:A4:4C:42:8C:E6:9D:5D:97:C7";
const char* APPSKEY = "AB:DE:71:8B:0D:AC:8F:0B:17:87:BE:0B:76:68:54:BE";

void enviaComandoLoRa(const String& comando, unsigned long timeout = 3000)
{
  LoRaSerial.print(comando + "\r\n");

  Serial.print("[COMANDO] ");
  Serial.println(comando);

  unsigned long start = millis();
  while (millis() - start < timeout) {
    while (LoRaSerial.available()) {
      char c = LoRaSerial.read();
      Serial.write(c);
    }
  }
  Serial.println();
}

void setup()
{
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n[START] Inicializando UART com módulo LoRa Radioenge...");
  LoRaSerial.begin(9600, SERIAL_8N1, LORA_RX, LORA_TX);
  delay(1000);
  while (LoRaSerial.available()) LoRaSerial.read();

  enviaComandoLoRa("AT+DEUI=?");                                // Informa o DevEUI
  enviaComandoLoRa("AT");                                       // Teste básico
  enviaComandoLoRa("AT+NJM=0");                                 // ABP mode
  enviaComandoLoRa("AT+CLASS=A");                               // Classe A
  enviaComandoLoRa("AT+ADR=0");                                 // ADR desativado
  enviaComandoLoRa("AT+DR=5");                                  // DataRate
  enviaComandoLoRa("AT+DADDR=" + String(DEVADDR));              // Device address
  enviaComandoLoRa("AT+NWKSKEY=" + String(NWKSKEY));            //
  enviaComandoLoRa("AT+APPSKEY=" + String(APPSKEY));            //
  enviaComandoLoRa("AT+CHMASK=0100:0000:0000:0000:0000:0000");  // Canal 2 (TTN BR/AU915)

  Serial.println("[READY] Pronto para enviar uplinks.");
  delay(2000);
}

void loop() {
  static unsigned long ultimaMensagem = 0;
  const unsigned long intervalo = 60000;

  if (millis() - ultimaMensagem > intervalo) {
    ultimaMensagem = millis();

    // Payload em HEX
    String payloadHex = "3:010203";

    Serial.println("[MSG] Enviando: " + payloadHex);
    enviaComandoLoRa("AT+SENDB=" + payloadHex, 5000);
  }

  while (LoRaSerial.available()) {
    Serial.write(LoRaSerial.read());
  }
}
