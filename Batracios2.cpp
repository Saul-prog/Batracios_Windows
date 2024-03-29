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
#define SEM_TRONCOS      7       //Para controlar que los troncos no pisen a las ranas
#define SEM_MOVIMIENTO   8       //Para comprobar que las ranas no pisen a los troncos
#define SEM_HILO         9
#define SEM_TRONCO0      10
#define SEM_TRONCO1      11
#define SEM_TRONCO2      12
#define SEM_TRONCO3      13
#define SEM_TRONCO4      14
#define SEM_TRONCO5      15
#define SEM_TRONCO6      16
#define SEM_TOTAL        17       //Semaforos totales

#define WAIT(i)   sem_wait( i)   //Operacion WAIT
#define SIGNAL(i) sem_signal( i) //Operacion SIGNAL
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
//TIPO_CRIAR                 CRIAR                  = NULL;
TIPO_FINRANAS              FIN_RANAS              = NULL;
TIPO_INICIORANAS           INICIO_RANAS           = NULL;
TIPO_PARTORANAS            PARTO_RANAS            = NULL;
TIPO_PAUSA                 PAUSA                  = NULL;
TIPO_PUEDOSALTAR           PUEDO_SALTAR           = NULL;
TIPO_PRINTMSG              PRINT_MSG              = NULL;
#endif
//Prototipos
//BOOL WINAPI CtrlHandler(DWORD CtrlType);
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
    int i,j,k,n,l,o,p,q,r,s;
    char* resto0, * resto1;
    //Manejadora Ctrl+C
    //BOOL manejadora=FALSE;

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
    //manejadora = SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler,TRUE);

   /* if (!manejadora) {
       PERROR("Fallo al crear la mascara");
    }*/
    
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
	
	idSemaforo[SEM_MAX_PROCESOS]=CreateSemaphore( NULL, 1, 1, NULL);
	idSemaforo[SEM_MADRE0]=CreateSemaphore( NULL, 1, 1, NULL);
	idSemaforo[SEM_MADRE1]=CreateSemaphore( NULL, 1, 1, NULL);
	idSemaforo[SEM_MADRE2]=CreateSemaphore( NULL, 1, 1, NULL);
	idSemaforo[SEM_MADRE3]=CreateSemaphore( NULL, 1, 1, NULL);
	idSemaforo[SEM_MEMORIA]=CreateSemaphore( NULL, 1, 1, NULL);
	idSemaforo[SEM_TRONCOS]=CreateSemaphore( NULL, 1, 1, NULL);
	idSemaforo[SEM_MOVIMIENTO]=CreateSemaphore( NULL, 0, 1, NULL);
	idSemaforo[SEM_HILO]=CreateSemaphore( NULL, 0, 1, NULL);
	idSemaforo[SEM_TRONCO0]=CreateSemaphore( NULL, 1, 1, NULL);
	idSemaforo[SEM_TRONCO1]=CreateSemaphore( NULL, 1, 1, NULL);
	idSemaforo[SEM_TRONCO2]=CreateSemaphore( NULL, 1, 1, NULL);
	idSemaforo[SEM_TRONCO3]=CreateSemaphore( NULL, 1, 1, NULL);
	idSemaforo[SEM_TRONCO4]=CreateSemaphore( NULL, 1, 1, NULL);
	idSemaforo[SEM_TRONCO5]=CreateSemaphore( NULL, 1, 1, NULL);
	idSemaforo[SEM_TRONCO6]=CreateSemaphore( NULL, 1, 1, NULL);
	INICIO_RANAS( delta_t,  lTroncos  ,lAguas, dirs,  t_Criar,f_Criar);
	
	hilo= CreateThread( NULL, 0, TRONCOS, &i, 0, NULL);
	




	Sleep(300000);
	m->terminar=FALSE;



	
	
	/*for (i= 1; i < SEM_TOTAL; i++) {
    	if(CloseHandle( idSemaforo[i])==0){
    		fprintf(stdout,"Cerrar semaforo %d",i);
    		PERROR("Cerrar sem")
		}
  	}*/
	
	for ( r = 1; r < SEM_TOTAL; r++)
	{
   
				fflush(stdout);
	        if(SIGNAL(r)==0){
	        	PERROR("SIGNAL");
			}
	}	Sleep(1000);
	
    
    	if( WaitForSingleObject(hilo, INFINITE)==WAIT_FAILED){
					 PERROR("WAITS");
		}

	
  	
	for (i= 0; i < 30; i++) {
  	if(m->id[i]!=0){
				if( WaitForSingleObject(m->pid[i], INFINITE)==WAIT_FAILED){
					 PERROR("WAITS");
				}
				 m->id[i]=0;		 
			}
	}
	fprintf(stderr,"Espera");
	fflush(stderr);
  	if(FIN_RANAS()==FALSE){
		fprintf(stderr,"Error al finalizar");
		fflush(stderr);
	}
	fprintf(stderr,"Espera2");
	fflush(stderr);
	if(COMPROBAR_ESTADISTICAS(m->r_nacidas,m->r_salvadas,m->r_perdidas)==FALSE	){
		fprintf(stderr,"Error al comprobar estadisticas");
		fflush(stderr);
	}else{
		return 0;
	}
	
  	
	
	/*for(q=0;q<30;q++){
  		if(CloseHandle(m->pid[q])==0){
  			PERROR("WAITS");
		  }
	}*/

	fprintf(stdout,"acabado\n");
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
				exit(1);
			}
	
			WAIT(madre);
			if(!(m->terminar)){
				SIGNAL(SEM_MAX_PROCESOS);
				SIGNAL(madre);
				exit(1);
			}
			
				fflush(stdout);
			WAIT(SEM_MEMORIA);
			if(!(m->terminar)){
				SIGNAL(SEM_MAX_PROCESOS);
				SIGNAL(madre);
				SIGNAL(SEM_MEMORIA);
				exit(1);
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
				SIGNAL(SEM_MEMORIA);
				WAIT(SEM_HILO);
				
			
				
				
				
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
	/*fprintf(stdout,"Madre: %d,%d\n",m->sem_madre[orden],madre);
	fflush(stdout);
	fprintf(stdout,"Posicion: %d,%d\n",m->dx[orden],m->dy[orden]);
	fflush(stdout);*/
	int movido=0;
	int direccion;
	int tronco_espera=0;
	BOOL NOterminar=TRUE;
	m->r_nacidas++;
	SIGNAL(SEM_HILO);
	char texto[20]="no puedo saltar";
	while(m->terminar /*&& NOterminar*/){
		
		sprintf(texto, "espero sem tronco %d soy rana",tronco_espera );
			PRINT_MSG(texto);	
		Esperar_sem(tronco_espera);
		if(!(m->terminar)){
			Liberar_sem(tronco_espera);
			
				break;
		}
		sprintf(texto, "pido memoria soy rana");
			PRINT_MSG(texto);
		WAIT(SEM_MEMORIA);
		sprintf(texto, "dentro memoria en rana");
			PRINT_MSG(texto);
			Sleep(1000);
		if(((m->dx[orden])<0)||((m->dx[orden]>79))){
			m->r_perdidas++;
			Liberar_sem(tronco_espera);
			SIGNAL(SEM_MEMORIA);
			break;
		}
		
		if(!(m->terminar)){
			Liberar_sem(tronco_espera);
			SIGNAL(SEM_MEMORIA);
				break;
		}
		
	
		if(PUEDO_SALTAR(m->dx[orden],m->dy[orden],ARRIBA)==TRUE) direccion=ARRIBA;
		else if(PUEDO_SALTAR(m->dx[orden],m->dy[orden],DERECHA)==TRUE) direccion=DERECHA;
		else if(PUEDO_SALTAR(m->dx[orden],m->dy[orden],IZQUIERDA)==TRUE) direccion=IZQUIERDA;
		else{
			PRINT_MSG(texto);
			SIGNAL(SEM_MEMORIA);
			Liberar_sem(tronco_espera);
			PAUSA();
			continue;
		}
		
		AVANCE_RANA_INI(m->dx[orden],m->dy[orden]);
		if(AVANCE_RANA(&m->dx[orden],&m->dy[orden],direccion)==FALSE){
			m->r_perdidas++;
			SIGNAL(SEM_MEMORIA);
			Liberar_sem(tronco_espera);
			break;
		}
		tronco_espera++;
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
			Liberar_sem(tronco_espera-1);
			break;
		}
		if(((m->dx[orden])<0)||((m->dx[orden]>79))){
			m->r_perdidas++;
			SIGNAL(SEM_MEMORIA);
			Liberar_sem(tronco_espera-1);
			break;
		}
		Liberar_sem(tronco_espera-1);
		SIGNAL(SEM_MEMORIA);
		
	}

	SIGNAL(SEM_MAX_PROCESOS);
	m->id[orden]=-2;
	
}
DWORD WINAPI TRONCOS( LPVOID parametro){
	int fila;
	int dirs[] = { IZQUIERDA,DERECHA,IZQUIERDA,DERECHA,IZQUIERDA,DERECHA,IZQUIERDA };
	int i;
	char texto[20]="tronco";
	while(m->terminar){
		for(fila=0; fila<7 && m->terminar;fila++){
			
			sprintf(texto, "pido sem tronco %d siendo tronco", fila+10);
			PRINT_MSG(texto);
			WAIT(fila+10);
			sprintf(texto, "pido memoria");
			PRINT_MSG(texto);
			WAIT(SEM_MEMORIA);
			if(!(m->terminar)){
				SIGNAL(SEM_MEMORIA);
				break;
			}
			
		
		
			
			
			if(!(m->terminar)){
				SIGNAL(fila+10);
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
			
			
			
			
			sprintf(texto, "libero sem tronco %d",fila+10);
			PRINT_MSG(texto);
			
			SIGNAL(fila+10);
			SIGNAL(SEM_MEMORIA);
			//	fprintf(stdout,"libero todo\n");
			//	fflush(stdout);
			PAUSA();
			
		}
	}
}
void Liberar_sem(int posicion){
	char texto[20]="libero";
	
	if(posicion==3){
		sprintf(texto, "libero %d", posicion);
		PRINT_MSG(texto);
		SIGNAL(19-posicion);
		
    }else if( posicion > 3){
    	sprintf(texto, "libero %d y %d", posicion, posicion+1);
		PRINT_MSG(texto);
    	SIGNAL(19-posicion);
    	SIGNAL(18-posicion);
	} 
}
void Esperar_sem(int posicion){
	char texto[20]="pido";
	if(posicion==3){
		sprintf(texto, "pido %d", posicion);
		WAIT(19-posicion);
    }else if( posicion > 3){
    	sprintf(texto, "pido %d y %d", posicion, posicion+1);
		PRINT_MSG(texto);
    	WAIT(19-posicion);
    	WAIT(18-posicion);
    	sprintf(texto, "pedidos %d y %d", posicion, posicion+1);
		PRINT_MSG(texto);
	} 
	PRINT_MSG(texto);
	Sleep(2000);
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


/*BOOL WINAPI CtrlHandler(DWORD CtrlType)
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
    
}*/

//Funciones de manejo de semaforos
int sem_wait( int indice)
{	
  return WaitForSingleObject( idSemaforo[indice], INFINITE);
}//sem_wait

int sem_signal( int indice)
{
  return ReleaseSemaphore( idSemaforo[indice], 1, NULL);
}//sem_signal
