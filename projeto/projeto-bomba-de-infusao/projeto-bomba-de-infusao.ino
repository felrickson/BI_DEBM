#include <LiquidCrystal.h>  //Inclui a biblioteca para display LCD
#include <Keypad.h>         //Inclui a biblioteca para o teclado
#include <string.h>         //Inclui a biblioteca para strings

// PINOS USADOS: 4 ao 9 (display, porém estes bloqueiam outros), (22, 24, 26, 28, 38, 36, 34, 32, 30) (teclado), 44 (motor), 48 (buzzer), 49 (led), (A1) conta bolha, (A2) conta gota

// ALARMES
int buzPin = 48;    // Pino do buzzer
int ledPin = 49;    // Pino do led
int buzFreq = 528;  // Frequência do alarme sonoro
int bolhaPin = A1; // Pino do conta bolha
int gotaPin = A2;  // Pino do conta gota

// DEFINIÇÕES DO DISPLAY
const int rs = 8, en = 9, d4 = 4, d5 = 5, d6 = 6, d7 = 7;   //Pinos para ligar o display
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);                  //Define os pinos que serão usados para ligar o display

// Variáveis para o temporizador da infusão
int horas = 0, minutos = 0, segundos = 0;

unsigned long tempo = 0;              // Tempo total de infusão em segundos
int volume = 0;                       // Volume total de infusão em ml
int vazao = 0;                        // Vazão de infusão em ml/h
char modo = " ";                      // Modo de infusão: F1 -> Modo A (usuário define volume e vazão) ou F2 -> Modo B (usuário define volume e tempo)
int estadoMaquina = 0;                // Se a máquina está ligada (1) ou desligada (0)
int infusaoEmAndamento = 0;           // Se uma infusão está em andamento (1) ou não (0)

// LIMITES DE OPERAÇÃO DO EQUIPAMENTO
int vazaoMin = 1;                     // Limite de 1 ml/h no mínimo
int vazaoMax = 4600;                  // Limite de 4600 ml/h no máximo
int volumeMin = 1;                    // Limite de 1 ml no mínimo
int volumeMax = 4600;                 // Limite de 4600 ml no máximo
unsigned long tempoMax = 59*3600;    // Limite de 59h (convertido para segundos)
int vazaoKVO = 3;                     // Vazão KVO padrão de 3 ml/h
int vazaoKVOMin = vazaoMin;           // Vazão KVO mínima igual a vazão mínima
int vazaoKVOMax = 10;                 // Vazão KVO máxima de 10 ml/h

// DEFINIÇÕES MOTOR
int  motorPin = 44;               // Pino do motor (precisa ser pwm)
const float VN = 255.0/vazaoMax;  // Vazão nominal
float velocidade = 0;             // Velocidade do motor

// DEFINIÇÕES DO TECLADO
const byte LINHAS = 5;      // Linhas do teclado
const byte COLUNAS = 4;     // Colunas do teclado

const char TECLAS_MATRIZ[LINHAS][COLUNAS] = {   // Matriz de caracteres (mapeamento do teclado)
  {'A', 'B', '#', '*'},
  {'1', '2', '3', 'T'},
  {'4', '5', '6', 'D'},
  {'7', '8', '9', 'X'},
  {'L', '0', 'R', 'E'}
};

byte PINOS_LINHAS[LINHAS] = {38, 36, 34, 32, 30};   // Pinos de conexão com as linhas do teclado
byte PINOS_COLUNAS[COLUNAS] = {22, 24, 26, 28};     // Pinos de conexão com as colunas do teclado

Keypad teclado = Keypad(makeKeymap(TECLAS_MATRIZ), PINOS_LINHAS, PINOS_COLUNAS, LINHAS, COLUNAS);   // Inicia teclado


