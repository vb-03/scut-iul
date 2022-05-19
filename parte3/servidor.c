/******************************************************************************
 ** ISCTE-IUL: Trabalho prático 2 de Sistemas Operativos
 **
 ** Aluno: Nº: 104670      Nome: Vasco Mendes Baleia
 ** Nome do Módulo: servidor.c versão 3
 ** Descrição/Explicação do Módulo: 
 **
 **
 ******************************************************************************/
#include "common.h"
#include "utils.h"

/******************************************************************************
 * FUNÇÕES UTILITÁRIAS FORNECIDAS PELOS VOSSOS QUERIDOS PROFESSORES DE SO...
 *****************************************************************************/

/******************************************************************************
 * FUNÇÕES IPC SEMÁFOROS
 *****************************************************************************/

 /**
  * @brief Função interna, não é para ser utilizada diretamente pelos alunos
  */
 int __semGet( int nrSemaforos, int semFlags ) {
    int id = semget( IPC_KEY, nrSemaforos, semFlags );
    if ( id < 0 ) {
        debug( "Não consegui criar/abrir o grupo de semáforos com key=0x%x", IPC_KEY );
    } else {
        debug( "Estou a usar o grupo de semáforos com key=0x%x e id=%d", IPC_KEY, id );
    }
    return id;
}

/**
 * @brief Cria um grupo de semáforos IPC associado a IPC_KEY com o número de semáforos passado
 * 
 * @param nrSemaforos número de semáforos deste grupo de semáforos
 * @return int em caso de sucesso, retorna o IPC_ID correspondente. Em caso de erro, retorna -1
 */
int semCreate( int nrSemaforos ) {
    return __semGet( nrSemaforos, IPC_CREAT | 0666 );
}

/**
 * @brief "Liga-se" a um grupo de semáforos IPC associado a IPC_KEY
 * 
 * @return int em caso de sucesso, retorna o IPC_ID correspondente. Em caso de erro, retorna -1
 */
int semGet() {
    return __semGet( 0, 0 );
}

/**
 * @brief Remove o grupo de semáforos IPC associado a IPC_KEY
 * 
 * @return int 0 if success, -1 if the semaphore group exists and could not be removed
 */
int semRemove() {
    int id = semGet();
    // Ignore any errors here, as this is only to check if the semaphore group exists and remove it
    if ( id > 0 ) {
        // If the semaphore group with IPC_KEY already exists, remove it.
        int result = semctl( id, 0, IPC_RMID, 0 );
        if ( result < 0) {
            debug( "Não consegui remover este grupo de semáforos" );
        } else {
            debug( "Removi este grupo de semáforos" );
        }
        return result;
    }
    return 0;
}

/**
 * @brief Define o valor do semáforo semNr do grupo de semáforos IPC associado a IPC_KEY
 * 
 * @param semNr índice do semáforo a definir (começando em 0)
 * @param value valor a ser definido no semáforo semNr
 * @return int success
 */
int semNrSetValue( int semNr, int value ) {
    int id = semGet();
    exit_on_error( id, "Erro semget" );
    int result = semctl( id, semNr, SETVAL, value );
    if ( result < 0) {
        debug( "Não consegui definir o valor do semáforo %d deste grupo de semáforos", semNr );
    } else {
        debug( "O semáforo %d deste grupo de semáforos ficou com o valor %d", semNr, value );
    }
    return result;
}

/**
 * @brief Define o valor do semáforo 0 do grupo de semáforos IPC associado a IPC_KEY
 * 
 * @param value valor a ser definido no semáforo 0
 * @return int success
 */
int semSetValue( int value ) {
    return semNrSetValue( 0, value );
}

/**
 * @brief Obtém o valor do semáforo semNr do grupo de semáforos IPC associado a IPC_KEY
 * 
 * @param semNr índice do semáforo cujo valor queremos obter (começando em 0)
 * @return int valor do semáforo, ou -1 em caso de erro
 */
int semNrGetValue( int semNr ) {
    int id = semGet();
    exit_on_error( id, "Erro semget" );
    int result = semctl( id, semNr, GETVAL, 0 );
    if ( result < 0 ) {
        debug( "Não consegui obter o valor do semáforo %d deste grupo de semáforos", semNr );
    } else {
        debug( "O semáforo %d deste grupo de semáforos tem o valor %d", semNr, result );
    }
    return result;
}

/**
 * @brief Obtém o valor do semáforo 0 do grupo de semáforos IPC associado a IPC_KEY
 * 
 * @return int valor do semáforo, ou -1 em caso de erro
 */
int semGetValue() {
    return semNrGetValue( 0 );
}

