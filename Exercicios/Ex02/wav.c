#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#define TAMANHO_NOME_ARQUIVO 50

typedef struct
{
	char ChunkID[4];
	char Format[4];
	char Subchunk1ID[4];
	char Subchunk2ID[4];
	int ChunkSize;
	int Subchunk1Size;
	int SampleRate;
	int ByteRate;
	int Subchunk2Size;
	short AudioFormat;
	short NumChannels;
	short BlockAlign;
	short BitsPerSample;
	short *data;
}estrutura_wav;

void imprimir_identificacao()
{
	printf("Universidade de Brasilia\n");
	printf("Disciplina: Sistemas Embarcados\n");
	printf("Aluno: Matheus Herlan dos Santos Ferraz - Matricula: 12/0018926\n\n");
}

short *alocar_memoria_buffer(FILE *arquivo_entrada, estrutura_wav estrutura)
{
	short *buffer = (short*)malloc(sizeof(short)*(estrutura.Subchunk2Size)*(estrutura.BitsPerSample/8));
	return buffer;
}

estrutura_wav ler_arquivo_wav(FILE *arquivo_entrada, estrutura_wav estrutura)
{
	fread(estrutura.ChunkID, 1, 4, arquivo_entrada);
	fread(&estrutura.ChunkSize, 4, 1, arquivo_entrada);
	fread(estrutura.Format, 1, 4, arquivo_entrada);
	fread(estrutura.Subchunk1ID, 1, 4, arquivo_entrada);
	fread(&estrutura.Subchunk1Size, 4, 1, arquivo_entrada);
	fread(&estrutura.AudioFormat, 2, 1, arquivo_entrada);
	fread(&estrutura.NumChannels, 2, 1, arquivo_entrada);
	fread(&estrutura.SampleRate, 4, 1, arquivo_entrada);
	fread(&estrutura.ByteRate, 4, 1, arquivo_entrada);
	fread(&estrutura.BlockAlign, 2, 1, arquivo_entrada);
	fread(&estrutura.BitsPerSample, 2, 1, arquivo_entrada);
	fread(estrutura.Subchunk2ID, 1, 4, arquivo_entrada);
	fread(&estrutura.Subchunk2Size, 4, 1, arquivo_entrada);

	short *buffer = alocar_memoria_buffer(arquivo_entrada, estrutura);
	estrutura.data = buffer;

	fread(estrutura.data, estrutura.BitsPerSample/8, estrutura.Subchunk2Size/(estrutura.BitsPerSample/8), arquivo_entrada);
	fclose(arquivo_entrada);

	return estrutura;
}

estrutura_wav escrever_arquivo_wav(FILE *arquivo_saida, estrutura_wav estrutura)
{
	fwrite(estrutura.ChunkID, 1, 4, arquivo_saida);
	fwrite(&estrutura.ChunkSize, 4, 1, arquivo_saida);
	fwrite(estrutura.Format, 1, 4, arquivo_saida);
	fwrite(estrutura.Subchunk1ID, 1, 4, arquivo_saida);
	fwrite(&estrutura.Subchunk1Size, 4, 1, arquivo_saida);
	fwrite(&estrutura.AudioFormat, 2, 1, arquivo_saida);
	fwrite(&estrutura.NumChannels, 2, 1, arquivo_saida);
	fwrite(&estrutura.SampleRate, 4, 1, arquivo_saida);
	fwrite(&estrutura.ByteRate, 4, 1, arquivo_saida);
	fwrite(&estrutura.BlockAlign, 2, 1, arquivo_saida);
	fwrite(&estrutura.BitsPerSample, 2, 1, arquivo_saida);
	fwrite(estrutura.Subchunk2ID, 1, 4, arquivo_saida);
	fwrite(&estrutura.Subchunk2Size, 4, 1, arquivo_saida);

	fwrite(estrutura.data, estrutura.BitsPerSample/8, estrutura.Subchunk2Size/(estrutura.BitsPerSample/8), arquivo_saida);
	fclose(arquivo_saida);

	return estrutura;
}

int main()
{
	char nome_arquivo_entrada[TAMANHO_NOME_ARQUIVO], nome_arquivo_saida[TAMANHO_NOME_ARQUIVO];
	FILE *wav_entrada, *wav_saida;
	estrutura_wav estrutura_entrada, estrutura_saida;
	pid_t main_pid, pid_fork;
	int descritores[2];
	int estado_processo_filho;

	main_pid = getpid();

	printf("PID deste processo (Processo Pai): %d.\n\n",main_pid);

	imprimir_identificacao();

	printf("Informe o nome do arquivo wav de entrada (Incluindo extensão .wav): \n");
	scanf("%40[^\n]",nome_arquivo_entrada);

	printf("Informe o nome do arquivo wav cópia que o processo filho ira ler (Incluindo a extensão .wav): \n");
	scanf(" %40[^\n]",nome_arquivo_saida);

	wav_entrada = fopen(nome_arquivo_entrada, "rb");

	if(wav_entrada == NULL)
	{
		printf("Processo Pai -> Abertura do arquivo wav mal sucedida.\n");
		exit(1);
	}

	estrutura_entrada = ler_arquivo_wav(wav_entrada, estrutura_entrada);

	wav_saida = fopen(nome_arquivo_saida, "wb");

	if(wav_saida == NULL)
	{
		printf("Processo Pai -> Abertura do arquivo wav de copia mal sucedida.\n");
		exit(1);
	}
	
	estrutura_saida = escrever_arquivo_wav(wav_saida, estrutura_entrada);

	printf("Processo Pai -> Arquivo wav de copia gerado com sucesso.\n");

	if(pipe(descritores) < 0)
	{
		printf("Processo Pai -> Erro ao criar o pipe.\n");
		exit(1);
	}

	if((pid_fork = fork()) < 0)
	{
		printf("Processo Pai -> Erro na criação do processo filho.\n");
		exit(1);
	}

	if(pid_fork == 0)
	{
		if(read(descritores[0], nome_arquivo_saida, TAMANHO_NOME_ARQUIVO) < 0)
		{
			printf("Processo Filho -> Erro na leitura da mensagem do Processo Pai.\n");
		}
		else
		{
			printf("Processo Filho -> Mensagem lida: %s (Nome do arquivo wav de cópia).\n", nome_arquivo_saida);
			char *lista_argumentos[] = {"aplay", nome_arquivo_saida, NULL};
			printf("Processo Filho -> Irei executar o aplay.\n");
			execvp("aplay", lista_argumentos);
			fprintf(stderr, "Um erro ocorreu na primitiva execvp.\n");
			exit(1);
		}
	}

	if(write(descritores[1], nome_arquivo_saida, TAMANHO_NOME_ARQUIVO) < 0)
	{
		printf("Processo Pai -> Ocorreu um erro na escrita da mensagem para o Processo Filho.\n");
		wait(&estado_processo_filho);
	}

	printf("Processo Pai -> Fim do programa.\n\n");

	return 0;
}