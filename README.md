# RCI-Peer-to-Peer-Stream-Delivery

## DUVIDAS NOVAS

na root so se acrescenta à lista de AP até best pops, ou mais em caso de uma pessoa encher mais do que é necesaio

POR A SEGUIR AO READ <= 0









## Decisoes e casos a ter em atençao:

- Quando se quebra a ligaçao com a stream faço remove() na root, e volto a fazer whoisroot, se nao der fecha-se!!

-As vezes dá mal quando retiro a ROOT LOL NSEI PQ! ahhh e pq nao esta a remover!!!nsei pq!

- Se a root for a baixo sem fazer CTR+C o outro vai fazer whoisroot, e vai dar que ROOTIS xxx, e o xxx ja nao existe, e assim o programa vai a baixo! temos de ter em atenção isto, pq nesse caso o novo gajo faz REMOVE()  e volta a fazer WHOISROOT! 
JA ESTA  A FUNCIONAR ISTO!! PARA TESTAR COMENTEI NA FUNÇAO DE CTR+C O (IF(ROOT) REMOVE!!) E FIZ COM O SERVIDOR DO STOR. O QUE ACONTECE EQ ELE TENTA 3X POP REQ PARA O SERVIDOR DE ACESSO(O ANTIGO! ) e NAO DA . ENT ELE NA FUNÇAO DADLEFT FAZ REMOVE E FAZ WHOISROOT AGAIN! ESTA DICA TA MUIRA BOA. PARA ISSO TENHO DE USAR O ESTADO status= DAD_LOST PQ NA FUNÇAO CONNECT TO TCP TENHO DE USAR ISSO SENAO ELE IA LOGO A BAIXO!

- acho que deviamos mudar todos os strcat para sprintf() como fiz no REMOVE.. NAO?

-Linha 262 do ficheiro APIROOTSERVER.c .. nao sei o que se faz nesse caso!!

-> experimenta ter a root com o ncat, filhos e mandar o servidor ncat a baixo!! ve se ta tudo ok nesse caso sff! (vai a baixo como e obvio mas as msgs que há.. ve sff)

-> no ctr+c temos de dar close a todos os sockets nao?

-> fds tou-me a passar! quero mandar a root a baixo a ver o que acontece. mas smp que mando a root a baixo, o servidor fonte tambem vai a baixo com o ncat! lol (fiz com o do stor)































## DUVIDAS VELHAS

-> LIMPEZA DE BUFFERS TA TUDO MAL ACHO EU!
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

# FEITO POR LEANDRO NO DIA 6/3 A NOITE!
1) Uma vez que o steamId não é propriamente o 1º parametro a ser colocado, tem de se correr do argv 1 ate a frente, e admite-se que o que nao for -i, -b , -h (...) admite-se que é streamId!.. No entanto, em que casos é que há erro no input introduzido? Se o porto do streamId for mal inserido, eu mando return 1, para ele fazer dump!!

2) NAO ESTAMOS A VERIFICAR OS SSCANF'S, O RETORNO!
3)eliminei o fd-rootServer. Ja crio sempre na funçao dump, whoisroot e remove um socket e fecho no fim, e espero pela resposta. BTW no dump há 2 cenas: nao vejo se a resposta é ERRO (se for nao e para verificar ate haver dois \n's. E a maneira que eu fiz para ler em ciclo parece-me fixe. No entanto meti um buffer de tamanho pequeno para testar(5 ou 10) e auqilo deu merda!!

4) muitas coisas comentadas, pois nao sei se é para manter o status..
5) acho que há dupla repetiçao de funçoes dos Fd's -> AddFd e initialize. 
Fiz isto agr a noite cheio de sono
