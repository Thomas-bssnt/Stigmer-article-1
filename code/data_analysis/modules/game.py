from collections import defaultdict
from json import load
from pathlib import Path

import numpy as np


class Game:
    def __init__(self, in_file: Path, out_file: Path):
        self.in_file = in_file
        self.out_file = out_file

        self.session = str(self.in_file.parents[1])[-4:]
        self.id = self.in_file.stem

        # In data
        with self.in_file.open() as f:
            in_data = load(f)
        self.players = in_data["players"]
        self.in_data = in_data["data"]

        self.in_data["mapSize"] = len(self.in_data["map"]["map"])

        self.game_type = in_data["data"]["altGameName"]
        if self.game_type == "NOMEM":
            self.shuffled_arrays = self.in_data["shuffledArrays"]

        # Out data
        out_data = np.genfromtxt(self.out_file, dtype=None, delimiter=",", names=True, encoding=None)
        self.cells_played = np.zeros((self.in_data["numberRounds"], self.in_data["mapSize"] ** 2), dtype=int)
        self.stars_played = np.zeros((self.in_data["numberRounds"], self.in_data["mapSize"] ** 2), dtype=int)
        self.scores = defaultdict(int)
        self.scores_R2 = defaultdict(int)
        self.index_cells_played_players = defaultdict(lambda: [[] for _ in range(self.in_data["numberRounds"])])
        self.value_cells_played_players = defaultdict(lambda: [[] for _ in range(self.in_data["numberRounds"])])
        for round_, player_id, mapX, mapY, value, number_stars, score in out_data:

            if self.game_type == "NOMEM":
                mapX, mapY = self.get_initial_coordinates(round_, mapX, mapY)
            index = mapY * self.in_data["mapSize"] + mapX

            self.cells_played[round_ - 1, index] += 1
            self.stars_played[round_ - 1, index] += number_stars
            self.scores[player_id] += score
            self.scores_R2[player_id] += value
            self.index_cells_played_players[player_id][round_ - 1].append(index)
            self.value_cells_played_players[player_id][round_ - 1].append(value)

        # Observables
        self.observables = {}

        self.V = np.array(self.in_data["map"]["map"]).flatten()

        self.q_c = self._get_fractions(self.cells_played)
        self.Q_c = self._get_fractions(np.cumsum(self.cells_played, axis=0))
        self.p_c = self._get_fractions(self.stars_played)
        self.P_c = self._get_fractions(np.cumsum(self.stars_played, axis=0))

        V_max_3, V_max_2, V_max_1 = np.sort(self.V)[-3:]
        self.observables["q_"] = self._get_performance(self.q_c, self.V, (V_max_1 + V_max_2 + V_max_3) / 3)
        self.observables["Q"] = self._get_performance(self.Q_c, self.V, (V_max_1 + V_max_2 + V_max_3) / 3)
        self.observables["p_"] = self._get_performance(self.p_c, self.V, V_max_1)
        self.observables["P"] = self._get_performance(self.P_c, self.V, V_max_1)

        self.observables["IPR_q_"] = self._get_IPR(self.q_c)
        self.observables["IPR_Q"] = self._get_IPR(self.Q_c)
        self.observables["IPR_p_"] = self._get_IPR(self.p_c)
        self.observables["IPR_P"] = self._get_IPR(self.P_c)

        self.observables["F_Q"] = self._get_fidelity(self.Q_c, self.V)
        self.observables["F_P"] = self._get_fidelity(self.P_c, self.V)

        (
            self.observables["V3"],
            self.observables["V2"],
            self.observables["V1"],
        ) = self._get_value_best_cells(self.value_cells_played_players)

        (
            self.observables["B3"],
            self.observables["B2"],
            self.observables["B1"],
        ) = self._get_proba_revisit_best_cells(self.index_cells_played_players, self.value_cells_played_players)

        (
            self.observables["VB3"],
            self.observables["VB2"],
            self.observables["VB1"],
        ) = self._get_value_highest_value_cells(self.index_cells_played_players, self.value_cells_played_players)

    def players_id(self):
        return (player["playerPseudo"] for player in self.players)

    def __repr__(self) -> str:
        return (
            f"Game(S{self.session}, {self.id}, {' '.join(sorted(player['playerPseudo'] for player in self.players))})"
        )

    @staticmethod
    def _get_fractions(array):
        sum_ = np.sum(array, axis=1)
        return np.divide(array.T, sum_, where=sum_ != 0).T

    @staticmethod
    def _get_performance(array, V, denominator):
        return np.sum(array * V, axis=1) / denominator

    @staticmethod
    def _get_IPR(array):
        sum_ = np.sum(array**2, axis=1)
        return np.divide(1, sum_, out=np.zeros(sum_.shape), where=sum_ != 0)

    @staticmethod
    def _get_fidelity(array, V):
        return np.sum(np.sqrt(array * V / np.sum(V)), axis=1)

    @staticmethod
    def _get_value_best_cells(value_cells_played_players):
        return np.mean(
            [np.sort(value_cells_played_player) for value_cells_played_player in value_cells_played_players.values()],
            axis=0,
        ).T

    @staticmethod
    def _get_proba_revisit_best_cells(index_cells_played_players, value_cells_played_players):

        numberPlayers, numberRounds, numberCellsOpenedPerRound = np.array(
            list(index_cells_played_players.values())
        ).shape

        iCellsSortedByValue = defaultdict(list)
        vCellsSortedByValue = defaultdict(list)
        for playerId in index_cells_played_players.keys():
            for round_ in range(numberRounds):
                iSort = np.argsort(value_cells_played_players[playerId][round_])
                iCellsSortedByValue[playerId].append([index_cells_played_players[playerId][round_][i] for i in iSort])
                vCellsSortedByValue[playerId].append([value_cells_played_players[playerId][round_][i] for i in iSort])

        playBestCellPlayer = np.zeros(numberRounds)
        playSecondBestCellPlayer = np.zeros(numberRounds)
        playThirdBestCellPlayer = np.zeros(numberRounds)
        for playerId in value_cells_played_players.keys():
            for round_ in range(1, numberRounds):
                for iTurn in range(numberCellsOpenedPerRound):
                    iCell = iCellsSortedByValue[playerId][round_][iTurn]
                    if iCell == iCellsSortedByValue[playerId][round_ - 1][-1]:
                        playBestCellPlayer[round_] += 1
                    elif iCell == iCellsSortedByValue[playerId][round_ - 1][-2]:
                        playSecondBestCellPlayer[round_] += 1
                    elif iCell == iCellsSortedByValue[playerId][round_ - 1][-3]:
                        playThirdBestCellPlayer[round_] += 1

        playBestCellPlayer /= numberPlayers
        playSecondBestCellPlayer /= numberPlayers
        playThirdBestCellPlayer /= numberPlayers

        return playThirdBestCellPlayer, playSecondBestCellPlayer, playBestCellPlayer

    @staticmethod
    def _get_value_highest_value_cells(index_cells_played_players, value_cells_played_players):
        values = []
        for player_id in index_cells_played_players.keys():
            values_player = []
            i_max_1 = -1
            v_max_1 = -1
            i_max_2 = -1
            v_max_2 = -1
            v_max_3 = -1
            for i_round in range(20):
                for i, v in zip(
                    index_cells_played_players[player_id][i_round], value_cells_played_players[player_id][i_round]
                ):
                    if v > v_max_1:
                        v_max_3 = v_max_2
                        v_max_2 = v_max_1
                        i_max_2 = i_max_1
                        v_max_1 = v
                        i_max_1 = i
                    elif v > v_max_2 and i != i_max_1:
                        v_max_3 = v_max_2
                        v_max_2 = v
                        i_max_2 = i
                    elif v > v_max_3 and i != i_max_1 and i != i_max_2:
                        v_max_3 = v
                values_player.append([v_max_3, v_max_2, v_max_1])
            values.append(values_player)
        return np.mean(values, axis=0).T

    def get_initial_coordinates(self, round_, x, y):
        if round_ == 1:
            return x, y
        original_cell_coords = self.shuffled_arrays[round_ - 2][y][x]
        new_x, new_y = original_cell_coords["x"], original_cell_coords["y"]
        return self.get_initial_coordinates(round_ - 1, new_x, new_y)
