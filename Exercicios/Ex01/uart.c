#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>

void imprimir_identificacao()
{
	printf("Universidade de Brasilia - UnB\n");
	printf("Faculdade UnB Gama - FGA\n");
	printf("Aluno: Matheus Herlan - Matricula: 120018926\n");
	printf("Disciplina: Sistemas Embarcados.\n\n");
}

void imprimir_opcoes()
{
	printf("*********************************\n");
	printf("**** 1 - Solicitar Inteiro ******\n");
	printf("**** 2 - Solicitar Real *********\n");
	printf("**** 3 - Solicitar String *******\n");
	printf("**** 4 - Enviar Inteiro *********\n");
	printf("**** 5 - Enviar Real ************\n");
	printf("**** 6 - Enviar String **********\n");
	printf(" Qualquer outro numero para sair \n");
	printf("*********************************\n\n");
}

void imprimir_inteiro(int inteiro)
{
	printf("Inteiro recebido: %d.\n\n",inteiro);
}

void imprimir_real(float real)
{
	printf("Real recebido: %f.\n\n",real);
}

void imprimir_char(char caractere)
{
	printf("Caractere recebido: %x.\n\n",caractere);
}

void fechar_porta(int descritor)
{
	close(descritor);
	printf("Porta fechada.\n");
}

int abrir_porta()
{
	int descritor = open("/dev/ttyAMA0", O_RDWR | O_NOCTTY | O_NDELAY);
	if (descritor == -1)
	{
		printf("Erro ao abrir a porta. Verifique se a porta nao esta ocupada.\n");
		exit(1);
	}
	else {
		configurar_porta(descritor);
		printf("Porta aberta com sucesso.\n\n");
		return descritor;
	}
}

int configurar_porta(int descritor)
{
	struct termios opcoes;
	tcgetattr(descritor, &opcoes);
	opcoes.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
	opcoes.c_iflag = IGNPAR;
	opcoes.c_oflag = 0;
	opcoes.c_lflag = 0;
	tcflush(descritor, TCIFLUSH);
	tcsetattr(descritor, TCSANOW, &opcoes);
}

int transmitir_dados(int descritor, unsigned char *buffer, size_t numero_bytes)
{
	int contador;
	contador = write(descritor, buffer, numero_bytes);
		if (contador < 0)	{
			printf("Erro ao escrever dados na porta.\n");
			exit(1);
		}
		else {
			printf("Dados enviados (numero de bytes): %d.\n", contador);
		}
	return contador;
}

int receber_dados(int descritor, void *buffer, size_t numero_bytes)
{
	int tamanho_buffer;
	int contador = 0;
	char buffer_recepcao[256];

	while(contador < numero_bytes)
	{
		tamanho_buffer = read(descritor, &buffer_recepcao[contador], 1);
		if(tamanho_buffer > 0)
			contador += tamanho_buffer;
	}

	memcpy(buffer, buffer_recepcao, numero_bytes);
	return tamanho_buffer;
}

int main()
{
	imprimir_identificacao();

	//Descritor de arquivo
	int descritor_uart = abrir_porta();
	
	//Identificacao do processo
	pid_t main_pid = getpid();

	//Massas de dados
	unsigned char buffer_transmissao[256];
	unsigned char buffer_recepcao[256];

	//Variaveis para envio
	int inteiro_para_envio;
	float real_para_envio;
	char string_para_envio[] = "Mensagem de Envio - Teste.";

	//Variáveis para recebimento
	int inteiro_recebido;
	float real_recebido;
	char caractere_recebido;

	//Dado fixo - Matricula
	char matricula_aluno[4] = "8926";

	//Variáveis auxiliares
	int escolha_usuario;
	char tamanho_string;

	printf("PID deste processo: %d.\n\n",main_pid);

	do 
	{
		imprimir_opcoes();

		printf("Informe o numero da opcao desejada: \n");
        scanf("%d",&escolha_usuario);

        switch(escolha_usuario)
        {
        	case 1:
        	{
        		buffer_transmissao[0] = 0xA1;
		        memcpy(&buffer_transmissao[1], matricula_aluno, 4);
		        transmitir_dados(descritor_uart, buffer_transmissao, 5);
			    sleep(1);
		        receber_dados(descritor_uart, &inteiro_recebido, 4);
				imprimir_inteiro(inteiro_recebido);
                break;
            }
            case 2:
            {
                buffer_transmissao[0] = 0xA2;
				memcpy(&buffer_transmissao[1], matricula_aluno, 4);
				transmitir_dados(descritor_uart, buffer_transmissao, 5);
				sleep(1);
				receber_dados(descritor_uart, &real_recebido, 4);
				imprimir_real(real_recebido);
                break;
            }
            case 3:
            {
                buffer_transmissao[0] = 0xA3;
				memcpy(&buffer_transmissao[1], matricula_aluno, 4);
				transmitir_dados(descritor_uart, buffer_transmissao, 5);
				sleep(1);
				receber_dados(descritor_uart, &tamanho_string, 1);
				receber_dados(descritor_uart, buffer_recepcao, tamanho_string);
				printf("String recebida: %s.\n\n", buffer_recepcao);
                break;
            }
            case 4:
            {
                printf("Informe o valor inteiro a ser enviado: \n");
                scanf("%d",&inteiro_para_envio);
                buffer_transmissao[0] = 0xB1;
                memcpy(&buffer_transmissao[1], &inteiro_para_envio, 4);
                memcpy(&buffer_transmissao[5], matricula_aluno, 4);
                transmitir_dados(descritor_uart, buffer_transmissao, 9);
                sleep(1);
                receber_dados(descritor_uart, &inteiro_recebido, 4);
                printf("Valor inteiro transmitido.\n");
                imprimir_inteiro(inteiro_recebido);
                break;
            }
            case 5:
            {
            	printf("Informe o valor real a ser enviado: \n");
            	scanf("%f",&real_para_envio);
            	buffer_transmissao[0] = 0xB2;
            	memcpy(&buffer_transmissao[1], &real_para_envio, 4);
            	memcpy(&buffer_transmissao[5], matricula_aluno, 4);
            	transmitir_dados(descritor_uart, buffer_transmissao, 9);
            	sleep(1);
            	receber_dados(descritor_uart, &real_recebido, 4);
            	printf("Valor real transmitido.\n");
            	imprimir_real(real_recebido);
            	break;
            }
            case 6:
            {
            	int bytes_comando = 6;
            	int dimensao_string = strlen(string_para_envio);
            	int total_bytes = dimensao_string + bytes_comando;
            	buffer_transmissao[0] = 0xB3;
            	buffer_transmissao[1] = (char) dimensao_string;
            	memcpy(&buffer_transmissao[2], string_para_envio, dimensao_string);
            	memcpy(&buffer_transmissao[dimensao_string + 2], matricula_aluno, 4);
            	transmitir_dados(descritor_uart, buffer_transmissao, total_bytes);
            	sleep(1);
            	receber_dados(descritor_uart, &caractere_recebido, 1);
            	printf("Valor da string transmitido.\n");
            	imprimir_char(caractere_recebido);
            	break;
            }
            default:
            {         
                printf("A opcao selecionada fara encerramento do programa.\n");
                break;
            }
        }
	}while((escolha_usuario == 1) || (escolha_usuario == 2) || (escolha_usuario == 3) || (escolha_usuario == 4) || (escolha_usuario == 5) || (escolha_usuario == 6));

	printf("Fim do programa.\n");
	
	fechar_porta(descritor_uart);

	return 0;
}