#ifndef API_PAIRCOMM_H_INCLUDE
#define API_PAIRCOMM_H_INCLUDE

/* ENTERING STREAM TREE */

int WELCOME(int _fd);

int NEW_POP(int _fd);

int REDIRECT(int _fd, char _ipaddr[], char _tport[]);

/* STOP and RESTABLESHIMENT of STREAMING */

int STREAM_FLOWING(int _fd);

int DATA(int _fd, int nbytes, char _data[]);

/* DISCORVERY OF THE ACCESS POINT */
int POP_QUERY(int _fd, uint16_t _queryID);

int POP_REPLY(int _fd, uint16_t _queryID, int avails);

#endif