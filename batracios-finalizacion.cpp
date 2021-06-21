/**
 * 2020/2021
 * Segunda pr醕tica Windows
 * Batracios
 * G09
 * Celia Torres Montero
 * Sa鷏 Otero Garc韆
 * 
 * 21/06/2021
*/
#include <Windows.h>
#include "ranas.h"

#include <stdio.h>


//Definiciones
#define ARG_OBLG    1       //Argumentos obligatorios minimos
#define MAX_RANITAS 30      //Ranitas maximas al mismo tiempo
#define MAX_RANAS   4       //Ranas madre m谩ximas
//--Semaforos

#define SEM_MAX_PROCESOS 1       //Semaforo para procesos maximos
#define SEM_MADRE0       2       //Semaforo para saber si la posici贸n de parir est谩 libre
#define SEM_MADRE1       3       //Semaforo para saber si la posici贸n de parir est谩 libre
#define SEM_MADRE2       4       //Semaforo para saber si la posici贸n de parir est谩 libre
#define SEM_MADRE3       5       //Semaforo para saber si la posici贸n de parir est谩 libre
#define SEM_MEMORIA      6       //Semaforo para el acceso a la memoria compartida
#define SEM_TRONCOS      7       //Para controlar que los troncos no pisen a las ranas
#define SEM_MOVIMIENTO   8       //Para comprobar que las ranas no pisen a los troncos
#define SEM_HILO         9
#define SEM_TOTAL        10       //Semaforos totales

#define WAIT(i)   sem_wait(i)   //Operacion WAIT
#define SIGNAL(i) sem_signal(i) //Operacion SIGNAL
//Globales
typedef struct {
    LONG r_nacidas;
    LONG r_salvadas;
    LONG r_perdidas;
    BOOL terminar;
    HANDLE pid[30];
    int dx[30];
    int dy[30];
    DWORD id[30];
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
TIPO_FINRANAS              FIN_RANAS              = NULL;
TIPO_INICIORANAS           INICIO_RANAS           = NULL;
TIPO_PARTORANAS            PARTO_RANAS            = NULL;
TIPO_PAUSA                 PAUSA                  = NULL;
TIPO_PUEDOSALTAR           PUEDO_SALTAR           = NULL;
TIPO_PRINTMSG              PRINT_MSG              = NULL;
#endif
//Prototipos

void perror(char* mensaje);
int cargar_libreria( int *fase_ext);
void f_Criar(int pos);

DWORD WINAPI rana_hija( LPVOID parametro);
DWORD WINAPI TRONCOS( LPVOID parametro);
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
   	HANDLE hilo;
    int dirs[] = { IZQUIERDA,DERECHA,IZQUIERDA,DERECHA,IZQUIERDA,DERECHA,IZQUIERDA };
    int i,k,q,r;
    char* resto0, * resto1;
    char texto[20]="no puedo saltar";
    

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
   	 
   
    
    //Crear e Iniciar Memoria Compartida.
  	m= (MEMORIA *)malloc( sizeof(MEMORIA));	
	  m->r_nacidas=  0;
	  m->r_perdidas= 0;
	  m->r_salvadas= 0;
	  m->terminar=   TRUE;
	for(k=0;k<30;k++){
	    m->pid[k]=NULL;
	    m->dx[k]=0;
	    m->dy[k]=0;
	    m->id[k]=0;
	    m->sem_madre[k]=0;
	}
	
	//Crear e Iniciar Semaforos
	idSemaforo[SEM_HILO]=CreateSemaphore( NULL, 0, 1  , NULL);	
	idSemaforo[SEM_MAX_PROCESOS]=CreateSemaphore( NULL, 30, 30, NULL);
	idSemaforo[SEM_MADRE0]=CreateSemaphore( NULL, 1, 1, NULL);
	idSemaforo[SEM_MADRE1]=CreateSemaphore( NULL, 1, 1, NULL);
	idSemaforo[SEM_MADRE2]=CreateSemaphore( NULL, 1, 1, NULL);
	idSemaforo[SEM_MADRE3]=CreateSemaphore( NULL, 1, 1, NULL);
	idSemaforo[SEM_MEMORIA]=CreateSemaphore( NULL, 1, 1, NULL);
	idSemaforo[SEM_TRONCOS]=CreateSemaphore( NULL, 1, 1, NULL);
	idSemaforo[SEM_MOVIMIENTO]=CreateSemaphore( NULL, 0, 1, NULL);
	idSemaforo[SEM_HILO]=CreateSemaphore( NULL, 0, 1, NULL);
	
	//Se crean las ranas madre
	INICIO_RANAS( delta_t,  lTroncos  ,lAguas, dirs,  t_Criar,f_Criar);
	
	//Se crea el hilo de troncos
	hilo= CreateThread( NULL, 0, TRONCOS, &i, 0, NULL);
	




	Sleep(30000);
	//Se manda terminar
	m->terminar=FALSE;



	
	

	//se levantan todos los semaforos	
	for ( r = 1; r < SEM_TOTAL; r++)
	{
   		(SIGNAL(r)==0);
	}
	

	//Se espera por el hilo de troncos
	if( WaitForSingleObject(hilo, INFINITE)==WAIT_FAILED){
				 PERROR("ranas peque馻s");
	}
    
    
		

  	//Se espera por los subprocesos
	for (i= 0; i < 30; i++) {
  		if(m->id[i]!=0){
				if( WaitForSingleObject(m->pid[i], INFINITE)==WAIT_FAILED){
					 PERROR("WAITS ranas peque馻s");
				}
				 m->id[i]=0;		 
			}
	}
	
  	FIN_RANAS();
	
	
	COMPROBAR_ESTADISTICAS(m->r_nacidas,m->r_salvadas,m->r_perdidas);
	
  	
	
	for(q=0;q<30;q++){
  		if(CloseHandle(m->pid[q])==0){
  			PERROR("WAITS");
		  }
	}
	
	
	//Liberar memoria
	free(m);
	//Cerrar los semaforos
	
  	return 0;
}