void setup() {
  pinMode(ledPin, OUTPUT);    // Pino do led como saída
  pinMode(buzPin, OUTPUT);    // Pino do buzzer como saída
  pinMode(bolhaPin,INPUT);
  lcd.begin(16, 2);           // Inicia display, definindo número de colunas e linhas (16x2)
  lcd.noDisplay();            // Desliga luz de fundo do display
  Serial.begin(19200);
}

void inicializacao() {            // Imprimindo mensagem de inicialização
  
  lcd.setCursor(0, 0);            // Selecionando coluna 0 e linha 0
  lcd.print("BOMBA DE INFUSAO");  // Print da mensagem
  lcd.setCursor(3, 1);            // Selecionando coluna 3 e linha 1
  lcd.print("EB 2023.1");         // Print da mensagem
  delay(3000);                    // Atraso de 3 segundos
  lcd.clear();                    // Limpa a tela
}

void telaInicial() {            // Mensagem de início de tela
  
  lcd.clear();                  // Limpa display
  lcd.setCursor(0, 0);          // Selecionando coluna 0 e linha 0
  lcd.print("Clicar em Menu");  // Print da mensagem
  lcd.setCursor(0, 1);          // Selecionando coluna 3 e linha 1
  lcd.print("para iniciar.");   // Print da mensagem
}

void iniciarInfusao(){   // ACIONA MOTOR, MOSTRA TEMPO E VOLUME RESTANTE NO DISPLAY

  infusaoEmAndamento = 1;               // Muda variável que indica infusão
  float inputBolha;                     // Variavel de leitura do conta bolha
  float leitorBolha;                    // Variavel de conversão do conta bolha
  int volumeInicial = volume;           // Pega o volume de infusão obtido através do menu
  unsigned long tempoInicial = tempo;   // Pega o tempo de infusão obtido/calculado através do menu 
  unsigned long tempoDecorrido = 0;     // Tempo decorrido de infusão
  int volumeInfundido = 0;              // Volume infundido
  char tecla = " ";                     // Variável para observar se alguma tecla foi pressionada
  
  while (tempo > 0)   // Enquanto tempo restante de infusão for maior que zero
  { 
    inputBolha = analogRead(bolhaPin);
    leitorBolha = inputBolha * 5/1023;
    //Serial.println(leitorBolha);
    // ACIONA MOTOR
    analogWrite(motorPin, map((int)velocidade, 0, 255, 51, 255));  // Aciona motor com velocidade calculada através da seleção dos parâmetros no menu

    // CONVERTE O TEMPO EM HORAS, MINUTOS E SEGUNDOS PARA MOSTRAR NO DISPLAY TEMPO RESTANTE
    tempo--;       // Decrementa o tempo restante
    delay(1000);   // Atraso de 1 segundo
    
    horas = ((tempo / 60)/ 60);    // Converte tempo restante em horas (t/3600)
    minutos = (tempo / 60) % 60;   // Converte tempo restante em minutos (ao separar as horas, pega o que sobrou de minutos)
    segundos = tempo % 60;         // Converte tempo restante em segundos (pega o que sobra de segundos ao separar os minutos)
    
    // MOSTRA HORA NA TELA
    lcd.setCursor(0,1);               // Posiciona cursor na coluna 0 e linha 1
    lcd.print("Tempo: ");             // Imprime mensagem
    lcd.setCursor(7,1);               // Posiciona cursor na coluna 7 e linha 1
    if (horas < 10) lcd.print("0");   // Se for menor que 10, adiciona um zero    
    lcd.print(horas);                 // Imprime mensagem                  
    lcd.print(":");                   // Imprime mensagem
    if (minutos < 10) lcd.print("0"); // Se for menor que 10, adiciona um zero      
    lcd.print(minutos);               // Imprime mensagem                
    lcd.print(":");                   // Imprime mensagem  
    if (segundos < 10) lcd.print("0");// Se for menor que 10, adiciona um zero      
    lcd.print(segundos);              // Imprime mensagem  
    
    // MOSTRA VOLUME NA TELA
    tempoDecorrido = tempoInicial - tempo;          // Calcula o tempo decorrido de infusão: tempo inicial - tempo restante
    volumeInfundido = (float)vazao * (float)tempoDecorrido/3600.0;  // Calcula o volume infundido até o momento; vazão (ml/h) * tempo decorrido (h)
    volume = volumeInicial - volumeInfundido;       // Atualiza o valor de volume restante a ser infundido em ml

    lcd.setCursor(0, 0);              // Posiciona cursor coluna 0 e linha 0
    lcd.print("V.rest.:");            // Imprime mensagem
    lcd.print(volume);                // Imprime mensagem
    lcd.print("ml ");                 // Imprime mensagem

    tecla = teclado.getKey();   // Lê teclado

    if (leitorBolha >= 0 && leitorBolha <= 0.1){  // caso encontre uma bolha
      tone(buzPin, buzFreq);
      botaoPausarInfusao(false);
      noTone(buzPin);
      continue;
      }
     
    // CONFERE SE DESEJA PAUSAR
    if(tecla == 'T'){         // Se botão de pausa for pressionado, pausa infusão
      botaoPausarInfusao(true);   // Pausa infusão
      continue;
    }  
             
    // CONFERE SE DESEJA PARAR
    if(tecla == 'L'){       // Se botão de parar for pressionado, interrompe infusão
      botaoPararInfusao();  // Para infusão
      continue;
    }   
    
    // CONFERE SE DESEJA MODO KVO
    if(tecla == 'D'){               // Se o modo KVO for acionado
      botaoKVO();                   // Aciona modo KVO
      tempoInicial = tempo;         // Tempoinicial da infusão recebe o tempo restante recalculado para a vazão de KVO
      volumeInicial = volume;       // Muda volume inicial para o volume ao acionar o KVO
    }

    // MENSAGEM FIM DE TEMPO
    if (tempo == 0)      // Quando o tempo de infusão acabar    
    {   
      analogWrite(motorPin, LOW);     // Desaciona o motor
      delay(1000);                    // Atraso de 1 segundo
      lcd.clear();                    // Limpa display         
      lcd.setCursor(0, 0);            // Posiciona cursor coluna 0 e linha 1         
      lcd.print("FINAL DE INFUSAO");  // Imprime mensagem        
      aguardarDesativarAlarme();      // Função: aguarda o usuário desativar o alarme para parar de alarmar
      horas = 0, minutos = 0, segundos = 0, tempo = 0, volume = 0, vazao = 0, modo = " ", velocidade = 0, infusaoEmAndamento = 0;  // Reseta todas variáveis globais de infusão
      telaInicial();                  // Volta para a tela inicial
    } 
  }
}

