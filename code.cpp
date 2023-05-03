#include <bits/stdc++.h>
using namespace std;

/* GLOBAL VARIABLES USED THROUGHOUT THE CODE */
int NUMBER_OBJECTS;     // populated automatically by processData
int NUMBER_CONSTRAINTS; // populated automatically by processData
int OPTIMUM;            // populated automatically by processData
int *CAPACITIES;        // populated automatically by processData
int **CONSTRAINTS;      // populated automatically by processData
int *VALUES;            // populated automatically by processData

const int POPULATION_SIZE = 100; // size of the population
const int GENERATIONS = 10000;   // number of generations to run the algorithm
const double ALPHA = 0.787;    // prarameter for determining the the amount of X1 to be taken in each chromosome
double pm = 0.2;   // Mutation probability

pair<int, int> CORE; // size of our core
int *WEIGHTED_SUMS;  // weighted sum for the corresponding constraint
vector<int> X1;

struct Item
{
    int value;
    int objId;
    double efficiency;
};
vector<Item> ITEMS; // efficiencies of the objects

struct Chromosome
{
    vector<int> selected_items;
    double fitness;
};
vector<Chromosome> Population; // Population

void processData(char *filename)
{
    FILE *file;
    file = fopen(filename, "r");
    if (file == NULL)
    {
        printf("Data File Not Found in Current Directory.");
        exit(1);
    }
    char *line = new char[1000];
    fgets(line, 1000, file);
    char *tok = strtok(line, " \t");
    NUMBER_CONSTRAINTS = atoi(tok);
    tok = strtok(NULL, " \t");
    NUMBER_OBJECTS = atoi(tok);
    int i = 0;

    /* VALUES (objective function) */
    VALUES = new int[NUMBER_OBJECTS];
    fgets(line, 1000, file);
    tok = strtok(line, " \t");
    while (true)
    {
        while (tok != NULL && i < NUMBER_OBJECTS)
        {
            int vv = atoi(tok);
            VALUES[i] = vv;
            i++;
            tok = strtok(NULL, " \t");
            if (tok == NULL)
            {
                break;
            }

            if (strcmp(tok, " ") == 0 || strcmp(tok, "") == 0)
            {
                continue;
            }

            if (strcmp(tok, "\n") == 0)
            {
                break;
            }
        }

        if (i < NUMBER_OBJECTS)
        {
            fgets(line, 1000, file);
            tok = strtok(line, " \t");
            int vv = atoi(tok);
            VALUES[i] = vv;
            tok = strtok(NULL, " \t");
            i++;
        }
        else
        {
            break;
        }
    }

    /* CAPACITIES */
    CAPACITIES = new int[NUMBER_CONSTRAINTS];
    i = 0;
    fgets(line, 1000, file);
    tok = strtok(line, " \t");
    while (true)
    {
        while (tok != NULL && i < NUMBER_CONSTRAINTS)
        {
            int vv = atoi(tok);
            CAPACITIES[i] = vv;
            i++;
            tok = strtok(NULL, " \t");
            if (tok == NULL)
            {
                break;
            }

            if (strcmp(tok, " ") == 0 || strcmp(tok, "") == 0)
            {
                continue;
            }

            if (strcmp(tok, "\n") == 0)
            {
                break;
            }
        }

        if (i < NUMBER_CONSTRAINTS)
        {
            fgets(line, 1000, file);
            tok = strtok(line, " \t");
            int vv = atoi(tok);
            CAPACITIES[i] = vv;
            i++;
            tok = strtok(NULL, " \t");
        }
        else
        {
            break;
        }
    }

    /* CONSTRAINTS */
    CONSTRAINTS = new int *[NUMBER_CONSTRAINTS];
    for (int n = 0; n < NUMBER_CONSTRAINTS; n++)
    {
        i = 0;
        fgets(line, 1000, file);
        CONSTRAINTS[n] = new int[NUMBER_OBJECTS];
        tok = strtok(line, " \t");
        int vv = atoi(tok);
        CONSTRAINTS[n][i] = vv;
        i++;
        while (true)
        {
            tok = strtok(NULL, " \t");
            while (tok != NULL && i < NUMBER_OBJECTS)
            {
                vv = atoi(tok);
                CONSTRAINTS[n][i] = vv;
                tok = strtok(NULL, " \t");
                i++;
                if (tok == NULL)
                {
                    break;
                }

                if (strcmp(tok, " ") == 0 || strcmp(tok, "") == 0)
                {
                    continue;
                }

                if (strcmp(tok, "\n") == 0)
                {
                    break;
                }
            }

            if (i < NUMBER_OBJECTS)
            {
                fgets(line, 1000, file);
                tok = strtok(line, " \t");
                int vv = atoi(tok);
                CONSTRAINTS[n][i] = vv;
                i++;
            }
            else
            {
                break;
            }
        }
    }

    fgets(line, 1000, file);
    tok = strtok(line, " \t");
    while (strcmp(tok, "") == 0 || strcmp(tok, " ") == 0 || strcmp(tok, "\n") == 0)
    {
        fgets(line, 1000, file);
        tok = strtok(line, " \t");
    }

    OPTIMUM = atoi(tok);

    delete[] (line);
}

