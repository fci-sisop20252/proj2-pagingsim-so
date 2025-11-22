[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/S4CMIoFL)
[![Open in Codespaces](https://classroom.github.com/assets/launch-codespace-2972f46106e565e64193e422d61a12cf1da4916b45550586e14ef0a7c637dd04.svg)](https://classroom.github.com/open-in-codespaces?assignment_repo_id=21790979)
# Projeto 2: Simulador de Gerenciamento de Memória Virtual

**Disciplina:** Sistemas Operacionais
**Professor:** Lucas Figueiredo
**Email:** lucas.figueiredo@mackenzie.br

## Descrição

Neste projeto, você implementará um simulador de gerenciamento de memória virtual com paginação. O simulador deve processar uma sequência de acessos à memória de múltiplos processos e aplicar algoritmos de substituição de páginas quando ocorrer page fault e a memória física estiver cheia.

Você implementará dois algoritmos de substituição: **FIFO (First In First Out)** e **Clock (Segunda Chance)**. O objetivo é compreender na prática como funciona a paginação, tradução de endereços, page faults e como diferentes algoritmos de substituição impactam o desempenho do sistema.

## Objetivo de Aprendizado

Este projeto aplica diretamente os conceitos apresentados nas **Aulas 11 e 12** sobre gerenciamento de memória virtual. Você terá a oportunidade de implementar estruturas de dados para tabelas de páginas, simular o comportamento do hardware de MMU (Memory Management Unit), e comparar algoritmos de substituição em cenários práticos.

## Arquivos Importantes

- **ENUNCIADO.md** - Especificação completa do projeto com formato de entrada/saída
- **CONCEITOS.md** - Explicação teórica de paginação, page fault e algoritmos de substituição
- **RELATORIO_TEMPLATE.md** - Template do relatório a ser entregue
- **FAQ.md** - Perguntas frequentes e dicas de implementação
- **tests/** - Casos de teste com entradas e saídas esperadas

## Requisitos Técnicos

**Linguagem:** O projeto deve ser implementado em **C**.

Você tem **autonomia** para decidir:

- Estrutura de código (arquivo único, múltiplos arquivos .c/.h)
- Estruturas de dados para representar tabelas de páginas e frames
- Sistema de compilação (Makefile, gcc direto, script build.sh)

**Requisito obrigatório:** Seu simulador deve seguir o formato de entrada e saída especificado no ENUNCIADO.md, permitindo testes automatizados e avaliação objetiva.

## Como Começar

1. Leia o **ENUNCIADO.md** completamente para entender os requisitos
2. Estude o **CONCEITOS.md** para revisar a teoria de paginação
3. Analise os casos de teste no diretório **tests/**
4. Planeje suas estruturas de dados (tabela de páginas, frames físicos)
5. Implemente incrementalmente (tradução de endereços → HIT → PAGE FAULT → algoritmos)
6. Teste frequentemente comparando sua saída com as saídas esperadas

## Avaliação

A avaliação será baseada em:

- **Funcionalidade** (6 pontos) - Testes automatizados verificando comportamento correto
- **Relatório** (2 pontos) - Decisões de design, análise comparativa FIFO vs Clock
- **Vídeo** (2 ponto) - Demonstração do simulador funcionando

Consulte o ENUNCIADO.md para detalhes completos da rubrica de avaliação.

## Entrega

1. Faça os commits de todo seu código e relatório no repositório
2. Verifique que seu código compila/executa conforme instruções do relatório
3. Certifique-se de incluir o link do vídeo no relatório
4. Push para o repositório antes da data limite

```bash
git add .
git commit -m "Projeto 2 completo"
git push
```

Boa sorte e bom trabalho!
