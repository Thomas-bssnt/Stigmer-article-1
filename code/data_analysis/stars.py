import json
from collections import defaultdict

import numpy as np
from modules.binning import BINNING_DICT, binning
from modules.player import get_players_games_sessions
from scipy.optimize import curve_fit


def main(path_data, path_data_figures, rule, game_type, bootstrap_reps):

    thresholds = np.loadtxt(path_data_figures / game_type / "exp" / "classification" / "thresholds.txt")
    values = np.unique(list(BINNING_DICT.values()))
    stars = np.arange(6)

    players_grouped = get_players_games_sessions(path_data / rule, game_type, classify=thresholds)

    probas_exp = bootstrap(values, stars, players_grouped, bootstrap_reps)

    params_model = {
        type_: get_params_model(values, probas_exp[type_]["mean"], type_) for type_ in ("col", "neu", "def")
    }

    values_model = np.arange(max(values) + 1)
    probas = {
        type_: get_probas_model(values_model, params_model[type_], funct)
        for type_, funct in (
            ("col", tanh_function),
            ("neu", linear_function),
            ("def", tanh_function),
        )
    }

    for type_ in probas_exp:
        for i, P in ((0, "P0"), (1, "P1234"), (5, "P5")):
            np.savetxt(
                path_data_figures / game_type / "exp" / "classification" / f"{P}_{type_}.txt",
                np.column_stack(
                    (
                        values,
                        probas_exp[type_]["mean"][:, i],
                        probas_exp[type_]["err"][:, :, i].T,
                    )
                ),
                fmt=("%d", "%f", "%f", "%f"),
            )

            np.savetxt(
                path_data_figures / game_type / "model" / "classification" / f"{P}_{type_}.txt",
                np.column_stack(
                    (
                        values_model,
                        probas[type_][:, i],
                    )
                ),
                fmt=("%d", "%f"),
            )

    with open(path_data_figures / game_type / "model" / "parameters" / "stars.json", "w") as file:
        json.dump(
            {
                "col": {
                    "functionType": "tanh",
                    "p0": list(params_model["col"]["p0"]),
                    "p5": list(params_model["col"]["p5"]),
                },
                "neu": {
                    "functionType": "linear",
                    "p0": list(params_model["neu"]["p0"]),
                    "p5": list(params_model["neu"]["p5"]),
                },
                "def": {
                    "functionType": "tanh",
                    "p0": list(params_model["def"]["p0"]),
                    "p5": list(params_model["def"]["p5"]),
                },
            },
            file,
            indent=4,
        )


def bootstrap(values, stars, players_grouped, bootstrap_reps):
    bs_probas = defaultdict(list)
    for _ in range(bootstrap_reps):
        bs_indices = np.random.choice(len(players_grouped), replace=True, size=len(players_grouped))
        bs_players_grouped = np.array(players_grouped)[bs_indices]
        bs_players = [player for players_group in bs_players_grouped for player in players_group]
        for type_, numbers_of_stars_played in get_numbers_of_stars_played_type(bs_players).items():
            bs_probas[type_].append(get_p0_p5_fit(values, stars, binning(numbers_of_stars_played)))
    return {type_: get_mean_err(np.array(bs_probas[type_], dtype=float)) for type_ in bs_probas}


def get_mean_err(bs_list):
    dict_ = {}
    dict_["mean"] = np.nanmean(bs_list, axis=0)
    dict_["err"] = np.abs(np.nanpercentile(bs_list, [50 - 34.13, 50 + 34.13], axis=0) - dict_["mean"])
    return dict_


def get_numbers_of_stars_played_type(players):
    numbers_of_stars_played_type = defaultdict(lambda: defaultdict(list))
    for player in players:
        for game in player.games.values():
            for value, stars in game["ratings"].items():
                numbers_of_stars_played_type[game["type"]][value] += stars
    return numbers_of_stars_played_type


def p0_p5_distribution(stars, p0, p5):
    p1 = p2 = p3 = p4 = (1 - p0 - p5) / 4
    return np.array([p0, p1, p2, p3, p4, p5])


