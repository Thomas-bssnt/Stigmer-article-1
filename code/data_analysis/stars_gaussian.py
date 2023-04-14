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

    params_model = {type_: get_params_model(values, probas_exp[type_]["mean"], type_) for type_ in ("col",)}

    values_model = np.arange(max(values) + 1)
    probas_model = get_probas_model(values_model, params_model["col"], gaussian_function)

    for s in stars:
        np.savetxt(
            path_data_figures / game_type / "model" / "classification" / f"P{s}_col.txt",
            np.column_stack(
                (
                    values_model,
                    probas_model[:, s],
                )
            ),
            fmt=("%d", "%f"),
        )

    with open(path_data_figures / game_type / "model" / "parameters" / "stars__.json", "w") as file:
        json.dump(
            {
                "col": {
                    "functionType": "gaussian",
                    "p1": list(params_model["col"]["p1"]),
                    "p2": list(params_model["col"]["p2"]),
                    "p3": list(params_model["col"]["p3"]),
                    "p4": list(params_model["col"]["p4"]),
                    "p5": list(params_model["col"]["p5"]),
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


def get_p0_p5_fit(values, stars, numbers_of_stars_played):
    probas = []
    for star in stars:
        p = []
        for value in values:
            if numbers_of_stars_played[value]:
                p.append(numbers_of_stars_played[value].count(star) / len(numbers_of_stars_played[value]))
            else:
                p.append(None)
        probas.append(p)
    probas = np.array(probas)

    # probas[1:5] = np.nanmean(probas[1:5], axis=0)

    return np.array(probas).T


def linear_function(v, a, b):
    return np.clip(a + b * v / 99, 0, 1)


def tanh_function(v, a, b, c, d):
    return np.clip(a + b * np.tanh((v - c) / 99 * d), 0, 1)


def gaussian_function(v, b, c, d):
    return np.clip(b * np.exp(-(((v - c) / 99 * d) ** 2)), 0, 1)


# def cauchy_function(v, b, c, d):
#     return b / (d**2 + (v - c) ** 2)


def get_params_gaussian(values, probas):
    def gaussian_function(v, *params):
        p1 = np.clip(params[0] * np.exp(-(((v - params[1]) / 99 * params[2]) ** 2)), 0, 1)
        p2 = np.clip(params[3] * np.exp(-(((v - params[4]) / 99 * params[5]) ** 2)), 0, 1)
        p3 = np.clip(params[6] * np.exp(-(((v - params[7]) / 99 * params[8]) ** 2)), 0, 1)
        p4 = np.clip(params[9] * np.exp(-(((v - params[10]) / 99 * params[11]) ** 2)), 0, 1)
        p5 = np.clip(params[12] * np.exp(-(((v - params[13]) / 99 * params[14]) ** 2)), 0, 1)
        p0 = 1 - p1 - p2 - p3 - p4 - p5
        return np.concatenate([p0, p1, p2, p3, p4, p5])

    popt, _ = curve_fit(
        gaussian_function,
        values,
        np.concatenate(probas, axis=0),
        p0=[0.6, 10, 1, 0.4, 40, 1, 0.3, 50, 1, 0.2, 60, 1, 1.0, 100, 1],
    )

    return popt[:3], popt[3:6], popt[6:9], popt[9:12], popt[12:]


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
        popt_1, popt_2, popt_3, popt_4, popt_5 = get_params_gaussian(values_without_None, probas_without_None.T)
    # elif type_ == "neu":
    #     mean = np.mean(np.dot(probas, np.arange(6)))
    #     popt_0, popt_5 = get_params_fit_linear(values_without_None, probas_without_None[:, 0], mean)
    # elif type_ == "def":
    #     popt_0, _ = get_params_fit_tanh(values_without_None, probas_without_None[:, 0], +1)
    #     popt_5, _ = get_params_fit_tanh(values_without_None, probas_without_None[:, 5], -1)
    return {
        "p1": popt_1,
        "p2": popt_2,
        "p3": popt_3,
        "p4": popt_4,
        "p5": popt_5,
    }
    # return {"p0": popt_0, "p5": popt_5}


def get_probas_model(values_model, popt, funct):
    probas = np.zeros((6, len(values_model)))
    probas[1] = funct(values_model, *popt["p1"])
    probas[2] = funct(values_model, *popt["p2"])
    probas[3] = funct(values_model, *popt["p3"])
    probas[4] = funct(values_model, *popt["p4"])
    probas[5] = funct(values_model, *popt["p5"])
    probas[0] = 1 - probas[1] - probas[2] - probas[3] - probas[4] - probas[5]
    # probas[0] = funct(values_model, *popt["p0"])
    # probas[1] = probas[2] = probas[3] = probas[4] = (1 - probas[0] - probas[5]) / 4

    if (probas < 0).any() or (1 < probas).any():
        print("Some probabilities are not in [0, 1]!")
        print(probas)

    return probas.T


if __name__ == "__main__":

    from modules.constants import PATH_DATA, PATH_DATA_FIGURES

    bootstrap_reps = 100

    main(PATH_DATA, PATH_DATA_FIGURES, "rule_1", "Group_R1", bootstrap_reps)
