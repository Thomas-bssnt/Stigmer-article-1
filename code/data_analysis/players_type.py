import json
from sys import path

import numpy as np

path.append("./code/")
from modules.player import get_players_games_sessions


def main(path_data, path_data_figures, rule, game_type):

    thresholds = np.loadtxt(path_data_figures / game_type / "exp" / "classification" / "thresholds.txt")

    players_grouped = get_players_games_sessions(path_data / rule, game_type, classify=thresholds)

    max_games = max(len(player.games) for players in players_grouped for player in players)

    types = ["def", "neu", "col"]

    counts = []
    counts_g = []
    for g in range(max_games):
        counts_games_g = []
        for players in players_grouped:
            try:
                counts_game = [list(player.games.values())[g]["type"] for player in players]
                counts_games_g.append([counts_game.count(type_) for type_ in types])
            except IndexError:
                pass
        counts += counts_games_g
        counts_g.append(counts_games_g)

    with open(path_data_figures / game_type / "model" / "parameters" / "players_type.json", "w") as file:
        json.dump(
            {
                "number_of_players": sum(counts[0]),
                "order": ["def", "neu", "col"],
                "fractions": list(np.mean(counts, axis=0) / sum(counts[0])),
                "means": list(np.mean(counts, axis=0)),
                "numbers": counts,
                "numbers_game": {g: counts for g, counts in enumerate(counts_g, 1)},
            },
            file,
            indent=4,
        )


if __name__ == "__main__":

    from modules.constants import PATH_DATA, PATH_DATA_FIGURES

    main(PATH_DATA, PATH_DATA_FIGURES, "rule_1", "Group_R1")
    main(PATH_DATA, PATH_DATA_FIGURES, "rule_2", "Group_R2")