// TECLADO
int capturaValorNumerico() {        // Captura valor númerico de algum parâmetro (volume, vazão ou tempo)
  
  int valorInteiro;                 // Variável para guardar valor do parâmetro como inteiro
  String valorString;               // Variável para guardar valor do parâmetro como string
  char tecla = teclado.getKey();    // Lê teclado

  while(tecla != 'E') {                             // Enquanto não pressionar "E" (Enter)
    tecla = teclado.getKey();                       // Lê o teclado
    if(tecla) {                                     // Verifica se alguma tecla foi pressionada
      if(tecla >= '0' && tecla <= '9'){             // Se a tecla pressionada for entre 0 e 9
        valorString += tecla;                       // Concatena o dígito na variável de valor (variável do tipo string)
        lcd.setCursor(15 - valorString.length(), 1);// Posiciona cursor na posicão que caiba a string à direita do display, linha 1, para aparecer os dígots enquanto o usuário está digitando na tela
        lcd.print(valorString);                     // Imprime o dígito pressionado
      } else if (tecla == 'E') {                    // Ou se a tecla pressionada for "E" (Enter)
        if(valorString.length() > 0) {              // Se tiver sido digitado algum valor numérico antes do Enter
          valorInteiro = valorString.toInt();       // Converte o valor para inteiro
          valorString = "";                         // Limpa entrada
          lcd.setCursor(11, 1);                     // Posiciona cursor no início do número
          lcd.print(valorString);                   // Imprime o valor pressionado completo, sobrescrevendo o que foi digitado antes
          delay(500);                               // Atraso de 0,5 segundo
          return valorInteiro;                      // Retorna valor como inteiro
        }
      } else if (tecla == 'X'){           // Se tecla digitada for "X" (Esc)  
        valorString = "";                 // Limpa entrada
        lcd.setCursor(11, 1);             // Posiciona cursor no início do número
        lcd.print("     ");               // Remove número digitado, e volta a repetir o laço
      }  
    }    
  }
}

