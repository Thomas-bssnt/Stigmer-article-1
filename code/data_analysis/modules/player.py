from collections import defaultdict
from json import load

import numpy as np
from scipy.optimize import curve_fit

from modules.binning import binning
from modules.files import get_files, get_files_sessions


class Player:
    def __init__(self, session, player_data):
        self.session = session
        self.id = player_data["playerPseudo"]
        self.age = player_data["playerAge"]
        self.gender = player_data["playerGender"]

        self.games = defaultdict(dict)
        self.aggregated_game = {}

    def __repr__(self):
        return f"Player({self.session}, {self.id})"

    def __getitem__(self, item):
        return self.games[item]

    def __eq__(self, other):
        return (self.session, self.id) == other

    def aggregate(self):
        self.aggregated_game["ratings"] = defaultdict(list)
        self.aggregated_game["score"] = 0
        self.aggregated_game["score_R2"] = 0
        for game in self.games.values():
            for value, ratings in game["ratings"].items():
                self.aggregated_game["ratings"][value] += ratings
            self.aggregated_game["score"] += game["score"]
            self.aggregated_game["score_R2"] += game["score_R2"]
        self.aggregated_game["score"] /= len(self.games)
        self.aggregated_game["score_R2"] /= len(self.games)

    def classify(self, game, u1_def_neu, u1_neu_col):
        game["binned_values"], game["mean_ratings"] = self._get_mean_ratings(
            game["ratings"]
        )
        values_without_nan = game["binned_values"][~np.isnan(game["mean_ratings"])]
        mean_ratings_without_nan = game["mean_ratings"][~np.isnan(game["mean_ratings"])]
        (game["u0"], game["u1"]), _ = curve_fit(
            self._ratings_fit_function,
            values_without_nan,
            mean_ratings_without_nan,
            p0=[2.5, 0],
        )

        if game["u1"] < u1_def_neu:
            game["type"] = "def"
        elif u1_neu_col < game["u1"]:
            game["type"] = "col"
        else:
            game["type"] = "neu"

    @staticmethod
    def _get_mean_ratings(ratings):
        binned_stars = binning(ratings)
        values = np.array(list(binned_stars.keys()))
        mean_ratings = np.array([np.mean(ratings) for ratings in binned_stars.values()])
        return values, mean_ratings

    @staticmethod
    def _ratings_fit_function(x, u0, u1):
        return u0 + 5 * u1 * x / 99


def get_players_games(path_data, game_type, **kwargs):
    return get_players_list_of_files(get_files(path_data, game_type), **kwargs)


def get_players_games_sessions(path_data, game_type, **kwargs):
    return [
        get_players_list_of_files(files_session, **kwargs)
        for files_session in get_files_sessions(path_data, game_type)
    ]


def get_players_list_of_files(files, aggregate=False, classify=None):

    players = []
    for in_file, out_file in files:

        # In data
        session = str(in_file.parents[1])[-4:]
        with in_file.open() as f:
            in_data = load(f)

        # Out data
        values = np.unique(in_data["data"]["map"]["map"])
        ratings = defaultdict(lambda: {value: [] for value in values})
        scores = defaultdict(int)
        scores_R2 = defaultdict(int)
        out_data = np.genfromtxt(
            out_file, dtype=None, delimiter=",", names=True, encoding=None
        )
        for _, player_id, _, _, value, number_stars, score in out_data:
            ratings[player_id][value].append(number_stars)
            scores[player_id] += score
            scores_R2[player_id] += value
        ranks = {
            player_id: rk
            for rk, (player_id, _) in enumerate(
                sorted(scores_R2.items(), key=lambda x: x[1], reverse=True), 1
            )
        }

        # Add player and data
        for player_data in in_data["players"]:
            if (session, player_data["playerPseudo"]) not in players:
                players.append(Player(session, player_data))
            player = players[players.index((session, player_data["playerPseudo"]))]

            player[in_data["_id"]]["ratings"] = ratings[player.id]
            player[in_data["_id"]]["score"] = scores[player.id]
            player[in_data["_id"]]["score_R2"] = scores_R2[player.id]
            player[in_data["_id"]]["rank"] = ranks[player.id]

    # Aggregate the games
    if aggregate:
        for player in players:
            player.aggregate()

    # Classify the players
    if classify is not None:
        for player in players:
            for game in player.games.values():
                player.classify(game, *classify)
            if aggregate:
                player.classify(player.aggregated_game, *classify)

    # Order the players games dictionary
    for player in players:
        player.games = {game_id: game for game_id, game in sorted(player.games.items())}

    return players
