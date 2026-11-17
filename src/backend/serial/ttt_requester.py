import requests
from model.ttt_request import TTTRequest


class TTTRequester:
    def __init__(self, endpoint):
        self.endpoint = endpoint
        
    def make_move(self, data: TTTRequest):
        try:
            self._update_board(data.board)
            self._update_player(data.player)
                        
            print("[ MOVE ] The request was successfull!")
        except requests.RequestException as error:
            print("[ MOVE ] An error happended: %s", error)
            
    def _update_board(self, board: list):
        # update na board interna
        requests.post(f"{self.endpoint}/board", json=board)
        
    
    def _update_player(self, player: bool):
        # update no player
        requests.post(f"{self.endpoint}/player", json=player)
    