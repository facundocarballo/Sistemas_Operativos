// #APL N. 3
// #EJERCICIO 4
// #INTEGRANTES:
//       #Carballo, Facundo Nicolas (DNI: 42774931)
//       #Garcia Burgio, Matias Nicolas (DNI: 42649117)
//       #Mottura, Agostina Micaela (DNI: 41898101)
//       #Povoli Olivera, Victor (DNI: 43103780)
//       #Szust, Ángel Elías (DNI: 43098495)

#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include <fcntl.h>
#include "lista.h"
#include <memory.h>
#include <signal.h>
#include "../global.h"

void crearServidor();
void cerrarServidor(char* pedido, Respuesta* respuesta, int shmid, sem_t* semComando, sem_t* semRespuesta, Lista* listaGatos);
void signalHandler(int signal);
int crearMemoriaCompartida();
void ayuda();
void validarParametros(int cantidadParametros, char* parametros[]);

void alta(Lista* listaGatos, Pedido* pedido, Respuesta* respuesta);
void baja(Lista* listaGatos, Pedido* pedido, Respuesta* respuesta);
void consulta(Lista* listaGatos, Pedido* pedido, Respuesta* respuesta);
void accionInvalida(Pedido* pedido, Respuesta* respuesta);
void atraparSeniales();
void toUpper(char* s);
int validarAlta(Pedido* pedido, Respuesta* respuesta);
int validarBaja(Pedido* pedido, Respuesta* respuesta);
int validarNombre(Pedido* pedido, char* mensaje, int valido);
int validarRaza(Pedido* pedido, char* mensaje, int valido);
int validarSexo(Pedido* pedido, char* mensaje, int valido);
int validarCondicion(Pedido* pedido, char* mensaje, int valido);
char* parsearCampo(char* texto, char* campo, char* nombreCampo, int maxCaracteres, char* error);
int parsearPedido(char* texto, Pedido* pedido, char* error);

int serverActivo = 1;
sem_t* semComando;
sem_t* semRespuesta;

int main(int argc, char* argv[])
{
    validarParametros(argc, argv);

    semComando = sem_open("/comando", O_CREAT | O_EXCL, 0666, 0);
    semRespuesta = sem_open("/respuesta", O_CREAT | O_EXCL, 0666, 0);
    if (semComando == SEM_FAILED || semRespuesta == SEM_FAILED) {
        sem_close(semComando);
        sem_close(semRespuesta);
        printf(YELLOW"El servidor ya se encuentra activo\n"RESET);
        exit(-1);
    }

    crearServidor();
    atraparSeniales();
    
    Lista listaGatos;
    crearLista(&listaGatos);

    int shmid = crearMemoriaCompartida();
    char* texto = (char*)shmat(shmid,NULL,0);
    Respuesta* respuesta = (Respuesta*)shmat(shmid,NULL,0);
    Pedido pedido;
   
    while (serverActivo)
    {
        sem_wait(semComando);
        char mensajeError[200];
        strcpy(mensajeError, "");
        int parseoValido = parsearPedido(texto, &pedido, mensajeError);
        if(parseoValido) {
            if(strcmp("ALTA", pedido.accion) == 0)
                alta(&listaGatos, &pedido, respuesta);

            else if(strcmp("BAJA", pedido.accion) == 0)
                baja(&listaGatos, &pedido, respuesta);

            else if(strcmp("CONSULTA", pedido.accion) == 0)
                consulta(&listaGatos, &pedido, respuesta);
            
            else
                accionInvalida(&pedido, respuesta);

        } else {
            respuesta->status = 400;
            strcpy(respuesta->contenido,  YELLOW"BAD REQUEST:\n");
            strcat(respuesta->contenido, mensajeError);
            strcat(respuesta->contenido, ""RESET);
        }
        sem_post(semRespuesta);
    }

    cerrarServidor(texto, respuesta, shmid, semComando, semRespuesta,  &listaGatos);

    return 0;
}

