/******************************************************************************
 ** ISCTE-IUL: Trabalho prático 2 de Sistemas Operativos
 **
 ** Aluno: Nº: 104670      Nome: Vasco Mendes Baleia
 ** Nome do Módulo: servidor.c v1
 ** Descrição/Explicação do Módulo: 
 **
 **
 ******************************************************************************/
#include "common.h"
#include "utils.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
//#define DEBUG_MODE FALSE                         // To disable debug messages, uncomment this line


/* Variáveis globais */
Passagem pedido;                                    // Variável que tem o pedido enviado do Cliente para o Servidor
Passagem lista_passagens[NUM_PASSAGENS];            // BD com o Número de pedidos em simultâneo que o servidor suporta
int indice_lista;                                   // Índice corrente da Lista, que foi reservado pela função reservaEntradaBD()
Contadores stats;                                   // Contadores de estatisticas

/* Protótipos de funções */
int init(Passagem*);                                // S1:   Função a ser implementada pelos alunos
int loadStats(Contadores*);                         // S2:   Função a ser implementada pelos alunos
int criaFicheiroServidor();                         // S3:   Função a ser implementada pelos alunos
int criaFifo();                                     // S4:   Função a ser implementada pelos alunos
int armaSinais();                                   // S5:   Função a ser implementada pelos alunos
Passagem lePedido();                                // S6:   Função a ser implementada pelos alunos
int validaPedido(Passagem);                         // S7:   Função a ser implementada pelos alunos
int reservaEntradaBD(Passagem*, Passagem);          // S8:   Função a ser implementada pelos alunos
int apagaEntradaBD(Passagem*, int);                 //       Função a ser implementada pelos alunos
int criaServidorDedicado(Passagem*, int);           // S9:   Função a ser implementada pelos alunos
void trataSinalSIGINT(int);                         // S10:  Função a ser implementada pelos alunos
void trataSinalSIGHUP(int, siginfo_t*, void*);      // S11:  Função a ser implementada pelos alunos
void trataSinalSIGCHLD(int);                        // S12:  Função a ser implementada pelos alunos
int sd_armaSinais();                                // SD13: Função a ser implementada pelos alunos
int sd_iniciaProcessamento(Passagem);               // SD14: Função a ser implementada pelos alunos
int sd_sleepRandomTime();                           // SD15: Função a ser implementada pelos alunos
int sd_terminaProcessamento(Passagem);              // SD16: Função a ser implementada pelos alunos
void sd_trataSinalSIGTERM(int);                     // SD17: Função a ser implementada pelos alunos


int main() {    // Não é suposto que os alunos alterem nada na função main()
    // S1
    exit_on_error(init(lista_passagens), "Init");
    // S2
    exit_on_error(loadStats(&stats), "loadStats");
    // S3
    exit_on_error(criaFicheiroServidor(), "criaFicheiroServidor");
    // S4
    exit_on_error(criaFifo(), "criaFifo");
    // S5
    exit_on_error(armaSinais(), "armaSinais");

    while (TRUE) {  // O processamento do Servidor é cíclico e iterativo
        // S6
        pedido = lePedido();
        if (pedido.tipo_passagem < 0)
            continue;
        // S7
        if (validaPedido(pedido) < 0)
            continue;
        // S8
        indice_lista = reservaEntradaBD(lista_passagens, pedido);
        if (indice_lista < 0)
            continue;
        // S9
        int pidFilho = criaServidorDedicado(lista_passagens, indice_lista);
        if (pidFilho < 0) {
            apagaEntradaBD(lista_passagens, indice_lista);
            continue;
        } else if (pidFilho > 0) // Processo Servidor - Pai
            continue;

        // Processo Servidor Dedicado - Filho
        // SD13
        exit_on_error(sd_armaSinais(), "sd_armaSinais");
        // SD14
        exit_on_error(sd_iniciaProcessamento(pedido), "sd_iniciaProcessamento");
        // SD15
        exit_on_error(sd_sleepRandomTime(), "sd_sleepRandomTime");
        // SD16
        exit_on_error(sd_terminaProcessamento(pedido), "sd_terminaProcessamento");
    }
}

/**
 *  O módulo Servidor de Passagens é responsável pelo processamento de pedidos de passagem que chegam ao sistema  Scut-IUL.
 *  Este módulo é, normalmente, o primeiro dos dois (Cliente e Servidor) a ser executado, e deverá estar sempre ativo,
 *  à espera de pedidos de passagem. O tempo de processamento destes pedidos varia entre os MIN_PROCESSAMENTO segundos
 *  e os MAX_PROCESSAMENTO segundos. Findo esse tempo, este módulo sinaliza ao condutor de que a sua passagem foi processada.
 *  Este módulo deverá possuir contadores de passagens por tipo, um contador de anomalias e uma lista com capacidade para processar NUM_PASSAGENS passagens.
 *  O módulo Servidor de Passagens é responsável por realizar as seguintes tarefas:
 */

