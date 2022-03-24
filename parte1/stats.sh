#!/bin/bash

###############################################################################
## ISCTE-IUL: Trabalho prático de Sistemas Operativos 2021/2022
##
## Aluno: Nº:104670       Nome: Vasco Mendes Baleia
## Nome do Módulo: stats.sh
## Descrição/Explicação do Módulo: Obter estatísticas do sistema
##
##
###############################################################################
tmp=$(mktemp $TMPDIR/SCUT-IUL-P1-STATS-XXXXXXXX) #Usar o comando para ficheiros temporários do Linux
if [[ $1 =~ [Ll]istar ]]; then #Aceitar o argumento "listar", seja com minúscula ou maiúscula
	if [ $# -ne 1 ]; then #Validar se o argumento "listar" está sozinho
		./error 2
		exit
	else #Listar as autoestradas em sistema
		por=portagens.txt
		(cat $por | cut -d ':' -f3 | sort -V | uniq) > $tmp
		./success 6 $tmp 
	exit
	fi
elif [[ $1 =~ [Rr]egistos ]]; then #Aceitar o argumento "registos", seja com minúscula ou maiúscula
	if [ $# -ne 2 ]; then #Validar se o argumento "registos" tem o argumento extra necessário à sua execução
		./error 2
		exit
	elif [ $2 -le 0 ]; then #Validar o formato do segundo argumento, ou seja, se é um valor inteiro positivo
		./error 3 $2
		exit
	else #Obter as autoestradas que tenham $2 ou mais registos
		usageData=./relatorio_utilizacao.txt
		getNumberOfUniqLancos=$(cat $usageData | cut -d ':' -f2 | sort | uniq | wc -l)
		for(( i=1; i <= $getNumberOfUniqLancos; i++)); do
			getLanco=$(cat $usageData | cut -d ':' -f2 | sort | uniq | head -$i | tail -1) 	
			getNumberofPassagesPerLanco=$(cat $usageData | grep -w "$getLanco" | wc -l)
				if ! [ $getNumberofPassagesPerLanco -lt $2 ]; then				
					echo "$(cat $usageData | cut -d ':' -f2 | sort | uniq | head -$i | tail -1)" >> $tmp
				fi		
		done
		./success 6 $tmp
		exit
	fi
elif [[ $1 =~ [Cc]ondutores ]]; then #Listar os condutores que já passaram nas portagens
		usageData=./relatorio_utilizacao.txt
		drivers=./condutores.txt
		getNumberOfPassedDrivers=$(cat $usageData | cut -d ':' -f3 | sort | uniq | wc -l) #Fez-se a filtragem por NIF de facilitar a pesquisa com grep
			for((i = 1; i <= $getNumberOfPassedDrivers; i++)); do
				#getDriverID=$(cat $drivers | awk -F "[-;]" '{print $2}' | sort | uniq | head -$i | tail -1) 
				getDriverID=$(cat $usageData | cut -d ':' -f3 | sort | uniq | head -$i | tail -1) 	
					echo "$(cat $drivers | grep -w "$getDriverID" | awk -F "[-;]" '{print $2}')" >> $tmp
			done
				./success 6 "$tmp"
		exit
else
	if [ $# -eq 0 ]; then #Dar erro se o script for executado sem qualquer argumento
		./error 2
		exit
	else #Dar erro em outros casos, ou seja, se forem dados argumentos não previstos
	./error 3 $1 
	exit
	fi
fi