/**
 * @brief Adiciona um valor ao semáforo semNr do grupo de semáforos IPC associado a IPC_KEY
 * 
 * @param semNr índice do semáforo a adicionar um valor (começando em 0)
 * @param addValue valor a ser adicionado no semáforo semNr
 * @return int success
 */
int semNrAddValue( int semNr, int addValue ) {
    int id = semGet();
    exit_on_error( id, "Erro semget" );
    int result = semctl( id, semNr, GETVAL, 0 );
    exit_on_error( result, "Erro semctl" );
    debug( "O semáforo %d deste grupo de semáforos tinha o valor %d", semNr, result );
  
    struct sembuf operation = { semNr, addValue, 0 };
    result = semop( id, &operation, 1 );

    if ( result < 0 ) {
        debug( "Não consegui adicionar o valor %d ao semáforo %d deste grupo de semáforos", addValue, semNr );
    } else {
        result = semctl( id, semNr, GETVAL, 0 );
        exit_on_error( result, "Erro semctl" );
        debug( "O semáforo %d deste grupo de semáforos passou a ter o valor %d", semNr, result );
    }
    return result;
}

/**
 * @brief Adiciona um valor ao semáforo 0 do grupo de semáforos IPC associado a IPC_KEY
 * 
 * @param addValue valor a ser adicionado no semáforo 0
 * @return int success
 */
int semAddValue( int addValue ) {
    return semNrAddValue( 0, addValue );
}

/**
 * @brief Adiciona 1 ao semáforo semNr do grupo de semáforos IPC associado a IPC_KEY
 * 
 * @param semNr índice do semáforo cujo valor queremos obter (começando em 0)
 * @return int success
 */
int semNrUp( int semNr ) {
    return semNrAddValue( semNr, 1 );
}

/**
 * @brief Adiciona -1 ao semáforo semNr do grupo de semáforos IPC associado a IPC_KEY
 * 
 * @param semNr índice do semáforo cujo valor queremos obter (começando em 0)
 * @return int success
 */
int semNrDown( int semNr ) {
    return semNrAddValue( semNr, -1 );
}

/**
 * @brief Adiciona 1 ao semáforo 0 do grupo de semáforos IPC associado a IPC_KEY
 * 
 * @return int success
 */
int semUp() {
    return semAddValue( 1 );
}

/**
 * @brief Adiciona -1 ao semáforo 0 do grupo de semáforos IPC associado a IPC_KEY
 * 
 * @return int success
 */
int semDown() {
    return semAddValue( -1 );
}

/* Variáveis globais */
int shmId;                                              // Variável que tem o ID da Shared Memory
int msgId;                                              // Variável que tem o ID da Message Queue
int semId;                                              // Variável que tem o ID do Array de Semáforos
Mensagem mensagem;                                      // Variável que tem o pedido enviado do Cliente para o Servidor
DadosServidor *dadosServidor;                           // Variável que vai ficar com a memória partilhada
int indice_lista;                                       // Índice corrente da Lista, que foi reservado pela função reservaEntradaBD()

/* Protótipos de funções */
int shmGet();                                           // S1:   Função a ser implementada pelos alunos
int shmCreateAndInit();                                 // S2:   Função a ser implementada pelos alunos
int loadStats( Contadores* );                           // S2.3: Função a ser implementada pelos alunos
int createIPC();                                        // S3:   Função a ser implementada pelos alunos
Mensagem recebePedido();                                // S4:   Função a ser implementada pelos alunos
int criaServidorDedicado();                             // S5:   Função a ser implementada pelos alunos
void trataSinalSIGINT( int );                           // S6:   Função a ser implementada pelos alunos
int sd_armaSinais();                                    // SD7:  Função a ser implementada pelos alunos
int sd_validaPedido( Mensagem );                        // SD8:  Função a ser implementada pelos alunos
int sd_reservaEntradaBD( DadosServidor*, Mensagem );    // SD9:  Função a ser implementada pelos alunos
int sd_apagaEntradaBD( DadosServidor*, int );           //       Função a ser implementada pelos alunos
int sd_iniciaProcessamento( Mensagem );                 // SD10: Função a ser implementada pelos alunos
int sd_sleepRandomTime();                               // SD11: Função a ser implementada pelos alunos
int sd_terminaProcessamento( Mensagem );                // SD12: Função a ser implementada pelos alunos
void sd_trataSinalSIGHUP( int );                        // SD13: Função a ser implementada pelos alunos
                                                        // SD14: Função a ser implementada pelos alunos

