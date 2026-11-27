#include <RF24.h>
#include <RF24_config.h>
#include <nRF24L01.h>
#include <printf.h>

#define CE_PIN  7
#define CSN_PIN 8

#define TIMEOUT_ENVIA        400  
#define TIMEOUT_RECEBE       400
#define BUTTON_DEBOUNCE       10
#define LONG_CLICK_DURATION 3000
#define BLINK_INTERVAL       350
#define BLINK_LOOP_INTERVAL 1300

#define ADDR_GATEWAY   3 //plaquina do gateway
#define ADDR_PLAYER_1 12
#define ADDR_PLAYER_2 27

// Tipos de pacote
#define TYPE_ACK  1
#define TYPE_ERR  2
#define TYPE_PLAY 3

// [tipo;origem;destino;posição;checksum]
#define ID_TIPO      0
#define ID_ORIGEM    1
#define ID_DESTINO   2
#define ID_POSICAO   3
#define ID_CHECKSUM  4
#define PAYLOAD_SIZE 5

RF24 radio(CE_PIN, CSN_PIN);

uint64_t address[2] = { 0x3030303030LL, 0x3030303030LL };

// !!!!!!!!!!!!!!! TROCAR DE ACORDO COM A PLACA
const byte MEU_ADDR = ADDR_PLAYER_1;

byte pacote[PAYLOAD_SIZE];

// Define o pino A0 com o nome 'botao'
const int botao = A0;

// Variáveis para controlar o estado do botão
bool estadoAnterior = HIGH; 
unsigned long tempoPressionado = 0;
bool comandoExecutado = false;
unsigned long ultimaMudanca = 0;

int posicao = 0;
int blinksRestantes = 0;
unsigned long ultimoBlink = 0;

void setup() {
  // Configura A0 como entrada e ativa o resistor pull-up interno
  // Com o INPUT_PULLUP, o pino fica em HIGH (1) por padrão e vai para LOW (0) quando pressionado
  pinMode(botao, INPUT_PULLUP);
  
  // AJUSTAR QUANDO O CONFLITO DO LED FOR RESOLVIDO.
  // pinMode(LED_BUILTIN, OUTPUT);
  // digitalWrite(LED_BUILTIN, LOW);
  
  // Inicia a comunicação serial para ver o resultado no Monitor Serial
  Serial.begin(115200);
  while (!Serial) {
    //some boards need to wait to ensure access to serial over USB
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
  radio.setPayloadSize(PAYLOAD_SIZE);
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

void envia(byte posicao) {
  pacote[ID_TIPO]     = TYPE_PLAY;
  pacote[ID_ORIGEM]   = MEU_ADDR;
  pacote[ID_DESTINO]  = ADDR_GATEWAY;
  pacote[ID_POSICAO]  = posicao;
  pacote[ID_CHECKSUM] = 0; // reservado

  while (true) {
    radio.stopListening();
    radio.startListening();
    delayMicroseconds(200);
    radio.stopListening();

    if (!radio.available() && !radio.testCarrier()) {
      radio.write(pacote, PAYLOAD_SIZE);
      Serial.println(F("Pacote enviado! Aguardando ACK..."));
    }

    radio.startListening();

    unsigned long tempo = millis();
    while (millis() - tempo < TIMEOUT_ENVIA) {
      if (!radio.available()) continue;

      byte bufferRx[PAYLOAD_SIZE];
      radio.read(bufferRx, PAYLOAD_SIZE);
      
      if (bufferRx[ID_DESTINO] != MEU_ADDR ||
          bufferRx[ID_ORIGEM] != ADDR_GATEWAY ||
          bufferRx[ID_POSICAO] != posicao) continue;

      if (bufferRx[ID_TIPO] == TYPE_ACK) {
        Serial.println(F("Jogada confirmada!"));
        return;
      }

      if (bufferRx[ID_TIPO] == TYPE_ERR) {
        Serial.println(F("Jogada rejeitada, tente novamente!"));
        return;
      }
    }

    Serial.println(F("Reenviando a mesma jogada.."));
    delay(100 + MEU_ADDR); // evita colisão.
  }
}

//O LED NÃO FUNCIONA AINDA PELO CONFLITO DO PINO d13 COM O RÁDIO.
void clique() {
   unsigned long agora = millis();
  // Lê o estado atual do botão
  bool estadoAtual = digitalRead(botao);

   // Verifica se o botão continua pressionado
  if (agora - ultimaMudanca >= BUTTON_DEBOUNCE && estadoAtual != estadoAnterior) {
    // Atualiza o estado anterior para a próxima leitura
    estadoAnterior = estadoAtual;
    ultimaMudanca = agora;

    // 'LOW' (pressionado)
    if (estadoAtual == LOW) {
      Serial.println("Botão clicado!");
      posicao++;

      tempoPressionado = agora;
      comandoExecutado = false;

      if (posicao > 8){
        posicao = 0;
      }

      Serial.println(posicao);
      // blinksRestantes = posicao;
      // digitalWrite(LED_BUILTIN, LOW);
      // ultimoBlink = agora;
    }
  }

  if (estadoAnterior == LOW && !comandoExecutado && agora - tempoPressionado >= LONG_CLICK_DURATION) {
    Serial.println("Botão segurado detectado!");
    comandoExecutado = true;
    envia((byte)posicao);
  }

  /* DESCOMENTAR QUANDO O CONFLITO DO LED FOR RESOLVIDO.
  // Pisca sem bloquear a leitura do botão
  if (blinksRestantes > 0 && agora - ultimoBlink >= BLINK_INTERVAL) {
    ultimoBlink = agora;
    bool ledLigado = !digitalRead(LED_BUILTIN);
    digitalWrite(LED_BUILTIN, ledLigado);
    
    if (!ledLigado) {
      blinksRestantes--;
    }
  }

  if (blinksRestantes <= 0 && agora - ultimoBlink >= BLINK_LOOP_INTERVAL){
    blinksRestantes = posicao;
  }
  */
}

void loop() {
  clique();
}
