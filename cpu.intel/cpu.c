#include <stdio.h>
#include <stdlib.h>

/****************************************************************************/
/*                                                                          */
/*               Module CPU                                     */
/*          External Declarations                   */
/*                                                                          */
/****************************************************************************/


/* OSP constant      */

#define   MAX_PAGE                   16 /* max size of page tables          */

/* OSP enumeration constants */

typedef enum {
    false, true                         /* the boolean data type            */
} BOOL;

typedef enum {
    running, ready, waiting, done       /* types of status                  */
} STATUS;


/* external type definitions */

typedef struct page_entry_node PAGE_ENTRY;
typedef struct page_tbl_node PAGE_TBL;
typedef struct event_node EVENT;
typedef struct pcb_node PCB;


/* external data structures */

struct page_entry_node {
    int frame_id;    /* frame id holding this page                       */
    BOOL valid;       /* page in main memory : valid = true; not : false  */
    BOOL ref;         /* set to true every time page is referenced AD     */
    int *hook;       /* can hook up anything here                        */
};

struct page_tbl_node {
    PCB *pcb;        /* PCB of the process in question                   */
    PAGE_ENTRY page_entry[MAX_PAGE];
    int *hook;       /* can hook up anything here                        */
};

struct pcb_node {
    int pcb_id;         /* PCB id                                        */
    int size;           /* process size in bytes; assigned by SIMCORE    */
    int creation_time;  /* assigned by SIMCORE                           */
    int last_dispatch;  /* last time the process was dispatched          */
    int last_cpuburst;  /* length of the previous cpu burst              */
    int accumulated_cpu;/* accumulated CPU time                          */
    PAGE_TBL *page_tbl;    /* page table associated with the PCB            */
    STATUS status;         /* status of process                             */
    EVENT *event;         /* event upon which process may be suspended     */
    int priority;       /* user-defined priority; used for scheduling    */
    PCB *next;          /* next pcb in whatever queue                    */
    PCB *prev;          /* previous pcb in whatever queue                */
    int *hook;          /* can hook up anything here                     */
};


/* external variables */

extern PAGE_TBL *PTBR;        /* page table base register */

extern int Quantum;        /* global time quantum; contains the value
                   entered at the beginning or changed
                   at snapshot. Has no effect on timer
                   interrupts, unless passed to set_timer() */



/* external routines */

extern prepage(/* pcb */);

extern int start_cost(/* pcb */);

/*  PCB    *pcb; */

extern set_timer(/* time_quantum */); //<-- INTERVAL TIMER
/*  int    time_quantum; */

extern int get_clock();

/******************************* General **************************************/
#define NULL ((void *)0)
#define BASE_PRIORITY 4


#define PAGE_SIZE      512                /* size of a page in bytes        */
#define MAX_TRACK      60                 /* num of tracks on a devices     */
#define TRACK_SIZE     1024               /* track size on each device      */
#define MAX_BLOCK      MAX_TRACK * TRACK_SIZE / PAGE_SIZE

typedef enum {
    iosvc, devint,                      /* types of interrupt               */
    pagefault, startsvc,
    termsvc, killsvc,
    waitsvc, sigsvc, timeint
} INT_TYPE;

typedef enum {
    read, write                         /* type of actions for I/O requests */
} IO_ACTION;

/* external type definitions */

typedef struct page_entry_node PAGE_ENTRY;
typedef struct page_tbl_node PAGE_TBL;
typedef struct event_node EVENT;
typedef struct ofile_node OFILE;
typedef struct pcb_node PCB;
typedef struct iorb_node IORB;
typedef struct int_vector_node INT_VECTOR;
typedef struct dev_entry_node DEV_ENTRY;


/* external data structures */

extern struct iorb_node {
    int iorb_id;     /* iorb id                                          */
    int dev_id;      /* associated device; index into the device table   */
    IO_ACTION action;   /* read/write                                       */
    int block_id;    /* block involved in the I/O                        */
    int page_id;     /* buffer page in the main memory                   */
    PCB *pcb;        /* PCB of the process that issued the request       */
    EVENT *event;      /* event used to synchronize processes with I/O     */
    OFILE *file;       /* associated entry in the open files table         */
    IORB *next;       /* next iorb in the device queue                    */
    IORB *prev;       /* previous iorb in the device queue                */
    int *hook;       /* can hook up anything here                        */
};

extern struct int_vector_node {
    INT_TYPE cause;           /* cause of interrupt                         */
    PCB *pcb;              /* PCB to be started (if startsvc) or pcb that*/
    /* caused page fault (if pagefault interrupt) */
    int page_id;           /* page causing pagefault                     */
    int dev_id;            /* device causing devint                      */
    EVENT *event;            /* event involved in waitsvc and sigsvc calls */
    IORB *iorb;             /* IORB involved in iosvc call                */
};

