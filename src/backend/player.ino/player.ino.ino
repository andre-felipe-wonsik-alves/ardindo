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
  Serial.begin(9600);
}

void mainLoop() {
  // Lê o estado atual do botão
  bool estadoAtual = digitalRead(botao);

   // Verifica se o botão continua pressionado
  if (estadoAtual == LOW) {
    // Se o tempo atual menos o tempo de início for maior que o limite
    if ((millis() - tempoPressionado >= tempoCliqueLongo) && !comandoExecutado) {
      Serial.println("Botao segurado detectado!");
      
      comandoExecutado = true; // Garante que a ação rode apenas uma vez por clique
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

  // Fazer aqui enviar a info da posição
    
    delay(50); // Pequeno atraso (Debounce) para evitar leituras falsas causadas pelo ruído mecânico
  }

  // Atualiza o estado anterior para a próxima leitura
  estadoAnterior = estadoAtual;
}

void loop() {
  mainLoop();
}
