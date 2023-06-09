// #APL N. 3
// #EJERCICIO 3
// #INTEGRANTES:
//       #Carballo, Facundo Nicolas (DNI: 42774931)
//       #Garcia Burgio, Matias Nicolas (DNI: 42649117)
//       #Mottura, Agostina Micaela (DNI: 41898101)
//       #Povoli Olivera, Victor (DNI: 43103780)
//       #Szust, Ángel Elías (DNI: 43098495)

#include "funciones.h"

void pedirComando(char *textoCliente)
{
    printf("Ingrese comando: ");
    fgets(textoCliente, 200, stdin);

    char *pos = strchr(textoCliente, '\n');
    *pos = '\0';
}

int validarSintaxis(char *textoCliente)
{
    if (strcasecmp(textoCliente, "SIN_STOCK") == 0 ||
        strcasecmp(textoCliente, "LIST") == 0 ||
        strcasecmp(textoCliente, "QUIT") == 0)
    {
        return 1;
    }
    else
    {
        char comando[200];
        char parametro[200];

        sscanf(textoCliente, "%s %s", comando, parametro);

        if ((strcasecmp(comando, "STOCK") == 0 ||
             strcasecmp(comando, "REPO") == 0) &&
            esNumero(parametro))
        {
            return 1;
        }
    }

    puts("Comando incorrecto");

    return 0;
}

int esNumero(char *cadena)
{
    if (*cadena == '\0')
        return 0;

    while (*cadena)
    {
        if (!isdigit(*cadena))
            return 0;
        cadena++;
    }

    return 1;
}