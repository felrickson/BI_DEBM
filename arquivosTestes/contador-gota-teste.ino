const int sensorPin = A2;
int gotas = 0;
bool gotaDetectada = false;
float limiar = 1; // Limiar para condiรงรฃo de contagem

void setup() {
   Serial.begin(9600);
   pinMode(sensorPin, INPUT);
}

void loop() {
   // Lรช o valor analรณgico do sensor e converte para tensรฃo
   int sensorTensao = analogRead(sensorPin) *5/ 1023;
   Serial.println(sensorTensao);

   // Verifica se a tensรฃo estรก acima do limiar para detectar uma gota
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
