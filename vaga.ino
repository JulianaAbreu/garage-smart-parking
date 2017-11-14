#include <SPI.h>
#include <UIPEthernet.h>
#include <PubSubClient.h>
#include <Ultrasonic.h>

// Tempo para reconexão
long timeCon = 0;
int intervaloCon = 2000;

// Tempo para verificar vagas
long timeVagas = 0;
int intervaloVagas = 1000;

// pino ao qual está conectado nosso LED
const int LED_FREE = 5;
const int LED_OCCUPED = 4;
#define pinoConexao  A2

// Atualizar ultimo valor para ID do seu Kit para evitar duplicatas
const byte mac[] = { 0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0x08 };

// Informações de conexão mqtt
IPAddress MQTT_SERVER (192, 168, 3, 186);
const int port = 1883;

// Identificação única do cliente, trocar número pelo id de seu kit
const char* clientId = "arduino-08";

// tópico mqtt com prefixos para garantir que seja único no servidor que utilizamos
// porque test.mosquitto.org é público
const char* topic = "vagas/08";

Ultrasonic ultrasonic2 (9, 8);


void ligarDesligarLED(int ligar) {
  if (ligar) {
    digitalWrite(LED_FREE, HIGH);
  } else {
    digitalWrite(LED_FREE, LOW);
  }
}

void ligarDesligarLED_OCCUPED(int ligar) {
  if (ligar) {
    digitalWrite(LED_OCCUPED, HIGH);
  } else {
    digitalWrite(LED_OCCUPED, LOW);
  }
}

EthernetClient ethernetClient;
PubSubClient client(MQTT_SERVER, port, callback, ethernetClient);

void atualizarTopico(int estado) {
  // Este método publica mensagens com a configuração Retain = true,
  // tornando suficiente publicar somente quando a lâmpada de fato muda de estado
  // ao invés de periodicamente

  if (estado) {
    client.publish(topic, "1", true);
  } else {
    client.publish(topic, "0", true);
  }
}

void atualizarTopicoOccuped (int state) {
  if (state) {
    client.publish(topic, "1", true);
  } else {
    client.publish(topic, "0", true);
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  // Truque para converter rapidamente o valor do primeiro caractere da mensagem
  // para seu valor numérico
  // Isto assume que o primeiro caractere é um dígito [0-9]

  //payload[length] = 0;
  //char* mensagem = payload;
  //Serial.println("Mensagem:");
  //Serial.println("");
  //Serial.flush();
  //int payloadAsInt = payload[0] - '0';
  //Serial.println(payloadAsInt);
  //ligarDesligarLED(payloadAsInt);
  //atualizarTopico(payloadAsInt);


  // int payloadAsIntOccuped = payload[0] - '0';
  // Serial.println(payloadAsIntOccuped);
  // ligarDesligarLED_OCCUPED(payloadAsIntOccuped);
  // ligarDesligarLED_OCCUPED(payloadAsIntOccuped);

}


// Função que conecta ao broker mqtt
int mqttConnect() {
  // Conexão do cliente MQTT com configuração de topico Will:
  // A linha seguinte configura o broker mqtt para que publique a seguinte mensagem
  // caso este cliente se desconecte abruptamente:

  // topico: "senai/codexp/42/luz"
  // QoS: 0
  // Retain: true
  // Payload: vazio

  // O payload vazio garante que a última mensagem publicada com Retain = true
  // seja removida da memória do broker

  if (client.connect(clientId, topic, 0, true, "")) {
    client.subscribe(topic);
    Serial.println("mqtt");

  }
  return client.connected();
}

void setup() {
  // Inicialização da comunicação serial
  Serial.begin(9600);
  while (!Serial) {}
  reconnectMQTT();

  // Inicialização da placa ethernet
  if (Ethernet.begin(mac)) {
    Serial.print("Endereço IP: ");
    Serial.println(Ethernet.localIP());
  }
  if (client.connect(clientId, topic, 0, true, "")) {
    client.subscribe(topic);
  }
  Serial.println("Conectado");
  return client.connected();

  pinMode(LED_OCCUPED, OUTPUT);
  pinMode(LED_FREE, OUTPUT);
  mqttConnect();

}

void loop() {
  verificaConexaoEMQTT(); //garante funcionamento da conexão ao broker MQTT
  client.loop();
  if (!client.connected()) {
    mqttConnect();
  }
  client.loop();

  int distancia2 = ultrasonic2.distanceRead();

  Serial.print("Distance in CM: ");
  Serial.println(distancia2);
  delay(1000);


  if (distancia2 <= 7 && distancia2 != 0) {
    Serial.println("1");
    client.publish(topic, "1", true);
    //ocupado
    digitalWrite(LED_OCCUPED, HIGH);
    digitalWrite(LED_FREE, LOW);
  } else {
    Serial.println("0");
    client.publish(topic, "0", true);
    //livre
    digitalWrite(LED_FREE, HIGH);
    digitalWrite(LED_OCCUPED, LOW);
   

    //counter = counter+1;
    //Serial.println(counter);
    //Serial.println("Vaga01 Livre");
    //digitalWrite(portaLed, LOW);
  }



}
void reconnectMQTT() {
  if (millis() - timeCon > intervaloCon) {
    Serial.print("* Tentando se conectar ao Broker MQTT: ");
    Serial.println(MQTT_SERVER);

    if (client.connect(clientId)) {
      Serial.println("Conectado com sucesso ao broker MQTT!");
//      turnLed(pinoConexao, 1);
      Serial.flush();

      if (client.subscribe(topic)) {
        Serial.print("Inscrito em: "); Serial.println(topic);
      }

    } else {
      Serial.println("Falha ao reconectar no broker.");
      Serial.println("Havera nova tentatica de conexao em 2s");
      Serial.flush();
      timeCon = millis();
//      turnLed(pinoConexao, 0);

    }
  }
}
void verificaConexaoEMQTT() {
  if (!client.connected()) {
    //turnLed(pinoConexao, 0);
    reconnectMQTT(); //se não há conexão com o Broker, a conexão é refeita
  }
}
