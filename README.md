# Projeto-Bar-Etico

## Integrantes
- Daniel Henriques Pamplona - RA: 260401
- Gabriel Pavani Giro - RA: 247114
- Jonatas de Sousa Santos - RA: 225334

## Descrição do problema
Este projeto consiste de um simulador de bar implementado em C que demonstra conceitos de programação concorrente usando threads e semáforos.
O programa simula o funcionamento de um bar com clientes e barmans, onde:

- Múltiplos clientes tentam entrar no bar
- Número limitado de cadeiras disponíveis
- Apenas um tipo de bebida é servido por vez
- Barmans atendem os clientes que conseguem entrar

O objetivo é demonstrar a coordenação de recursos compartilhados e controle de concorrência.

## Visualização

O programa exibe no terminal o estado atual do bar, mostrando:

- Tipo de bebida sendo servida
- Estado dos clientes
- Ocupação das cadeiras
- Estado dos barmans

## Uso
Compile o programa em C:

```bash
gcc -o bar barman.c -lpthread
```
Rodar o programa:
```bash

./bar
```

## Documentação
Para uma explicação detalhada da implementação, incluindo os estados, fluxo do programa e mecanismos de sincronização, consulte o arquivo x neste repositório.