int confirmacaoDuasEtapas(String pergunta) {    // Confirmação de duas etapas de alguma pergunta passada como parâmetro
  
  lcd.clear();                       // Limpa display
  lcd.setCursor(0, 0);               // Posiciona cursor
  lcd.print(pergunta);               // Printa mensagem
  lcd.setCursor(0, 1);               // Posiciona cursor
  lcd.print("SIM:Ent|NAO:Esc");      // Printa mensagem
  char resposta = " ";               // inicia resposta vazia

  while(!(resposta == 'E' || resposta == 'X')) {        // Enquanto resposta não for Enter ou Esc
    resposta = teclado.getKey();                      // Lê teclado
    if (resposta == 'E') {                            // Se a resposta for Enter
      resposta = " ";                                 // Limpa resposta para a próxima pergunta
      lcd.clear();                                    // Limpa display          
      lcd.setCursor(0, 0);                            // Posiciona cursor na coluna 0 e linha 0
      lcd.print("Tem certeza?");                      // Imprime mensagem
      lcd.setCursor(0, 1);                            // Posiciona cursor na coluna 0 e linha 1
      lcd.print("SIM:Ent|NAO:Esc");                   // Imprime mensagem
      while(!(resposta == 'E' || resposta == 'X')) {  // Enquanto resposta não for Enter ou Esc
        resposta = teclado.getKey();                  // Lê teclado
        if (resposta == 'E') {                        // Se a resposta for Enter
          lcd.clear();                     // Limpa display
          return 1;                        // Retorna 1
        } else if (resposta == 'X') {      // Se a resposta for Esc, isto é, se usuário recusar na segunda pergunta
          lcd.clear();                     // Limpa display
          return 0;                        // Retorna 0
        }   
      }
     } else if (resposta == 'X') {    // Se a resposta for Esc, isto é, se usuário recusar na primeira pergunta
      lcd.clear();                    // Limpa display
      return 0;                       // Retorna 0
    }    
  }  
}

