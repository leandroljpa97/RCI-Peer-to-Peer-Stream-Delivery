#ifndef API_PAIRCOMM_H_INCLUDE
#define API_PAIRCOMM_H_INCLUDE

/* ENTERING STREAM TREE */

int WELCOME(int _fd);

int NEW_POP(int _fd);

int REDIRECT(int _fd, char _ipaddr[], char _tport[]);

/* STOP and RESTABLESHIMENT of STREAMING */

int STREAM_FLOWING(int _fd);

int BROKEN_STREAM(int _fd);

int DATA(int _fd, int nbytes, char _data[]);

/* DISCORVERY OF THE ACCESS POINT */
int POP_QUERYroot(int _fd, uint16_t _queryId, int _bestPops);

int POP_QUERYclients(int _fd, char _queryId[], int _bestPops);

int POP_REPLY(int _fd, char _queryID[], char _ipaddr[], char _tport[], int _avails);

int TREE_QUERY(int _fd, char _ipaddr[], char _tport[]);

int TREE_REPLY(int _fd);

#endif