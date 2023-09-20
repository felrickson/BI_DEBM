const int sensorPin = A2;
int gotas = 0;
bool gotaDetectada = false;
float limiar = 20; // Limiar para condicao de contagem

void setup() {
   Serial.begin(9600);
   pinMode(sensorPin, INPUT);
}

void loop() {
   // Le o valor analogico do sensor e converte para tensao em mV
   int sensorTensao = analogRead(sensorPin) *5000/ 1023;
   Serial.println(sensorTensao);

   // Verifica se a tensao esta acima do limiar para detectar uma gota
   if (sensorTensao > limiar && !gotaDetectada) {
      gotas++;
      gotaDetectada = true;
      Serial.print("Gotas contadas: ");
      Serial.println(gotas);
   } else if (sensorTensao < limiar) {
      gotaDetectada = false;
   }

   delay(100);
}
