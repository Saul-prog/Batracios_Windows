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
#define SEM_HILO         0
#define SEM_HIJAS        1
#define SEM_MAX_PROCESOS 2       //Semaforo para procesos maximos
#define SEM_MADRE0       3       //Semaforo para saber si la posición de parir está libre
#define SEM_MADRE1       4       //Semaforo para saber si la posición de parir está libre
#define SEM_MADRE2       5       //Semaforo para saber si la posición de parir está libre
#define SEM_MADRE3       6       //Semaforo para saber si la posición de parir está libre
#define SEM_MEMORIA      7       //Semaforo para el acceso a la memoria compartida
#define SEM_TRONCOS0     8       //Para controlar que los troncos 
#define SEM_TRONCOS1     9       //Para controlar que los troncos 
#define SEM_TRONCOS2    10       //Para controlar que los troncos 
#define SEM_TRONCOS3    11      //Para controlar que los troncos 
#define SEM_TRONCOS4    12       //Para controlar que los troncos 
#define SEM_TRONCOS5    13       //Para controlar que los troncos 
#define SEM_TRONCOS6    14       //Para controlar que los troncos 

#define SEM_TOTAL       15       //Semaforos totales
#define WAIT(i)   sem_wait( i)   //Operacion WAIT
#define SIGNAL(i) sem_signal( i) //Operacion SIGNAL
//Globales
typedef struct {
    int r_nacidas;
    int r_salvadas;
    int r_perdidas;
    BOOL terminar;
    DWORD pid[30];
    int dx[30];
    int dy[30];
    int id[30];
    int sem_madre[30];
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
//TIPO_CRIAR                 CRIAR                  = NULL;
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
void f_criar(int pos);
DWORD WINAPI rana_madre( LPVOID parametro);
DWORD WINAPI rana_hija( LPVOID parametro);
int sem_wait( int indice);
int sem_signal( int indice);
void Esperar_sem(int posicion);
void Liberar_sem(int posicion);

int main(int argc, char const* argv[])
{
    //Argumentos 
    int delta_t = 0; //equivalencia en ms de tiempo real de un tic de reloj
    int t_Criar = 50; //tics de reloj que necesita una rana madre descansar entre dos partos
    int lTroncos[] = { 4,9,6,5,3,5,4 };
    int lAguas[] = { 2,1,3,2,1,2,3 };
    int fase=0, error=0;
   	HANDLE hilo[MAX_RANAS];
    int dirs[] = { IZQUIERDA,DERECHA,IZQUIERDA,DERECHA,IZQUIERDA,DERECHA,IZQUIERDA };
    int i,j,k;
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
    if (error != 0) {
    	fprintf(stderr, "Error al cargar la libreria\n");
        exit(EXIT_FAILURE);
    }
   	 
    //Se crea la mascara
    manejadora = SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler,TRUE);

    if (!manejadora) {
       PERROR("Fallo al crear la mascara");
    }
    
    //Crear e Iniciar Memoria Compartida.
  	m= (MEMORIA *)malloc( sizeof(MEMORIA));	
	  m->r_nacidas=  0;
	  m->r_perdidas= 0;
	  m->r_salvadas= 0;
	  m->terminar=   True;
	for(k=0;k<30;k++){
	    m->pid[k]=NULL;
	    m->dx[k]=0;
	    m->dy[k]=0;
	    m->id[k]=0;
	    m->sem_madre[k]=0;
	}
	
	//Crear e Iniciar Semaforos
	idSemaforo[SEM_HILO]=CreateSemaphore( NULL, 0, 1  , NULL);
	idSemaforo[SEM_HIJAS]=CreateSemaphore( NULL, 0, 1  , NULL);
	idSemaforo[SEM_MAX_PROCESOS]=CreateSemaphore( NULL, 30, 30, NULL);
	for(i=3;i<SEM_TOTAL;i++){
		idSemaforo[i]=CreateSemaphore( NULL, 1, 1, NULL);		
	}
	//INICIO_RANAS( delta_t,  lTroncos[],  lAguas[], dirs[],  t_Criar,  CRIAR);


	//Crear productoras
	for(j=0;j<4;j++){
		hilo[j]= CreateThread( NULL, 0, rana_madre, &j, 0, NULL);
		WAIT(SEM_HILO);
	}




	Sleep(30000);
	m->terminar=FALSE;
	SIGNAL(0);
	for(o=0;o<PROCESOS_MAX;o++){
		SIGNAL(1);
	}
	for ( n = 2; n < SEM_TOTAL; l++)
	{
	    SIGNAL(n);    
	}
	for (l= 0; l < 4; l++) {
    	WaitForSingleObject( hilo[l], INFINITE);
  	}
		
	//Liberar memoria
	free(m);
	//Cerrar los semaforos
	for (i= 0; i < semTOTAL; i++) {
    	CloseHandle( idSemaforo[i]);
  	}
}