void crearServidor() {
    pid_t process_id = fork();

    if(process_id < 0)
        exit(1);

    if (process_id > 0) {
        printf("process_id of child process %d \n", process_id);
        exit(0);
    }
    FILE* archivoGatos = fopen("gatos", "a");
    fclose(archivoGatos);
}

void cerrarServidor(char* pedido, Respuesta* respuesta, int shmid, sem_t* semComando, sem_t* semRespuesta, Lista* listaGatos) {
    shmdt(&pedido);
    shmdt(&respuesta);
    shmctl(shmid, IPC_RMID, NULL);

    sem_close(semComando);
    sem_close(semRespuesta);

    sem_unlink("/comando");
    sem_unlink("/respuesta");
    vaciarLista(listaGatos);
}

void signalHandler(int signal)
{
    if (signal == SIGUSR1) {
        serverActivo = 0;
        sem_post(semComando);
    }
}

int crearMemoriaCompartida() {
    char* clave = "../refugio";
    key_t key = ftok(clave, 4);
    int shmid = shmget(key, sizeof(int), IPC_CREAT | 0666);

    if (shmid == -1) {
        printf("Hubo un error al intentar abrir el área de memoria compartida.");
        exit(1);
    }

    return shmid;
}

void alta(Lista* listaGatos, Pedido* pedido, Respuesta* respuesta) {

    if(validarAlta(pedido, respuesta)) {
        Gato gato;
        strcpy(gato.nombre, pedido->nombre);
        strcpy(gato.raza, pedido->raza);
        gato.sexo = *(pedido->sexo);
        strcpy(gato.condicion, pedido->condicion);

        int resultado = insertarEnListaOrdenada(listaGatos, &gato, sizeof(Gato));

        if(resultado == TODO_OK) {
            respuesta->status = 200;
            strcpy(respuesta->contenido, gato.nombre);
            strcat(respuesta->contenido, " ingresado correctamente");
        } else if (resultado == DUPLICADO){
            respuesta->status = 409;
            strcpy(respuesta->contenido, "CONFLICTO: ");
            strcat(respuesta->contenido, gato.nombre);
            strcat(respuesta->contenido, " ya existe");
        }
    }
}

void baja(Lista* listaGatos, Pedido* pedido, Respuesta* respuesta) {
    
    if(validarBaja(pedido, respuesta)) {

        Gato gato;
        strcpy(gato.nombre, pedido->nombre);

        int resultado = eliminarDeLista(listaGatos, &gato, sizeof(Gato));

        if(resultado == ELIMINADO) {
            respuesta->status = 204;
            strcpy(respuesta->contenido, gato.nombre);
            strcat(respuesta->contenido, " eliminado correctamente");
        } 
        else if (resultado == NOT_FOUND) {
            respuesta->status = 404;
            strcpy(respuesta->contenido, "NOT FOUND: ");
            strcat(respuesta->contenido, gato.nombre);
            strcat(respuesta->contenido, " no existe");
        }
    }
}

void consulta(Lista* listaGatos, Pedido* pedido, Respuesta* respuesta) {
    Gato gato;
    strcpy(gato.nombre, pedido->nombre);

    if(strcmp("", pedido->nombre) == 0) {
        respuesta->status = 200;
        listarTodos(listaGatos, respuesta->contenido);
    } else {
        int encontrado = buscarEnLista(listaGatos, &gato, sizeof(Gato));
        if(encontrado) {
            respuesta->status = 200;
            char mensaje[200];
            char aux[70];

            sprintf (mensaje, GREEN"|%17s|%17s|%5s|%10s|\n"RESET, 
            "Nombre", "Raza", "Sexo", "Condicion");
            sprintf (aux, "|%17s|%17s|%5c|%10s|\n", 
            gato.nombre, gato.raza, gato.sexo, gato.condicion);

            strcat(mensaje, aux);
            strcpy(respuesta->contenido, mensaje);
        } else {
            respuesta->status = 404;
            strcpy(respuesta->contenido, gato.nombre);
            strcat(respuesta->contenido, " no encontrado");
        }
    }
}

