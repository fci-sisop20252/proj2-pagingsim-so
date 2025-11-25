#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int pid;            // processo dono da página
    int page;           // número da página
    int ref_bit;        // usado pelo Clock (segunda chance)
    int dirty;          // 1 se foi escrita
    int occupied;       // 1 se o frame está em uso
    unsigned long load_time; // para FIFO
} Frame;

typedef enum { ALG_FIFO, ALG_CLOCK } Algorithm;

typedef struct {
    Frame *frames;
    int num_frames;
    int page_size;          // tamanho da página em bytes
    Algorithm algo;

    unsigned long time;     // contador lógico de acessos
    int clock_hand;         // ponteiro do algoritmo Clock

    unsigned long accesses;
    unsigned long hits;
    unsigned long faults;
    unsigned long writes_to_disk;
} Simulator;

// ---------------- Funções auxiliares ----------------

Algorithm parse_algorithm(const char *s) {
    if (strcmp(s, "FIFO") == 0 || strcmp(s, "fifo") == 0 || strcmp(s, "Fifo") == 0)
        return ALG_FIFO;
    if (strcmp(s, "CLOCK") == 0 || strcmp(s, "clock") == 0 || strcmp(s, "Clock") == 0)
        return ALG_CLOCK;

    fprintf(stderr, "Algoritmo invalido: %s (use fifo ou clock)\n", s);
    exit(EXIT_FAILURE);
}

void init_simulator(Simulator *sim, int num_frames, int page_size, Algorithm algo) {
    sim->num_frames = num_frames;
    sim->page_size = page_size;
    sim->algo = algo;

    sim->time = 0;
    sim->clock_hand = 0;
    sim->accesses = 0;
    sim->hits = 0;
    sim->faults = 0;
    sim->writes_to_disk = 0;

    sim->frames = (Frame *)malloc(sizeof(Frame) * num_frames);
    if (!sim->frames) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < num_frames; i++) {
        sim->frames[i].pid = -1;
        sim->frames[i].page = -1;
        sim->frames[i].ref_bit = 0;
        sim->frames[i].dirty = 0;
        sim->frames[i].occupied = 0;
        sim->frames[i].load_time = 0;
    }
}

// Retorna índice do frame que contém (pid,page) ou -1 se não encontrar
int find_frame(Simulator *sim, int pid, int page) {
    for (int i = 0; i < sim->num_frames; i++) {
        if (sim->frames[i].occupied &&
            sim->frames[i].pid == pid &&
            sim->frames[i].page == page) {
            return i;
        }
    }
    return -1;
}

// Retorna índice de frame livre ou -1 se não houver
int find_free_frame(Simulator *sim) {
    for (int i = 0; i < sim->num_frames; i++) {
        if (!sim->frames[i].occupied) return i;
    }
    return -1;
}

// Escolhe vítima pelo algoritmo FIFO
int choose_victim_fifo(Simulator *sim) {
    int victim = -1;
    unsigned long oldest_time = (unsigned long)-1;

    for (int i = 0; i < sim->num_frames; i++) {
        if (sim->frames[i].occupied &&
            sim->frames[i].load_time < oldest_time) {

            oldest_time = sim->frames[i].load_time;
            victim = i;
        }
    }

    if (victim == -1) {
        fprintf(stderr, "Erro FIFO: nao encontrou vitima.\n");
        exit(EXIT_FAILURE);
    }
    return victim;
}

// Escolhe vítima pelo algoritmo Clock
int choose_victim_clock(Simulator *sim) {
    while (1) {
        Frame *f = &sim->frames[sim->clock_hand];

        if (f->occupied) {
            if (f->ref_bit == 0) {
                int victim = sim->clock_hand;
                sim->clock_hand = (sim->clock_hand + 1) % sim->num_frames;
                return victim;
            } else {
                // Zera o bit e avança (segunda chance)
                f->ref_bit = 0;
            }
        }

        sim->clock_hand = (sim->clock_hand + 1) % sim->num_frames;
    }
}

// Carrega página (pid,page,op) em um frame específico (índice frame_idx)
void load_page_into_frame(Simulator *sim, int frame_idx, int pid, int page, char op) {
    Frame *f = &sim->frames[frame_idx];

    // Se estava ocupado e sujo
    if (f->occupied && f->dirty) {
        sim->writes_to_disk++;
    }

    f->pid = pid;
    f->page = page;
    f->dirty = (op == 'W');
    f->ref_bit = 1;     // página recém usada
    f->occupied = 1;
    f->load_time = sim->time;
}

