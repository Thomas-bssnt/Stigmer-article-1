from collections import Counter, defaultdict

BINNING_DICT = {
    0: 1.5918367346938775,
    1: 1.5918367346938775,
    2: 1.5918367346938775,
    3: 1.5918367346938775,
    4: 5.909090909090909,
    5: 5.909090909090909,
    6: 5.909090909090909,
    7: 5.909090909090909,
    8: 5.909090909090909,
    9: 5.909090909090909,
    11: 12.75,
    12: 12.75,
    13: 12.75,
    14: 12.75,
    19: 21.083333333333332,
    20: 21.083333333333332,
    21: 21.083333333333332,
    22: 21.083333333333332,
    24: 21.083333333333332,
    27: 27.25,
    28: 27.25,
    43: 44.25,
    44: 44.25,
    45: 44.25,
    46: 44.25,
    51: 52.5,
    53: 52.5,
    71: 71.5,
    72: 71.5,
    84: 85.25,
    85: 85.25,
    86: 85.25,
    99: 99.0,
}


def get_binning_dict(binning_list, map):
    counts = Counter(map)
    binning_dict = {}
    for sublist in binning_list:
        weights = [counts[v] for v in sublist]
        new_value = sum(value * weight for value, weight in zip(sublist, weights)) / sum(weights)
        for value in sublist:
            binning_dict[value] = new_value
    return binning_dict


def binning(dict_, binning_dict=BINNING_DICT):
    new_dict = defaultdict(list)
    for value, new_value in binning_dict.items():
        new_dict[new_value] += dict_[value]
    return new_dict
