
# Projeto de Exibição com WS2812 e LED RGB

## Descrição

Projeto desenvolvido para o curso de capacitação Embarcatech, onde demonstra o uso do Raspberry Pi Pico (ou Pico W) 
com o SDK Pico para implementar as seguintes funcionalidades:

- **Matriz 5x5 de LEDs WS2812:**  
  Utiliza um buffer para definir quais dos 25 LEDs (organizados em 5 linhas por 5 colunas) estarão acesos. Cada dígito (de 0 a 9) possui um padrão pré-definido no estilo digital.

- **LED RGB (Canal Vermelho):**  
  Apenas o canal vermelho do LED RGB pisca a 5 Hz, ou seja, o LED alterna seu estado a cada 100 ms, resultando em um ciclo completo de ligar e desligar a cada 200 ms.

- **Controle via Botões:**  
  Dois botões conectados aos pinos GPIO 5 e 6 permitem:
  - **Botão A:** Incrementar o dígito exibido.
  - **Botão B:** Decrementar o dígito exibido.  
  O projeto implementa *debounce* por software para evitar múltiplos acionamentos indesejados.

## Requisitos

- **Hardware:**
  - Raspberry Pi Pico ou Pico W
  - Matriz de LEDs WS2812 (5x5)
  - LED RGB
  - Botões (para incrementar e decrementar)

- **Software:**
  - Raspberry Pi Pico SDK configurado (usando VS Code)
  - Git instalado

## Como Clonar o Repositório

Para clonar este repositório, siga os passos abaixo:

1. Abra o terminal ou o prompt de comando.
2. Execute o comando de clone (substitua `seu-usuario` e `nome-repositorio` pelos dados correspondentes):

   ```bash
   git clone https://github.com/seu-usuario/nome-repositorio.git
   ```

3. Navegue até a pasta do projeto clonado:

   ```bash
   cd nome-repositorio
   ```

4. Abra o projeto no VS Code:

   ```bash
   code .
   ```

## Estrutura do Projeto

- **main.c:**  
  Código fonte principal contendo a lógica para exibir os dígitos na matriz WS2812, controlar o LED RGB e tratar os botões.

- **ws2812.pio.h:**  
  Arquivo gerado pelo `pioasm` contendo o programa PIO para controlar os LEDs WS2812.

- **.gitignore:**  
  Arquivo que define quais arquivos e pastas serão ignorados pelo Git (ex.: arquivos de build, arquivos temporários, etc).

- **README.md:**  
  Este arquivo, que descreve o projeto, suas funcionalidades e instruções para clonar o repositório.

## Como Utilizar

1. **Abra o projeto no VS Code:**  
   Utilize o comando `code .` na pasta do projeto ou abra-o diretamente pelo VS Code.

2. **Compilação e Upload:**  
   Com o SDK configurado no VS Code, utilize as tarefas integradas (tasks) para compilar e fazer o upload do firmware para o seu Raspberry Pi Pico. Siga as instruções do VS Code para compilar o projeto e transferir o firmware para o dispositivo.

## Funcionamento do Código

O código é dividido em partes:

- **Configuração Inicial:**  
  Inicializa os pinos para o LED RGB e os botões (com pull-up interno) e configura as interrupções para tratar os botões.

- **Controle do LED RGB:**  
  Utiliza um delay não bloqueante, baseado em tempo absoluto, para alternar o estado do LED a cada 100 ms, fazendo-o piscar a 5 Hz.

- **Controle da Matriz WS2812:**  
  Um buffer de 25 posições armazena o padrão de cada dígito (0 a 9). Ao acionar os botões, o dígito é atualizado e o padrão correspondente é exibido na matriz WS2812.

- **Loop Principal:**  
  O loop principal monitora se há atualizações do dígito e mantém o sistema responsivo, sem bloquear a execução com delays tradicionais.

## Licença

Este projeto é licenciado sob a [MIT License](LICENSE).

## Contato

Desenvolvido por [Heitor Lemos]  


---

Basta salvar esse conteúdo em um arquivo chamado **README.md** na raiz do seu repositório. Esse arquivo fornece uma visão geral clara do projeto, explica como clonar o repositório, estrutura do projeto, funcionamento do código, e demais informações úteis para usuários e desenvolvedores.
