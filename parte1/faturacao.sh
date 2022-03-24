#!/bin/bash

###############################################################################
## ISCTE-IUL: Trabalho prático de Sistemas Operativos 2021/2022
##
## Aluno: Nº:104670   Nome:Vasco Mendes Baleia
## Nome do Módulo: faturacao.sh
## Descrição/Explicação do Módulo: Obter as faturas com base no relatorio
## 
##
###############################################################################
data=./relatorio_utilizacao.txt 
if ! [ -f $data ]; then #Verificar se o ficheiro ./relatorio_utilizacao.txt existe
	./error 1 $data
	exit
fi
	fat=./faturas.txt
	if [ -f $fat ]; then #Verificar se o ficheiro faturas.txt existe, se sim removê-lo para correr o script
		rm $fat
		./faturacao.sh	
	else #Validados os argumentos, correr o código
		if [ -s $data ]; then	
			people=./pessoas.txt
			getNumberofDrivers=$(cat $people | wc -l)
			for(( i=1 ; i <= $getNumberofDrivers ; i++ )); do
				getDriverNIF=$(cat $people | head -$i | tail -1 | cut -d ':' -f3) #Como o ID contém o NIF, usamo-lo para pesquisar o condutor
				getDriverName=$(cat $people | head -$i | tail -1 | cut -d ':' -f2) #Pesquisar o nome do condutor pelo ID, de forma a colocá-lo no output
					echo "Cliente:" "$getDriverName" >> $fat 
					echo "$(grep "$getDriverNIF" $data)" >> $fat
					echo "$(grep "$getDriverNIF" $data | cut -d ':' -f5 | awk -F "[:]" '{t = t + $0} END { print "Total: " (t=="" ? "0" : t) " créditos" }')" >> $fat
					echo "" >> $fat
			done
		./success 5 $fat
		fi	
	fi