////-------------------------------
void f_Criar (int pos){
int madre= pos+2;
int i;

	
		for(i=0;i<30;i++){
			WAIT(SEM_MAX_PROCESOS);
			if(!(m->terminar)){
			
				SIGNAL(SEM_MAX_PROCESOS);
				break;
			}
	
			WAIT(madre);
			if(!(m->terminar)){
			
				SIGNAL(SEM_MAX_PROCESOS);
				SIGNAL(madre);
				break;
			}
			
				fflush(stdout);
			WAIT(SEM_MEMORIA);
			if(!(m->terminar)){
			
				SIGNAL(SEM_MAX_PROCESOS);
				SIGNAL(madre);
				SIGNAL(SEM_MEMORIA);
				break;
			}
			
				fflush(stdout);
			if(m->id[i]==-2){
				 WaitForSingleObject(m->pid[i], INFINITE);
				 m->id[i]=0;		 
			}
			
			if(m->id[i]==0){
				m->sem_madre[i]=madre;
 				PARTO_RANAS(pos);
 				m->dx[i]=15+(16*pos);
				m->dy[i]=0;
 				
 				//fprintf(stdout,"Se crea la rana\n");
				m->pid[i]= CreateThread( NULL, 0, rana_hija, &i, 0, &m->id[i]);
				m->r_nacidas++;
				
				WAIT(SEM_HILO);
				SIGNAL(SEM_MEMORIA);
			
				
				
				
			}else{
				
				SIGNAL(SEM_MAX_PROCESOS);
				SIGNAL(madre);
				SIGNAL(SEM_MEMORIA);
				
			}
			
		}

}


