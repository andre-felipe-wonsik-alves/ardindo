import json
import re

from model.ttt_request import TTTRequest


def parse_message(raw: str) -> TTTRequest | None:
    """Recebe uma string vinda da serial e tenta montar um TTTRequest."""
    try:
        normalized = _normalize(raw.strip())
        data = json.loads(normalized)

        board = data["board"]
        is_player_one = data["isPlayerOne"]

        return TTTRequest(board=board, player=is_player_one)

    except (KeyError, ValueError, TypeError) as error:
        print(f"[ PARSER ] Mensagem inválida ignorada: {error!r}  |  raw={raw!r}")
        return None


def _normalize(raw: str) -> str:
    """Adiciona aspas nas chaves sem aspas para tornar o JSON válido."""
    return re.sub(r'(?<!["\w])(\w+)(?=\s*:)', r'"\1"', raw)