/* Evaluating the weighted sums */
void getWeightedSums()
{
    int *ws = new int[NUMBER_CONSTRAINTS];
    for (int i = 0; i < NUMBER_CONSTRAINTS; i++)
    {
        long sum = 0;
        for (int j = 0; j < NUMBER_OBJECTS; j++)
        {
            sum += CONSTRAINTS[i][j];
        }
        ws[i] = sum;
    }
    WEIGHTED_SUMS = ws;
}

/* Evaluating the utility ratio values */
double *getUtilityRatios()
{
    double *ratios = new double[NUMBER_CONSTRAINTS];
    getWeightedSums();
    for (int i = 0; i < NUMBER_CONSTRAINTS; i++)
    {
        ratios[i] = (WEIGHTED_SUMS[i] - CAPACITIES[i]) / (double)(WEIGHTED_SUMS[i]);
    }
    return ratios;
}

/* Evaluating efficiencies of the items */
void evaluateEfficiencies()
{
    double *ratios = getUtilityRatios();
    for (int i = 0; i < NUMBER_OBJECTS; i++)
    {
        double new_weight = 0;
        for (int j = 0; j < NUMBER_CONSTRAINTS; j++)
        {
            new_weight += (ratios[j] * CONSTRAINTS[j][i]);
        }
        ITEMS.push_back({VALUES[i], i + 1, VALUES[i] / new_weight});
    }
}

/* Sorting the solutions by their efficiencies */
bool compareItem(Item &i1, Item &i2)
{
    return (i1.efficiency > i2.efficiency);
}
void sortByEfficiencies()
{
    evaluateEfficiencies();
    sort(ITEMS.begin(), ITEMS.end(), compareItem);
}

inline void setCoreSize()
{
    CORE.first = ceil(NUMBER_OBJECTS / 3) - 1;
    CORE.second = ceil((2 * NUMBER_OBJECTS) / 3) - 1;
}

void setX1()
{
    X1.resize(NUMBER_OBJECTS);
    for (int i = 0; i <= CORE.first; i++)
    {
        X1[ITEMS[i].objId - 1] = 1;
    }
}

double calc_fitness(vector<int> &chromosome)
{
    double fitness = 0;
    double jaccardSimilarity = 0;
    int intersection = 0;
    int Union = 0;
    int valueSum = 0;
    for (int i = 0; i < NUMBER_OBJECTS; i++)
    {
        if (X1[i] and chromosome[i])
        {
            intersection++;
        }
        if (X1[i] or chromosome[i])
        {
            Union++;
        }
    }
    jaccardSimilarity = (1.0 * intersection) / Union;
    for (int i = 0; i < NUMBER_OBJECTS; i++)
    {
        if (chromosome[i] == 1)
        {
            valueSum += VALUES[i];
        }
    }
    jaccardSimilarity *= 100;
    fitness = (1 + jaccardSimilarity) * valueSum;
    return fitness;
}

