#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define NUM_LETTERS 26
#define NUM_ROWS 3  // Para impressão
#define MAX_WORDS 1000
#define MAX_WORD_LEN 64

// Estrutura para posição
typedef struct {
    double x;
    double y;
} Position;

// Estrutura para layout
typedef struct {
    Position pos[NUM_LETTERS];
} Layout;

// Posições fixas das teclas
Position positions[NUM_LETTERS] = {
    {0, 2}, {1, 2}, {2, 2}, {3, 2}, {4, 2}, {5, 2}, {6, 2}, {7, 2}, {8, 2}, {9, 2},  // Linha superior
    {0.5, 1}, {1.5, 1}, {2.5, 1}, {3.5, 1}, {4.5, 1}, {5.5, 1}, {6.5, 1}, {7.5, 1}, {8.5, 1},  // Linha do meio
    {1.5, 0}, {2.5, 0}, {3.5, 0}, {4.5, 0}, {5.5, 0}, {6.5, 0}, {7.5, 0}   // Linha inferior
};

// Letras
const char letters[NUM_LETTERS] = "abcdefghijklmnopqrstuvwxyz";

// Função para calcular distância euclidiana
double distance(Position p1, Position p2) {
    return sqrt(pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2));
}

// Função de custo: média da distância percorrida por palavra
double layout_cost(const Layout* layout, const char** words, int num_words) {
    double total_dist = 0.0;
    int num_segments = 0;
    for (int w = 0; w < num_words; w++) {
        const char* word = words[w];
        int len = strlen(word);
        if (len < 2) continue;
        for (int i = 0; i < len - 1; i++) {
            char l1 = tolower(word[i]);
            char l2 = tolower(word[i+1]);
            if (l1 >= 'a' && l1 <= 'z' && l2 >= 'a' && l2 <= 'z') {
                int idx1 = l1 - 'a';
                int idx2 = l2 - 'a';
                double d = distance(layout->pos[idx1], layout->pos[idx2]);
                total_dist += d;
                num_segments++;
            }
        }
    }
    if (num_segments == 0) return 0.0;
    return total_dist / num_segments;
}

// Função de fitness: negativo do custo
double fitness(const Layout* layout, const char** words, int num_words) {
    return -layout_cost(layout, words, num_words);
}

// Comparador para qsort: ordena por fitness decrescente (melhor primeiro)
int compare_layouts(const void* a, const void* b) {
    const Layout* la = (const Layout*)a;
    const Layout* lb = (const Layout*)b;
    // Nota: Como fitness pode ser calculada múltiplas vezes, para otimização real, poderia pré-computar fitness, mas para simplicidade, calculamos aqui
    extern const char** global_words;  // Declaração forward (definir em main)
    extern int global_num_words;
    double fa = fitness(la, global_words, global_num_words);
    double fb = fitness(lb, global_words, global_num_words);
    if (fa > fb) return -1;  // Descendente
    if (fa < fb) return 1;
    return 0;
}

// Gera um layout aleatório
void random_layout(Layout* layout) {
    Position temp[NUM_LETTERS];
    memcpy(temp, positions, sizeof(positions));
    for (int i = NUM_LETTERS - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        Position swap = temp[i];
        temp[i] = temp[j];
        temp[j] = swap;
    }
    memcpy(layout->pos, temp, sizeof(temp));
}

// Seleção por torneio
void tournament_selection(const Layout* population, int pop_size, const char** words, int num_words, int tournament_size, Layout* selected) {
    Layout candidates[10];  // Assumindo tournament_size <= 10
    int best_idx = -1;
    double best_fit = -INFINITY;
    for (int i = 0; i < tournament_size; i++) {
        int rand_idx = rand() % pop_size;
        candidates[i] = population[rand_idx];
        double fit = fitness(&candidates[i], words, num_words);
        if (fit > best_fit) {
            best_fit = fit;
            best_idx = i;
        }
    }
    *selected = candidates[best_idx];
}

// Crossover: Order Crossover (OX)
void order_crossover(const Layout* parent1, const Layout* parent2, Layout* child) {
    int start = rand() % NUM_LETTERS;
    int end = rand() % NUM_LETTERS;
    if (start > end) {
        int temp = start;
        start = end;
        end = temp;
    }

    // Mapa para rastrear posições usadas
    int used[NUM_LETTERS] = {0};
    for (int i = start; i <= end; i++) {
        child->pos[i] = parent1->pos[i];
        // Marcar como usado (usando índice invertido)
        for (int j = 0; j < NUM_LETTERS; j++) {
            if (positions[j].x == child->pos[i].x && positions[j].y == child->pos[i].y) {
                used[j] = 1;
                break;
            }
        }
    }

    // Preencher o resto com parent2, mantendo ordem
    int child_idx = (end + 1) % NUM_LETTERS;
    for (int i = 0; i < NUM_LETTERS; i++) {
        Position p = parent2->pos[i];
        int pos_idx = -1;
        for (int j = 0; j < NUM_LETTERS; j++) {
            if (positions[j].x == p.x && positions[j].y == p.y) {
                pos_idx = j;
                break;
            }
        }
        if (pos_idx != -1 && used[pos_idx] == 0) {
            child->pos[child_idx] = p;
            used[pos_idx] = 1;
            child_idx = (child_idx + 1) % NUM_LETTERS;
        }
    }
}