void mostrarMenu(){   // PEDE PARÂMETROS DE INFUSÃO (VOLUME, MODO, VAZÃO OU TEMPO) E INICIA INFUSÃO
  
  lcd.clear();      // limpa o display para a próxima tela
  
  // CAPTURA VOLUME
  while(volume < volumeMin || volume > volumeMax) {   // Enquanto volume digitado não estiver dentro dos limites
    lcd.setCursor(0, 0);                              // Posiciona cursor na coluna 0 e linha 0
    lcd.print("Selecione volume:");                   // Imprime mensagem
    lcd.setCursor(0, 1);                              // Posiciona cursor na coluna 0 e linha 1
    lcd.print("V.tot(mL):");                          // Imprime mensagem
    volume = capturaValorNumerico();                  // Captura volume
    if (volume != 0) {                                // Verifica se valor foi digitado ou não foi nulo
      if (volume < volumeMin || volume > volumeMax){  // Verifica se valor está fora dos limites
        lcd.clear();                           // Limpa display
        lcd.setCursor(0, 1);                   // Posiciona cursor na coluna 0 e linha 1
        lcd.print("Fora dos limites");         // Imprime mensagem
        delay(1000);                           // Atraso de 1 segundo
        lcd.clear();                           // Limpa display e volta a repetir o laço
      }
    }
  } 
  
  lcd.clear();    // Limpa o display para a próxima tela
 
  // CAPTURA MODO
  while (!(modo == 'A' || modo == 'B')) {   // Enquanto tecla selecionada não for "A"(F1) ou "B" (F2)
    lcd.setCursor(0, 0);                    // Posiciona cursor na coluna 0 e linha 0
    lcd.print("Selecione modo:");           // Imprime mensagem
    lcd.setCursor(0, 1);                    // Posiciona cursor na coluna 0 e linha 1
    lcd.print("F1-VAZ F2-TEMP: ");          // Imprime mensagem
    modo = teclado.getKey();                // Captura tecla
  }
  
  lcd.clear();

  // SE MODO A (F1)
  if (modo == 'A') {
    // CAPTURA VAZÃO
    while(vazao < vazaoMin || vazao > vazaoMax) {   // Enquanto vazão digitada não estiver dentro dos limites
      lcd.setCursor(0, 0);                          // Posiciona cursor na coluna 0 e linha 0     
      lcd.print("Selecione vazao:");                // Imprime mensagem
      lcd.setCursor(0, 1);                          // Posiciona cursor na coluna 0 e linha 1      
      lcd.print("Vaz(ml/h):");                      // Imprime mensagem
      vazao = capturaValorNumerico();               // Captura vazão (ml/h)
      if (vazao != 0) {                             // Verifica se valor foi digitado ou não foi nulo          
        if (vazao < vazaoMin || vazao > vazaoMax){  // Se a vazão estiver fora dos limites 
          lcd.clear();                      // Limpa display
          lcd.setCursor(0, 1);              // Posiciona cursor na coluna 0 e linha 1  
          lcd.print("Fora dos limites");    // Imprime mensagem
          delay(1000);                      // Atraso de 1 segundo
          lcd.clear();                      // Limpa display e volta a pedir a vazão
        } else {
          if (confirmacaoDuasEtapas("Iniciar infusao?")) {  // Confirmação de duas etapas para iniciar infusão
            velocidade = VN*vazao;                          // Calcula velocidade do motor
            tempo = ((float)volume/(float)vazao)*3600.0;     // Calcula o tempo de infusão em segundos com volume (ml) e vazão (ml/h) passados como parâmetros
            lcd.setCursor(0, 0);             // Posiciona cursor na coluna 0 e linha 0
            lcd.print("Iniciando");          // Imprime mensagem
            lcd.setCursor(0, 1);             // Posiciona cursor na coluna 0 e linha 1 
            lcd.print("infusao...");         // Imprime mensagem
            delay(3000);                     // Atraso de 3 segundos
            lcd.clear();                     // Limpa display
          } else {
            tempo = 0, volume = 0, vazao = 0, modo = " ", velocidade = 0; // Reseta todas variáveis modificadas no menu
            telaInicial();
          }
        }
      }
    }
  }  
  
  // SE MODO B
  if (modo == 'B') {
    // CAPTURA TEMPO
    while(tempo == 0 || tempo > tempoMax) { // Enquanto o tempo for zero ou maior que tempo limite
      lcd.setCursor(0, 0);                  // Posiciona cursor na coluna 0 e linha 0   
      lcd.print("Digite horas+ENT");        // Imprime mensagem
      lcd.setCursor(0, 1);                  // Posiciona cursor na coluna 0 e linha 1    
      lcd.print("T(hh:mm):");               // Imprime mensagem
      
      horas = capturaValorNumerico();   // Captura horas
      lcd.setCursor(11, 1);             // Posiciona cursor na coluna 10 e linha 1
      lcd.print(horas)  ;      // Imprime mensagem
      lcd.print(":   ")  ;      // Imprime mensagem
  
      lcd.setCursor(0, 0);                  // Posiciona cursor na coluna 0 e linha 0    
      lcd.print("Minutos + ENT:  ");        // Imprime mensagem
      minutos = capturaValorNumerico();     // Captura minutos
      tempo = horas * 3600 + minutos * 60;  // Calcula o tempo total de infusão em segundos
      
      if (tempo != 0) {                   // Verifica se o tempo foi digitado ou não foi nulo
        if (tempo > tempoMax){            // Se tempo digitado pelo usuário for maior que o tempo máximo permitido no equipamento 
          lcd.clear();                    // Limpa o display
          lcd.setCursor(0, 1);            // Posiciona o cursor na coluna 0 e linha 1
          lcd.print("Fora dos limites");  // Imprime mensagem
          delay(1000);                    // Atraso de 1 segundo
          lcd.clear();                    // Limpa o display
        } else {                          // Se o tempo digitado estiver dentro do limite permitido
          delay(1000);                    // Atraso de 1 segundo
          lcd.clear();                    // Limpa display
          if (confirmacaoDuasEtapas("Iniciar infusao?")) {  // Confirmação de duas etapas para iniciar infusão
            vazao = (float)volume/((float)tempo/3600.0);    // Calcula a vazão de infusão em ml/h
            velocidade = VN*vazao;          // Calcula velocidade do motor
            lcd.clear();                    // Limpa display
            lcd.setCursor(0, 0);            // Posiciona cursor na coluna 0 e linha 0
            lcd.print("Iniciando");         // Imprime mensagem
            lcd.setCursor(0, 1);            // Posiciona cursor na coluna 0 e linha 1
            lcd.print("infusao...");        // Imprime mensagem
            delay(3000);                    // Atraso de 3 segundos
          } else {
            horas = 0, minutos = 0, tempo = 0, volume = 0, vazao = 0, modo = " ", velocidade = 0; // Reseta todas variáveis modificadas no menu
            telaInicial();
          }
        }
      }
    }    
  }
      
  iniciarInfusao();  // Função: Aciona motor, mostra volume e tempo restante no display até concluir infusão
}