int main() {    // Não é suposto que os alunos alterem nada na função main()
    // S1
    if ( !shmGet() ) {
        // S2
        shmCreateAndInit();
    }
    // S3
    createIPC();

    while ( TRUE ) {  // O processamento do Servidor é cíclico e iterativo
        // S4
        mensagem = recebePedido();
        // S5
        int pidFilho = criaServidorDedicado();
        if ( !pidFilho ) {  // Processo Servidor Dedicado - Filho
            // SD7
            sd_armaSinais();
            // SD8
            sd_validaPedido( mensagem );
            // SD9
            indice_lista = sd_reservaEntradaBD( dadosServidor, mensagem );
            // SD10
            sd_iniciaProcessamento( mensagem );
            // SD11
            sd_sleepRandomTime();
            // SD12
            sd_terminaProcessamento( mensagem );
        }
    }
}

/**
 * @brief Função utilitária, fornecida pelos vossos queridos professores de SO... Utility to Display the values of the shared memory
 * 
 * @param shm Shared Memory
 * @param ignoreInvalid Do not display the elements that have the default value
 */
void shmView( DadosServidor* shm, int ignoreInvalid ) {
    debug( "Conteúdo da SHM Contadores: Normal: %d | Via Verde: %d | Anomalias: %d", shm->contadores.contadorNormal, shm->contadores.contadorViaVerde, shm->contadores.contadorAnomalias );
    debug( "Conteúdo da SHM Passagens:" );
    for ( int i = 0; i < NUM_PASSAGENS; ++i ) {
        if ( !ignoreInvalid || -1 != shm->lista_passagens[i].tipo_passagem ) {
            debug( "Posição %2d: %6d | %-9s | %-20s | %d", i, shm->lista_passagens[i].tipo_passagem, shm->lista_passagens[i].matricula, shm->lista_passagens[i].lanco, shm->lista_passagens[i].pid_cliente );
        }
    }
}

/**
 *  O módulo Servidor de Passagens é responsável pelo processamento de pedidos de passagem que chegam ao sistema Scut-IUL.
 *  Este módulo é, normalmente, o primeiro dos dois (Cliente e Servidor) a ser executado, e deverá estar sempre ativo,
 *  à espera de pedidos de passagem. O tempo de processamento destes pedidos varia entre os MIN_PROCESSAMENTO segundos
 *  e os MAX_PROCESSAMENTO segundos. Findo esse tempo, este módulo sinaliza ao condutor de que a sua passagem foi processada.
 *  Este módulo deverá possuir contadores de passagens por tipo, um contador de anomalias e uma lista com capacidade para
 *  processar NUM_PASSAGENS passagens. O módulo Servidor de Passagens é responsável por realizar as seguintes tarefas:
 */

/**
 * S1   Tenta abrir uma memória partilhada (shared memory) IPC que tem a KEY IPC_KEY definida em common.h
 *      (alterar esta KEY para ter o valor do nº do aluno, como indicado nas aulas).
 *      Se essa memória partilhada ainda não existir passa para o passo S2 sem erro. Caso contrário, liga-se a ela.
 *      Em caso de erro, dá error S1 "<Problema>", e termina o processo Servidor com exit code -1.
 *      Senão, dá success S1 "Abri Shared Memory já existente com ID <shmId>"
 *      --> Preenche as variáveis globais shmId e dadosServidor.
 *
 * @return int 0 se a memória partilhada ainda não existe no sistema ou 1 se a memória partilhada já existe no sistema
 */
int shmGet() {
    debug("S1 <");
    shmId = shmget(IPC_KEY,sizeof(*dadosServidor),0666);
    if(shmId >= 0){
        dadosServidor = shmat(shmId, 0, 0);
        if(dadosServidor == NULL ){
            error("S1", "Erro ao abrir a shared memory");
            exit(-1);
        }
        else{
            success("S1", "Abri Shared Memory já existente com ID %d", shmId);
        }
    }
    else{
       //Passa para o passo S2 pois este criará a memória
    }
    debug("S1 >");
    return ( shmId > 0 ); //Maior ou igual
}

/**
 * S2   Se no ponto S1 a memória partilhada ainda não existia, então realiza as seguintes operações:
 *      S2.1    Cria uma memória partilhada com a KEY IPC_KEY definida em common.h e com o tamanho para conter os Dados do Servidor,
 *              e liga-se a ela. Em caso de erro, dá error S2.1 "<Problema>", e termina o processo Servidor com exit code -1.
 *              Caso contrário, dá success S2.1 "Criei Shared Memory com ID <shmId>";
 *              --> Preenche as variáveis globais shmId e dadosServidor;
 *
 *      S2.2    Inicia a lista de passagens, preenchendo em todos os elementos o campo tipo_passagem=-1 (“Limpa” a lista de passagens).
 *              Em caso de qualquer erro, dá error S2.2 "<Problema>", e termina o processo Servidor com exit code -1.
 *              Caso contrário, dá success S2.2 "Iniciei Shared Memory Passagens";
 *
 *      S2.3    (realizado na função loadStats()): Tem 3 contadores, para as passagens Normal, Via Verde e passagens com anomalia.
 *              Se o ficheiro FILE_STATS existir na diretoria local, abre-o e lê os seus dados (em formato binário, ver formato em S6.2)
 *              e carrega o valor nos contadores. Se houver erro na leitura do ficheiro, dá error S2.3 "<Problema>", 
 *              e termina o Servidor com exit code -1. Caso contrário, dá success S2.3 "Estatísticas Carregadas".
 *              Se o ficheiro não existir, inicia os três contadores com o valor 0 e dá success S2.3 "Estatísticas Iniciadas";
 *
 * @return int shmId
 */
