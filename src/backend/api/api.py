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
}


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

    state["board"] = board
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


if __name__ == "__main__":
    app.run(port=5000, debug=True)