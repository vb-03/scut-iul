#!/bin/bash

###################################################################################
## ISCTE-IUL: Trabalho prático de Sistemas Operativos 2021/2022
##
##		Aluno: Nº:104670       Nome:Vasco Mendes Baleia 
##		Nome do Módulo: altera_taxa_portagem.sh
##		Descrição/Explicação do Módulo: Neste módulo, iremos atualizar o valor do  
##	lanço ou, se o mesmo não existir criá-lo
##
###################################################################################
if [ $# -ne 3 ]; then
	./error 2
elif [ -z "$1" ] || ! [[ $1 =~ ^[a-zA-Z][a-zA-Z]*'-'[a-zA-Z][a-zA-Z]*$ ]]; then #Validar o lanço no formato "Localidade-Localidade"
	./error 3 $1
	exit
elif [ -z "$2" ]; then #Validar se o argumento não é vazio
	./error 3 $2
    exit
elif [ -z "$3" ] || ! [[ $3 =~ ^[1-9]*$ ]]; then #Validar se o argumento é numérico e não vazio
	./error 3 $3
	exit
fi
res=portagens.txt #Definir o ficheiro em variável, de forma a facilitar alterações de código
if [ -f $res ]; then #Se o ficheiro existir
getLanco=$(grep -w "$1" $res | cut -d ':' -f2) #Verificar a existência do lanço
LASTID=0
	if [ ! $getLanco ]; then  #Se o lanço não existir
		LASTID=$(cut -d ':' -f1 $res | sort -n | tail -1) #Saber que ID utilizar ao adicionar um novo
		LASTID=$(($LASTID + 1)) #Iterar o $LASTID 
		echo "$LASTID:$1:$2:$3" >> portagens.txt
		./success 3 $1
 	else #Se o lanço existir
		getID=$(grep -w "$1" $res | cut -d ':' -f1 | sort -n | tail -1) #Obter o ID do lanço
		useCurrentAR=$(grep -w "$1" $res | cut -d ':' -f3) #Se o lanço existir, ignorar a autoestrada dada em $2 
		getLine=$(grep -w "$1" $res) #Definir a linha a alterar
		sed -i "s/"$getLine"/$getID:$1:$useCurrentAR:$3/" $res #Efetuar a substituição
		./success 3 $1
	fi 
else #Se o ficheiro não existir
	echo ""1":$1:$2:$3" > portagens.txt
	./success 3 $1
fi
cat $res | sort -t ':' -k2,2 | sort -t ':' -k3,3 -V -s >> tmp.txt #Devido à natureza do >>, temos que alojar 
																  #os dados num ficheiro temporário de forma a não os apagar
mv -f tmp.txt $res
./success 4 $res 
