#include <RF24.h>
#include <RF24_config.h>
#include <nRF24L01.h>
#include <printf.h>

#define CE_PIN   7
#define CSN_PIN  8

#define ADDR_GATEWAY   3
#define ADDR_PLAYER_1  12
#define ADDR_PLAYER_2  27

// Tipos de pacote
#define TYPE_ACK   1
#define TYPE_ERR   2
#define TYPE_PLAY  3

// [tipo;origem;destino;posição;checksum]
#define ID_TIPO      0
#define ID_ORIGEM    1
#define ID_DESTINO   2
#define ID_POSICAO   3
#define ID_CHECKSUM  4
#define PAYLOAD_SIZE 5

RF24 radio(CE_PIN, CSN_PIN);

uint64_t address[2] = { 0x3030303030LL, 0x3030303030LL };

bool isPlayerOne = true;
char board[3][3];

byte winConditions[8][3] = {
  {0, 1, 2}, {3, 4, 5}, {6, 7, 8},
  {0, 3, 6}, {1, 4, 7}, {2, 5, 8},
  {0, 4, 8}, {2, 4, 6}
};

void _resetBoard() {
  for (int r = 0; r < 3; r++)
    for (int c = 0; c < 3; c++)
      board[r][c] = '-';
}

bool _isMoveValid(byte pos) {
  if (pos > 8) return false;
  byte row = pos / 3;
  byte col = pos % 3;
  return board[row][col] == '-';
}

void _applyMove(byte pos) {
  byte row = pos / 3;
  byte col = pos % 3;
  board[row][col] = isPlayerOne ? 'X' : 'O';
}

void _switchTurn() {
  isPlayerOne = !isPlayerOne;
}

void _sendSerial() {
  Serial.print(F("{board: ["));
  for (int r = 0; r < 3; r++) {
    Serial.print(F("["));
    for (int c = 0; c < 3; c++) {
      Serial.print(F("\""));
      Serial.print(board[r][c]);
      Serial.print(F("\""));
      if (c < 2) Serial.print(F(","));
    }
    Serial.print(F("]"));
    if (r < 2) Serial.print(F(","));
  }
  Serial.print(F("], isPlayerOne: "));
  Serial.print(isPlayerOne ? F("true") : F("false"));
  Serial.println(F("}"));
}

void _sendResponse(byte tipo, byte destino, byte pos) {
  byte payload[PAYLOAD_SIZE] = {tipo, ADDR_GATEWAY, destino, pos, 0};
  radio.stopListening();
  radio.write(payload, PAYLOAD_SIZE);
  radio.startListening();
}

void _sendAck(byte destino, byte pos) { _sendResponse(TYPE_ACK, destino, pos); }
void _sendErr(byte destino, byte pos) { _sendResponse(TYPE_ERR, destino, pos); }

void _handleIncomingPlay(byte* payload) {
  byte origem  = payload[ID_ORIGEM];
  byte pos     = payload[ID_POSICAO];

  bool senderIsPlayerOne = (origem == ADDR_PLAYER_1);

  if (payload[ID_DESTINO] != ADDR_GATEWAY) return;
  if (origem != ADDR_PLAYER_1 && origem != ADDR_PLAYER_2) return;

  // jogada repetida (pode ser ACK não recebido).
  if (pos <= 8 && board[pos / 3][pos % 3] == (senderIsPlayerOne ? 'X' : 'O')) {
    _sendAck(origem, pos);
    return;
  }

  if (senderIsPlayerOne != isPlayerOne) {
    Serial.println(F("[ERR] Fora de turno"));
    _sendErr(origem, pos);
    return;
  }

  if (!_isMoveValid(pos)) {
    Serial.println(F("[ERR] Posicao invalida ou ocupada"));
    _sendErr(origem, pos);
    return;
  }

  _applyMove(pos);
  _switchTurn();
  _sendAck(origem, pos);
  _sendSerial();
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {}

  _resetBoard();

  if (!radio.begin()) {
    Serial.println(F("[ERROR] Radio hardware is not responding!!"));
    while (1) {}
  }

  radio.setPALevel(RF24_PA_MAX);
  radio.setChannel(100);
  radio.setPayloadSize(PAYLOAD_SIZE);
  radio.setAutoAck(false);
  radio.setCRCLength(RF24_CRC_DISABLED);
  radio.setDataRate(RF24_250KBPS);
  radio.openWritingPipe(address[0]);
  radio.openReadingPipe(0, address[1]);
  radio.startListening();

  printf_begin();
  radio.printDetails();
  radio.printPrettyDetails();
}

void loop() {
  if (!radio.available()) return;

  byte buf[PAYLOAD_SIZE];
  radio.read(buf, PAYLOAD_SIZE);

  byte tipo = buf[ID_TIPO];
  if (tipo == TYPE_PLAY) {
    _handleIncomingPlay(buf);
  }
}
