import sys
import random
import math
from copy import deepcopy

# Alphabet letters (presuming a-z, without diacritics for simplicity)
letters = list('abcdefghijklmnopqrstuvwxyz')

# QWERTY-like layout for reference
# coordinates x, y
# Adjust the coordinates if necessary to reflect real distances
positions = [
    (0, 2), (1, 2), (2, 2), (3, 2), (4, 2), (5, 2), (6, 2), (7, 2), (8, 2), (9, 2),  # Upper row
    (0.5, 1), (1.5, 1), (2.5, 1), (3.5, 1), (4.5, 1), (5.5, 1), (6.5, 1), (7.5, 1), (8.5, 1),  # Middle row
    (1.5, 0), (2.5, 0), (3.5, 0), (4.5, 0), (5.5, 0), (6.5, 0), (7.5, 0),  # Lower row
]

assert len(positions) == 26, "Número de posições deve ser 26"

# Function to calculate Euclidean distance between two positions
def distance(p1, p2):
    return math.sqrt((p1[0] - p2[0]) ** 2 + (p1[1] - p2[1]) ** 2)

# Cost function: average distance traveled per word
def layout_cost(layout, words):
    total_dist = 0.0
    num_segments = 0
    for word in words:
        word = word.lower().strip()  # Normlize to lowercase
        if len(word) < 2:
            continue
        for i in range(len(word) - 1):
            l1 = word[i]
            l2 = word[i + 1]
            if l1 in layout and l2 in layout:  # Ignore non-alphabet characters.
                d = distance(layout[l1], layout[l2])
                total_dist += d
                num_segments += 1
    if num_segments == 0:
        return 0.0
    return total_dist / num_segments

# Generates a random layout: mapping letter -> position
def random_layout():
    pos = list(positions)
    random.shuffle(pos)
    return dict(zip(letters, pos))

# Fitness function: the lower the cost, the better (we use negative cost for maximization)
def fitness(layout, words):
    return -layout_cost(layout, words)

# Tournament selection
def tournament_selection(population, words, tournament_size=3):
    selected = random.sample(population, tournament_size)
    return max(selected, key=lambda l: fitness(l, words))

# Crossover: Order Crossover (OX) for permutations
def order_crossover(parent1, parent2):
    # Cast layouts to position lists ordered by letters
    pos1 = [parent1[l] for l in letters]
    pos2 = [parent2[l] for l in letters]
    
    size = len(pos1)
    start, end = sorted(random.sample(range(size), 2))
    
    child_pos = [None] * size
    child_pos[start:end+1] = pos1[start:end+1]
    
    remaining = [p for p in pos2 if p not in child_pos[start:end+1]]
    idx = 0
    for i in range(size):
        if child_pos[i] is None:
            child_pos[i] = remaining[idx]
            idx += 1
    
    child = dict(zip(letters, child_pos))
    return child

# Mutation: swap two letters
def mutate(layout, mutation_rate=0.05):
    if random.random() < mutation_rate:
        l1, l2 = random.sample(letters, 2)
        layout[l1], layout[l2] = layout[l2], layout[l1]
    return layout

# Genetic Algorithm
def genetic_algorithm(words, population_size=100, generations=500, mutation_rate=0.05, elitism=0.1):
    # Initialize population
    population = [random_layout() for _ in range(population_size)]
    
    for gen in range(generations):
        # Fitness evaluation and sorting
        population.sort(key=lambda l: fitness(l, words), reverse=True)
        
        # Elitism: keep the best individuals
        elite_count = int(population_size * elitism)
        new_population = population[:elite_count]
        
        # Generate new individuals until we reach the population size
        while len(new_population) < population_size:
            parent1 = tournament_selection(population, words)
            parent2 = tournament_selection(population, words)
            child = order_crossover(parent1, parent2)
            child = mutate(child, mutation_rate)
            new_population.append(child)
        
        population = new_population
        
        # Report every 50 generations
        if gen % 50 == 0:
            best_cost = -fitness(population[0], words)
            print(f"Geração {gen}, Melhor custo: {best_cost:.4f}")
    
    # Return the best layout and its cost
    best = population[0]
    best_cost = layout_cost(best, words)
    return best, best_cost

# Print layout in a keyboard-like format
def print_layout(layout):
    rows = {2: [], 1: [], 0: []}
    for l, p in layout.items():
        rows[p[1]].append((p[0], l))
    for y in sorted(rows, reverse=True):  # De cima para baixo
        row = sorted(rows[y])
        print(' '.join(l.upper() for _, l in row))

if len(sys.argv) < 2:
    print(f"Uso: {sys.argv[0]} <word-list.txt>")
    sys.exit(1)

filename = sys.argv[1]
with open(filename, 'r', encoding='utf-8') as f:
    words = [line.strip() for line in f.readlines()]

# Execute the optimization
best_layout, best_cost = genetic_algorithm(words, population_size=100, generations=500, mutation_rate=0.1, elitism=0.2)

# Show the results
print("Best layout found:")
print_layout(best_layout)
print(f"Average distance per word: {best_cost:.4f}")