const int sensorPin = A15;
int gotas = 0;
int intervalo = 1;
int contador = 0;
bool gotaDetectada = false;
float limiar = 75; // Limiar para condicao de contagem

void setup() {
   Serial.begin(9600);
   pinMode(sensorPin, INPUT);
}

void loop() {

    while(contador<1000){

    gotas = 0;

        int sensorTensao = analogRead(sensorPin)*4.88758;

   // Verifica se a tensao esta acima do limiar para detectar uma gota
   if (sensorTensao > limiar && !gotaDetectada) {
      gotas++;
      gotaDetectada = true;
   } else if (sensorTensao < limiar) {
      gotaDetectada = false;
   }
   delay(intervalo);
}
      Serial.println("contei");
}
