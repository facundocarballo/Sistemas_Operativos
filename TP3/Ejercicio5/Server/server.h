#ifndef SERVER_H_INCLUDED
#define SERVER_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#include "cat.h"
#include "query.h"
#include "sem.h"
#include "cliente.h"

void server_create();
void server_wait_query(Client*);
void server_read_query(Client*);
int server_up(const Query *p_query, Client*);
int server_down(const Query* p_query, Client*);
int server_query(const Query* p_query, Client*);
char server_check_name_exist(const Query *p_query, FILE *arch);
void server_write(const Query* p_query, FILE* arch);

#endif // SERVER_H_INCLUDED