int shmCreateAndInit() {
    debug("S2 <");
    shmId = shmget(IPC_KEY,sizeof(*dadosServidor), IPC_CREAT | 0666);
    if(shmId >= 0){
        dadosServidor = shmat(shmId, 0, 0);
        if(dadosServidor == NULL ){
            error("S2.1", "Erro ao abrir a shared memory");
            exit(-1);
        }
        else{
            success("S2.1", "Criei Shared Memory com ID %d", shmId);
        }
    }
    //S2.2
    for(int i = 0; i < NUM_PASSAGENS; i++){
        dadosServidor->lista_passagens[i].tipo_passagem = TIPO_PASSAGEM_INVALIDO;
        if(dadosServidor->lista_passagens[i].tipo_passagem != TIPO_PASSAGEM_INVALIDO){
            error("S2.2", "Erro na inicialização da lista de passagens");
            exit(-1);
        }
    }
    success("S2.2","Iniciei Shared Memory Passagens");

    loadStats( &dadosServidor->contadores );
    debug("S2 >");
    return shmId;
}

/**
 *      S2.3    Tem 3 contadores, para as passagens Normal, Via Verde e passagens com anomalia.
 *              Se o ficheiro FILE_STATS existir na diretoria local, abre-o e lê os seus dados (em formato binário, ver formato em S6.2)
 *              e carrega o valor nos contadores. Se houver erro na leitura do ficheiro, dá error S2.3 "<Problema>", 
 *              e termina o Servidor com exit code -1. Caso contrário, dá success S2.3 "Estatísticas Carregadas".
 *              Se o ficheiro não existir, inicia os três contadores com o valor 0 e dá success S2.3 "Estatísticas Iniciadas";
 *
 * @return int Sucesso
 */
int loadStats( Contadores* pStats ) {
    debug("S2.3 <");
    int pidServer = getpid();
    FILE *stats = fopen(FILE_STATS, "rb");
    if (stats == NULL) {
            pStats->contadorAnomalias = 0;
            pStats->contadorNormal = 0;
            pStats->contadorViaVerde = 0;
            success("S2.3", "Estatísticas Iniciadas");
        }
    else{ 
           if(fread(pStats, sizeof(*pStats), 1, stats) < 1){
                error("S2","Problema ao carregar as estatisticas");
                exit(-1);
           }
            else{
            fclose(stats);
            success("S2.3", "Estatísticas Carregadas");
            }
        
    }
    debug("S2.3 >");
    return 0;
}

/**
 * S3   Cria uma message queue com a KEY IPC_KEY definida em common.h,
 *      Se a message queue já existir, apaga-a e cria de novo, 
 *      --> Preenche a variável global msgId.
 *
 *      Arma os sinais SIGINT (ver S6) e SIGCHLD (programa para ignorar este sinal).
 *      Se houver erros, dá error S3 "<Problema>" e termina o Servidor com exit code -1.
 *      Caso contrário, dá success S3 "Criei mecanismos IPC";
 *
 * @return int msgId
 */
int createIPC() {
    debug("S3 <");
    msgId = msgget(IPC_KEY, IPC_CREAT | 0666);
    signal(SIGINT,trataSinalSIGINT);
    signal(SIGCHLD,SIG_IGN);
    if(msgId < 0){
        error("S3","Erro ao criar a message queue");
    }
    success("S3","Criei mecanismos IPC");
    semId = semCreate(2);
    if(semId < 0){
        error("SD14","Semáforos não criados");
        exit(-1);
    }
    semNrSetValue(SEM_ESTATISTICAS,1);
    semNrSetValue(SEM_LISTAPASSAGENS,1);
    debug("S3 >");
    return 0;
}

/**
 * S4   Lê a informação da message queue numa mensagem com o tipo de mensagem 1.
 *      Essa mensagem deverá ter a action 1 – Pedido e deverá conter um elemento do tipo Passagem.
 *      Se houver erro na operação, dá error S4 "<Problema>", e termina o processo Servidor com exit code -1.
 *      Caso contrário, dá success S4 "Li Pedido do Cliente";
 *
 * @return Mensagem Elemento com os dados preenchidos.
 */