// Trata um acesso à memória: (pid, endereco, op)
void access_address(Simulator *sim, int pid, int addr, char op) {
    // Tradução do endereço virtual
    int page = addr / sim->page_size;
    int offset = addr % sim->page_size;

    sim->accesses++;
    sim->time++; // tempo lógico

    int idx = find_frame(sim, pid, page);

    // ----------------- CENÁRIO 1: HIT -----------------
    if (idx != -1) {
        sim->hits++;
        Frame *f = &sim->frames[idx];
        f->ref_bit = 1; // referenced bit deve ser 1 em todo acesso

        if (op == 'W') {
            f->dirty = 1;
        }

        // Saída para HIT
        printf("Acesso: PID %d, Endereço %d (Página %d, Deslocamento %d) -> HIT: Página %d (PID %d) já está no Frame %d\n",
               pid, addr, page, offset, page, pid, idx);
        return;
    }

    // ----------------- CENÁRIO 2: PAGE FAULT -----------------
    sim->faults++;

    // Tenta achar frame livre
    int free_idx = find_free_frame(sim);
    if (free_idx != -1) {
        load_page_into_frame(sim, free_idx, pid, page, op);

        // PAGE FAULT com frame livre
        printf("Acesso: PID %d, Endereço %d (Página %d, Deslocamento %d) -> PAGE FAULT -> Página %d (PID %d) alocada no Frame livre %d\n",
               pid, addr, page, offset, page, pid, free_idx);
        return;
    }

    // Memória cheia: escolher vítima 
    int victim_idx;
    if (sim->algo == ALG_FIFO) {
        victim_idx = choose_victim_fifo(sim);
    } else {
        victim_idx = choose_victim_clock(sim);
    }

    Frame *victim = &sim->frames[victim_idx];
    int old_pid = victim->pid;
    int old_page = victim->page;

    // Saída precisa mostrar qual página será desalocada
    printf("Acesso: PID %d, Endereço %d (Página %d, Deslocamento %d) -> PAGE FAULT -> Memória cheia. Página %d (PID %d) (Frame %d) será desalocada. ",
           pid, addr, page, offset, old_page, old_pid, victim_idx);

    // Carrega a nova página no mesmo frame
    load_page_into_frame(sim, victim_idx, pid, page, op);

    // Completa a linha
    printf("-> Página %d (PID %d) alocada no Frame %d\n",
           page, pid, victim_idx);
}

// ----------------- main -----------------

int main(int argc, char *argv[]) {
    // ./simulador <num_frames> <tamanho_pagina> <fifo|clock> <arquivo_entrada>
    if (argc != 5) {
        fprintf(stderr, "Uso: %s <num_frames> <tamanho_pagina> <fifo|clock> <arquivo_entrada>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int num_frames = atoi(argv[1]);
    if (num_frames <= 0) {
        fprintf(stderr, "Numero de frames deve ser > 0\n");
        return EXIT_FAILURE;
    }

    int page_size = atoi(argv[2]);
    if (page_size <= 0) {
        fprintf(stderr, "Tamanho de pagina deve ser > 0\n");
        return EXIT_FAILURE;
    }

    Algorithm algo = parse_algorithm(argv[3]);

    FILE *in = fopen(argv[4], "r");
    if (!in) {
        perror("fopen");
        return EXIT_FAILURE;
    }

    Simulator sim;
    init_simulator(&sim, num_frames, page_size, algo);

    int pid;
    int addr;
    char op;

    // Formato assumido do arquivo
    while (fscanf(in, "%d %d %c", &pid, &addr, &op) == 3) {
        if (op != 'R' && op != 'W') {
            fprintf(stderr, "Operacao invalida '%c'. Use R ou W.\n", op);
            continue;
        }
        access_address(&sim, pid, addr, op);
    }
    fclose(in);
    // Resumo final
    printf("--- Simulação Finalizada (Algoritmo: %s)\n",
           (sim.algo == ALG_FIFO) ? "fifo" : "clock");
    printf("Total de Acessos: %lu\n", sim.accesses);
    printf("Total de Page Faults: %lu\n", sim.faults);
    free(sim.frames);
    return EXIT_SUCCESS;
} 