def p0_p5_distribution_same_mean(mean):
    def f(stars, p0):
        distribution = p0_p5_distribution(stars, p0, 2 / 5 * mean + p0 - 1)
        return distribution

    return f


def get_p0_p5_fit(values, stars, numbers_of_stars_played):
    probas = []
    for value in values:
        if nspv := numbers_of_stars_played[value]:
            mean = np.mean(nspv)
            f = p0_p5_distribution_same_mean(mean)
            y_data = [nspv.count(star) / len(nspv) for star in stars]
            bounds = (
                max(0, 1 - 2 / 5 * mean - 1e-10),
                min(1, 2 * (1 - mean / 5) + 1e-10),
            )  # p0, p5 in [0, 1]
            p0 = min(1, curve_fit(f, stars, y_data, bounds=bounds)[0][0])
            p5 = max(0, 2 / 5 * mean + p0 - 1)  # constant mean
            p1 = p2 = p3 = p4 = max(0, (1 - p0 - p5) / 4)  # sum equal to 1
            probas.append([p0, p1, p2, p3, p4, p5])
        else:
            probas.append([None] * 6)
    return np.array(probas)


def linear_function(v, a, b):
    return a + b * v / 99


def tanh_function(v, a, b, c, d):
    return np.clip(a + b * np.tanh((v - c) / 99 * d), 0, 1)


def get_params_fit_tanh(values_without_None, probas_without_None, slope_sign):
    return curve_fit(
        tanh_function,
        values_without_None,
        probas_without_None,
        p0=(0.5, 0.5, 20, slope_sign * 5),
        maxfev=5000,
    )


def get_params_fit_linear(values_without_None, probas_without_None, mean):
    popt_0, _ = curve_fit(linear_function, values_without_None, probas_without_None, p0=(2.5, 0))
    popt_5 = [
        popt_0[0] + 2 / 5 * mean - 1,
        popt_0[1],
    ]
    return popt_0, popt_5


def get_params_model(values, probas, type_):
    values_without_None = []
    probas_without_None = []
    for value, proba in zip(values, probas):
        if not (proba[0] is None or np.isnan(proba[0])):
            values_without_None.append(value)
            probas_without_None.append(proba)
    values_without_None = np.array(values_without_None)
    probas_without_None = np.array(probas_without_None)

    if type_ == "col":
        popt_0, _ = get_params_fit_tanh(values_without_None, probas_without_None[:, 0], -1)
        popt_5, _ = get_params_fit_tanh(values_without_None, probas_without_None[:, 5], +1)
    elif type_ == "neu":
        mean = np.mean(np.dot(probas, np.arange(6)))
        popt_0, popt_5 = get_params_fit_linear(values_without_None, probas_without_None[:, 0], mean)
    elif type_ == "def":
        popt_0, _ = get_params_fit_tanh(values_without_None, probas_without_None[:, 0], +1)
        popt_5, _ = get_params_fit_tanh(values_without_None, probas_without_None[:, 5], -1)
    return {"p0": popt_0, "p5": popt_5}


def get_probas_model(values_model, popt, funct):
    probas = np.zeros((6, len(values_model)))
    probas[0] = funct(values_model, *popt["p0"])
    probas[5] = funct(values_model, *popt["p5"])
    probas[1] = probas[2] = probas[3] = probas[4] = (1 - probas[0] - probas[5]) / 4

    if (probas < 0).any() or (1 < probas).any():
        print("Some probabilities are not in [0, 1]!")
        print(probas)

    return probas.T


if __name__ == "__main__":

    from modules.constants import PATH_DATA, PATH_DATA_FIGURES

    bootstrap_reps = 1000

    main(PATH_DATA, PATH_DATA_FIGURES, "rule_1", "Group_R1", bootstrap_reps)
    main(PATH_DATA, PATH_DATA_FIGURES, "rule_2", "Group_R2", bootstrap_reps)
