const int sensorPin = A15;
int gotas = 0;
bool gotaDetectada = false;
float limiar = 75; // Limiar para condicao de contagem

void setup() {
   Serial.begin(9600);
   pinMode(sensorPin, INPUT);
}

void loop() {
   // Le o valor analogico do sensor e converte para tensao em mV
   int sensorTensao = analogRead(sensorPin)*4.88758;

   // Verifica se a tensao esta acima do limiar para detectar uma gota
   if (sensorTensao > limiar && !gotaDetectada) {
      gotas++;
      gotaDetectada = true;
      Serial.print("Gotas contadas: ");
      Serial.println(gotas);
      Serial.println(sensorTensao);
   } else if (sensorTensao < limiar) {
      gotaDetectada = false;
   }
}