void f_criar (int pos){
	PARTO_RANAS(pos);
	m.dx[pos]=15+16*pos;
	m.dx[pos]=0;
	

}
DWORD WINAPI rana_madre( LPVOID parametro){
int orden= *((int *)parametro);
int madre= orden+3;
SINGAL(SEM_HILO)
	while(m->terminar){
		for(i=0;i<30;i++){
			WAIT(SEM_MAX_PROCESOS);
			if(!(m.terminar)){
				break;
			}
			WAIT(madre);
			if(!(m.terminar)){
				break;
			}
			WAIT(SEM_MEMORIA);
			if(!(m.terminar)){
				break;
			}
			if(m.id[i]=-2){
				 WaitForSingleObject(m.hilo[i], INFINITE);
				 m.id[i]=0;		 
			}
			
			if(m.id[i]==0){
				m->sem_madre[i]=madre;
 				f_criar(orden);
 				m.nacidas++;
				m.pid[i]= CreateThread( NULL, 0, rana_hija, &i, 0, &m.id[i]);
				
				SIGNAL(SEM_MEMORIA);
			}else{
				SIGNAL(SEM_MAX_PROCESOS);
				SIGNAL(madre);
				SIGNAL(SEM_MEMORIA);
			}
			
		}
		//Si es la primera rana espera por todas las hijas
		if(orden==0){
			for(i=0;i<30;i++){
				if(m.id[i]!=0){
					WaitForSingleObject(m.hilo[i], INFINITE);
				}
			}
		}
		
	}
	return 0;
}

DWORD WINAPI rana_hija( LPVOID parametro){
	int orden= *((int *)parametro);
	int madre=orden+3;
	int movido=0;
	int direccion;
	while(m.terminar){
		
		Esperar_sem(m.dy[orden]);
		if(!(m.terminar)){
				break;
		}
		WAIT(SEM_MEMORIA);
		if(((m.dx[orden])<0)||((m.dx[orden]>79)){
			m.perdidas++;
			SIGNAL(SEM_MEMORIA);
			Liberar_sem(posicion+1);
		}
		if(!(m.terminar)){
				break;
		}
		if(PUEDO_SALTAR(m.dx[orden],m.dy[orden],ARRIBA)==TRUE) direccion=ARRIBA;
		else if(PUEDO_SALTAR(m.dx[orden],m.dy[orden],DERECHA)==TRUE) direccion=DERECHA;
		else if(PUEDO_SALTAR(m.dx[orden],m.dy[orden],IZQUIERDA)==TRUE) direccion=IZQUIERDA;
		else{
			SIGNAL(SEM_MEMORIA);
			Liberar_sem(posicion);
			PAUSA();
			continue;
		}
		AVANCE_RANA_INI(m.dx[orden],m.dy[orden]);
		if(AVANCE_RANA(&m.dx[orden],&m.dy[orden],direccion)==FALSE){
			m.perdidas++;
			SIGNAL(SEM_MEMORIA);
			Liberar_sem(orden);
			break;
		}
		SIGNAL(SEM_MEMORIA);
		PAUSA();
		WAIT(SEM_MEMORIA);
		if(AVANCE_RANA_FIN(m.dx[orden],m.dy[orden])){
			PRINT_MSG("Error al finalizar el avance";)
		}
		movido++;
		if(movido==1){
			SIGNAL(madre);
		}
		if(m.dy[orden]==11){
			m.salvadas++;
			SIGNAL(SEM_MEMORIA);
			Liberar_sem(orden);
		}
			if(((m.dx[orden])<0)||((m.dx[orden]>79)){
			m.perdidas++;
			SIGNAL(SEM_MEMORIA);
			Liberar_sem(posicion);
			break;
		}
		SIGNAL(SEM_MEMORIA);
		Liberar_sem(orden);
	}
	
	SIGNAL(SEM_MAX_PROCESOS);
	m.pid[orden]=-2;
	
}

void Esperar_sem(int posicion){
	if(posicion==3){
		SIGNAL(SEM_TRONCOS0);
    }else if( posicion > 3){
    	SIGNAL(posicion+5);
    	SIGNAL(posicion+6);
	} 
}
void Liberar_sem(int posicion){
	if(posicion==3){
		WAIT(SEM_TRONCOS0);
    }else if( posicion > 3){
    	WAIT(posicion+4);
    	WAIT(posicion+5);
	} 
}
int cargar_libreria(int* fase_ext) {
    int error = 0, fase = 0;
    const char* nombreDll;
    const char* nombreFun;
    fase++;
    
    nombreDll = "ranas_v2021.dll";
    hLibreria = LoadLibrary(nombreDll);
    if (hLibreria == NULL) {
    	 fprintf(stderr, "Error de carga de DLL[%d].\n",GetLastError());
    	PERROR("DLL");
       
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

//Funciones de manejo de semaforos
int sem_wait( int indice)
{	
  return WaitForSingleObject( idSemaforo[indice], INFINITE);
}//sem_wait

int sem_signal( int indice)
{
  return ReleaseSemaphore( idSemaforo[indice], 1, NULL);
}//sem_signal
