// Batracios2.cpp : Este archivo contiene la función "main". La ejecución del programa comienza y termina ahí.
//
#include <Windows.h>
#include "ranas.h"

#include <stdio.h>


//Definiciones
#define ARG_OBLG    1       //Argumentos obligatorios minimos
#define MAX_RANITAS 30      //Ranitas maximas al mismo tiempo
#define MAX_RANAS   4       //Ranas madre máximas
//--Semaforos
#define SEM_MAX_PROCESOS 1       //Semaforo para procesos maximos
#define SEM_MADRE0       2       //Semaforo para saber si la posición de parir está libre
#define SEM_MADRE1       3       //Semaforo para saber si la posición de parir está libre
#define SEM_MADRE2       4       //Semaforo para saber si la posición de parir está libre
#define SEM_MADRE3       5       //Semaforo para saber si la posición de parir está libre
#define SEM_MEMORIA      6       //Semaforo para el acceso a la memoria compartida
#define SEM_TRONCOS0     7       //Para controlar que los troncos 
#define SEM_TRONCOS1     8       //Para controlar que los troncos 
#define SEM_TRONCOS2     9       //Para controlar que los troncos 
#define SEM_TRONCOS3    10       //Para controlar que los troncos 
#define SEM_TRONCOS4    11       //Para controlar que los troncos 
#define SEM_TRONCOS5    12       //Para controlar que los troncos 
#define SEM_TRONCOS6    13       //Para controlar que los troncos 
#define SEM_TOTAL       14       //Semaforos totales

//Globales
typedef struct {
    int r_nacidas;
    int r_salvadas;
    int r_perdidas;
    int terminar;
    DWORD pid[30];
    int dx[30];
    int dy[30];
} MEMORIA;
HANDLE idSemaforo[SEM_TOTAL];
MEMORIA * m;

HINSTANCE hLibreria = NULL;
#if (!defined(LIBRERIA_EXPORTS) && !defined(LIBRERIA_IMPORTS))
TIPO_AVANCERANA            AVANCE_RANA            = NULL;
TIPO_AVANCERANAFIN         AVANCE_RANA_FIN        = NULL;
TIPO_AVANCERANAINI         AVANCE_RANA_INI        = NULL;
TIPO_AVANCETRONCOS         AVANCE_TRONCOS         = NULL;
TIPO_COMPROBARESTADISTICAS COMPROBAR_ESTADISTICAS = NULL;
TIPO_CRIAR                 CRIAR                  = NULL;
TIPO_FINRANAS              FIN_RANAS              = NULL;
TIPO_INICIORANAS           INICIO_RANAS           = NULL;
TIPO_PARTORANAS            PARTO_RANAS            = NULL;
TIPO_PAUSA                 PAUSA                  = NULL;
TIPO_PUEDOSALTAR           PUEDO_SALTAR           = NULL;
TIPO_PRINTMSG              PRINT_MSG              = NULL;
#endif
//Prototipos
BOOL WINAPI CtrlHandler(DWORD CtrlType);
void perror(char* mensaje);
int cargar_libreria( int *fase_ext);





int main(int argc, char const* argv[])
{
    //Argumentos 
    int delta_t = 0; //equivalencia en ms de tiempo real de un tic de reloj
    int t_Criar = 50; //tics de reloj que necesita una rana madre descansar entre dos partos
    int lTroncos[] = { 4,9,6,5,3,5,4 };
    int lAguas[] = { 2,1,3,2,1,2,3 };
    int fase=0, error=0;
   
    int dirs[] = { IZQUIERDA,DERECHA,IZQUIERDA,DERECHA,IZQUIERDA,DERECHA,IZQUIERDA };
    
    char* resto0, * resto1;
    //Manejadora Ctrl+C
    BOOL manejadora=FALSE;

    if (argc < ARG_OBLG + 1)
    {
        fprintf(stderr, "Se necesita al menos %d argumento:\n"
            "\t-Numero de ms por tic\n"
            "\t-Opcional: Tiempo de descanso\n", ARG_OBLG);
        return 1;
    }
    else
    {
        delta_t = strtol(argv[1], &resto0, 10);
        if (strlen(resto0) != 0)
        {
            fprintf(stderr, "Argumento 1 no valido\n");
            return 2;
        }
        else if (delta_t < 0 || delta_t>1000)
        {
            fprintf(stderr, "Numero de ms debe ser entre 0 y 1000\n");
            return 3;
        }

        if (argc > ARG_OBLG + 1) //Argumento opcional
        {
            t_Criar = strtol(argv[2], &resto1, 10);
            if (strlen(resto1) != 0)
            {
                fprintf(stderr, "Argumento 2 no valido\n");
                return 4;
            }
            else if (t_Criar <= 0)
            {
                fprintf(stderr, "Tiempo de descanso debe ser mayor que 0\n");
                return 5;
            }
        }
        printf("ms:%d\n descanso:%d\n", delta_t, t_Criar);
    }

    //Se carga la libreria
    error=cargar_libreria(&fase);
    if (error == 0) {
        exit(EXIT_FAILURE);
    }
     void (*ptr)(int) = &CRIAR; 
    //Se crea la mascara
    manejadora = SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler,TRUE);

    if (!manejadora) {
        perror("Fallo al crear la mascara");
    }