void accionInvalida(Pedido* pedido, Respuesta* respuesta) {
    char accion[9];
    strcpy(accion, pedido->accion);
    respuesta->status = 400;
    strcpy(respuesta->contenido, "BAD REQUEST: accion no valida ");
    strcat(respuesta->contenido, accion);
}

void atraparSeniales() {
    if (signal(SIGUSR1, signalHandler) == SIG_ERR)
        printf("No se pudo capturar la senal SIGUSR1\n");
    if (signal(SIGINT, signalHandler) == SIG_ERR)
        printf("No se pudo capturar la senal SIGUSR1\n");
}

void toUpper(char* s) {
    while(*s != '\0') {
        if(*s >= 'a' && *s <= 'z') {
            *s = *s -32;
        }
      s++;
    }
}

int validarAlta(Pedido* pedido, Respuesta* respuesta) {
    int valido = 1;
    char mensaje[200] = "";

    valido = validarNombre(pedido, mensaje, valido);
    valido = validarRaza(pedido, mensaje, valido);
    valido = validarSexo(pedido, mensaje, valido);
    valido = validarCondicion(pedido, mensaje, valido);

    if(!valido) {
        respuesta->status = 400;
        strcpy(respuesta->contenido, "BAD REQUEST:");
        strcat(respuesta->contenido, mensaje);
    }

    return valido;
}

int validarBaja(Pedido* pedido, Respuesta* respuesta) {
    int valido = 1;
    char mensaje[200] = "";

    valido = validarNombre(pedido, mensaje, valido);

    if(!valido) {
        respuesta->status = 400;
        strcpy(respuesta->contenido, "BAD REQUEST:");
        strcat(respuesta->contenido, mensaje);
    }

    return valido;
}

int validarNombre(Pedido* pedido, char* mensaje, int valido) {
    if(strcmp(pedido->nombre, "") == 0) {
        strcat(mensaje, "\n● El campo nombre es requerido");    
        valido = 0;
    }
    return valido;
}

int validarRaza(Pedido* pedido, char* mensaje, int valido) {
    if(strcmp(pedido->raza, "") == 0) {
        strcat(mensaje, "\n● El campo raza es requerido");    
        valido = 0;
    }
    return valido;
}

int validarSexo(Pedido* pedido, char* mensaje, int valido) {
    toUpper(pedido->sexo);
    if(strcmp(pedido->sexo, "M") != 0 && strcmp(pedido->sexo, "F") != 0) {
        if(strcmp(pedido->condicion, "") == 0) {
            strcat(mensaje, "\n● El campo sexo es requerido");    
        } else {
            strcat(mensaje, "\n● Sexo no valido: ");
            strcat(mensaje, pedido->sexo);
        }
        valido = 0;
    }
    return valido;
}

int validarCondicion(Pedido* pedido, char* mensaje, int valido) {
    toUpper(pedido->condicion);
    if(strcmp(pedido->condicion, "CA") != 0 && strcmp(pedido->condicion, "SC") != 0) {
        if(strcmp(pedido->condicion, "") == 0) {
            strcat(mensaje, "\n● El campo condicion es requerido");    
        } else {
            strcat(mensaje, "\n● Condicion no valida: ");
            strcat(mensaje, pedido->condicion);
        }
        valido = 0;
    }
    return valido;
}

char* parsearCampo(char* texto, char* campo, char* nombreCampo, int maxCaracteres, char* error) {
    char caracteres[200];
    int i = 0;

    while(*texto != ' ' && *texto != '\0' && *texto != '\n') {
        caracteres[i] = *texto;
        i++;
        texto++;
    }

    if(*texto != '\0')
        texto++;

    caracteres[i] = '\0';

    if(i<=maxCaracteres) {
        strcpy(campo, caracteres);
    }
    else {
        char aux[200];
        if(maxCaracteres > 1) {
            sprintf (aux, "● El campo %s no puede contener mas de %d caracteres\n", nombreCampo, maxCaracteres); 
            strcat(error, aux);
        } else {
            sprintf(aux, "● El campo %s no pueden contener mas de %d caracter\n", nombreCampo, maxCaracteres);
            strcat(error, aux);
        }
        strcpy(campo, "?");
    }

    return texto;
}

