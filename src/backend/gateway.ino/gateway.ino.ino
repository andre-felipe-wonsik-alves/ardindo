#include <RF24.h>
#include <RF24_config.h>
#include <nRF24L01.h>
#include <printf.h>

RF24 radio(CE_PIN, CSN_PIN);

uint64_t address[2] = { 0x3030303030LL, 0x3030303030LL };

bool isPlayerOne = true;

//  tipo, player, posicao, sequencia, checksum
byte payload[5] = { 0, 0, 0, 0, 0};
byte winConditions[8][3] = {
  {0, 1, 2}, {3, 4, 5}, {6, 7, 8}, 
  {0, 3, 6}, {1, 4, 7}, {2, 5, 8}, 
  {0, 4, 8}, {2, 4, 6}    
};

char board[3][3] = {
  {"-","-","-"},
  {"-","-","-"},
  {"-","-","-"}
  };

void sendBoard(){
  // envia a board para o pc
  Serial.print("board:[");
  Serial.print("], \"current_player\":\"X\", \"status\":\"playing\"");
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {
  }

  if (!radio.begin()) {
    Serial.println(F("[ERROR] Radio hardware is not responding!!"));
    while (1) {}  
  }

  radio.setPALevel(RF24_PA_MAX);  
  radio.setChannel(100);
  radio.setPayloadSize(5);  
  radio.setAutoAck(false);
  radio.setCRCLength(RF24_CRC_DISABLED);
  radio.setDataRate(RF24_250KBPS);
  radio.openWritingPipe(address[0]);  
  radio.openReadingPipe(0, address[1]); 

  printf_begin();              
  radio.printDetails();        
  radio.printPrettyDetails();  
}

void loop() {
  // recebi do player

}