/**
 * S1   Inicia a lista de passagens, preenchendo em todos os elementos o campo tipo_passagem=-1 (“Limpa” a lista de passagens).
 *      Em seguida, dá success S1 "Init Servidor";
 *
 * @return int Sucesso
 */
int init(Passagem* bd) {
    debug("S1", "<");
    for(int i = 0; i < NUM_PASSAGENS; i++){
        bd[i].tipo_passagem = -1;
    }
    success("S1", "Init Servidor");
    debug("S1", ">");
    return 0;
}

/**
 * S2   Deverá manter um contador por cada tipo de passagem (Normal ou Via Verde) e um contador para as passagens com anomalia.
 *      Se o ficheiro FILE_STATS existir na diretoria local, abre-o e lê os seus dados (em formato binário, ver formato em S10.4)
 *      para carregar o valor guardado de todos os contadores. Se houver erro na leitura do ficheiro, dá error S2 "<Problema>",
 *      caso contrário, dá success S2 "Estatísticas Carregadas".
 *      Se o ficheiro FILE_STATS não existir, inicia os três contadores com o valor 0 e dá success S2 "Estatísticas Iniciadas";
 *
 * @return int Sucesso
 */
int loadStats(Contadores *pStats){
    debug("S2", "<");
    int pidServer = getpid();
   // char str[20];
    FILE *stats = fopen(FILE_STATS, "rb");
    if (stats == NULL) {
            pStats->contadorAnomalias = 0;
            pStats->contadorNormal = 0;
            pStats->contadorViaVerde = 0;
            success("S2", "Estatísticas Iniciadas");
        }
    else{ 
           if(fread(&pStats, sizeof(pStats), 1, stats) < 1){
                error("S2","Problema ao carregar as estatisticas");
                kill(pidServer, SIGKILL);
           }
            else{
            fclose(stats);
            success("S2", "Estatísticas Carregadas");
            }
        
    } 
    debug("S2", ">");
    return 0;
}

/**
 * S3   Cria o ficheiro FILE_SERVIDOR, e escreve nesse ficheiro o PID do Servidor (em formato de texto).
 *      Se houver erro em qualquer das operações, dá error S3 <Problema>, caso contrário, dá success S3 "<PID Servidor>";
 *
 * @return int Sucesso
 */
int criaFicheiroServidor() {
    debug("S3", "<");
    int pidServer = getpid();
    FILE *pSV = fopen(FILE_SERVIDOR,"w");
    if(pSV == NULL){
        error("S3","Não foi possível criar o ficheiro");
        kill(pidServer, SIGKILL);
    }
    if(fwrite(&pidServer, sizeof(pidServer), 1, pSV) < 1){
        error("S3","Erro de Escrita");
        kill(pidServer, SIGKILL);
    }
    else{
           success("S3", "%d", pidServer);
       }
    fclose(pSV);    
    debug("S3", ">");
    return 0;
}

/**
 * S4   Cria o ficheiro com organização FIFO (named pipe) FILE_PEDIDOS. 
 *      Se houver erro na operação, dá error S4 "<Problema>", caso contrário, dá  success S4 "Criei FIFO";
 *
 * @return int Sucesso
 */
int criaFifo() {
    debug("S4", "<");
    mkfifo(FILE_PEDIDOS,0666);
    FILE* fifo = fopen(FILE_PEDIDOS,"rb");
    if(fifo != NULL){
        success("S4", "Criei FIFO");
    }
    else{
        error("S4", "Erro na criação do FIFO %s",FILE_PEDIDOS);
    }
    fclose(fifo);
    debug("S4", ">");
    return 0;
}

/**
 * S5   Arma e trata os sinais SIGINT (ver S10), SIGHUP (usando sigaction(), ver S11) e SIGCHLD (ver S12).
 *      Depois de armar os sinais, dá success S5 "Armei sinais";
 *
 * @return int Sucesso
 */
int armaSinais() {
    debug("S5", "<");
    signal(SIGHUP,trataSinalSIGHUP);
    struct sigaction sa;
        sa.sa_flags = SA_SIGINFO;
        sa.sa_sigaction = trataSinalSIGHUP;
        sigaction(SIGHUP, &sa, NULL);
    signal(SIGCHLD,trataSinalSIGCHLD);
    success("S5","Armei sinais");
    debug("S5", ">");
    return 0;
}

/**
 * S6   Lê a informação do FIFO FILE_PEDIDOS, (em formato binário) que deverá ser um elemento do tipo Passagem.
 *      Se houver erro na operação, dá error S6 "<Problema>", caso contrário, dá success S6 "Li FIFO";
 *
 * @return Passagem Elemento com os dados preenchidos. Se tipo_passagem = -1, significa que o elemento é imválido
 */