// Mutação: troca duas posições
void mutate(Layout* layout, double mutation_rate) {
    if ((double)rand() / RAND_MAX < mutation_rate) {
        int idx1 = rand() % NUM_LETTERS;
        int idx2 = rand() % NUM_LETTERS;
        Position temp = layout->pos[idx1];
        layout->pos[idx1] = layout->pos[idx2];
        layout->pos[idx2] = temp;
    }
}

// Algoritmo Genético
void genetic_algorithm(const char** words, int num_words, int population_size, int generations, double mutation_rate, double elitism, Layout* best_layout, double* best_cost) {
    Layout* population = malloc(population_size * sizeof(Layout));
    for (int i = 0; i < population_size; i++) {
        random_layout(&population[i]);
    }

    for (int gen = 0; gen < generations; gen++) {
        // Avaliar e ordenar população (por fitness decrescente)
        // Avaliar e ordenar população com qsort (por fitness decrescente)
        qsort(population, population_size, sizeof(Layout), compare_layouts);

        // Elitismo
        int elite_count = (int)(population_size * elitism);
        Layout* new_population = malloc(population_size * sizeof(Layout));
        memcpy(new_population, population, elite_count * sizeof(Layout));
        
        // Gerar novos indivíduos
        int new_idx = elite_count;
        while (new_idx < population_size) {
            Layout parent1, parent2, child;
            tournament_selection(population, population_size, words, num_words, 3, &parent1);
            tournament_selection(population, population_size, words, num_words, 3, &parent2);
            order_crossover(&parent1, &parent2, &child);
            mutate(&child, mutation_rate);
            new_population[new_idx++] = child;
        }

        free(population);
        population = new_population;

        if (gen % 50 == 0) {
            double curr_best_cost = -fitness(&population[0], words, num_words);
            printf("Geração %d, Melhor custo: %.4f\n", gen, curr_best_cost);
        }
    }

    *best_layout = population[0];
    *best_cost = layout_cost(best_layout, words, num_words);
    free(population);
}

// Mapear posições para letras
typedef struct {
    double x;
    char l;
} RowItem;

// Função de comparação para ordenar RowItem por x
int compare_rowitem(const void* a, const void* b) {
    const double xa = ((const RowItem*)a)->x;
    const double xb = ((const RowItem*)b)->x;
    if (xa < xb) return -1;
    if (xa > xb) return 1;
    return 0;
}

// Função para imprimir o layout como um teclado
void print_layout(const Layout* layout) {
    // Agrupar por linhas
    RowItem rows[3][10];  // Ajuste tamanho (máx 10 por linha)
    int row_counts[3] = {0};
    int row_y[3] = {2, 1, 0};  // y=2,1,0
    
    for (int i = 0; i < NUM_LETTERS; i++) {
        Position p = layout->pos[i];
        char l = letters[i];
        int row_idx = -1;
        if (p.y == 2) row_idx = 0;
        else if (p.y == 1) row_idx = 1;
        else if (p.y == 0) row_idx = 2;
        if (row_idx != -1) {
            rows[row_idx][row_counts[row_idx]].x = p.x;
            rows[row_idx][row_counts[row_idx]].l = toupper(l);
            row_counts[row_idx]++;
        }
    }

    // Ordenar cada linha por x com qsort
    for (int r = 0; r < 3; r++) {
        qsort(rows[r], row_counts[r], sizeof(RowItem), compare_rowitem);
    }

    // Imprimir linhas
    for (int r = 0; r < 3; r++) {
        for (int i = 0; i < row_counts[r]; i++) {
            printf("%c ", rows[r][i].l);
        }
        printf("\n");
    }
}

// Função para ler palavras de um arquivo
int read_words(const char* filename, char*** words_out) {
    FILE* f = fopen(filename, "r");
    if (!f) {
        perror("Erro ao abrir arquivo");
        return 0;
    }
    char** words = malloc(MAX_WORDS * sizeof(char*));
    int count = 0;
    char buf[MAX_WORD_LEN];
    while (fgets(buf, sizeof(buf), f)) {
        // Remove newline
        buf[strcspn(buf, "\r\n")] = 0;
        if (buf[0] == 0) continue;
        words[count] = strdup(buf);
        count++;
        if (count >= MAX_WORDS) break;
    }
    fclose(f);
    *words_out = words;
    return count;
}

// Globais para compare_layouts (para evitar passar parâmetros extras)
const char** global_words;
int global_num_words;

// Exemplo de uso
int main(int argc, char* argv[]) {
    srand(time(NULL));

    if (argc < 2) {
        fprintf(stderr, "Uso: %s <arquivo_palavras.txt>\n", argv[0]);
        return 1;
    }

    char** words;
    int num_words = read_words(argv[1], &words);
    
    if (num_words == 0) {
        fprintf(stderr, "Nenhuma palavra lida.\n");
        return 1;
    }

    // Definir globais para compare_layouts
    global_words = (const char **)words;
    global_num_words = num_words;
    
    Layout best_layout;
    double best_cost;
    genetic_algorithm((const char**)words, num_words, 100, 500, 0.05, 0.1, &best_layout, &best_cost);

    printf("Melhor layout encontrado:\n");
    print_layout(&best_layout);
    printf("Média da distância percorrida: %.4f\n", best_cost);

    // Libera memória
    for (int i = 0; i < num_words; i++) free(words[i]);
    free(words);

    return 0;
}