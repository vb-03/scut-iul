#!/bin/bash

###############################################################################
## ISCTE-IUL: Trabalho prático de Sistemas Operativos 2021/2022
##
## Aluno: Nº: 104670   Nome: Vasco Mendes Baleia
## Nome do Módulo: menu.sh
## Descrição/Explicação do Módulo: Menu para evocar os diferentes scripts
##
##
###############################################################################
while [ true ]; do
echo ""
echo "1. Listar condutores"
echo "2. Altera taxa de portagem"
echo "3. Stats"
echo "4. Faturação"
echo "0. Sair"
echo -n "Opção: "
read number #Receber a opção do menu principal
	case $number in #Case para o menu principal
	0) exit;;
	1) echo "" 
	./lista_condutores.sh;; 
	2)  echo ""
		echo "Altera taxa de portagem"
		echo -n "Lanço: " 
		read lanco
		echo -n "Auto-estrada: "
		read road
		echo -n "Novo valor taxa: "
		read value
		./altera_taxa_portagem.sh $lanco $road $value
		;;
	3)		echo ""
			echo "Stats"
			echo "1. Nome de todas as autoestradas"
			echo "2. Registos de utilização"
			echo "3. Listagem de condutores"
			echo "0. Voltar"
			echo ""
			echo -n "Opção: "
			read subnumber #Receber a opção do submenu
				case $subnumber in #Case para o submenu
				0) ;;
				1) ./stats.sh listar;;
				2) echo -n "Mínimo de registos: "
				read reg
				./stats.sh registos $reg;;
				3) ./stats.sh condutores;;
				*) echo "Opção Inválida";;
			esac;;
4) ./faturacao.sh;;
*) echo ""
echo "Opção Inválida";;
	
esac
done