extern struct dev_entry_node {
    int dev_id;      /* device id - index into Dev_Tbl                   */
    BOOL busy;        /* the busy flag ("true", if busy)                  */
    BOOL free_blocks[MAX_BLOCK];
    /* block i is free: free_blocks[i] = true;          */
    /* else: = false                                    */
    IORB *iorb;       /* the iorb currently being processed by the device */
    int *dev_queue;
    int *hook;       /* can hook up anything here                        */
};

extern INT_VECTOR Int_Vector;                   /* interrupt vector         */

/*******************************************************/

#define NULL ((void *)0)
#define MAX_PRIORITY 5;

int quantum_time = 5;

typedef struct ready_queue_node READY_QUEUE_NODE; //cria estrutura recursiva

struct ready_queue_node { //estrutura possui um ponteiro para um pcd e um ponteiro para a próxima estrutura da fila
    PCB *pcb;
    READY_QUEUE_NODE *next;
};

struct READY_QUEUE { //estrutura que aponta para o primeiro e último nó da fila
    int id;
    int new_quantum_value;
    READY_QUEUE_NODE *first;
    READY_QUEUE_NODE *last;
};

struct READY_QUEUE ready_queue_priority_0;
struct READY_QUEUE ready_queue_priority_1;
struct READY_QUEUE ready_queue_priority_2;
struct READY_QUEUE ready_queue_priority_3;
struct READY_QUEUE ready_queue_priority_4;
struct READY_QUEUE ready_queue_priority_5;

struct READY_QUEUE vector[6];

BOOL ready_queue_is_empty(int id_queue) //retorna verdadeiro se a fila estiver vazia
{
    if (vector[id_queue].first == NULL)
        return true;
    else
        return false;
}

/* IMPLEMENTAÇÃO DO ROUND ROBIN SEM PRIORIDADE ENTRE PROCESSOS*/
// utiliza filas do tipo FIFO
// Deleta o processo da fila quando o encontra
PCB *get_next_pcb() {
    PCB *next_pcb;

    int count = 0;
    while (count < 6) {
        if (!ready_queue_is_empty(count)) {
            READY_QUEUE_NODE *first_node = vector[count].first; //salva o primeiro elemento em uma variável
            vector[count].first = vector[count].first->next; // transforma o segundo em primeiro
            next_pcb = first_node->pcb; //passa o processo para uma variável
            quantum_time = vector[count].new_quantum_value; //atualiza valor global do quantum
            free(first_node); //libera o espaço da memória
            break;
        } else {
            if (count == 5) {
                return NULL;
            }
        }
        count++;
    }
    return next_pcb; //retorna o processo
}

BOOL exists(PCB *pcb) //verifica se já existe o processo na fila de pronto
{
    BOOL found = false;

    int count = 0;
    while (count < 6) {
        if (!ready_queue_is_empty(count)) //se a fila não estiver vazia
        {
            READY_QUEUE_NODE *current = vector[count].first; //pega o primeiro elemento
            do {
                if (current->pcb->pcb_id != pcb->pcb_id) //percorre a fila até achar
                    current = current->next;
                else {
                    found = true;
                    break;
                }
            } while (current != NULL && !found); // ou até o fim da fila
        }
        count++;
    }
    return found; //retorna true se encontrou
}

READY_QUEUE_NODE *create_new_node(PCB *pcb) {
    READY_QUEUE_NODE *new_node = (READY_QUEUE_NODE *) malloc(sizeof(READY_QUEUE_NODE));
    new_node->pcb = pcb;
    new_node->next = NULL;
    return new_node;
}

void print_queue() {

    fprintf(stderr, "-------------------INICIO DO METODO PRINT QUEUE------------------------\n");
    int count = 0;
    while (count < 6) {

        fprintf(stderr, "PRiNTANDO FILA %d\n", count);
        if (!ready_queue_is_empty(count)) {
            fprintf(stderr, "FILA NAO VAZIA %d\n", count);
            READY_QUEUE_NODE *current = vector[count].first; //pega o primeiro elemento
            fprintf(stderr, "PEGA PRIMEIRO ELEMENTO %d\n", count);

            do {
                fprintf(stderr,
                        "------------------------------NODE: %d----PROIRIDADE: %d---STATUS: %d----QUANTUM: %d------\n",
                        current->pcb->pcb_id,
                        current->pcb->priority, (int) current->pcb->status, vector[count].new_quantum_value);

                if (current->next != NULL)
                    current = current->next;
                else
                    current = NULL;

            } while (current != NULL); //até o fim da fila
        } else {
            fprintf(stderr, "FILA VAZIA %d\n", count);
        }
        count++;
    }

    fprintf(stderr, "-------------------FIM DO METODO PRINT QUEUE------------------------\n");
}

