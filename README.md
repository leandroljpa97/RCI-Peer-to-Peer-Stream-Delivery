# RCI-Peer-to-Peer-Stream-Delivery

Realização de uma rede Peer-to-Peer à semelhança do conhecido ACE Stream, utilizando protocolos de UDP, TCP para sockets. 
Existe uma fonte que transmite a stream para um cliente, esse cliente transmite para todos os seus filhos, e, posteriormente, todos os seus filhos transmitem para os seus. Todas os clientes, além de serem clientes são também servidores, pois transmitem a stream para quem se liga a si.
- Fez-se toda a gestão de rede, quer quando um novo cliente entra saber a quem se ligar, quer quando um deles saí, como é feita a readesão à árvore
