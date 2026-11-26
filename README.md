# Tópicos Especiais em Interfaces Computacionais

## Resumo do projeto

Consiste em estabelecer uma comunicação entre 3 _arduinos_, usando a seguinte topologia:

```mermaid
flowchart TD
    A[Arduino 1] <-->|Comunicam| B(Arduino 2)
    C[Arduino 3] <-->|Comunicam| B(Arduino 2)
    B[Arduino 2] --> |Repassa| D(Computador)
```

## Nosso trabalho

Seguindo essa estrutura como base, pensamos em fazer um jogo da velha. Seu desenho consiste em:

```mermaid
flowchart TD
    P1[Jogador 1] <-->|RF24| GW[Gateway]
    P2[Jogador 2] <-->|RF24| GW
    GW -->|Mensagem pela porta serial| SERIAL_MONITOR[Monitor Python]
    SERIAL_MONITOR -->|POST /board e /player| API[API Flask]
    API -->|GET /state| FRONTEND[Interface web]
```

### Fluxo de uma jogada

```mermaid
flowchart TD
    JOGADA_BTN[Jogador envia posição pelo botão] --> TYPE_PLAY[Arduino jogador envia TYPE_PLAY]
    TYPE_PLAY --> GW_RECEBE[Gateway recebe o pacote]
    GW_RECEBE --> VALIDA_JOGADA{Turno correto e posição válida?}
    VALIDA_JOGADA -->|Não| TYPE_ERR[Envia TYPE_ERR]
    VALIDA_JOGADA -->|Sim| APLICAR_MOVIMENTO[Aplica X ou O no tabuleiro]
    APLICAR_MOVIMENTO --> TYPE_ACK[Envia TYPE_ACK]
    TYPE_ACK --> SERIAL_ESTADO[Envia estado pela porta serial]
    SERIAL_ESTADO --> SERIAL_MONITOR[Monitor Python]
    SERIAL_MONITOR --> POST_BOARD[POST /board]
    SERIAL_MONITOR --> POST_PLAYER[POST /player]
    POST_BOARD --> API[API Flask]
    POST_PLAYER --> API
    API --> GET_STATE[GET /state]
    GET_STATE --> FRONTEND[Frontend atualiza a página]
```

### Vitória, empate e reinício

```mermaid
flowchart TD
    JOGADA_GATEWAY[Jogada aplicada pelo gateway] --> VERIFICAR_VITORIA{Existe vencedor?}
    VERIFICAR_VITORIA -->|Sim| VITORIA[Vencedor detectado]
    VERIFICAR_VITORIA -->|Não| VERIFICAR_EMPATE{Todas as casas estão preenchidas?}
    VERIFICAR_EMPATE -->|Sim| EMPATE[Empate detectado]
    VERIFICAR_EMPATE -->|Não| ALTERNA_TURNO[Alterna o turno]

    VITORIA --> ENVIAR_SERIAL[Envia estado pela serial]
    EMPATE --> ENVIAR_SERIAL
    ALTERNA_TURNO --> ENVIAR_SERIAL
    ENVIAR_SERIAL --> API_PLACAR[API atualiza estado e placar]

    BOTAO_RESET[Frontend: botão Reiniciar] --> POST_RESET[POST /reset]
    POST_RESET --> RESET_PENDENTE[API sinaliza reinício pendente]
    RESET_PENDENTE --> GET_RESET_STATUS[Monitor consulta GET /reset/status]
    GET_RESET_STATUS --> ENVIAR_RESET[Monitor envia RESET pela serial]
    ENVIAR_RESET --> GW_LIMPA[Gateway limpa o tabuleiro]
    GW_LIMPA --> GW_ENVIA_VAZIO[Gateway envia estado vazio pela serial]
    GW_ENVIA_VAZIO --> POST_RESET_ACK[Monitor confirma POST /reset/ack]
    POST_RESET_ACK --> API_PLACAR
```

## Como executar

### Monitor serial

Vá para a pasta `cd ./src/backend/serial` Crie um ambiente virtual do python com:

```python
python -m venv .venv
```

Depois, na root do projeto instale as dependências usando:

```makefile
make run install
```

Por fim, execute o monitor usando:

```makefile
make run
```

Em outro terminal, execute a API:

```makefile
make run-api
```

A API ficará disponível em `http://localhost:5000`.

## Uso de IA

Durante o projeto, houve o uso de LLMs para o desenvolvimento de alguns trechos de código. Especificamente:

1. Expressão regular no arquivo `src/backend/serial/ttt_parser.py`
2. Métodos `_sendSerial()` e `_handleSerialReset()` no `src/backend/gateway.ino/gateway.ino.ino`
3. Base da página html `src/frontend/index.html` e estilização `src/frontend/style.css`
4. Explicado como utilizar o Flask e criar suas rotas no `src/backend/api/api.py` e script no `src/frontend/index.html`

## Referências

Monitor de porta serial - [https://rahulsreedharan.medium.com/serial-port-programming-on-linux-using-python-and-pyserial-54f052f9f714](https://rahulsreedharan.medium.com/serial-port-programming-on-linux-using-python-and-pyserial-54f052f9f714)