void initializePopulation()
{
    for (int i = 0; i < POPULATION_SIZE; i++)
    {
        vector<int> chromosome(NUMBER_OBJECTS, 0);
        unordered_set<int> set;
        for (int j = 0; j <= CORE.first; j++)
        {
            double alpha_temp = (float)rand() / RAND_MAX;
            if (alpha_temp <= ALPHA)
            {
                chromosome[ITEMS[j].objId - 1] = 1;
            }
        }
        int remaining_genes = ceil(NUMBER_OBJECTS * 0.08);
        int count = 0;
        while (count < remaining_genes)
        {
            int select = (CORE.first + 1) + (rand() % ((2 * NUMBER_OBJECTS) / 3));
            if (set.find(select) == set.end())
            {
                chromosome[ITEMS[select].objId - 1] = 1;
                set.insert(select);
                count++;
            }
        }
        double fitness = calc_fitness(chromosome);
        Chromosome C;
        C.selected_items = chromosome;
        C.fitness = fitness;
        Population.push_back(C);
    }
}

// Define the selection function
int random(int low, int high)
{
    return low + rand() % (high - low + 1);
}
void parentsSelection(vector<int> &mating_pool)
{
    // Use tournament selection
    int tournament_size = 5, curr = 0;
    while (curr < POPULATION_SIZE)
    {
        int best = 0;
        double best_fit = -1;

        // generate distinct K(tournament_size) random numbers from 1 to Population size
        vector<int> rand_nums;
        for (int i = 0; i < tournament_size; i++)
        {
            rand_nums.push_back(random(0, POPULATION_SIZE - 1));
        }
        // select the best solution from these random numbers indexes from the Population
        for (int i = 0; i < tournament_size; i++)
        {
            if (best_fit < Population[rand_nums[i]].fitness)
            {
                best_fit = Population[rand_nums[i]].fitness;
                best = rand_nums[i];
            }
        }
        mating_pool[curr] = best;
        curr++;
    }
}

// apply crossover on the mating pool
void crossover(vector<Chromosome> &offsprings, vector<int> &mating_pool)
{
    int curr = 0;
    while (curr < POPULATION_SIZE / 2)
    {
        int parent1_index = mating_pool[rand() % POPULATION_SIZE];
        int parent2_index = mating_pool[rand() % POPULATION_SIZE];

        Chromosome child1, child2;
        // find a crossover point from 1 to Chromosome.size()-1
        int crossover_point = (rand() % (NUMBER_OBJECTS - 1)) + 1;
        child1.selected_items.resize(NUMBER_OBJECTS);
        child2.selected_items.resize(NUMBER_OBJECTS);

        for (int i = 0; i < crossover_point; i++)
        {
            child1.selected_items[i] = Population[parent1_index].selected_items[i];
            child2.selected_items[i] = Population[parent2_index].selected_items[i];
        }
        for (int i = crossover_point; i < NUMBER_OBJECTS; i++)
        {
            child1.selected_items[i] = Population[parent2_index].selected_items[i];
            child2.selected_items[i] = Population[parent1_index].selected_items[i];
        }

        offsprings.push_back(child1);
        offsprings.push_back(child2);
        curr++;
    }
}

void mutation(vector<Chromosome> &offspring)
{
    for (int i = 0; i < offspring.size(); i++)
    {
        for (int j = 0; j < NUMBER_OBJECTS; j++)
        {
            double p = (float)rand() / RAND_MAX;
            if (p <= pm)
            {
                if (offspring[i].selected_items[j] == 1)
                {
                    offspring[i].selected_items[j] = 0;
                }
                else
                {
                    offspring[i].selected_items[j] = 1;
                }
            }
        }
    }
}

bool cmp(Chromosome &c1, Chromosome &c2)
{
    return (c1.fitness > c2.fitness);
}

