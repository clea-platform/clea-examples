from .definitions import products, emotions, genders, age_intervals
import json
import random
from os import path, makedirs


name_products = list(products.keys())
suggestions = {}


def init_suggestions():
    folder = "strategies"
    if not path.exists(f"{folder}/strategy_0.json"):
        num_prod_for_suggestion = 3
        for e in emotions:
            suggestions[e] = {}
            for g in genders:
                suggestions[e][g] = {}
                for ai in age_intervals:
                    suggestions[e][g][ai] = []
                    for _ in range(num_prod_for_suggestion):
                        r = random.randint(0, len(name_products)-1)
                        suggestions[e][g][ai].append(name_products[r])
        makedirs(folder)
        a_file = open(f"{folder}/strategy_0.json", "w")
        json.dump(suggestions, a_file)
    else:
        a_file = open(f"{folder}/strategy_0.json", "r")
        suggestions.update(json.load(a_file))

    pass


def suggest_product(emotion="", gender="", age=0):
    if emotion and gender and age:
        age_interval = age_intervals[int(age/10)-1 % len(age_intervals)]
        return suggestions[emotion][gender][age_interval]
    else:
        return []
