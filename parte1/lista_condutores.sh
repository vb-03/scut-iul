#!/bin/bash

###############################################################################
## ISCTE-IUL: Trabalho prático de Sistemas Operativos 2021/2022
##
## Aluno: Nº:104670     Nome:Vasco Mendes Baleia 
## Nome do Módulo: lista_condutores.sh
## Descrição/Explicação do Módulo: Transformar o conteúdo do ficheiro
##  pessoas.txt conforme indicado no enunciado e dar output para o ficheiro
##  condutores.txt
##
##
###############################################################################
if [ ! -f pessoas.txt ]; then #Verificar se o ficheiro pessoas existe, caso não exista emitir o respetivo erro
./error 1 pessoas.txt
else #Escrever no ficheiro "condutores.txt" no formato pedido
cat pessoas.txt | awk -F "[:]" '{print $3, $2, $1, $4, $3, 150}' | sed "s/^/ID/" | sed "s/ /-/" | sed "s/ /;/g" | sed "s/;/ /" > condutores.txt
./success 2 condutores.txt
fi