Mensagem recebePedido() {
    debug("S4 <");
    Mensagem mensagem;
    if(msgrcv(msgId,&mensagem, sizeof(mensagem.conteudo),1,0 >= 0)){
        if(mensagem.conteudo.action != 1){
            error("S4","Action incorreto");
            exit(-1);
        } 
        else{
            success("S4","Li Pedido do Cliente");
        }
    }
    else{
        error("S4","Erro na leitura do pedido do Cliente");
        exit(-1);
    }
    
    debug("S4 >");
    return mensagem;
}

/**
 * S5   Cria um processo filho (fork) Servidor Dedicado. Se houver erro, dá error S5 "Fork".
 *      Senão, o processo Servidor Dedicado (filho) continua no passo SD7, 
 *      e o processo Servidor (pai) dá success S5 "Criado Servidor Dedicado com PID <pid Filho>".
 *      Em qualquer dos casos, recomeça o processo no passo S4;
 *
 * @return int PID do processo filho, se for o processo Servidor (pai), 0 se for o processo Servidor Dedicado (filho), ou -1 em caso de erro.
 */
int criaServidorDedicado() {
    debug("S5 <");
    int pidFilho = -1;
    pidFilho = fork();
        if(pidFilho == -1){
            error("S5","Fork");
            return -1;
        }
        if(pidFilho !=0){
            success("S5","Criado Servidor Dedicado com PID %d", pidFilho);
        }
    debug("S5 >");
    return pidFilho;
}

/**
 * S6  O sinal armado SIGINT serve para o Diretor da Portagem encerrar o Servidor, usando o atalho <CTRL+C>. 
 *      Se receber esse sinal (do utilizador via Shell), o Servidor dá success S6 "Shutdown Servidor", e depois:
 *      S6.1    Envia o sinal SIGHUP a todos os Servidores Dedicados da Lista de Passagens, 
 *              para que concluam o seu processamento imediatamente. Depois, dá success S6.1 "Shutdown Servidores Dedicados";
 *      S6.2    Cria o ficheiro FILE_STATS, escrevendo nele o valor de 3 inteiros (em formato binário), correspondentes a
 *              <contador de passagens Normal>  <contador de passagens Via Verde>  <contador Passagens com Anomalia>
 *              Em caso de erro, dá error S6.2, caso contrário, dá success S6.2 "Estatísticas Guardadas";
 *      S6.3    Dá success S6.3 "Shutdown Servidor completo", apaga a Message Queue e o grupo de Semáforos 
*               criados (mas não apaga a Shared Memory), e termina o processo Servidor com exit code 0
 */
void trataSinalSIGINT( int sinalRecebido ) {
    debug("S6 <");
    success("S6", "Shutdown Servidor");
        //S6.1
        semNrDown(SEM_ESTATISTICAS);
        for(int i = 0; i < NUM_PASSAGENS; i++){
            if (dadosServidor->lista_passagens[i].tipo_passagem > 0){
                kill(dadosServidor->lista_passagens[i].pid_servidor_dedicado, SIGHUP);
            }
        }
        semNrUp(SEM_ESTATISTICAS);
        success("S6.1", "Shutdown Servidores Dedicados");
        //S6.2
        FILE* st = fopen(FILE_STATS,"wb");
        if(st == NULL){
            error("S6.2","Erro ao abrir o ficheiro %s", FILE_STATS);
        }
        else{
            semNrDown(SEM_ESTATISTICAS);
            if(fwrite(&dadosServidor->contadores, sizeof(dadosServidor->contadores), 1, st) < 1){
                error("S6.2", "Erro a carregar estatísticas");
            }   
            else{
                semNrUp(SEM_ESTATISTICAS);
                success("S6.2", "Estatísticas Guardadas");
                fclose(st);
            }
        }
        //S6.3
        if(semRemove() < 0)
            error("S6.3","Erro ao remover semáforos");
        success("S6.3","Shutdown Servidor completo");
        if(msgctl( msgId, IPC_RMID, NULL) < 0 ){
            error("S6.3","Erro ao eliminar a fila de mensagens");
            exit(-1);
        }
        else{
            exit(0);
        }
        debug("S6 >");
}

/**
 * SD7  O novo processo Servidor Dedicado (filho) arma os sinais SIGHUP (ver SD13) e SIGINT (programa para ignorar este sinal).
 *      Depois de armar os sinais, dá success SD7 "Servidor Dedicado Armei sinais";
 *
 * @return int Sucesso
 */
