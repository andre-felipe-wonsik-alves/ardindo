from pathlib import Path

from flask import Flask, jsonify, request, send_from_directory

FRONTEND_DIR = Path(__file__).resolve().parents[2] / "frontend"

app = Flask(
    __name__,
    static_folder=str(FRONTEND_DIR),
    static_url_path="",
)

state = {
    "board": [[""] * 3 for _ in range(3)],
    "player": True,
    "winner": None,
    "draw": False,
    "score": {
        "player1": 0,
        "player2": 0,
        "draw": 0,
    },
}

# Sinal temporário compartilhado entre as rotas HTTP e o monitor serial
reset_requested = False


def get_winner(board):
    win_conditions = (
        (0, 1, 2), (3, 4, 5), (6, 7, 8),
        (0, 3, 6), (1, 4, 7), (2, 5, 8),
        (0, 4, 8), (2, 4, 6),
    )

    flat_board = [value for row in board for value in row]
    for first, second, third in win_conditions:
        if flat_board[first] and flat_board[first] == flat_board[second] == flat_board[third]:
            return flat_board[first]
    return None


@app.get("/")
def index():
    """Serve a página inicial do frontend."""
    return send_from_directory(FRONTEND_DIR, "index.html")


@app.post("/board")
def update_board():
    """Valida e armazena o tabuleiro recebido como uma matriz 3x3."""
    board = request.get_json()

    if (
        not isinstance(board, list)
        or len(board) != 3
        or any(not isinstance(row, list) or len(row) != 3 for row in board)
    ):
        return jsonify(error="O tabuleiro deve ter 3 linhas e 3 colunas"), 400

    previous_winner = state["winner"]
    previous_draw = state["draw"]
    winner = get_winner(board)
    draw = winner is None and all(value for row in board for value in row)

    state["board"] = board
    state["winner"] = winner
    state["draw"] = draw

    if previous_winner is None and previous_draw is False:
        if winner == "X":
            state["score"]["player1"] += 1
        elif winner == "O":
            state["score"]["player2"] += 1
        elif draw:
            state["score"]["draw"] += 1

    return jsonify(ok=True)


@app.post("/player")
def update_player():
    """Valida e armazena de quem é a vez, representado por um booleano."""
    player = request.get_json()

    if not isinstance(player, bool):
        return jsonify(error="player deve ser true ou false"), 400

    state["player"] = player
    return jsonify(ok=True)


@app.get("/state")
def get_state():
    """Retorna o estado atual do tabuleiro e do jogador em formato JSON."""
    return jsonify(state)


@app.post("/reset")
def reset_game():
    """Reinicia a partida atual sem apagar o placar."""
    global reset_requested
    reset_requested = True
    state["board"] = [[""] * 3 for _ in range(3)]
    state["player"] = True
    state["winner"] = None
    state["draw"] = False
    return jsonify(ok=True)


@app.get("/reset/status")
def reset_status():
    """Informa ao monitor serial se existe uma solicitação de reinício pendente."""
    return jsonify(reset=reset_requested)


@app.post("/reset/ack")
def acknowledge_reset():
    """Confirma o reinício enviado ao gateway pela porta serial."""
    global reset_requested
    reset_requested = False
    return jsonify(ok=True)


if __name__ == "__main__":
    app.run(port=5000, debug=True)