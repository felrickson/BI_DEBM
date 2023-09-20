const int sensorPin = A2;
float limiar = 1; // Limiar para condição de contagem

void setup() {
   Serial.begin(9600);
   pinMode(sensorPin, INPUT);
}

void loop() {
   // Lê valor analógico do sensor e converte para tensão
   int sensorTensao = analogRead(sensorPin) *5/ 1023;
   Serial.println(sensorTensao);

   // Verifica se a tensão está acima do limiar para detectar uma gota
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