int sd_armaSinais() {
    debug("SD7 <");
    signal(SIGHUP,sd_trataSinalSIGHUP);
    signal(SIGINT, SIG_IGN);
    success("SD7","Servidor Dedicado Armei sinais");
    debug("SD7 >");
    return 0;
}

/**
 * SD8  O Servidor Dedicado deve validar se o pedido que “herdou” do Servidor está corretamente formatado.
 *      Esse pedido inclui uma estrutura Passagem cuja formatação correta tem de validar se:
 *      •   O Tipo de passagem é válido (1 para pedido Normal, ou 2 para Via Verde);
 *      •   A Matrícula e o Lanço não são strings vazias (não é necessário fazer mais validações sobre o seu conteúdo);
 *      •   O pid_cliente é um valor > 0.
 *      Em caso de erro na formatação:
 *      •   Dá error SD8 "<Problema>", e incrementa o contador de anomalias;
 *      •   Se pid_cliente é um valor > 0, envia uma mensagem com action 4 – Pedido Cancelado, para a Message Queue com tipo de mensagem igual ao pid_cliente.
 *          Se o envio tiver erro, dá error SD8 "<Problema>";
 *      •   Ignora o pedido, e termina o processo Servidor Dedicado com exit code -1.
 *      Caso contrário, se não houver erro na formatação, 
 *      dá success SD8 "Chegou novo pedido de passagem do tipo <Normal | Via Verde> solicitado pela viatura com matrícula <matricula> para o Lanço <lanco> e com PID <pid_cliente>";
 *
 * @return int Sucesso
 */
void sendMessageifError( Mensagem pedido){
        if(pedido.conteudo.dados.pedido_cliente.pid_cliente > 0){
            pedido.conteudo.action = ACTION_PEDIDO_CANCELADO;
            pedido.tipoMensagem = pedido.conteudo.dados.pedido_cliente.pid_cliente;
            if(msgsnd(msgId,&pedido,sizeof(pedido.conteudo),0) < 0){
                error("SD8","Erro ao enviar a mensagem");
                exit(-1);
              }
            else{
                exit(-1);
                //O cliente recebe a mensagem e informará o utilizador
            }
        }
        exit(-1);
    }

int sd_validaPedido( Mensagem pedido ) {
    debug("SD8 <");
    //char tipoNomePassagem[20];
    if (pedido.conteudo.dados.pedido_cliente.tipo_passagem < TIPO_PASSAGEM_NORMAL || pedido.conteudo.dados.pedido_cliente.tipo_passagem > TIPO_PASSAGEM_VIAVERDE){
        error("SD8", "Tipo de passagem inválida");
        semNrDown(SEM_ESTATISTICAS);
        dadosServidor->contadores.contadorAnomalias++;
        semNrUp(SEM_ESTATISTICAS);
        sendMessageifError(pedido);
        exit(-1);
    }
    else if (pedido.conteudo.dados.pedido_cliente.matricula == NULL || strcmp(pedido.conteudo.dados.pedido_cliente.matricula,"") == 0 ){ 
            error("SD8", "Matrícula inválida");
            semNrDown(SEM_ESTATISTICAS);
            dadosServidor->contadores.contadorAnomalias++;
            semNrUp(SEM_ESTATISTICAS);
            sendMessageifError(pedido);
            exit(-1);
        }
    else if (pedido.conteudo.dados.pedido_cliente.lanco == NULL || strcmp(pedido.conteudo.dados.pedido_cliente.lanco,"") == 0 ){
            error("SD8", "Lanço inválida");
            semNrDown(SEM_ESTATISTICAS);
            dadosServidor->contadores.contadorAnomalias++;
            semNrUp(SEM_ESTATISTICAS);
            sendMessageifError(pedido);
            exit(-1);
        }
    else if(pedido.conteudo.dados.pedido_cliente.pid_cliente <= 0){
            error("SD8", "PID Inválido");
            semNrDown(SEM_ESTATISTICAS);
            dadosServidor->contadores.contadorAnomalias++;
            semNrUp(SEM_ESTATISTICAS);
            sendMessageifError(pedido);
            exit(-1);
        }
    else{
        if(pedido.conteudo.dados.pedido_cliente.tipo_passagem == TIPO_PASSAGEM_NORMAL){
            //char tipoNomePassagem[20] = "Normal";
            success("SD8", "Chegou novo pedido de passagem do tipo Normal solicitado pela viatura com matrícula %s para o Lanço %s e com PID %d", pedido.conteudo.dados.pedido_cliente.matricula, pedido.conteudo.dados.pedido_cliente.lanco, pedido.conteudo.dados.pedido_cliente.pid_cliente);
        }
        else if(pedido.conteudo.dados.pedido_cliente.tipo_passagem == TIPO_PASSAGEM_VIAVERDE){
            //char tipoNomePassagem[20] = "Via Verde";
            success("SD8", "Chegou novo pedido de passagem do tipo Via Verde solicitado pela viatura com matrícula %s para o Lanço %s e com PID %d", pedido.conteudo.dados.pedido_cliente.matricula, pedido.conteudo.dados.pedido_cliente.lanco, pedido.conteudo.dados.pedido_cliente.pid_cliente);
        }
    }
    debug("SD8 >");
    return 0;
}