Passagem lePedido() {
    debug("S6", "<");
    Passagem p;
    p.tipo_passagem = -1;   // Por omissão, retorna valor inválido
    FILE *fifo = fopen(FILE_PEDIDOS, "rb");
    if(FIFO != NULL){
        fread((&p, sizeof(p), 1, fifo) < 1);
        success("S6","Li FIFO");
        fclose(fifo);
    }
    else{
        error("S6","O ficheiro FIFO %s não existe",FILE_PEDIDOS);
    }
    debug("S6", ">");
    return p;
}

/**
 * S7   O Servidor deve validar se o pedido está corretamente formatado. A formatação correta de um pedido será:
 *      •  O Tipo de passagem é válido (1 para pedido Normal, ou 2 para Via Verde);
 *      •  A Matrícula e o Lanço não são strings vazias (não é necessário fazer mais validações sobre o seu conteúdo);
 *      •  O pid_cliente é um valor > 0. Não se fazem validações sobre pid_servidor_dedicado.
 *      Em caso de erro na formatação do pedido, dá error S7, indicando qual foi o erro detetado, 
 *      incrementa o contador de anomalias, ignora o pedido, e recomeça o processo no passo S6. Caso contrário, 
 *      dá success S7 "Chegou novo pedido de passagem do tipo <Normal | Via Verde> solicitado pela viatura com matrícula <matricula> para o Lanço <lanco> e com PID <pid_cliente>";
 *
 * @return int Sucesso
 */
int validaPedido(Passagem pedido) {
    debug("S7", "<");

    debug("S7", ">");
    return 0;
}

/**
 * S8   Verifica se existe disponibilidade na Lista de Passagens. Se todas as entradas da Lista de Passagens estiverem ocupadas, 
 *      dá error S8 "Lista de Passagens cheia", incrementa o contador de passagens com anomalia, manda o sinal SIGHUP ao processo 
 *      com PID <pid_cliente>, ignora o pedido, e recomeça o processo no passo S6. 
 *      Caso contrário, preenche uma entrada da lista com os dados deste pedido, incrementa o contador de passagens do tipo de passagem correspondente 
 *      e dá success S8 "Entrada <índice lista> preenchida";
 *
 * @return int Em caso de sucesso, retorna o índice da lista preenchido. Caso contrário retorna -1
 */
int reservaEntradaBD(Passagem* bd, Passagem pedido) {
    debug("S8", "<");
    int indiceLista = -1;

    debug("S8", ">");
    return indiceLista;
}

/**
 * "Apaga" uma entrada da Lista de Passagens, colocando tipo_passagem = -1
 *
 * @return int Sucesso
 */
int apagaEntradaBD(Passagem* bd, int indiceLista) {
    debug("", "<");

    debug("", ">");
    return 0;
}

/**
 * S9   Cria um processo filho (fork) Servidor Dedicado. Se houver erro, dá error S9 "Fork". 
 *      Caso contrário: O processo Servidor Dedicado (filho) continua no passo SD13, 
 *      e o processo Servidor (pai) completa o preenchimento da entrada atual da Lista de Passagens com o PID do Servidor Dedicado, 
 *      e dá success S9 "Criado Servidor Dedicado com PID <pid Filho>". Em qualquer dos casos, de erro ou de sucesso, recomeça o processo no passo S6;
 *
 * @return int PID do processo filho, se for o processo Servidor (pai), 0 se for o processo Servidor Dedicado (filho), ou -1 em caso de erro.
 */
int criaServidorDedicado(Passagem* bd, int indiceLista) {
    debug("S9", "<");
    int pidFilho = -1;

    debug("S9", ">");
    return pidFilho;
}

/**
 * S10  O sinal armado SIGINT serve para o Diretor da Portagem encerrar o Servidor, usando o atalho <CTRL+C>. 
 *      Se receber esse sinal (do utilizador via Shell), o Servidor dá success S10 "Shutdown Servidor", e depois:
 *      S10.1   Envia o sinal SIGTERM a todos os Servidores Dedicados da Lista de Passagens, 
 *              para que concluam o seu processamento imediatamente. Depois, dá success S10.1 "Shutdown Servidores Dedicados";
 *      S10.2   Remove o ficheiro servidor.pid. Em caso de erro, dá error S10.2, caso contrário, dá success S10.2;
 *      S10.3   Remove o FIFO pedidos.fifo. Em caso de erro, dá error S10.3, caso contrário, dá success S10.3;
 *      S10.4   Cria o ficheiro estatisticas.dat, escrevendo nele o valor de 3 inteiros (em formato binário), correspondentes a
 *              <contador de passagens Normal>  <contador de passagens Via Verde>  <contador Passagens com Anomalia>
 *              Em caso de erro, dá error S10.4, caso contrário, dá success S10.4 "Estatísticas Guardadas";
 *      S10.5   Dá success S10.5 e termina o processo Servidor.
 */