void aguardarDesativarAlarme(){

  lcd.setCursor(0, 1);            // Posiciona cursor coluna 0 e linha 1 
  lcd.print("Press ENT ->Sair");  // Imprime mensagem
  tone(buzPin, buzFreq);          // Liga o buzzer
  digitalWrite(ledPin, HIGH);     // Liga o led
  delay(2000);                    // Atraso de 2 segundos
      
  while(teclado.getKey() != 'E') {  // Enquanto tecla digitada não for Enter, ficar piscando a tela
    lcd.noDisplay();     // Apaga display
    delay(500);          // Atraso de 0,5 segundo
    lcd.display();       // Acende display
    delay(2000);         // Atraso de 2 segundos
   }
   noTone(buzPin);                 // Desliga o buzzer
   digitalWrite(ledPin, LOW);      // Desliga o led
   lcd.clear();                    // Limpa cursor
}

void botaoLigar() {   // Se botão de ligar for pressionado, ligar a máquina
  
  if(teclado.getKey() == '*') { // Se o botão pressionado for o de ligar ("*")
    estadoMaquina = 1;          // Indica que máquina está ligada
    lcd.display();              // Liga luz de fundo do display
    inicializacao();            // Mostra mensagem de inicialização da máquina
    telaInicial();              // Mostra o menu de parâmetros para o usuário 
  }
}

void botaoMenu() {   // Se botão menu for pressionado, mostra o menu
  
  if(teclado.getKey() == 'R') {   // Se o botão pressionado for o de menu ("->", seta para a direita)
    if(estadoMaquina == 1){       // Se a máquina estiver ligada
      mostrarMenu();              // Mostra o menu de parâmetros para o usuário
    }     
  }
}

