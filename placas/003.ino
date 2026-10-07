#include <RF24.h>
#include <RF24_config.h>
#include <nRF24L01.h>
#include <printf.h>

#define CE_PIN 7
#define CSN_PIN 8
#define TIMEOUT 200 //millis

#define DATA 0
#define ACK 1

// instantiate an object for the nRF24L01 transceiver
RF24 radio(CE_PIN, CSN_PIN);

byte meuEnd = 3;

// Let these addresses be used for the pair
uint64_t address[2] = { 0x3030303030LL, 0x3030303030LL };

byte payload[5] = {26,3,0,2,1};

void setup() {

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
  radio.openReadingPipe(0,address[1]);  // using pipe 1

  // For debugging info
   printf_begin();             // needed only once for printing details
   radio.printDetails();       // (smaller) function that prints raw register values
   radio.printPrettyDetails(); // (larger) function that prints human readable data

}  // setup

void printPacote(byte *pac, int tamanho){
  Serial.print(F("Rcvd "));
  Serial.print(tamanho);
  Serial.print(F(" O: "));
  Serial.print(pac[1]);
  Serial.print(F(" D: "));
  Serial.print(pac[0]);
  Serial.print(F(" C: "));
  Serial.print(pac[2]);
  Serial.print(F(" i: "));
  Serial.print(pac[3]);
  Serial.print(F(" : "));
  for (int i = 4; i < tamanho; i++) {
    Serial.print(pac[i]);
  }
  Serial.println();
}

void envia(byte destino, byte* pacote, int tamanho, byte controle) {
  radio.stopListening();
  pacote[0] = destino;
  pacote[1] = meuEnd;
  pacote[2] = controle;

  radio.write(pacote, tamanho);
  Serial.println(F("Pacote enviado! Aguardando ACK..."));

  radio.startListening();

  bool confirmado = false;
  unsigned long int tempo = millis();

  while (!confirmado && (millis() - tempo < TIMEOUT)) {
    if (radio.available()) {
      byte bufferRx[5];
      radio.read(bufferRx, 5);

      if (bufferRx[0] == meuEnd && bufferRx[1] == destino && bufferRx[2] == ACK) {
        Serial.println(F("Confirmacao recebida com sucesso!"));
        confirmado = true;
      }
    }
  }

  if (!confirmado) {
    Serial.println(F("Timeout: Nenhuma confirmacao recebida."));
  }
}

void recebe(byte* pacote, byte controle) {
  radio.startListening();

  if (radio.available()) {
    int tamanho = radio.getPayloadSize();
    radio.read(pacote, tamanho);

    if (pacote[0] == meuEnd) {
      Serial.println(F("Pacote para mim!"));

      if (pacote[2] == DATA && controle == DATA) {
        printPacote(pacote, tamanho);

        envia(pacote[1], pacote, 5, ACK);
      }
      else if (pacote[2] == ACK && controle == ACK) {
        Serial.println(F("Confirmacao (ACK) recebida no modo escuta!"));
      }
    }
  }
}


// void envia(int destino, int tipo){
//  radio.flush_tx();
//  payload[0]= destino;
//  payload[1]= origem;
//  payload[2]= tipo;
//  unsigned long inicio=millis();

//  while(millis() - inicio < TIMEOUT){
//    radio.startListening();
//    delayMicroseconds(50);
//    radio.stopListening();
//    if (!radio.testCarrier()) {a
//      if(tipo==ACK){
//       radio.write(&payload[0], 3);
//      }else if(tipo==DATA){
//       radio.write(&payload[0], 5);
//      }
//      return;
//    }else{
//      Serial.println("Meio Ocupado");
//      delayMicroseconds(270);
//    }
//    radio.flush_tx();
//  }
//  Serial.println("TimeOut!");
// }

// int recebe(int tipo){
//   int tamanho;
//   radio.startListening();
//   unsigned long inicio=millis();
//   while(millis() - inicio < TIMEOUT){
//     if (radio.available()) {
//       delayMicroseconds(160);
//       tamanho=radio.getPayloadSize();
//       radio.read(&payloadRX[0], tamanho);
//       if (payloadRX[0]==origem){
//         Serial.println("é pra mim");
//         if (payloadRX[2]==0){
//           printPacote(&payloadRX[0], tamanho);
//         }
//         if (payloadRX[2]==tipo){
//           return 1;
//         }
//       }
//       radio.flush_rx();
//     }
//   }
//   Serial.println("timeout recebe");
//   return 0;
// }

void loop() {

  envia(payload[0], payload, 5, DATA);
  // recebe(payload, DATA);
  delay(1000);

}  // loop
