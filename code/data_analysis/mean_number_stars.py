from collections import defaultdict

import numpy as np
from modules.binning import BINNING_DICT
from modules.player import get_players_games_sessions


def main(path_data, path_data_figures, rule, game_type, bootstrap_reps):

    thresholds = np.loadtxt(path_data_figures / game_type / "exp" / "classification" / "thresholds.txt")
    players = get_players_games_sessions(path_data / rule, game_type, classify=thresholds)
    player_types = ["col", "neu", "def", "all"]

    mean, err = bootstrap(players, player_types, bootstrap_reps)

    for type_ in player_types:
        np.savetxt(
            path_data_figures / game_type / "exp" / "classification" / f"MNS_{type_}.txt",
            np.column_stack(
                (
                    list(mean[type_].keys()),
                    list(mean[type_].values()),
                    *np.array(list(err[type_].values())).T,
                )
            ),
            fmt=("%f", "%f", "%f", "%f"),
        )


def binning(dict_, binning_dict):
    new_dict = defaultdict(list)
    for v, new_v in binning_dict.items():
        new_dict[new_v] += dict_[v]
    return new_dict


def bootstrap(players, player_types, bootstrap_reps):

    bs_means = {player_type: defaultdict(list) for player_type in player_types}

    for _ in range(bootstrap_reps):
        bs_indices = np.random.choice(len(players), replace=True, size=len(players))
        bs_players = [player for i in bs_indices for player in players[i]]

        for player_type in player_types:
            numbers_of_stars_played = defaultdict(list)
            for player in bs_players:
                for game in player.games.values():
                    if game["type"] == player_type or player_type == "all":
                        for value, stars in game["ratings"].items():
                            numbers_of_stars_played[value] += stars
            numbers_of_stars_played_binned = binning(numbers_of_stars_played, BINNING_DICT)
            for value, stars in numbers_of_stars_played_binned.items():
                bs_means[player_type][value].append(np.mean(stars))

    mean_ = {
        player_type: {value: np.nanmean(bs_mean) for value, bs_mean in bs_means[player_type].items()}
        for player_type in player_types
    }
    err = {
        player_type: {
            value: np.abs(np.nanpercentile(bs_mean, [50 - 34.13, 50 + 34.13]) - mean_[player_type][value])
            for value, bs_mean in bs_means[player_type].items()
        }
        for player_type in player_types
    }

    return mean_, err


if __name__ == "__main__":

    from modules.constants import PATH_DATA, PATH_DATA_FIGURES

    bootstrap_reps = 1000

    main(PATH_DATA, PATH_DATA_FIGURES, "rule_1", "Indiv_R1", bootstrap_reps)
    main(PATH_DATA, PATH_DATA_FIGURES, "rule_1", "Group_R1", bootstrap_reps)

    main(PATH_DATA, PATH_DATA_FIGURES, "rule_2", "Indiv_R2", bootstrap_reps)
    main(PATH_DATA, PATH_DATA_FIGURES, "rule_2", "Group_R2", bootstrap_reps)
