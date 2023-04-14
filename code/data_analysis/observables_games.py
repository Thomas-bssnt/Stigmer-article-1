from collections import defaultdict

import numpy as np
from modules.games import Games


def main(path_data, path_data_figures, rule, game_type, bootstrap_reps):
    games = Games(path_data / rule, game_type)

    observables = bootstrap(games, bootstrap_reps)
    for name, (mean, err) in observables.items():
        np.savetxt(
            path_data_figures / game_type / "exp" / "observables" / f"{name}.txt",
            np.column_stack(
                (
                    np.arange(1, len(mean) + 1),
                    mean,
                    err.T,
                )
            ),
            fmt=("%d", "%f", "%f", "%f"),
        )


def bootstrap(games, bootstrap_reps):
    bs_observables = defaultdict(list)
    for _ in range(bootstrap_reps):
        bs_games_groups = np.random.choice(list(games.session), replace=True, size=len(games.session))
        bs_games = [game for bs_games_group in bs_games_groups for game in bs_games_group]
        for observable in bs_games[0].observables.keys():
            bs_observables[observable].append(np.mean([game.observables[observable] for game in bs_games], axis=0))
    return {
        observable: (
            np.mean(bs_observables[observable], axis=0),
            np.abs(
                np.percentile(bs_observables[observable], [50 - 34.13, 50 + 34.13], axis=0)
                - np.mean(bs_observables[observable], axis=0)
            ),
        )
        for observable in bs_observables
    }


if __name__ == "__main__":

    from modules.constants import PATH_DATA, PATH_DATA_FIGURES

    bootstrap_reps = 10000

    main(PATH_DATA, PATH_DATA_FIGURES, "rule_1", "Indiv_R1", bootstrap_reps)
    main(PATH_DATA, PATH_DATA_FIGURES, "rule_1", "Group_R1", bootstrap_reps)

    main(PATH_DATA, PATH_DATA_FIGURES, "rule_2", "Indiv_R2", bootstrap_reps)
    main(PATH_DATA, PATH_DATA_FIGURES, "rule_2", "Group_R2", bootstrap_reps)
