#include <RF24.h>
#include <RF24_config.h>
#include <nRF24L01.h>
#include <printf.h>

#define CE_PIN 7
#define CSN_PIN 8
#define TIMEOUT 400  //millis
#define DATA 0
#define ACK 1
#define MAX_TENTATIVAS 6

// instantiate an object for the nRF24L01 transceiver
RF24 radio(CE_PIN, CSN_PIN);

// Let these addresses be used for the pair
uint64_t address[2] = { 0x3030303030LL, 0x3030303030LL };

// tipo, player, posição, sequência, checksum
byte payload[5] = { 26, 3, 0, 2, 1 };

const int destino = 3; //plaquina do gateway

const int tamanho = 6; //tamanho do pacote de envio

byte* pacote[6];

// Define o pino A0 com o nome 'botao'
const int botao = A0;

// Variáveis para controlar o estado do botão
bool estadoAnterior = HIGH; 
unsigned long tempoPressionado = 0;
bool comandoExecutado = false;

int posicao = 0;
const unsigned long tempoCliqueLongo = 3000;

void setup() {
  // Configura A0 como entrada e ativa o resistor pull-up interno
  // Com o INPUT_PULLUP, o pino fica em HIGH (1) por padrão e vai para LOW (0) quando pressionado
  pinMode(botao, INPUT_PULLUP);
  
  // Inicia a comunicação serial para ver o resultado no Monitor Serial
  Serial.begin(115200);
  while (!Serial) {
    // some boards need to wait to ensure access to serial over USB
  }

  // initialize the transceiver on the SPI bus
  if (!radio.begin()) {
    Serial.println(F("radio hardware is not responding!!"));
    while (1) {}  // hold in infinite loop
  }

  // because these examples are likely run with nodes in close proximity to
  // each other.
  radio.setPALevel(RF24_PA_MAX);  // RF24_PA_MAX is default.
  radio.setChannel(100);
  // save on transmission time by setting the radio to only transmit the
  // number of bytes we need to transmit a float
  radio.setPayloadSize(5);  // float datatype occupies 4 bytes
  radio.setAutoAck(false);
  radio.setCRCLength(RF24_CRC_DISABLED);
  radio.setDataRate(RF24_250KBPS);
  // set the TX address of the RX node into the TX pipe
  radio.openWritingPipe(address[0]);  // always uses pipe 0

  // set the RX address of the TX node into a RX pipe
  radio.openReadingPipe(0, address[1]);  // using pipe 1

  // For debugging info
  printf_begin();              // needed only once for printing details
  radio.printDetails();        // (smaller) function that prints raw register values
  radio.printPrettyDetails();  // (larger) function that prints human readable data
}

void envia(byte tipo, byte jogador, byte posicao, byte sequencia, byte checksum, int tamanho) {
  radio.stopListening();
  pacote[0] = destino;
  pacote[1] = tipo;
  pacote[2] = jogador; 
  pacote[3] = posicao;
  pacote[4] = sequencia;
  pacote[5] = checksum;

  radio.flush_rx();
  radio.startListening();
  delayMicroseconds(200);
  radio.stopListening();
  if (!radio.testCarrier()) {

    radio.write(pacote, tamanho);
    Serial.println(F("Pacote enviado! Aguardando ACK..."));

    radio.startListening();

    bool confirmado = false;
    unsigned long int tempo = millis();

    while (!confirmado && (millis() - tempo < TIMEOUT)) {
      if (radio.available()) {
        byte bufferRx[5];
        radio.read(bufferRx, 5);

        if (bufferRx[0] == jogador && bufferRx[1] == destino && bufferRx[2] == ACK) {
          Serial.println(F("Confirmacao recebida com sucesso!"));
          confirmado = true;
        }
      }
    }

    if (!confirmado) {
      Serial.println(F("Timeout: Nenhuma confirmacao recebida."));
    }
  // } else if (tentativas > 0) {
  //   Serial.println(tentativas);
  //   envia(destino, pacote, tamanho, controle, tentativas - 1);
  // } else {
  //   Serial.println(F("Tentativas esgotadas"));
  }
} 

void clique() {
  // Lê o estado atual do botão
  bool estadoAtual = digitalRead(botao);

   // Verifica se o botão continua pressionado
  if (estadoAtual == LOW) {
    // Se o tempo atual menos o tempo de início for maior que o limite
    if ((millis() - tempoPressionado >= tempoCliqueLongo) && !comandoExecutado) {
      Serial.println("Botao segurado detectado!");
      
      comandoExecutado = true; // Garante que a ação rode apenas uma vez por clique
      envia(ACK, 26, 2, 0, 67, tamanho);
  }

  // Detecta a transição: o botão ERA 'HIGH' (solto) e AGORA é 'LOW' (pressionado)
  else if (estadoAnterior == HIGH && estadoAtual == LOW) {
    tempoPressionado = millis();
    comandoExecutado = false;

    Serial.println("Botão clicado!");
    posicao++;

    if (posicao > 8){
      posicao = 0;
    }
    Serial.println(posicao);
  }

    delay(50); // Pequeno atraso (Debounce) para evitar leituras falsas causadas pelo ruído mecânico
  }

  // Atualiza o estado anterior para a próxima leitura
  estadoAnterior = estadoAtual;
}

void loop() {
  clique();
}