INICIO_RANAS( delta_t,  lTroncos[],  lAguas[],  dirs[],  t_Criar,  CRIAR);






}





int cargar_libreria(int* fase_ext) {
    int error = 0, fase = 0;
    const char* nombreDll;
    const char* nombreFun;
    fase++;
    
    nombreDll = "ranas_v2021.dll";
    hLibreria = LoadLibrary(nombreDll);
    if (hLibreria == NULL) {
    	perror("DLL");
        fprintf(stderr, "Error de carga de DLL[%s].\n", nombreDll);
        error = fase;
    }
#if (!defined(LIBRERIA_EXPORTS) && !defined(LIBRERIA_IMPORTS))
    if (!error) {
        fase++;
        nombreFun = "AvanceRana";
        AVANCE_RANA = (TIPO_AVANCERANA)GetProcAddress(hLibreria, nombreFun);
        if (AVANCE_RANA == NULL) {
        	
			fprintf(stderr, "Error de carga de funcion [%s].\n", nombreFun);
            
            error = fase;
        }
        else {
            fprintf(stderr, "Cargada la funcion [%s] en [%08X].\n", nombreFun, AVANCE_RANA);
        }
    }
    if (!error) {
        fase++;
        nombreFun = "AvanceRanaFin";
        AVANCE_RANA_FIN = (TIPO_AVANCERANAFIN)GetProcAddress(hLibreria, nombreFun);
        if (AVANCE_RANA_FIN == NULL) {
            fprintf(stderr, "Error de carga de funcion [%s].\n", nombreFun);
            error = fase;
        }
        else {
            fprintf(stderr, "Cargada la funcion [%s] en [%08X].\n", nombreFun, AVANCE_RANA_FIN);
        }
    }
    if (!error) {
        fase++;
        nombreFun = "AvanceRanaIni";
        AVANCE_RANA_INI = (TIPO_AVANCERANAINI)GetProcAddress(hLibreria, nombreFun);
        if (AVANCE_RANA_INI == NULL) {
            fprintf(stderr, "Error de carga de funcion [%s].\n", nombreFun);
            error = fase;
        }
        else {
            fprintf(stderr, "Cargada la funcion [%s] en [%08X].\n", nombreFun, AVANCE_RANA_INI);
        }
    }
    if (!error) {
        fase++;
        nombreFun = "AvanceTroncos";
        AVANCE_TRONCOS = (TIPO_AVANCETRONCOS)GetProcAddress(hLibreria, nombreFun);
        if (AVANCE_TRONCOS == NULL) {
            fprintf(stderr, "Error de carga de funcion [%s].\n", nombreFun);
            error = fase;
        }
        else {
            fprintf(stderr, "Cargada la funcion [%s] en [%08X].\n", nombreFun, AVANCE_TRONCOS);
        }
    }
    if (!error) {
        fase++;
        nombreFun = "ComprobarEstadIsticas";
        COMPROBAR_ESTADISTICAS = (TIPO_COMPROBARESTADISTICAS)GetProcAddress(hLibreria, nombreFun);
        if (COMPROBAR_ESTADISTICAS == NULL) {
            fprintf(stderr, "Error de carga de funcion [%s].\n", nombreFun);
            error = fase;
        }
        else {
            fprintf(stderr, "Cargada la funcion [%s] en [%08X].\n", nombreFun, COMPROBAR_ESTADISTICAS);
        }
    }
    if (!error) {
        fase++;
        nombreFun = "f_Criar";
        CRIAR = (TIPO_CRIAR)GetProcAddress(hLibreria, nombreFun);
        if (CRIAR == NULL) {
            fprintf(stderr, "Error de carga de funcion [%s].\n", nombreFun);
            error = fase;
        }
        else {
            fprintf(stderr, "Cargada la funcion [%s] en [%08X].\n", nombreFun, CRIAR);
        }
    }
    if (!error) {
        fase++;
        nombreFun = "FinRanas";
        FIN_RANAS = (TIPO_FINRANAS)GetProcAddress(hLibreria, nombreFun);
        if (FIN_RANAS == NULL) {
            fprintf(stderr, "Error de carga de funcion [%s].\n", nombreFun);
            error = fase;
        }
        else {
            fprintf(stderr, "Cargada la funcion [%s] en [%08X].\n", nombreFun, FIN_RANAS);
        }
    }
    if (!error) {
        fase++;
        nombreFun = "InicioRanas";
        INICIO_RANAS = (TIPO_INICIORANAS)GetProcAddress(hLibreria, nombreFun);
        if (INICIO_RANAS == NULL) {
            fprintf(stderr, "Error de carga de funcion [%s].\n", nombreFun);
            error = fase;
        }
        else {
            fprintf(stderr, "Cargada la funcion [%s] en [%08X].\n", nombreFun, INICIO_RANAS);
        }
    }
    if (!error) {
        fase++;
        nombreFun = "PartoRanas";
        PARTO_RANAS = (TIPO_PARTORANAS)GetProcAddress(hLibreria, nombreFun);
        if (PARTO_RANAS == NULL) {
            fprintf(stderr, "Error de carga de funcion [%s].\n", nombreFun);
            error = fase;
        }
        else {
            fprintf(stderr, "Cargada la funcion [%s] en [%08X].\n", nombreFun, PARTO_RANAS);
        }
    }
    if (!error) {
        fase++;
        nombreFun = "Pausa";
        PAUSA = (TIPO_PAUSA)GetProcAddress(hLibreria, nombreFun);
        if (PAUSA == NULL) {
            fprintf(stderr, "Error de carga de funcion [%s].\n", nombreFun);
            error = fase;
        }
        else {
            fprintf(stderr, "Cargada la funcion [%s] en [%08X].\n", nombreFun, PAUSA);
        }
    }
    if (!error) {
        fase++;
        nombreFun = "PuedoSaltar";
        PUEDO_SALTAR = (TIPO_PUEDOSALTAR)GetProcAddress(hLibreria, nombreFun);
        if (PUEDO_SALTAR == NULL) {
            fprintf(stderr, "Error de carga de funcion [%s].\n", nombreFun);
            error = fase;
        }
        else {
            fprintf(stderr, "Cargada la funcion [%s] en [%08X].\n", nombreFun, PUEDO_SALTAR);
        }
    }
    if (!error) {
        fase++;
        nombreFun = "PrintMsg";
        PRINT_MSG = (TIPO_PRINTMSG)GetProcAddress(hLibreria, nombreFun);
        if (PRINT_MSG == NULL) {
            fprintf(stderr, "Error de carga de funcion [%s].\n", nombreFun);
            error = fase;
        }
        else {
            fprintf(stderr, "Cargada la funcion [%s] en [%08X].\n", nombreFun, PRINT_MSG);
        }
    }
#endif

    //Actualizar la fase y devolver el error.
    (*fase_ext) = fase;
    return error;


}