DWORD WINAPI rana_hija( LPVOID parametro){
	int orden= *((int *)parametro);
	int madre=m->sem_madre[orden];
	int movido=0;
	int direccion;
	
	BOOL NOterminar=TRUE;
	
	SIGNAL(SEM_HILO);
	while(m->terminar /*&& NOterminar*/){
		
		WAIT(SEM_MOVIMIENTO);
		if(!(m->terminar)){
			
			SIGNAL(SEM_MOVIMIENTO);
				break;
		}
		
		WAIT(SEM_MEMORIA);
			
		if(((m->dx[orden])<0)||((m->dx[orden]>79))){
			m->r_perdidas++;
			
			SIGNAL(SEM_MEMORIA);
			SIGNAL(SEM_TRONCOS);
			break;
		}
		if(!(m->terminar)){
			
			SIGNAL(SEM_MOVIMIENTO);
			SIGNAL(SEM_MEMORIA);
				break;
		}
	
		if(PUEDO_SALTAR(m->dx[orden],m->dy[orden],ARRIBA)==TRUE) direccion=ARRIBA;
		else if(PUEDO_SALTAR(m->dx[orden],m->dy[orden],DERECHA)==TRUE) direccion=DERECHA;
		else if(PUEDO_SALTAR(m->dx[orden],m->dy[orden],IZQUIERDA)==TRUE) direccion=IZQUIERDA;
		else{
			
			SIGNAL(SEM_MEMORIA);
			SIGNAL(SEM_TRONCOS);
			PAUSA();
			continue;
		}
		
		AVANCE_RANA_INI(m->dx[orden],m->dy[orden]);
		if(AVANCE_RANA(&m->dx[orden],&m->dy[orden],direccion)==FALSE){
			m->r_perdidas++;
			SIGNAL(SEM_MEMORIA);
			SIGNAL(SEM_TRONCOS);
			break;
		}
		SIGNAL(SEM_MEMORIA);
		PAUSA();
		
		WAIT(SEM_MEMORIA);
		
		if(AVANCE_RANA_FIN(m->dx[orden],m->dy[orden])==FALSE){
			fprintf(stderr, "Error al avanzar");
		}
		movido++;
		if(movido==1){
			SIGNAL(madre);
		}
		if(m->dy[orden]==11){
			m->r_salvadas++;
			SIGNAL(SEM_MEMORIA);
			SIGNAL(SEM_TRONCOS);
			break;
		}
			if(((m->dx[orden])<0)||((m->dx[orden]>79))){
			m->r_perdidas++;
			SIGNAL(SEM_MEMORIA);
			SIGNAL(SEM_TRONCOS);
			break;
		}
		SIGNAL(SEM_MEMORIA);
		SIGNAL(SEM_TRONCOS);
		if(!(m->terminar)){
		
				break;
		}
	}

	SIGNAL(SEM_MAX_PROCESOS);
	m->id[orden]=-2;
	
}
DWORD WINAPI TRONCOS( LPVOID parametro){
	int fila;
	int dirs[] = { IZQUIERDA,DERECHA,IZQUIERDA,DERECHA,IZQUIERDA,DERECHA,IZQUIERDA };
	int i;
	 char texto[20]="no puedo saltar";
	while(m->terminar){
		for(fila=0; fila<7 && m->terminar;fila++){
			WAIT(SEM_TRONCOS);
		
			if(!(m->terminar)){
				
				SIGNAL(SEM_MOVIMIENTO);
				break;
			}
		
			WAIT(SEM_MEMORIA);
				
			
			if(!(m->terminar)){
				
				SIGNAL(SEM_MOVIMIENTO);
				SIGNAL(SEM_MEMORIA);
				break;
			}
			if(AVANCE_TRONCOS(fila)==FALSE){
				break;
			}
			for(i=0; i<30;i++){
				if(m->dy[i]==(10-fila)){
					if(dirs[fila]==DERECHA){
						m->dx[i]++;
					}else{
						m->dx[i]--;
					}
				}
			}
			
			SIGNAL(SEM_MEMORIA);
			SIGNAL(SEM_MOVIMIENTO);
			//	fprintf(stdout,"libero todo\n");
			//	fflush(stdout);
			PAUSA();
			
		}
	}
}
/*void Esperar_sem(int posicion){
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
}*/
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
        
    }
    if (!error) {
        fase++;
        nombreFun = "AvanceRanaFin";
        AVANCE_RANA_FIN = (TIPO_AVANCERANAFIN)GetProcAddress(hLibreria, nombreFun);
        if (AVANCE_RANA_FIN == NULL) {
            fprintf(stderr, "Error de carga de funcion [%s].\n", nombreFun);
            error = fase;
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
       
    }
    if (!error) {
        fase++;
        nombreFun = "AvanceTroncos";
        AVANCE_TRONCOS = (TIPO_AVANCETRONCOS)GetProcAddress(hLibreria, nombreFun);
        if (AVANCE_TRONCOS == NULL) {
            fprintf(stderr, "Error de carga de funcion [%s].\n", nombreFun);
            error = fase;
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
        
    }
    
    if (!error) {
        fase++;
        nombreFun = "FinRanas";
        FIN_RANAS = (TIPO_FINRANAS)GetProcAddress(hLibreria, nombreFun);
        if (FIN_RANAS == NULL) {
            fprintf(stderr, "Error de carga de funcion [%s].\n", nombreFun);
            error = fase;
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
        
    }
    if (!error) {
        fase++;
        nombreFun = "PartoRanas";
        PARTO_RANAS = (TIPO_PARTORANAS)GetProcAddress(hLibreria, nombreFun);
        if (PARTO_RANAS == NULL) {
            fprintf(stderr, "Error de carga de funcion [%s].\n", nombreFun);
            error = fase;
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
        
    }
    if (!error) {
        fase++;
        nombreFun = "PuedoSaltar";
        PUEDO_SALTAR = (TIPO_PUEDOSALTAR)GetProcAddress(hLibreria, nombreFun);
        if (PUEDO_SALTAR == NULL) {
            fprintf(stderr, "Error de carga de funcion [%s].\n", nombreFun);
            error = fase;
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


//Funciones de manejo de semaforos
int sem_wait( int indice)
{	
  return WaitForSingleObject( idSemaforo[indice], INFINITE);
}//sem_wait

int sem_signal( int indice)
{
  return ReleaseSemaphore( idSemaforo[indice], 1, NULL);
}//sem_signal
