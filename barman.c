#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
//É possível alterar o número aqui para verificar a flexibilidade do código
#define N_CLIENTES 25
#define N_BARMANS 2
#define N_CADEIRAS 3

typedef enum {A, W, B, S, E, L} estado_cliente;  // A: chegou, W: esperando, B: bebendo, S: saiu, E: encerrado, L: largou
const char* estadoClienteStr[] = {" Chegou ", " Esperando ", " Bebendo ", " Saiu ", " Encerrado ", " Largou "};

typedef enum {ALCOOLICA, NAO_ALCOOLICA} tipo_bebida;
const char* bebidaStr[] = {"Alc", "Non"};

estado_cliente estadoC[N_CLIENTES];
tipo_bebida bebidaCliente[N_CLIENTES];
int clientesCadeira[N_CADEIRAS];
int clientesServidos[N_BARMANS];

estado_cliente estadoBarman[N_BARMANS]; // B (servindo) ou S (dormindo)

tipo_bebida bebidaAtual;
int bebidaEmUso = 0;
int clientesNoBar = 0;

sem_t sem_cadeiras;
sem_t mutex_bebida;
sem_t mutex_impressao;
sem_t mutex_contador;
sem_t sem_cliente[N_CLIENTES];
sem_t sem_barman[N_BARMANS];

void imprimeEstado() {
    sem_wait(&mutex_impressao);
    printf("\033[H\033[J");
    printf("\n============= ESTADO DO BAR =============\n");
    //Imprime o tipo da bebida e se não tiver ninguém, printa "nenhuma"
    printf("Tipo de bebida sendo servida: %s\n", bebidaEmUso ? bebidaStr[bebidaAtual] : "Nenhuma");

    printf("\nClientes:\n");
    for (int i = 0; i < N_CLIENTES; i++) {
        //Caso o cliente ainda não tenha sido criado, seu tipo de bebida ainda não foi definido
        if (estadoC[i]== E)
        {
            printf("C%02d [%s| ]  ", i, estadoClienteStr[estadoC[i]]);
            //Pula a linha depois de cinco clientes
            if ((i+1)%5==0) printf("\n");
        }else{
        
        printf("C%02d [%s|%s]  ", i, estadoClienteStr[estadoC[i]], bebidaStr[bebidaCliente[i]]);
        if ((i+1)%5==0) printf("\n");
        }
    }

    printf("\n\nCadeiras de espera: ");
    for (int i = 0; i < N_CADEIRAS; i++) {
        if (clientesCadeira[i] == -1) printf("[   ] ");
        //Printa o cliente da cadeira.
        else printf("[C%02d] ", clientesCadeira[i]);
    }

    printf("\n\nBarmans:\n");
    for (int i = 0; i < N_BARMANS; i++) {
        if (estadoBarman[i] == B && clientesServidos[i] != -1)
            printf("Barman %d atendendo C%02d\n", i, clientesServidos[i]);
        else
            printf("Barman %d dormindo\n", i);
    }
    printf("========================================\n\n");
    sem_post(&mutex_impressao);
}

void* cliente(void* arg) {
    int id = *(int*)arg;
    //Cria o cliente e define sua bebida aleatoriamente, seu estado vira A
    bebidaCliente[id] = rand() % 2;
    estadoC[id] = A;
    imprimeEstado();
    sleep(rand()%3);
    //Se tem cadeiras começa o processo
    if (sem_trywait(&sem_cadeiras) == 0) {
        sem_wait(&mutex_bebida);
        //Se o bar não está servindo nenhum tipo de bebida, o tipo vira o do cliente
        if (!bebidaEmUso) {
            bebidaAtual = bebidaCliente[id];
            bebidaEmUso = 1;
        }
        //Se for outro tipo ele larga
        if (bebidaCliente[id] != bebidaAtual) {
            sem_post(&mutex_bebida);
            estadoC[id] = L;
            imprimeEstado();
            sem_post(&sem_cadeiras);
            return NULL;
        }

        clientesNoBar++;
        sem_post(&mutex_bebida);
        //Senta na cadeira e espera ser servido
        estadoC[id] = W;
        for (int i = 0; i < N_CADEIRAS; i++) {
            if (clientesCadeira[i] == -1) {
                clientesCadeira[i] = id;
                break;
            }
        }
        imprimeEstado();

        sem_wait(&sem_cliente[id]);
        //Começa a ser servido
        estadoC[id] = B;
        imprimeEstado();

        sleep(rand()%3 + 1);
        //Sai depois de um tempo
        estadoC[id] = S;
        imprimeEstado();

        sem_wait(&mutex_bebida);
        clientesNoBar--;
        if (clientesNoBar == 0) {
            bebidaEmUso = 0;
        }
        sem_post(&mutex_bebida);

        sem_post(&sem_cadeiras);
    } else {
    //Larga se não tiver cadeiras
        estadoC[id] = L;
        imprimeEstado();
    }

    return NULL;
}

void* barman(void* arg) {
    int id = *(int*)arg;
    while (1) {
        //Vê se tem clientes esperando
        sem_wait(&mutex_contador);
        int cid = -1;
        for (int i = 0; i < N_CADEIRAS; i++) {
            if (clientesCadeira[i] != -1) {
                cid = clientesCadeira[i];
                clientesCadeira[i] = -1;
                break;
            }
        }
        sem_post(&mutex_contador);

        if (cid != -1) {
            //Chama o cliente que estava esperando, muda seu estado pra B e serve o cliente, depois volta a dormir
            estadoBarman[id] = B;
            clientesServidos[id] = cid;
            imprimeEstado();
            sem_post(&sem_cliente[cid]);
            sleep(rand()%2 + 1);
            estadoBarman[id] = S;
            clientesServidos[id] = -1;
            imprimeEstado();
        } else {
            //Se não tem cliente continua dormindo
            estadoBarman[id] = S;
            clientesServidos[id] = -1;
            imprimeEstado();
            sleep(1);
        }
    }
    return NULL;
}

int main() { //Criação dos semáforos e threads
    srand(time(NULL));
    pthread_t thr_clientes[N_CLIENTES], thr_barmans[N_BARMANS];
    int id_cl[N_CLIENTES], id_bm[N_BARMANS];

    sem_init(&sem_cadeiras, 0, N_CADEIRAS);
    sem_init(&mutex_bebida, 0, 1);
    sem_init(&mutex_impressao, 0, 1);
    sem_init(&mutex_contador, 0, 1);

    for (int i = 0; i < N_CLIENTES; i++) {
        sem_init(&sem_cliente[i], 0, 0);
        estadoC[i] = E;
    }
    for (int i = 0; i < N_CADEIRAS; i++) {
        clientesCadeira[i] = -1;
    }
    for (int i = 0; i < N_BARMANS; i++) {
        estadoBarman[i] = S;
        clientesServidos[i] = -1;
    }

    for (int i = 0; i < N_BARMANS; i++) {
        id_bm[i] = i;
        pthread_create(&thr_barmans[i], NULL, barman, &id_bm[i]);
    }
    for (int i = 0; i < N_CLIENTES; i++) {
        id_cl[i] = i;
        pthread_create(&thr_clientes[i], NULL, cliente, &id_cl[i]);
        sleep(1);
    }
    for (int i = 0; i < N_CLIENTES; i++) {
        pthread_join(thr_clientes[i], NULL);
    }

    printf("\nTodos os clientes foram atendidos ou foram embora. Encerrando o bar.\n");
    return 0;
}
