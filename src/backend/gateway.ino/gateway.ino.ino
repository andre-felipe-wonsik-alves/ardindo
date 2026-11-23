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

// [tipo;origem;destino;jogador;posição;checksum]
#define ID_TIPO     0
#define ID_ORIGEM   1
#define ID_DESTINO  2
#define ID_JOGADOR  3
#define ID_POSICAO  4
#define ID_CHECKSUM 5
#define PAYLOAD_SIZE 6

RF24 radio(CE_PIN, CSN_PIN);

uint64_t address[2] = { 0x3030303030LL, 0x3030303030LL };

bool isPlayerOne = true;
char board[3][3];
bool gameOver = false;

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

bool _checkWinner(char symbol) {
  for (int i = 0; i < 8; i++) {
    bool won = true;
    for (int j = 0; j < 3; j++) {
      byte pos = winConditions[i][j];
      if (board[pos / 3][pos % 3] != symbol) {
        won = false;
        break;
      }
    }
    if (won) return true;
  }
  return false;
}

bool _isDraw() {
  for (int r = 0; r < 3; r++)
    for (int c = 0; c < 3; c++)
      if (board[r][c] == '-') return false;
  return true;
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
  Serial.print(F(", winner: "));

  if (_checkWinner('X')) Serial.print(F("\"X\""));
  else if (_checkWinner('O')) Serial.print(F("\"O\""));
  else Serial.print(F("null"));

  Serial.print(F(", draw: "));
  Serial.print(_isDraw() && !_checkWinner('X') && !_checkWinner('O') ? F("true") : F("false"));
  Serial.println(F("}"));
}

void _sendResponse(byte tipo, byte destino) {
  byte payload[PAYLOAD_SIZE] = { tipo, ADDR_GATEWAY, destino, 0, 0, 0 };
  radio.stopListening();
  radio.write(payload, PAYLOAD_SIZE);
  radio.startListening();
}

void _sendAck(byte destino) { _sendResponse(TYPE_ACK, destino); }
void _sendErr(byte destino) { _sendResponse(TYPE_ERR, destino); }

void _handleIncomingPlay(byte* payload) {
  byte origem  = payload[ID_ORIGEM];
  byte jogador = payload[ID_JOGADOR];
  byte pos     = payload[ID_POSICAO];

  if (gameOver) {
    Serial.println(F("[ERR] Partida encerrada"));
    _sendErr(origem);
    return;
  }

  bool senderIsPlayerOne = (jogador == 1);

  if (senderIsPlayerOne != isPlayerOne) {
    Serial.println(F("[ERR] Fora de turno"));
    _sendErr(origem);
    return;
  }

  if (!_isMoveValid(pos)) {
    Serial.println(F("[ERR] Posicao invalida ou ocupada"));
    _sendErr(origem);
    return;
  }

  _applyMove(pos);
  _sendAck(origem);
  char symbol = isPlayerOne ? 'X' : 'O';
  if (_checkWinner(symbol)) {
    gameOver = true;
  } else if (_isDraw()) {
    gameOver = true;
  } else {
    _switchTurn();
  }
  _sendSerial();
}

void _handleSerialReset() {
  if (!Serial.available()) return;

  String command = Serial.readStringUntil('\n');
  command.trim();
  if (command == "RESET") {
    _resetBoard();
    isPlayerOne = true;
    gameOver = false;
    Serial.println(F("[RESET] Partida reiniciada"));
    _sendSerial();
  }
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
  _handleSerialReset();

  if (!radio.available()) return;

  byte buf[PAYLOAD_SIZE];
  radio.read(buf, PAYLOAD_SIZE);

  byte tipo = buf[ID_TIPO];
  if (tipo == TYPE_PLAY) {
    _handleIncomingPlay(buf);
  }
}