int parsearPedido(char* texto, Pedido* pedido, char* error) {
    texto = parsearCampo(texto, pedido->accion, "accion", 25, error);
    texto = parsearCampo(texto, pedido->nombre, "nombre", 25, error);
    texto = parsearCampo(texto, pedido->raza, "raza", 25, error);
    texto = parsearCampo(texto, pedido->sexo, "sexo", 1, error);
    texto = parsearCampo(texto, pedido->condicion, "condicion", 2, error);

    toUpper(pedido->accion);
    toUpper(pedido->nombre);
    toUpper(pedido->raza);

    if(strcmp(pedido->accion, "?") == 0 || 
        strcmp(pedido->nombre, "?") == 0 || 
        strcmp(pedido->raza, "?") == 0 || 
        strcmp(pedido->sexo, "?") == 0 ||
        strcmp(pedido->condicion, "?") == 0)
        return 0;
    
    return 1;
}

void ayuda() {
    puts(YELLOW"\nDESCRIPCION"RESET);
    puts("Con este script crearas un servidor que estara escuchando peticiones de un cliente a traves de memoria compartida.");
    puts("Solo acepta peticiones de un cliente a la vez.");
    puts(YELLOW"\nEJECUCION"RESET);
    puts("./servidor");
    puts(YELLOW"\nDETENCION"RESET);
    puts("Para detener el servidor debera enviarle una senial 'SIGUSR1 junto al process id que sera informado a la hora de ejecutar':");
    puts("kill -SIGUSR1 <process-id>");
    puts(YELLOW"\nUSO"RESET);
    puts("El cliente tiene 3 tipos de peticiones para hacerle a este servidor:");
    puts(CYAN"- ALTA:"RESET);
    printf("\t● ALTA ");
    printf(GREEN"[nombre] [raza] [sexo] [condicion]\n"RESET);
    puts(CYAN"- BAJA:"RESET);
    printf("\t● BAJA ");
    printf(GREEN"[nombre]\n"RESET);
    puts(CYAN"- CONSULTA:"RESET);
    puts("\t● CONSULTA");
    printf("\t● CONSULTA ");
    printf(GREEN"[nombre]\n"RESET);
    puts(CYAN"- SALIR\n"RESET);
    puts(YELLOW"PARAMETROS"RESET);
    printf(GREEN"[nombre]\t"RESET);
    printf("Nombre del gato (MAXIMO 25 CARACTERES)\n");
    printf(GREEN"[raza]\t\t"RESET);
    printf("Raza del gato (MAXIMO 25 CARACTERES)\n");
    printf(GREEN"[sexo]\t\t"RESET);
    printf("Sexo del gato (M o F) (SE DEBE COLOCAR UN SOLO CARACTER)\n");
    printf(GREEN"[condicion]\t"RESET);
    printf("Castrado (CA) o Sin Castrar (SC) (SE DEBEN COLOCAR SOLO 2 CARACTERES)\n\n");
}

void validarParametros(int cantidadParametros, char* parametros[]) {
    if (cantidadParametros > 2) {
        printf("%s no soporta %d parametros\n", parametros[0], cantidadParametros-1);
        exit(0);
    }

    if(cantidadParametros == 2) {
        if (strcmp(parametros[1],"-help") == 0 || strcmp(parametros[1], "-h") == 0 || strcmp(parametros[1], "--help") == 0) {
            ayuda();
            exit(1);
        } else {
            printf(YELLOW"Argumento invalido: %s\n"RESET, parametros[1]);
            exit(0); 
        }
    }
}