int main(int argc, char **argv)
{
    if (argc <= 2)
    {
        printf("Usage: ./a.out <datafile> <weing/orlib>\n");
        printf("Examples:\n");
        printf("./a.out data.DAT weing\n\n");
        exit(0);
    }
    if (strcmp(argv[2], "weing") == 0)
    {
        processData(argv[1]);
    }
    else
    {
        printf("Unknown file format. Exiting...\n");
        exit(1);
    }

    /* Initialize the random number generator */
    srand((unsigned)time(NULL));

    // efficiency calculation of the values
    sortByEfficiencies();

    // divide items into X1, Core and X0
    setCoreSize();
    setX1();

    /********* APPLYING GENETIC ALGORITHM *********/

    // create population
    initializePopulation();

    int n = 0;
    Chromosome global_best;
    global_best.fitness = -1;
    while (n < GENERATIONS)
    {
        // Elitism

        // sort(Population.begin(), Population.end(), cmp);
        // int lambda = ceil((NUMBER_OBJECTS * 5) / 100);

        // // int elitism_threshold = 620000;
        // vector<int> elites;
        // for (int i = 0; i < lambda; i++)
        // {
        //     elites.push_back(i);
        // }

        // parental selection for creating mating pool
        vector<int> mating_pool(POPULATION_SIZE);
        parentsSelection(mating_pool);

        // apply crossover on the mating pool elements
        vector<Chromosome> offsprings;
        crossover(offsprings, mating_pool);

        // apply mutation on the offsprings
        mutation(offsprings);

        // evaluate fitness for the offsprings
        for (int i = 0; i < POPULATION_SIZE; i++)
        {
            offsprings[i].fitness = calc_fitness(offsprings[i].selected_items);
        }

        // find the best solution of the current generation
        double best = 0, idx = 0;
        for (int i = 0; i < POPULATION_SIZE; i++)
        {
            if (best < offsprings[i].fitness)
            {
                best = offsprings[i].fitness;
                idx = i;
            }
        }
        if (global_best.fitness < best)
        {
            global_best.selected_items.resize(NUMBER_OBJECTS);
            global_best.fitness = best;
            for (int i = 0; i < NUMBER_OBJECTS; i++)
            {
                global_best.selected_items[i] = offsprings[idx].selected_items[i];
            }
        }

        // generational model
        // sort(offsprings.begin(), offsprings.end(), cmp);
        // for (int i = 0; i < elites.size(); i++)
        // {
        //     Population[i] = Population[elites[i]];
        // }
        // for (int i = elites.size(); i < POPULATION_SIZE; i++)
        // {
        //     Population[i] = offsprings[i - elites.size()];
        // }
        Population=offsprings;
        n++;
    }
    cout << "The global best solution's fitness: " << global_best.fitness << endl;
    cout << "The global best solution is: " << endl;
    long long max_value = 0;
    for (int i = 0; i < NUMBER_OBJECTS; i++)
    {
        if (global_best.selected_items[i])
        {
            cout << i + 1 << " ";
            max_value += (VALUES[i]);
        }
    }
    cout << endl
         << "The max value of the knapsack is: " << max_value << endl;

    return 0;
}

/* Printing the contraints, values, capacities */

// for (int i = 0; i < NUMBER_OBJECTS; ++i)
//     printf("%d: %d\n", i, VALUES[i]);
// cout << endl;
// for (int i = 0; i < NUMBER_CONSTRAINTS; ++i)
//     printf("%d: %d\n", i, CAPACITIES[i]);
// cout << endl;
// for (int i = 0; i < NUMBER_CONSTRAINTS; ++i)
// {
//     printf("%d:\n", i);
//     for (int j = 0; j < NUMBER_OBJECTS; ++j)
//         printf("%d: %d, ", j, CONSTRAINTS[i][j]);
//     cout << endl;
// }

/* Printing Chromosome */

// for(int j = 0; j < chromosome.size(); j++) {
//     printf("%d: %d\n", j, chromosome[j]);
// }

/* Printing Items */

// for (int i = 0; i < NUMBER_OBJECTS; i++)
// {
//     printf("%d: %d\t%d\t%f\n", i, ITEMS[i].value, ITEMS[i].objId, ITEMS[i].efficiency);
// }

/* Elitism by threshold value */

// for (int i = 0; i < POPULATION_SIZE; i++)
// {
//     if (Population[i].fitness > elitism_threshold)
//     {
//         elites.push_back(i);
//     }
// }

/* Printing Population */

// for (int i = 0; i < POPULATION_SIZE; i++)
// {
//     printf("%d: %f\n", i + 1, offsprings[i].fitness);
//     for (int j = 0; j < NUMBER_OBJECTS; j++)
//     {
//         printf("%d ", offsprings[i].selected_items[j]);
//     }
//     cout << endl
//          << endl;
// }

/* Printing Generations */

// printf("Generation number %d: %f\n", n, best);
// for (int i = 0; i < NUMBER_OBJECTS; i++)
// {
//     if (offsprings[idx].selected_items[i])
//     {
//         printf("%d, ", i + 1);
//     }
// }
// cout << endl;
// cout << endl;