void botaoPausarInfusao(int verificacao) {   // Pausar infusão 
  
    if (verificacao == true){
       if(confirmacaoDuasEtapas("Deseja pausar?")){      // Pede confirmação de duas etapas
      lcd.setCursor(0, 0);                            // Posiciona cursor na coluna 0 e linha 0
      lcd.print("Infusao pausada!");                  // Imprime mensagem
      delay(2000);                                    // Atraso de 2 segundos
      if(!confirmacaoDuasEtapas("Deseja retomar?")){  // Pede confirmação de duas etapas - caso negada:
        analogWrite(motorPin, LOW);                   // Desaciona o motor
        horas = 0, minutos = 0, segundos = 0, tempo = 0, volume = 0, vazao = 0, velocidade = 0, modo = " ", infusaoEmAndamento = 0; // Reseta todas variáveis        
        lcd.setCursor(0, 0);                // Posiciona cursor na coluna 0 e linha 0
        lcd.print("Fim de infusao!");       // Imprime mensagem
        delay(2000);                        // Atraso de 2 segundos
        telaInicial();                      // Vai para a tela inicial
      }
    }  
      }
   else{
      lcd.setCursor(0, 0);                            // Posiciona cursor na coluna 0 e linha 0
      lcd.print("Infusao pausada devido a bolha na mangueira!");                  // Imprime mensagem
      delay(2000);
      if(!confirmacaoDuasEtapas("Deseja retomar?")){  // Pede confirmação de duas etapas - caso negada:
        analogWrite(motorPin, LOW);                   // Desaciona o motor
        horas = 0, minutos = 0, segundos = 0, tempo = 0, volume = 0, vazao = 0, velocidade = 0, modo = " ", infusaoEmAndamento = 0; // Reseta todas variáveis        
        lcd.setCursor(0, 0);                // Posiciona cursor na coluna 0 e linha 0
        lcd.print("Fim de infusao!");       // Imprime mensagem
        delay(2000);                        // Atraso de 2 segundos
        telaInicial();                      // Vai para a tela inicial
      }
   }
}

int botaoPararInfusao() {   // Parar infusão 
  
    if(confirmacaoDuasEtapas("Deseja parar?")){       // Pede confirmação de duas etapas
      lcd.setCursor(0, 0);                            // Posiciona cursor na coluna 0 e linha 0
      lcd.print("Fim da infusao!");                   // Imprime mensagem
      analogWrite(motorPin, LOW);                     // Desaciona o motor
      horas = 0, minutos = 0, segundos = 0, tempo = 0, volume = 0, vazao = 0, velocidade = 0, modo = " ", infusaoEmAndamento = 0; // Reseta todas variáveis  
      delay(2000);                        // Atraso de 2 segundos      
      telaInicial();                      // Vai para a tela inicial
    }
}

int botaoKVO() {   // Se botão de modo KVO for pressionado, durante infusão, entra no modo KVO
  
    if(confirmacaoDuasEtapas("Deseja modo KVO?")){  // Pede confirmação de duas etapas
      vazao = vazaoKVO;                             // Configura vazão de KVO
      velocidade = VN*vazao;                        // Configura velocidade do motor com vazão de KVO
      tempo = ((float)volume/(float)vazao)*3600.0;  // Calcula o novo tempo de infusão, com a vazão de KVO
      lcd.clear();                         // Limpa cursor
      lcd.setCursor(0, 0);                 // Posiciona cursor coluna 0 e linha 0
      lcd.print("KVO acionado!");          // Imprime mensagem
      delay(2000);                         // Atraso de 2 segundos
      lcd.clear();                         // Limpa cursor
    }
}