/**
 * SD9  Verifica se existe disponibilidade na Lista de Passagens. Se todas as entradas da Lista de Passagens estiverem ocupadas,
 *      dá error SD9 "Lista de Passagens cheia", incrementa o contador de passagens com anomalia, 
 *      manda uma mensagem com action 4 – Pedido Cancelado, para a Message Queue com tipo de mensagem igual ao pid_cliente,
 *      ignora o pedido, e termina o processo Servidor Dedicado com exit code -1.
 *      Caso contrário, preenche uma entrada da lista com os dados deste pedido, incrementa o contador de passagens do tipo de passagem correspondente
 *      e dá success SD9 "Entrada <índice lista> preenchida";
 *
 * @return int Em caso de sucesso, retorna o índice da lista preenchido. Caso contrário retorna -1
 */
int sd_reservaEntradaBD( DadosServidor* dadosServidor, Mensagem pedido ) {
    debug("SD9 <");
    int indiceLista = -1;
    semNrDown(SEM_LISTAPASSAGENS);
        for(int i = 0; i < NUM_PASSAGENS; i++){
            if(dadosServidor->lista_passagens[i].tipo_passagem == TIPO_PASSAGEM_INVALIDO){ 
                indiceLista = i;
                semNrDown(SEM_ESTATISTICAS);
                dadosServidor->lista_passagens[i] = pedido.conteudo.dados.pedido_cliente;
                semNrUp(SEM_ESTATISTICAS);
                if(pedido.conteudo.dados.pedido_cliente.tipo_passagem == TIPO_PASSAGEM_NORMAL){
                    semNrDown(SEM_ESTATISTICAS);
                    dadosServidor->contadores.contadorNormal++;
                    semNrUp(SEM_ESTATISTICAS);
                }else if(pedido.conteudo.dados.pedido_cliente.tipo_passagem == TIPO_PASSAGEM_VIAVERDE){
                    semNrDown(SEM_ESTATISTICAS);
                    dadosServidor->contadores.contadorViaVerde++;
                    semNrUp(SEM_ESTATISTICAS);
                    }
                pedido.conteudo.dados.pedido_cliente.pid_servidor_dedicado = getpid();
                semNrUp(SEM_LISTAPASSAGENS);
                success("SD9","Entrada %d preenchida",indiceLista);
                return indiceLista;
            }
        }
    semNrUp(SEM_LISTAPASSAGENS);
    error("SD9","Lista de Passagens cheia");
    semNrDown(SEM_ESTATISTICAS);
    dadosServidor->contadores.contadorAnomalias++;
    semNrUp(SEM_ESTATISTICAS);
    kill(pedido.conteudo.dados.pedido_cliente.pid_cliente,SIGHUP);
    pedido.conteudo.action = ACTION_PEDIDO_CANCELADO;
    pedido.tipoMensagem = pedido.conteudo.dados.pedido_cliente.pid_cliente;
        if(msgsnd(msgId,&pedido,sizeof(pedido.conteudo),0) < 0){
                error("SD9","Erro ao enviar a mensagem");
                exit(-1);
        } else{
                  //O cliente recebe a mensagem e informará o utilizador
                  exit(-1);
            }
    debug("SD9 >");
    return indiceLista;
}

/**
 * "Apaga" uma entrada da Lista de Passagens, colocando tipo_passagem = -1
 *
 * @return int Sucesso
 */
int apagaEntradaBD( DadosServidor* dadosServidor, int indice_lista ) {
    debug("<");
    semNrDown(SEM_LISTAPASSAGENS);
    dadosServidor->lista_passagens[indice_lista].tipo_passagem = TIPO_PASSAGEM_INVALIDO;
    semNrUp(SEM_LISTAPASSAGENS);
    debug(">");
    return 0;
}


/**
 * SD10 O Servidor Dedicado envia uma mensagem com action 2 – Pedido ACK, para a Message Queue com tipo de mensagem igual ao pid_cliente,
 *      indicando o início do processamento da passagem, e dá success SD10 "Início Passagem <PID Cliente> <PID Servidor Dedicado>";
 *
 * @return int Sucesso
 */