void perror(char* mensaje)
{
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf,
        0,
        NULL
    );
    fprintf(stderr, "%s:%s\n", mensaje, lpMsgBuf);
    LocalFree(lpMsgBuf);
}//perror


BOOL WINAPI CtrlHandler(DWORD CtrlType)
{
    BOOL res = TRUE;
    switch (CtrlType) {

        // Handle the CTRL-C signal.
    case CTRL_C_EVENT:
        printf("Ctrl-C event\n\n");
        Beep(750, 300);
        res = TRUE;
        break;

    default:
        res = TRUE;
        break;
    }

    return res;
    
}



// Ejecutar programa: Ctrl + F5 o menú Depurar > Iniciar sin depurar
// Depurar programa: F5 o menú Depurar > Iniciar depuración

// Sugerencias para primeros pasos: 1. Use la ventana del Explorador de soluciones para agregar y administrar archivos
//   2. Use la ventana de Team Explorer para conectar con el control de código fuente
//   3. Use la ventana de salida para ver la salida de compilación y otros mensajes
//   4. Use la ventana Lista de errores para ver los errores
//   5. Vaya a Proyecto > Agregar nuevo elemento para crear nuevos archivos de código, o a Proyecto > Agregar elemento existente para agregar archivos de código existentes al proyecto
//   6. En el futuro, para volver a abrir este proyecto, vaya a Archivo > Abrir > Proyecto y seleccione el archivo .sln
