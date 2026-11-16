// Define o pino A0 com o nome 'botao'
const int botao = A0;

// Variáveis para controlar o estado do botão
bool estadoAnterior = HIGH; 
unsigned long tempoPressionado = 0;
bool comandoExecutado = false;
unsigned long ultimaMudanca = 0;
const unsigned long tempoDebounce = 50;

int posicao = 0;
const unsigned long tempoCliqueLongo = 3000;
const unsigned long intervaloPiscadas = 250;
int piscadasRestantes = 0;
unsigned long ultimaPiscada = 0;

void setup() {
  // Configura A0 como entrada e ativa o resistor pull-up interno
  // Com o INPUT_PULLUP, o pino fica em HIGH (1) por padrão e vai para LOW (0) quando pressionado
  pinMode(botao, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  
  // Inicia a comunicação serial para ver o resultado no Monitor Serial
  Serial.begin(9600);
}

void mainLoop() {
  unsigned long agora = millis();
  // Lê o estado atual do botão
  bool estadoAtual = digitalRead(botao);

   // Verifica se o botão continua pressionado
  if (estadoAtual != estadoAnterior) {
    ultimaMudanca = agora;
  }

  estadoAnterior = estadoAtual;

  if (agora - ultimaMudanca >= tempoDebounce && estadoAtual != estadoAnterior) {
    // Atualiza o estado anterior para a próxima leitura
    estadoAnterior = estadoAtual;

    // Detecta a transição: o botão ERA 'HIGH' (solto) e AGORA é 'LOW' (pressionado)
    if (estadoAtual == LOW) {
      tempoPressionado = agora;
      comandoExecutado = false;

      Serial.println("Botão clicado!");
      posicao++;

      if (posicao > 8){
        posicao = 0;
      }

      Serial.println(posicao);
      piscadasRestantes = posicao;
      digitalWrite(LED_BUILTIN, LOW);
      ultimaPiscada = agora;
    }
  }

  if (estadoAnterior == LOW && !comandoExecutado && agora - tempoPressionado >= tempoCliqueLongo) {
    Serial.println("Botão segurado detectado!");
    comandoExecutado = true;
  }

  // Pisca sem bloquear a leitura do botão
  if (piscadasRestantes > 0 && agora - ultimaPiscada >= intervaloPiscadas) {
    ultimaPiscada = agora;
    bool ledLigado = !digitalRead(LED_BUILTIN);
    digitalWrite(LED_BUILTIN, ledLigado);
    
    if (!ledLigado) {
      piscadasRestantes--;
    }
  }
}

void loop() {
  mainLoop();
}
