import numpy as np
from modules.constants import S_MAX
from modules.games import Games


def main(path_data, path_data_figures, rule, game_type, bootstrap_reps):

    dx = 0.15
    n_bins = 25

    games = Games(path_data / rule, game_type)
    x_values, player_scores_dict, team_scores_dict = bootstrap(games, bootstrap_reps, dx, n_bins)

    for filename, dict_ in (("S", player_scores_dict), ("S_team", team_scores_dict)):
        np.savetxt(
            path_data_figures / game_type / "exp" / "observables" / f"{filename}.txt",
            np.column_stack(
                (
                    x_values,
                    dict_["pdf"][0],
                    dict_["pdf"][1].T,
                )
            ),
            fmt=("%f", "%f", "%f", "%f"),
        )

    np.savetxt(
        path_data_figures / game_type / "exp" / "observables" / "<S>.txt",
        [[player_scores_dict["mean"][0]] + list(player_scores_dict["mean"][1])],
        fmt=("%f", "%f", "%f"),
    )


def bootstrap(games, bootstrap_reps, dx, n_bins):

    scores_grouped = [
        [np.array(list(game.scores_R2.values())) / S_MAX for game in games_session] for games_session in games.session
    ]

    player_scores_pdf = []
    player_scores_means = []
    player_scores_medians = []
    team_scores_pdf = []
    team_scores_means = []
    team_scores_medians = []
    for _ in range(bootstrap_reps):

        bs_indices = np.random.choice(len(scores_grouped), replace=True, size=len(scores_grouped))
        bs_scores_grouped = [scores for i in bs_indices for scores in scores_grouped[i]]
        # bs_scores_grouped = np.random.choice(scores_grouped, replace=True, size=len(scores_grouped))
        # bs_scores_grouped = [scores for bs_scores_session in bs_scores_grouped for scores in bs_scores_session]

        bs_player_scores = [score for scores in bs_scores_grouped for score in scores]
        bs_team_scores = [np.mean(scores) for scores in bs_scores_grouped]

        x_values, counts = get_hist(bs_player_scores, dx, n_bins)
        player_scores_pdf.append(counts)
        player_scores_means.append(np.mean(bs_player_scores))
        player_scores_medians.append(np.median(bs_player_scores))

        x_values, counts = get_hist(bs_team_scores, dx, n_bins)
        team_scores_pdf.append(counts)
        team_scores_means.append(np.mean(bs_team_scores))
        team_scores_medians.append(np.median(bs_team_scores))

    def _get_mean_err(list_):
        mean = np.mean(list_, axis=0)
        err = np.abs(np.percentile(list_, [50 - 34.13, 50 + 34.13], axis=0) - mean)
        return mean, err

    player_scores_dict = {
        "pdf": _get_mean_err(player_scores_pdf),
        "mean": _get_mean_err(player_scores_means),
        "median": _get_mean_err(player_scores_medians),
    }
    team_scores_dict = {
        "pdf": _get_mean_err(team_scores_pdf),
        "mean": _get_mean_err(team_scores_means),
        "median": _get_mean_err(team_scores_medians),
    }
    return x_values, player_scores_dict, team_scores_dict


def get_hist(list_, dx, n_bins):

    assert dx >= (1 - dx) / (n_bins - 1)  # to be sure that there is no gap

    x_values = np.linspace(0, 1, n_bins)

    Y = []
    for x in x_values:
        number_of_element = 0
        for element in list_:
            if x - dx / 2 < element <= x + dx / 2:
                number_of_element += 1
        Y.append(number_of_element / len(list_) / dx)

    return x_values, Y


if __name__ == "__main__":

    from modules.constants import PATH_DATA, PATH_DATA_FIGURES

    bootstrap_reps = 10000

    main(PATH_DATA, PATH_DATA_FIGURES, "rule_1", "Indiv_R1", bootstrap_reps)
    main(PATH_DATA, PATH_DATA_FIGURES, "rule_1", "Group_R1", bootstrap_reps)

    main(PATH_DATA, PATH_DATA_FIGURES, "rule_2", "Indiv_R2", bootstrap_reps)
    main(PATH_DATA, PATH_DATA_FIGURES, "rule_2", "Group_R2", bootstrap_reps)
