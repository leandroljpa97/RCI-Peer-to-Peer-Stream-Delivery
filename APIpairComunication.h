#ifndef API_PAIRCOMM_H_INCLUDE
#define API_PAIRCOMM_H_INCLUDE

/* ENTERING STREAM TREE */

int WELCOME(int _fd);

int NEW_POP(int _fd);

int REDIRECT(int _fd, char _ipaddr[], char _tport[]);

/* STOP and RESTABLESHIMENT of STREAMING */

/* DISCORVERY OF THE ACCESS POINT */
int POP_QUERY(int _fd, uint16_t *_queryID);

int POP_REPLY(int _fd, uint16_t *_queryID, int avails);

#endif