#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFER 64

/* Estrutura da lista declarada para armazenar nossos dados. */
typedef struct lista {
        char *nome;
        int idade;
        struct lista *proximo;
} Dados;

/* Prototipo das funcoes de manuseio dos dados. */
Dados *inicia_dados(char *nome, int idade);
Dados *insere_dados(Dados *dados, char *nome, int idade);
void busca_dados(Dados *dados, char *chave);
Dados *deleta_dados(Dados *dados);
int checa_vazio(Dados *dados);
