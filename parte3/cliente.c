/******************************************************************************
 ** ISCTE-IUL: Trabalho prático 2 de Sistemas Operativos
 **
 ** Aluno: Nº: 104670      Nome: Vasco Mendes Baleia
 ** Nome do Módulo: cliente.c versão 3
 ** Descrição/Explicação do Módulo: 
 **
 **
 ******************************************************************************/
#include "common.h"
#include "utils.h"

/* Variáveis globais */
Mensagem mensagem;                      // Variável que tem o pedido enviado do Cliente para o Servidor
int msgId;                              // Variável que tem o ID da Message Queue
char passagemIniciada = FALSE;          // Variável que indica que o Servidor já enviou sinal de início da passagem

/* Protótipos de funções */
int getMsg();                           // C1: Função a ser implementada pelos alunos
Passagem getDadosPedidoUtilizador();    // C2: Função a ser implementada pelos alunos
int enviaPedido( Passagem, int );       // C3: Função a ser implementada pelos alunos
Mensagem recebeMensagem();              // C4: Função a ser implementada pelos alunos
void pedidoAck();                       // C5: Função a ser implementada pelos alunos
void pedidoConcluido( Mensagem );       // C6: Função a ser implementada pelos alunos
void pedidoCancelado();                 // C7: Função a ser implementada pelos alunos

/**
 * Main: Processamento do processo Cliente
 * 
 * @return int Exit value
 */
int main() {    // Os alunos em princípio não deverão alterar esta função
    // C1
    msgId = getMsg();
    // C2
    Passagem pedido = getDadosPedidoUtilizador();
    // C3
    enviaPedido( pedido, msgId );
    // Aguarda processamento por parte do Servidor
    while (TRUE) {
        debug("Aguarda processamento por parte do Servidor");
        // C4
        mensagem = recebeMensagem( msgId );
        switch (mensagem.conteudo.action) {
            // C5
            case 2: pedidoAck();
                    break;
            // C6
            case 3: pedidoConcluido( mensagem );
                    break;
            // C7
            case 4: pedidoCancelado();
        }
    }
}

/**
 *  O módulo Cliente é responsável pelo pedido das passagens. Este módulo será utilizado para solicitar a passagem das viaturas pelas portagens.
 *  Após identificação da viatura, é realizado o pedido da respetiva passagem, ficando este módulo a aguardar até que o processamento esteja concluído.
 *  Assim, definem-se as seguintes tarefas a desenvolver:
 */

/**
 * C1   Tenta abrir uma fila de mensagens (message queue) IPC que tem a KEY IPC_KEY definida em common.h
 *      (alterar esta KEY para ter o valor do nº do aluno, como indicado nas aulas).
 *      Deve assumir que a fila de mensagens já foi criada.
 *      Em caso de erro, dá error C1 "<Problema>" e termina o Cliente com exit status -1. Caso contrário dá success C1 "<msg id>";
 *
 * @return int a msgID IPC desta Message Queue
 */
int getMsg() {
    debug("C1 <");
    int msgId = -1;
    msgId = msgget(IPC_KEY,0);
        if(msgId == -1){
            error("C1", "Erro na message queue ou queue inexistente");
            exit(-1);
        }
        else{
        //if(msgId == IPC_KEY){
            success("C1", "%d" , msgId);
        }
        /* else{
            error("C1", "Got unexpected IPC KEY");
            exit(-1);
        }
        */
    debug("C1 >");
    return msgId;
}

/**
 * C2   Pede ao Condutor (utilizador) que preencha os dados referentes à passagem da viatura (Matrícula e Lanço),
 *      criando um elemento do tipo Passagem com essas informações, e preenchendo o valor pid_cliente com o PID do seu próprio processo Cliente.
 *      Em caso de ocorrer qualquer erro, dá error C2 "<Problema>", e termina o processo Cliente;
 *      caso contrário dá success C2 "Passagem do tipo <Normal | Via Verde> solicitado pela viatura com matrícula <matricula> para o Lanço <lanco> e com PID <pid_cliente>";
 *
 * @return Passagem Elemento com os dados preenchidos. Se tipo_passagem = -1, significa que o elemento é inválido
 */
Passagem getDadosPedidoUtilizador() {
    debug("C2 <");
    Passagem p;
    p.tipo_passagem = -1;   // Por omissão, retorna valor inválido
    p.pid_cliente = getpid();
    //char tipoNomePassagem[20];

        printf("Qual o tipo de portagem? \n 1 - Normal \n 2 - Via Verde \n");
        char getTipo[20];
        my_fgets(getTipo,10,stdin);
        p.tipo_passagem = atoi(getTipo);
        
        printf("Insira a matrícula: \n");
        my_fgets(p.matricula,9,stdin);
                
        printf("Insira o lanço: \n");
        my_fgets(p.lanco,50,stdin);

        if(p.tipo_passagem == 1){
               // char tipoNomePassagem[20] = "Normal";
               success("C2", "Passagem do tipo Normal solicitado pela viatura com matrícula %s para o Lanço %s e com PID %d", p.matricula,p.lanco,p.pid_cliente);
            }
        else if(p.tipo_passagem == 2){
                //char tipoNomePassagem[20] = "Via Verde";
                success("C2", "Passagem do tipo Via Verde solicitado pela viatura com matrícula %s para o Lanço %s e com PID %d", p.matricula,p.lanco,p.pid_cliente);  
            }
        else{
                error("C2", "Tipo de passagem inválido");
                p.tipo_passagem = -1;
        }
    debug("C2 >");
    return p;
}

