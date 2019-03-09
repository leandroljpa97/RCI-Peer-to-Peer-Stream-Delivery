#ifndef API_PAIRCOMM_H_INCLUDE
#define API_PAIRCOMM_H_INCLUDE

int WELCOME(int _fd);

/* DISCORVERY OF THE ACCESS POINT */
int POP_QUERY(int _fd, uint16_t *_queryID);

int POP_REPLY(int _fd, uint16_t *_queryID, int avails);

#endif