/****************************************************************************/
/*                                                                          */
/*              Module CPU                  */
/*               Internal Routines                  */
/*                                                                          */
/****************************************************************************/


void cpu_init() {
    set_timer(quantum_time);

    ready_queue_priority_0.first = NULL; //maior prioridade
    ready_queue_priority_0.last = NULL;
    ready_queue_priority_0.id = 0;
    ready_queue_priority_0.new_quantum_value = 5; //menor quantum
    vector[0] = ready_queue_priority_0;

    ready_queue_priority_1.first = NULL;
    ready_queue_priority_1.last = NULL;
    ready_queue_priority_1.id = 1;
    ready_queue_priority_1.new_quantum_value = 10;
    vector[1] = ready_queue_priority_1;

    ready_queue_priority_2.first = NULL;
    ready_queue_priority_2.last = NULL;
    ready_queue_priority_2.id = 2;
    ready_queue_priority_2.new_quantum_value = 15;
    vector[2] = ready_queue_priority_2;

    ready_queue_priority_3.first = NULL;
    ready_queue_priority_3.last = NULL;
    ready_queue_priority_3.id = 3;
    ready_queue_priority_3.new_quantum_value = 20;
    vector[3] = ready_queue_priority_3;

    ready_queue_priority_4.first = 0;
    ready_queue_priority_4.last = 0;
    ready_queue_priority_4.id = 4;
    ready_queue_priority_4.new_quantum_value = 25;
    vector[4] = ready_queue_priority_4;

    ready_queue_priority_5.first = 0; //menor prioridade
    ready_queue_priority_5.last = 0;
    ready_queue_priority_5.id = 5;
    ready_queue_priority_5.new_quantum_value = 30; //maior quantum
    vector[5] = ready_queue_priority_5;

}

void insert_into_queue(int id_queue, READY_QUEUE_NODE *new_node) {
    if (!ready_queue_is_empty(id_queue)) {
        READY_QUEUE_NODE *current = vector[id_queue].first; //pega o primeiro elemento
        do {
            if (current->next != NULL) { //percorre a fila até achar
                current = current->next;
            } else {
                current->next = new_node;
                break;
            }
        } while (current != NULL); //até o fim da fila

    } else {
        vector[id_queue].first = new_node;
    }
    vector[id_queue].last = new_node;
}

void find_new_queue(READY_QUEUE_NODE *new_node) {
    switch (new_node->pcb->priority) {
        case 0:
            insert_into_queue(5, new_node);
            break;
        case 1:
            insert_into_queue(4, new_node);
            break;
        case 2:
            insert_into_queue(3, new_node);
            break;
        case 3:
            insert_into_queue(2, new_node);
            break;
        case 4:
            insert_into_queue(1, new_node);
            break;
        case 5:
            insert_into_queue(0, new_node);
            break;
    }
}


void dispatch() {
    // Verifica se havia outro processo rodando antes e o coloca no final da lista de pronto

    if (PTBR != NULL && PTBR->pcb->status == running) {
        insert_ready(PTBR->pcb);
    }

    // Pega o primeiro processo da fila de pronto
    PCB *next_pcb = get_next_pcb();


    // Aloca o processo

    if (next_pcb != NULL) {
        PTBR = next_pcb->page_tbl;
        next_pcb->status = running;
        prepage(next_pcb);
        next_pcb->last_dispatch = get_clock();
        set_timer(quantum_time);
    } else {
        PTBR = NULL;
    }
}

void insert_ready(PCB *pcb) {

    if (!exists(pcb)) {

        READY_QUEUE_NODE *new_node = create_new_node(pcb);
        if (ready_queue_is_empty(0) && ready_queue_is_empty(1) &&
            ready_queue_is_empty(2) && ready_queue_is_empty(3) &&
            ready_queue_is_empty(4) && ready_queue_is_empty(5)) {

            new_node->pcb->priority = MAX_PRIORITY;


        } else {
            // Calculando nova prioridade //
            if (pcb->accumulated_cpu == 0) { // Novo processo
                pcb->priority = MAX_PRIORITY;
            } else {
                int cpu_time_used = new_node->pcb->last_cpuburst;

                if (cpu_time_used > quantum_time * 0.9 && new_node->pcb->priority != 0) {
                    new_node->pcb->priority--; //prioridade do processo diminui caso ele use mais de 90% do quantum

                } else if (cpu_time_used < quantum_time * 0.5 && new_node->pcb->priority != 5) {
                    new_node->pcb->priority++; // prioridade do processo aumenta caso ele use menos de 50% do quantum
                }
            }

        }
        find_new_queue(new_node);
        // muda o status do processo para "ready"
        pcb->status = ready;
    }
}

/* end of module */