/**
 * C3   Envia as informações do elemento Passagem numa mensagem com o tipo de mensagem 1 e action 1 – Pedido para a message queue.
 *      Em caso de erro na escrita, dá error C3 "<Problema>", e termina o processo Cliente com exit code -1.
 *      Caso contrário, dá success C3 "Enviei mensagem";
 *      --> Preenche a variável global mensagem.
 *
 * @param pedido Passagem a ser enviada
 * @param msgId a msgID IPC desta Message Queue
 * @return int Sucesso
 */
int enviaPedido( Passagem pedido, int msgId ) {
    debug("C3 <");
    mensagem.tipoMensagem = 1;                          //Preenche a variável global mensagem.
    mensagem.conteudo.action = 1;                       //Preenche a variável global mensagem.
    mensagem.conteudo.dados.pedido_cliente = pedido;    //Preenche a variável global mensagem.
    if((msgsnd(msgId, &mensagem, sizeof(mensagem.conteudo), 0)) < 0){
        error("C3", "Erro no envio da mensagem");
        exit(-1);
    }    
    else{
        success("C3","Enviei mensagem");
    }
    debug("C3 >");
    return 0;
}

/**
 * C4   Lê da message queue com msgId uma mensagem cujo tipo de mensagem é igual ao PID deste processo Cliente, 
 *      mensagem essa que poderá vir do Servidor Dedicado. Se houver algum erro dá error C4 "<Problema>"
 *      e termina o Cliente com exit code -1. Caso contrário, dá success C4 "Li mensagem do Servidor".
 *
 * @param msgId a msgID IPC desta Message Queue
 * @return Mensagem a mensagem lida
 */
Mensagem recebeMensagem( int msgId ) {
    debug("C4 <");
    Mensagem mensagem;
    //pause();    // Código temporário para o Cliente não ficar em espera ativa, os alunos deverão remover esta linha quando a leitura à message queue estiver feita.
    if(msgrcv(msgId, &mensagem, sizeof(mensagem),getpid(), 0) < 0){
        error("C4", "Erro ao receber a mensagem");
        exit(-1);
    }
    else{
        success("C4","Li mensagem do servidor");
    }
    debug("C4 >");
    return mensagem;
}

/**
 * C5   Se a mensagem que chegou em C4 veio com action 2 – Pedido ACK,
 *      serve para o Servidor Dedicado indicar que o processamento da passagem foi iniciado.
 *      Se o Cliente receber essa mensagem, dá success C5 "Passagem Iniciada", assinala que o processamento iniciou,
 *      e retorna para aguardar a conclusão do processamento do lado do Servidor Dedicado;
 */
void pedidoAck() {
    debug("C5 <");
    passagemIniciada = TRUE;
    success("C5", "Passagem Iniciada");
    debug("C5 >");
}

/**
 * C6   Se a mensagem que chegou em C4 veio com action 3 – Pedido Concluído,
 *      serve para o Servidor Dedicado indicar que o processamento da passagem foi concluído.
 *      Se o Cliente receber essa mensagem, que inclui os contadores de estatística atualizados,
 *      dá success C6 "Passagem Concluída com estatísticas: <contador normal> <contador via verde> <contador anomalias>", e termina o processo Cliente. 
 *      ATENÇÂO: Deverá previamente validar que anteriormente este Cliente já tinha recebido a mensagem com action 2 – Pedido ACK (ver C5),
 *               indicando que o processamento do lado do Servidor Dedicado teve início,
 *               caso contrário, em vez de sucesso, dá error C6 e termina o processo Cliente com exit code -1;
 *
 * @param mensagem Mensagem recebida do Servidor Dedicado
 */
void pedidoConcluido( Mensagem mensagem ) {
    debug("C6 <");
    if(passagemIniciada != TRUE){
        error("C6", "O cliente ainda não recebeu o Pedido ACK");
        exit(-1);
    }else{
        success("C6", "Passagem Concluída com estatísticas: %d %d %d", mensagem.conteudo.dados.contadores_servidor.contadorNormal, mensagem.conteudo.dados.contadores_servidor.contadorViaVerde, mensagem.conteudo.dados.contadores_servidor.contadorAnomalias);
        exit(0);
    }
    debug("C6 >");
}

/**
 * C7   Se a mensagem que chegou em C4 veio com action 4 – Pedido Cancelado,
 *      serve para o Servidor Dedicado indicar que o processamento a passagem não foi concluído.
 *      Se o Cliente receber esse sinal, dá success C7 "Processo Não Concluído e Incompleto", e termina o processo Cliente om exit code -1.
 */
void pedidoCancelado() {
    debug("C7 <");
    success ("C7", "Processo Não Concluído e Incompleto");
    exit(-1);
    debug("C7 >");
}