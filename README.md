# nodemcupima
Realiza medição de medidores padrão de rede PIMA
📄 Monitor de Consumo de Energia - ESP8266
Este projeto utiliza um ESP8266 para ler dados de um medidor de energia via Serial (protocolo específico), processar o consumo em kWh e exibir os valores de custo em tempo real através de uma interface Web hospeda no próprio chip.

🚀 Funcionalidades
Leitura em Tempo Real: Captura o índice de energia ativa diretamente do medidor.

Cálculo de Custos: Converte kWh em Reais (R$) instantaneamente.

Gestão de Consumo:

Consumo Parcial: Útil para medir aparelhos ou períodos curtos (com botão de reset).

Consumo Mensal: Armazena o fechamento do mês para controle de fatura.

Memória Não Volátil (EEPROM): Os dados de consumo e o preço da tarifa permanecem salvos mesmo se o dispositivo for desligado.

Tarifa Configurável: Interface para alterar o preço do kWh via navegador sem precisar reprogramar o código.

IP Fixo: Configurado para facilitar o acesso na rede local (192.168.0.101).

🛠️ Tecnologias Utilizadas
[C++] (Linguagem principal)

Arduino IDE

ESP8266 Core para Arduino

Bibliotecas: ESP8266WiFi.h, EEPROM.h, time.h

📋 Pré-requisitos
Antes de carregar o código, certifique-se de ter:

Placa de desenvolvimento ESP8266 (NodeMCU, Wemos D1 Mini, etc).

Medidor de energia compatível com saída serial de 2400 baud.

Acesso à rede Wi-Fi configurada no código.

🔧 Instalação e Configuração
Conexão:

Conecte a saída TX do medidor ao pino RX do ESP8266.

Nota: Lembre-se que o ESP8266 trabalha com 3.3V.

Configuração do Wi-Fi: No arquivo principal, altere as variáveis de rede:

C++

const char* ssid = "NOME_DA_REDE";
const char* password = "SENHA_DA_REDE";
Upload: Selecione a placa correta na Arduino IDE e clique em Upload.

🖥️ Como Usar
Após ligar o dispositivo, acesse pelo navegador o endereço: http://192.168.0.101.

Na tela inicial, você verá o total acumulado do medidor.

Ajuste de Tarifa: Digite o valor do kWh (ex: 1.22) no campo e clique em "Alterar".

Resets: Utilize os botões para zerar a contagem parcial ou iniciar um novo ciclo mensal.

💾 Estrutura da Memória (EEPROM)
O projeto reserva 64 bytes para armazenamento: | Endereço | Dado | Tipo | | :--- | :--- | :--- | | 0 | Consumo Parcial | unsigned long (4 bytes) | | 4 | Data do Reset Parcial | time_t (4 bytes) | | 8 | Consumo Mensal | unsigned long (4 bytes) | | 12 | Data do Fechamento Mensal | time_t (4 bytes) | | 16 | Preço do kWh | float (4 bytes) |

Desenvolvido por: ejesusprimeiro@gmail.com
Dou os crédito ao Gustavo Henrique Alves Silva que não conheço mas postou o método de comunicação em seu projeto de fim de curso.
