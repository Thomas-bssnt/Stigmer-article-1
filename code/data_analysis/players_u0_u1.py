import numpy as np
from modules.player import get_players_games


def main(path_data, path_data_figures, rule, game_type):

    thresholds = np.loadtxt(path_data_figures / game_type / "exp" / "classification" / "thresholds.txt")
    players = get_players_games(path_data / rule, game_type, classify=thresholds)

    params = sorted(
        [(game["u0"], game["u1"]) for player in players for game in player.games.values()],
        key=lambda x: x[1],
    )

    np.savetxt(
        path_data_figures / game_type / "exp" / "classification" / "players_u0_u1.txt",
        params,
        fmt=("%f", "%f"),
    )


if __name__ == "__main__":

    from modules.constants import PATH_DATA, PATH_DATA_FIGURES

    main(PATH_DATA, PATH_DATA_FIGURES, "rule_1", "Indiv_R1")
    main(PATH_DATA, PATH_DATA_FIGURES, "rule_1", "Group_R1")

    main(PATH_DATA, PATH_DATA_FIGURES, "rule_2", "Indiv_R2")
    main(PATH_DATA, PATH_DATA_FIGURES, "rule_2", "Group_R2")