int sd_iniciaProcessamento( Mensagem pedido ) {
    debug("SD10 <");
    pedido.conteudo.action = ACTION_PEDIDO_ACK;
    pedido.tipoMensagem = pedido.conteudo.dados.pedido_cliente.pid_cliente;
    pedido.conteudo.dados.pedido_cliente.pid_servidor_dedicado = getpid();
    if(msgsnd(msgId,&pedido,sizeof(pedido.conteudo),0) < 0){
        error("SD10","Erro ao enviar a mensagem");
        exit(-1);
    } else{
        //O cliente recebe a mensagem e informará o utilizador
        }
    success("SD10","Início Passagem %d %d",pedido.conteudo.dados.pedido_cliente.pid_cliente,getpid());
    debug("SD10 >");
    return 0;
}

/**
 * SD11 O Servidor Dedicado calcula um valor aleatório (usando my_rand()) entre os valores MIN_PROCESSAMENTO e MAX_PROCESSAMENTO,
 *      dá success SD11 "<Tempo>", e aguarda esse valor em segundos (sleep);
 *
 * @return int Sucesso
 */
int sd_sleepRandomTime() {
    debug("SD11 <");
    int aleatorio = (my_rand() % (MAX_PROCESSAMENTO - MIN_PROCESSAMENTO + 1)) + MIN_PROCESSAMENTO;
    success("SD11", "%d", aleatorio);
    sleep(aleatorio);
    debug("SD11 >");
    return 0;
}

/**
 * SD12 O Servidor Dedicado envia uma mensagem com action 3 – Pedido Concluído, para a Message Queue com tipo de mensagem igual ao pid_cliente,
 *      onde também deverá incluir os valores atuais das estatísticas na estrutura contadores_servidor,
 *      indicando o fim do processamento da passagem ao processo <pid_cliente>,
 *      apaga a entrada do Cliente na lista de passagens,
 *      dá success SD12 "Fim Passagem <PID Cliente> <PID Servidor Dedicado>",
 *      e termina o processo Servidor Dedicado;
 *
 * @return int Sucesso
 */
int sd_terminaProcessamento( Mensagem pedido ) {
    debug("SD12 <");
    pedido.conteudo.action = ACTION_PEDIDO_CONCLUIDO;
    pedido.tipoMensagem = pedido.conteudo.dados.pedido_cliente.pid_cliente;
    semNrDown(SEM_ESTATISTICAS);
    pedido.conteudo.dados.contadores_servidor = dadosServidor->contadores;
    semNrUp(SEM_ESTATISTICAS);
    apagaEntradaBD(dadosServidor, indice_lista);
    if(msgsnd(msgId,&pedido,sizeof(pedido.conteudo),0) < 0){
        error("SD12","Erro ao enviar a mensagem");
        exit(-1);
    } else{
        //O cliente recebe a mensagem e informará o utilizador
        }
    success("SD12","Fim Passagem %d %d",pedido.conteudo.dados.pedido_cliente.pid_cliente,getpid());
    exit(0);
    debug("SD12 >");
    return 0;
}

/**
 * SD13 O sinal armado SIGHUP serve para o Servidor indicar que deseja terminar imediatamente o pedido de processamento da passagem.
 *      Se o Servidor Dedicado receber esse sinal, não incrementa o contador de passagens com anomalia, mas manda uma mensagem com action 4 – Pedido Cancelado,
 *      para a Message Queue com tipo de mensagem igual ao pid_cliente, dá success SD13 "Processamento Cancelado", e termina o Servidor Dedicado
 */
void sd_trataSinalSIGHUP(int sinalRecebido) {
    debug("SD13 <");
    mensagem.conteudo.action = ACTION_PEDIDO_CANCELADO;
    mensagem.tipoMensagem = mensagem.conteudo.dados.pedido_cliente.pid_cliente;
    apagaEntradaBD(dadosServidor,indice_lista);
    if(msgsnd(msgId,&mensagem,sizeof(mensagem.conteudo),0) < 0){
        error("SD13","Erro ao enviar a mensagem");
        exit(-1);
    } else{
        //O cliente recebe a mensagem e informará o utilizador
        }
    success("SD13","Processamento Cancelado");
    exit(0);
    debug("SD13 >");
}

/**
 * SD14 Repare que os vários Servidores Dedicados têm todos acesso concorrente à Memória Partilhada.
 *      Acrescente em S3 a criação de um grupo com dois semáforos do tipo MUTEX,
 *      um dedicado à lista de passagens e outro dedicado às estatísticas.
 *      Altere o código do Servidor e do Servidor Dedicado por forma a garantir a exclusão mútua no acesso a cada um
 *      destes dois elementos dos Dados do Servidor, garantindo que o tempo passado em exclusão é sempre o menor possível.
 */
 // Este tópico não tem uma função associada, porque terá de ser implementada no resto do código.