void trataSinalSIGINT(int sinalRecebido) {
    debug("S10", "<");

    debug("S10", ">");
}

/**
 * S11  O sinal armado SIGHUP serve para o Cliente indicar que deseja cancelar o pedido de processamento a passagem. 
 *      Se o Servidor receber esse sinal, dá success S11 "Cancel", e em seguida, terá de fazer as seguintes ações:
 *      S11.1   Identifica o PID do processo Cliente que enviou o sinal (usando sigaction), dá success S11.1 "Cancelamento enviado pelo Processo <PID Cliente>";
 *      S11.2   Pesquisa na Lista de Passagens pela entrada correspondente ao PID do Cliente que cancelou. Se não encontrar, dá error S11.2.
 *              Caso contrário, descobre o PID do Servidor Dedicado correspondente, dá success S11.2 "Cancelamento <PID Filho>";
 *      S11.3   Envia o sinal SIGTERM ao Servidor Dedicado da Lista de Passagens correspondente ao cancelamento, 
 *              para que conclua o seu processamento imediatamente. Depois, dá success S10.1 "Cancelamento Shutdown Servidor Dedicado", 
 *              e recomeça o processo no passo S6.
 */
void trataSinalSIGHUP(int sinalRecebido, siginfo_t* info, void* uap) {
    debug("S11", "<");

    debug("S11", ">");
}

/**
 * S12  O sinal armado SIGCHLD serve para que o Servidor seja alertado quando um dos seus filhos Servidor Dedicado terminar.
 *      Se o Servidor receber esse sinal, dá success S12 "Servidor Dedicado Terminou", e em seguida, terá de fazer as seguintes ações:
 *      S12.1   Identifica o PID do Servidor Dedicado que terminou (usando wait), dá success S12.1 "Terminou Servidor Dedicado <PID Filho>";
 *      S12.2   Pesquisa na Lista de Passagens pela entrada correspondente ao PID do Filho que terminou. 
 *              Se não encontrar, dá error S12.2. Caso contrário, “apaga” a entrada da Lista de Passagens 
 *              correspondente (colocando tipo_passagem=-1), dá success S12.2, e recomeça o processo no passo S6.
 */
void trataSinalSIGCHLD(int sinalRecebido) {
    debug("S12", "<");

    debug("S12", ">");
}

/**
 * SD13 O novo processo Servidor Dedicado (filho) arma os sinais SIGTERM (ver SD17) e SIGINT (programa para ignorar este sinal). 
 *      Depois de armar os sinais, dá success SD13 "Servidor Dedicado Armei sinais";
 *
 * @return int Sucesso
 */
int sd_armaSinais() {
    debug("SD13", "<");

    debug("SD13", ">");
    return 0;
}

/**
 * SD14 O Servidor Dedicado envia o sinal SIGUSR1, indicando o início do processamento da passagem, ao processo <pid_cliente> 
 *      que pode obter da estrutura Passagem do pedido que “herdou” do Servidor ou da entrada da Lista de Passagens, 
 *      e dá success SD14 "Início Passagem <PID Cliente> <PID Servidor Dedic>";
 *
 * @return int Sucesso
 */
int sd_iniciaProcessamento(Passagem pedido) {
    debug("SD14", "<");

    debug("SD14", ">");
    return 0;
}

/**
 * SD15 O Servidor Dedicado calcula um valor de tempo aleatório entre os valores MIN_PROCESSAMENTO e MAX_PROCESSAMENTO, 
 *      dá success SD15 "<Tempo>", e aguarda esse valor em segundos (sleep);
 *
 * @return int Sucesso
 */
int sd_sleepRandomTime() {
    debug("SD15", "<");

    debug("SD15", ">");
    return 0;
}

/**
 * SD16 O Servidor Dedicado envia o sinal SIGTERM, indicando o fim do processamento da passagem, ao processo <pid_cliente>, 
 *      dá success SD16 "Fim Passagem <PID Cliente> <PID Servidor Dedicado>", e termina o Servidor Dedicado;
 *
 * @return int Sucesso
 */
int sd_terminaProcessamento(Passagem pedido) {
    debug("SD16", "<");

    debug("SD16", ">");
    return 0;
}

/**
 * SD17 O sinal armado SIGTERM serve para o Servidor indicar que deseja terminar imediatamente o pedido de processamento da passagem.
 *      Se o Servidor Dedicado receber esse sinal, envia o sinal SIGHUP ao <pid_cliente>, 
 *      dá success SD17 "Processamento Cancelado", e termina o Servidor Dedicado.
 */
void sd_trataSinalSIGTERM(int sinalRecebido) {
    debug("SD17", "<");

    debug("SD17", ">");
}