int botaoConfigurarKVO() {   // Se botão de modo KVO for pressionado, fora da infusão, modifica a vazão de KVO
  
  if (!infusaoEmAndamento){                               // Se nenhuma infusão está em andamento
    if(teclado.getKey() == 'D') {                         // Se o botão pressionado for o de KVO ("D" - down)
      if(confirmacaoDuasEtapas("Mudar vazao KVO?")){      // Pede confirmação de duas etapas
        int resposta = 0;
        while(resposta < vazaoKVOMin || resposta > vazaoKVOMax) { // Enquanto resposta digitada não estiver dentro dos limites
          lcd.setCursor(0, 0);                            // Posiciona cursor na coluna 0 e linha 0
          lcd.print("Digite vazao KVO");                  // Imprime mensagem
          lcd.setCursor(0, 1);                            // Posiciona cursor na coluna 0 e linha 1
          lcd.print("Vaz(mL/h):");                        // Imprime mensagem
          resposta = capturaValorNumerico();              // Captura nova vazão KVO
          if (resposta != 0) {                              // Verifica se valor foi digitado ou não foi nulo
            if (resposta < vazaoKVOMin || resposta > vazaoKVOMax){  // Verifica se valor está fora dos limites
              lcd.clear();                          // Limpa display
              lcd.setCursor(0, 1);                  // Posiciona cursor na coluna 0 e linha 1
              lcd.print("Fora dos limites");        // Imprime mensagem
              delay(1000);                          // Atraso de 1 segundo
              lcd.clear();                          // Limpa display e volta a repetir o laço
            } else {                                // Se a resposta estiver dentro dos limites
              vazaoKVO = resposta;                  // Salva a nova vazão de KVO
              lcd.clear();                          // Limpa cursor
              lcd.setCursor(0, 0);                  // Posiciona cursor coluna 0 e linha 0
              lcd.print("Vazao KVO mudada");        // Imprime mensagem
              lcd.setCursor(0, 1);                  // Posiciona cursor coluna 0 e linha 0
              lcd.print("Vazao KVO:");              // Imprime mensagem
              lcd.print(vazaoKVO);              // Imprime mensagem
              lcd.print("ml");              // Imprime mensagem
              delay(2000);                          // Atraso de 2 segundos
              lcd.clear();                          // Limpa display
              telaInicial();                        // Volta para a tela inicial
            }
          }
        } 
      } else {            // Caso não confirme
        telaInicial();    // Volta para a tela inicial                           
      }
    } 
  }
}

void botaoDesligar() {    // Se botão de desligar ('#') for pressionado, desliga o equipamento
 
  if(teclado.getKey() == '#') {                   // Se o botão pressionado for o de desligar ("#")
    if(confirmacaoDuasEtapas("Desligar bomba?")){ // Confimação de duas etapas para desligar      
      estadoMaquina = 0;                          // Indica que máquina está desligada
      digitalWrite(motorPin, LOW);      // Desligar motor
      lcd.clear();                      // Limpa display
      lcd.setCursor(0, 0);              // Posiciona cursor na coluna 0 e linha 0
      lcd.print("Desligando...");       // Imprime mensagem
      delay(3000);                      // Atraso de 3 segundos
      lcd.clear();                      // Limpa display
      lcd.noDisplay();                  // Desliga luz de fundo do display
      horas = 0, minutos = 0, segundos = 0, tempo = 0, volume = 0, vazao = 0, velocidade = 0, modo = " ", infusaoEmAndamento = 0; // Reseta todas variáveis
    } else if(!infusaoEmAndamento){     // Se não há uma infusão em andamento
      telaInicial();                    // Volta para a tela inicial
    }
  }  
}

void loop() {
  
  if(estadoMaquina == 0) {  // Caso a máquina esteja desligada  
    botaoLigar();           // Verifica se botão Ligar foi pressionado 
  }

  if(estadoMaquina == 1) {  // Caso a máquina esteja ligada   
    botaoDesligar();        // Verifica se botão Desligar foi pressionado
    botaoConfigurarKVO();   // Verifica se botão Configurar vazão de KVO foi pressionado
    botaoMenu();            // Verifica se botão Menu foi pressionado
  }
}
