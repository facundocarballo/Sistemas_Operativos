#include "lista.h"

void crearLista(List *l){
    *l = NULL;
}

void add(List *l, void *d, unsigned t){
    tNodo *nuevo = (tNodo*)malloc(sizeof(tNodo));
    nuevo->dato = malloc(t);
    memcpy(nuevo->dato, d, t);
    nuevo->tam = t;
    nuevo->sig = *l;
    *l = nuevo;
}

void *getFromList(List *l, unsigned i){
    tNodo *aux = *l;
    unsigned j;
    for(j = 0; j < i; j++){
        aux = aux->sig;
    }
    return aux->dato;
}

void *getByContent(List *l, void *d, unsigned t, boolean (*cmp)(void*, void*)){
    tNodo *aux = *l;
    while(aux != NULL){
        if(cmp(aux->dato, d)){
            return aux->dato;
        }
        aux = aux->sig;
    }
    return NULL;
}

boolean cmp(void *a, void *b){
    tHilo *h1 = (tHilo*)a;
    tHilo *h2 = (tHilo*)b;
    printf("Comparando %s con %s\n", h1->path, h2->path);
    return strcmp(h1->path, h2->path) == 0;
}

void *removeByContent(List *l, void *d, unsigned t, boolean (*cmp)(void*, void*)){
    tNodo *aux = *l;
    tNodo *prev = NULL;
    while(aux != NULL){
        if(cmp(aux->dato, d)){
            if(prev == NULL){
                *l = aux->sig;
            }
            else{
                prev->sig = aux->sig;
            }
            void *ret = aux->dato;
            free(aux->dato);
            free(aux);
            return ret;
        }
        prev = aux;
        aux = aux->sig;
    }
    return NULL;
}

void toString(List *l, void (*print)(void*)){
    tNodo *aux = *l;
    while(aux != NULL){
        print(aux->dato);
        aux = aux->sig;
    }
}

void print(void *a){
    tHilo *h = (tHilo*)a;
    printf("%s\t%d\n", h->path, *(int*)(h->hilo));
}

void *removeFromList(List *l, unsigned i){
    tNodo *aux = *l;
    tNodo *aux2;
    unsigned j;
    void *d;
    if(i == 0){
        *l = aux->sig;
        d = aux->dato;
        free(aux);
        return d;
    }
    for(j = 0; j < i-1; j++){
        aux = aux->sig;
    }
    aux2 = aux->sig;
    aux->sig = aux2->sig;
    d = aux2->dato;
    free(aux2);
    return d;
}

void *removeFirstFromList(List *l){
    tNodo *aux = *l;
    void *d;
    *l = aux->sig;
    d = aux->dato;
    free(aux);
    return d;
}

boolean isEmpty(List *l){
    return *l == NULL;
}

boolean isFull(List *l){
    return false;
}

void empty(List *l){
    tNodo *aux = *l;
    tNodo *aux2;
    while(aux != NULL){
        aux2 = aux->sig;
        free(aux->dato);
        free(aux);
        aux = aux2;
    }
    *l = NULL;
}

void destroy(List *l){
    empty(l);
}

// Path: Lista/lista.h