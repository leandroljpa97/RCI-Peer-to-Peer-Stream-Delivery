# RCI-Peer-to-Peer-Stream-Delivery

## DUVIDAS

-> se meter os argumentos de entrada do iamRoot trocados? tenho de prevenir isso?
-> tenho de verificar se os IPS sao validos e têm os carateres especificados?
Temos de criar estados para o programa. É mais fácil trabalhar assim!

-> quando sái por timeout do select o que fazer? Vai depender do estado em que estou..

-> se um nó de cima sair, o seu 'filho' ao detetar tem de dizer aos seus filhos ' houve perda de uma lligação..aguardar'
 -> user tem a opçao de meter WHOISROOT na command line? 

-> NAO FECHEI O SOCKET UDP.. QUANDO E QUE SE DEVE FECHAR.?? XD

-> Nao sei se preciso da variavel fd_udp!!! verificar

-> tamanho do buffer quando recebo streams nao pode ser 128, podem haver 100000000000000 streams! mudar isto

->verificar os tamanhos do streamId!! verificar os tamanhos das streams e assim
-> temos de limitar o nº de pontos de acesso ao nr de ips validos  . bestpops!
-> tem de se confirmar que os ips sao validos.. tipo meter um ip assim 'olaaaaaa' sai-se logo?

-> USER INTERFACE INSENSÍVEL A MAIUSCULAS