#include "../Comportamientos_Jugador/jugador.hpp"
#include "motorlib/util.h"

#include <iostream>
#include <cmath>
#include <set>
#include <stack>
#include <queue>
#include <vector>


// Este es el método principal que debe contener los 4 Comportamientos_Jugador
// que se piden en la práctica. Tiene como entrada la información de los
// sensores y devuelve la acción a realizar.
Action ComportamientoJugador::think(Sensores sensores) {
	Action sigAccion = actIDLE;
	numOperaciones--;

	// Estoy en el nivel 1
	if(sensores.nivel!=4){
		if(!hayplan){
			actual.fila        = sensores.posF;
			actual.columna     = sensores.posC;
			actual.orientacion = sensores.sentido;

			cout << "Fila: " << actual.fila << endl;
			cout << "Col : " << actual.columna << endl;
			cout << "Ori : " << actual.orientacion << endl;

			destino.fila       = sensores.destinoF;
			destino.columna    = sensores.destinoC;

			hayplan = pathFinding(sensores.nivel, actual, destino, plan);

		}

		if(hayplan and plan.size()>0){
				sigAccion = plan.front();
				plan.erase(plan.begin());
		} else {
			cout << "No es posible encontrar la siguiente accion" << endl;
		}

	} else {

		actual.fila        = sensores.posF;
		actual.columna     = sensores.posC;
		actual.orientacion = sensores.sentido;
		actualLetra		   = mapaResultado[actual.fila][actual.columna];

		if(!hayplan){

			cout << "Fila: " << actual.fila << endl;
			cout << "Col : " << actual.columna << endl;
			cout << "Ori : " << actual.orientacion << endl;

			if(enMisionBikini || enMisionZapatillas){
				destino.fila = filDestinoAux;
				destino.columna = colDestinoAux;
			} else {
				destino.fila       = sensores.destinoF;
				destino.columna    = sensores.destinoC;
			}

			hayplan = pathFinding(sensores.nivel, actual, destino, plan);
			if(hayplan){
				PintaPlan(plan);
				VisualizaPlan(actual, plan);
			}
		}

		if(hayplan && plan.size()>0 && !ultimaRecarga){
			sigAccion = plan.front();
			plan.erase(plan.begin());
		} else if(plan.size()==0){
			cout << "Plan vacío" << endl;
			hayplan = false;
		}

		//Detectamos obstáculos
		if(sigAccion==actFORWARD){
			//Muros y precipicios
			if(sensores.terreno[2]=='M'||sensores.terreno[2]=='P'){
				sigAccion=actIDLE;
				hayplan=false;
				plan.clear();
			}
			//Aldeanos
			if(sensores.superficie[2]=='a'){
				sigAccion=actIDLE;
				plan.push_front(actFORWARD);
			}
		}

		for(int i=0;i<16;i++){
			if(!tengoBikini && !enMisionBikini){
				if(sensores.terreno[i]=='K'){
					int f=actual.fila, c=actual.columna;
					devolverPosicionConoMapa(actual.orientacion,i,f,c);
					filDestinoAux = f;
					colDestinoAux = c;
					hayplan = false;
					enMisionBikini = true;
					sigAccion = actIDLE;
					//cout << "eyyyy " << hayplan << " " << filDestinoAux << " " << colDestinoAux << endl;
				}
			}
			
			if(!tengoZapatillas && !enMisionZapatillas){
				//cout << sensores.terreno[i] << " ";
				if(sensores.terreno[i]=='D'){
					int f=actual.fila, c=actual.columna;
					devolverPosicionConoMapa(actual.orientacion,i,f,c);
					filDestinoAux= f;
					colDestinoAux = c;
					hayplan = false;
					enMisionZapatillas = true;
					sigAccion = actIDLE;
					//cout << "eyyyy2 " << hayplan << " " << filDestinoAux << " " << colDestinoAux << endl;
				}
			}

			if(sensores.terreno[i]=='X'){
				int f=actual.fila, c=actual.columna;
				devolverPosicionConoMapa(actual.orientacion,i,f,c);
				bool insertar = true;
				for(vector<pair<int,int> >::iterator it=casillasRecarga.begin();it!=casillasRecarga.end();++it){
					if(f==(*it).first&&c==(*it).second){
						insertar = false;
					}
				}
				if(insertar){
					casillasRecarga.push_back(make_pair(f,c));
				}
			}
		}
		//cout << endl;

		if(sensores.bateria < 1500 && sensores.bateria < 3.5*costeFinal && casillasRecarga.size()>0 && !enMisionRecarga && sensores.bateria < 1.7*numOperaciones && numOperaciones > 300){
			hayplan = false;
			sigAccion = actIDLE;
			list<Action> planMax;
			int costeMax = 10000000;
			for(vector<pair<int,int> >::iterator it=casillasRecarga.begin();it!=casillasRecarga.end();++it){
				destino.fila = (*it).first;
				destino.columna = (*it).second;
				hayplan = pathFinding_CostoUniforme(actual,destino,plan);
				if(costeFinal<costeMax){
					planMax=plan;
				}
				plan = planMax;
			}
			enMisionRecarga = true;
			enMisionBikini = false;
			enMisionZapatillas = false;
			//cout << "recargaaa" << endl;
		}

		//cout << "CASILLAS RECARGA: ";
		// for(vector<pair<int,int> >::iterator it=casillasRecarga.begin();it!=casillasRecarga.end();++it){
		// 	cout << (*it).first << " " << (*it).second << " -- ";
		// }
		// cout << endl;
		
		
		if(actualLetra=='X'){
			if((sensores.bateria<2900)||ultimaRecarga){
				ultimaRecarga = true;
				if(!(sensores.bateria<2900)){  //&& sensores.bateria < 2.0*numOperaciones
					ultimaRecarga = false;
					hayplan = false;
					enMisionRecarga = false;
				}
				sigAccion = actIDLE;
					
				enMisionBikini = false;
				enMisionZapatillas = false;
			}
		}else if(actualLetra=='K'){
			if(enMisionBikini){
				hayplan = false;
				enMisionBikini = false;
				enMisionZapatillas = false;
				enMisionRecarga = false;
			}
			tengoBikini = true;
		} else if(actualLetra=='D'){
			if(enMisionZapatillas){
				hayplan = false;
				enMisionZapatillas = false;
				enMisionBikini = false;
				enMisionRecarga = false;
			}
			tengoZapatillas = true;
		}
	}

	rellenarMapa(sensores);
	
  	return sigAccion;
}


void ComportamientoJugador::devolverPosicionConoMapa(int orientacion, int posCono, int & fil, int & col){
	
	switch(orientacion){
		case 0:
			switch(posCono){
				case 1: fil=fil-1; col=col-1; break;
				case 2: fil=fil-1; 			  break;
				case 3: fil=fil-1; col=col+1; break;

				case 4: fil=fil-2; col=col-2; break;
				case 5: fil=fil-2; col=col-1; break;
				case 6: fil=fil-2; 			  break;
				case 7: fil=fil-2; col=col+1; break;
				case 8: fil=fil-2; col=col+2; break;

				case 9: fil=fil-3; col=col-3; break;
				case 10: fil=fil-3; col=col-2; break;
				case 11: fil=fil-3; col=col-1; break;
				case 12: fil=fil-3; 		   break;
				case 13: fil=fil-3; col=col+1; break;
				case 14: fil=fil-3; col=col+2; break;
				case 15: fil=fil-3; col=col+3; break;
			}
		break;
		case 1:
			switch(posCono){
				case 1: fil=fil-1; col=col+1; break;
				case 2: 		   col=col+1; break;
				case 3: fil=fil+1; col=col+1; break;

				case 4: fil=fil-2; col=col+2; break;
				case 5: fil=fil-1; col=col+2; break;
				case 6: 		   col=col+2; break;
				case 7: fil=fil+1; col=col+2; break;
				case 8: fil=fil+2; col=col+2; break;

				case 9: fil=fil-3; col=col+3; break;
				case 10: fil=fil-2; col=col+3; break;
				case 11: fil=fil-1; col=col+3; break;
				case 12: 			col=col+3; break;
				case 13: fil=fil+1; col=col+3; break;
				case 14: fil=fil+2; col=col+3; break;
				case 15: fil=fil+3; col=col+3; break;
			}
		break;
		case 2:
			switch(posCono){
				case 1: fil=fil+1; col=col+1; break;
				case 2: fil=fil+1; 			  break;
				case 3: fil=fil+1; col=col-1; break;

				case 4: fil=fil+2; col=col+2; break;
				case 5: fil=fil+2; col=col+1; break;
				case 6: fil=fil+2; 			  break;
				case 7: fil=fil+2; col=col-1; break;
				case 8: fil=fil+2; col=col-2; break;

				case 9: fil=fil+3; col=col+3; break;
				case 10: fil=fil+3; col=col+2; break;
				case 11: fil=fil+3; col=col+1; break;
				case 12: fil=fil+3; 		   break;
				case 13: fil=fil+3; col=col-1; break;
				case 14: fil=fil+3; col=col-2; break;
				case 15: fil=fil+3; col=col-3; break;
			}
		break;
		case 3:
			switch(posCono){
				case 1: fil=fil+1; col=col-1; break;
				case 2: 		   col=col-1; break;
				case 3: fil=fil-1; col=col-1; break;

				case 4: fil=fil+2; col=col-2; break;
				case 5: fil=fil+1; col=col-2; break;
				case 6: 		   col=col-2; break;
				case 7: fil=fil-1; col=col-2; break;
				case 8: fil=fil-2; col=col-2; break;

				case 9: fil=fil+3; col=col-3; break;
				case 10: fil=fil+2; col=col-3; break;
				case 11: fil=fil+1; col=col-3; break;
				case 12: 			col=col-3; break;
				case 13: fil=fil-1; col=col-3; break;
				case 14: fil=fil-2; col=col-3; break;
				case 15: fil=fil-3; col=col-3; break;
			}
		break;

	}
}

void ComportamientoJugador::rellenarMapa(Sensores sensores){
	mapaResultado[sensores.posF][sensores.posC] = sensores.terreno[0];

	switch(sensores.sentido){
		case 0: 
			mapaResultado[sensores.posF-1][sensores.posC-1] = sensores.terreno[1];
			mapaResultado[sensores.posF-1][sensores.posC] = sensores.terreno[2];
			mapaResultado[sensores.posF-1][sensores.posC+1] = sensores.terreno[3];

			mapaResultado[sensores.posF-2][sensores.posC-2] = sensores.terreno[4];
			mapaResultado[sensores.posF-2][sensores.posC-1] = sensores.terreno[5];
			mapaResultado[sensores.posF-2][sensores.posC] = sensores.terreno[6];
			mapaResultado[sensores.posF-2][sensores.posC+1] = sensores.terreno[7];
			mapaResultado[sensores.posF-2][sensores.posC+2] = sensores.terreno[8];

			mapaResultado[sensores.posF-3][sensores.posC-3] = sensores.terreno[9];
			mapaResultado[sensores.posF-3][sensores.posC-2] = sensores.terreno[10];
			mapaResultado[sensores.posF-3][sensores.posC-1] = sensores.terreno[11];
			mapaResultado[sensores.posF-3][sensores.posC] = sensores.terreno[12];
			mapaResultado[sensores.posF-3][sensores.posC+1] = sensores.terreno[13];
			mapaResultado[sensores.posF-3][sensores.posC+2] = sensores.terreno[14];
			mapaResultado[sensores.posF-3][sensores.posC+3] = sensores.terreno[15];
		break;
		case 1:
			mapaResultado[sensores.posF-1][sensores.posC+1] = sensores.terreno[1];
			mapaResultado[sensores.posF][sensores.posC+1] = sensores.terreno[2];
			mapaResultado[sensores.posF+1][sensores.posC+1] = sensores.terreno[3];

			mapaResultado[sensores.posF-2][sensores.posC+2] = sensores.terreno[4];
			mapaResultado[sensores.posF-1][sensores.posC+2] = sensores.terreno[5];
			mapaResultado[sensores.posF][sensores.posC+2] = sensores.terreno[6];
			mapaResultado[sensores.posF+1][sensores.posC+2] = sensores.terreno[7];
			mapaResultado[sensores.posF+2][sensores.posC+2] = sensores.terreno[8];

			mapaResultado[sensores.posF-3][sensores.posC+3] = sensores.terreno[9];
			mapaResultado[sensores.posF-2][sensores.posC+3] = sensores.terreno[10];
			mapaResultado[sensores.posF-1][sensores.posC+3] = sensores.terreno[11];
			mapaResultado[sensores.posF][sensores.posC+3] = sensores.terreno[12];
			mapaResultado[sensores.posF+1][sensores.posC+3] = sensores.terreno[13];
			mapaResultado[sensores.posF+2][sensores.posC+3] = sensores.terreno[14];
			mapaResultado[sensores.posF+3][sensores.posC+3] = sensores.terreno[15];
		break;
		case 2:
			mapaResultado[sensores.posF+1][sensores.posC+1] = sensores.terreno[1];
			mapaResultado[sensores.posF+1][sensores.posC] = sensores.terreno[2];
			mapaResultado[sensores.posF+1][sensores.posC-1] = sensores.terreno[3];

			mapaResultado[sensores.posF+2][sensores.posC+2] = sensores.terreno[4];
			mapaResultado[sensores.posF+2][sensores.posC+1] = sensores.terreno[5];
			mapaResultado[sensores.posF+2][sensores.posC] = sensores.terreno[6];
			mapaResultado[sensores.posF+2][sensores.posC-1] = sensores.terreno[7];
			mapaResultado[sensores.posF+2][sensores.posC-2] = sensores.terreno[8];

			mapaResultado[sensores.posF+3][sensores.posC+3] = sensores.terreno[9];
			mapaResultado[sensores.posF+3][sensores.posC+2] = sensores.terreno[10];
			mapaResultado[sensores.posF+3][sensores.posC+1] = sensores.terreno[11];
			mapaResultado[sensores.posF+3][sensores.posC] = sensores.terreno[12];
			mapaResultado[sensores.posF+3][sensores.posC-1] = sensores.terreno[13];
			mapaResultado[sensores.posF+3][sensores.posC-2] = sensores.terreno[14];
			mapaResultado[sensores.posF+3][sensores.posC-3] = sensores.terreno[15];
		break;
		case 3:
			mapaResultado[sensores.posF+1][sensores.posC-1] = sensores.terreno[1];
			mapaResultado[sensores.posF][sensores.posC-1] = sensores.terreno[2];
			mapaResultado[sensores.posF-1][sensores.posC-1] = sensores.terreno[3];

			mapaResultado[sensores.posF+2][sensores.posC-2] = sensores.terreno[4];
			mapaResultado[sensores.posF+1][sensores.posC-2] = sensores.terreno[5];
			mapaResultado[sensores.posF][sensores.posC-2] = sensores.terreno[6];
			mapaResultado[sensores.posF-1][sensores.posC-2] = sensores.terreno[7];
			mapaResultado[sensores.posF-2][sensores.posC-2] = sensores.terreno[8];

			mapaResultado[sensores.posF+3][sensores.posC-3] = sensores.terreno[9];
			mapaResultado[sensores.posF+2][sensores.posC-3] = sensores.terreno[10];
			mapaResultado[sensores.posF+1][sensores.posC-3] = sensores.terreno[11];
			mapaResultado[sensores.posF][sensores.posC-3] = sensores.terreno[12];
			mapaResultado[sensores.posF-1][sensores.posC-3] = sensores.terreno[13];
			mapaResultado[sensores.posF-2][sensores.posC-3] = sensores.terreno[14];
			mapaResultado[sensores.posF-3][sensores.posC-3] = sensores.terreno[15];
		break;
	}
}


// Llama al algoritmo de busqueda que se usará en cada comportamiento del agente
// Level representa el comportamiento en el que fue iniciado el agente.
bool ComportamientoJugador::pathFinding (int level, const estado &origen, const estado &destino, list<Action> &plan){
	switch (level){
		case 1: cout << "Busqueda en profundad\n";
			      return pathFinding_Profundidad(origen,destino,plan);
						break;
		case 2: cout << "Busqueda en Anchura\n";
			      return pathFinding_Anchura(origen,destino,plan);
						break;
		case 3: cout << "Busqueda Costo Uniforme\n";
						return pathFinding_CostoUniforme(origen,destino,plan);
						break;
		case 4: cout << "Busqueda para el reto\n";
						return pathFinding_CostoUniforme(origen,destino,plan);
						break;
	}
	cout << "Comportamiento sin implementar\n";
	return false;
}


//---------------------- Implementación de la busqueda en profundidad ---------------------------

// Dado el código en carácter de una casilla del mapa dice si se puede
// pasar por ella sin riegos de morir o chocar.
bool EsObstaculo(unsigned char casilla){
	if (casilla=='P' or casilla=='M')
		return true;
	else
	  return false;
}


// Comprueba si la casilla que hay delante es un obstaculo. Si es un
// obstaculo devuelve true. Si no es un obstaculo, devuelve false y
// modifica st con la posición de la casilla del avance.
bool ComportamientoJugador::HayObstaculoDelante(estado &st){
	int fil=st.fila, col=st.columna;

  // calculo cual es la casilla de delante del agente
	switch (st.orientacion) {
		case 0: fil--; break;
		case 1: col++; break;
		case 2: fil++; break;
		case 3: col--; break;
	}

	// Compruebo que no me salgo fuera del rango del mapa
	if (fil<0 or fil>=mapaResultado.size()) return true;
	if (col<0 or col>=mapaResultado[0].size()) return true;

	// Miro si en esa casilla hay un obstaculo infranqueable
	if (!EsObstaculo(mapaResultado[fil][col])){
		// No hay obstaculo, actualizo el parámetro st poniendo la casilla de delante.
    st.fila = fil;
		st.columna = col;
		return false;
	}
	else{
	  return true;
	}
}




struct nodo{
	estado st;
	list<Action> secuencia;
	bool bikini;
	bool zapatillas;
	int coste;
};

struct ComparaEstados{
	bool operator()(const estado &a, const estado &n) const{
		if ((a.fila > n.fila) or (a.fila == n.fila and a.columna > n.columna) or
	      (a.fila == n.fila and a.columna == n.columna and a.orientacion > n.orientacion))
			return true;
		else
			return false;
	}
};


//****************************************************************************************************************************
//************************PROFUNDIDAD*****************************************************************************************
//****************************************************************************************************************************

// Implementación de la búsqueda en profundidad.
// Entran los puntos origen y destino y devuelve la
// secuencia de acciones en plan, una lista de acciones.
bool ComportamientoJugador::pathFinding_Profundidad(const estado &origen, const estado &destino, list<Action> &plan) {
	//Borro la lista
	cout << "Calculando plan\n";
	plan.clear();
	set<estado,ComparaEstados> generados; // Lista de Cerrados
	stack<nodo> pila;					  // Lista de Abiertos

  	nodo current;
	current.st = origen;
	current.secuencia.empty();

	pila.push(current);

  	while (!pila.empty() and (current.st.fila!=destino.fila or current.st.columna != destino.columna)){

		pila.pop();
		generados.insert(current.st);

		// Generar descendiente de girar a la derecha
		nodo hijoTurnR = current;
		hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion+1)%4;
		if (generados.find(hijoTurnR.st) == generados.end()){
			hijoTurnR.secuencia.push_back(actTURN_R);
			pila.push(hijoTurnR);
		}

		// Generar descendiente de girar a la izquierda
		nodo hijoTurnL = current;
		hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion+3)%4;
		if (generados.find(hijoTurnL.st) == generados.end()){
			hijoTurnL.secuencia.push_back(actTURN_L);
			pila.push(hijoTurnL);
		}

		// Generar descendiente de avanzar
		nodo hijoForward = current;
		if (!HayObstaculoDelante(hijoForward.st)){
			if (generados.find(hijoForward.st) == generados.end()){
				hijoForward.secuencia.push_back(actFORWARD);
				pila.push(hijoForward);
			}
		}

		// Tomo el siguiente valor de la pila
		if (!pila.empty()){
			current = pila.top();
		}
	}

  	cout << "Terminada la busqueda\n";

	if (current.st.fila == destino.fila and current.st.columna == destino.columna){
		cout << "Cargando el plan\n";
		plan = current.secuencia;
		cout << "Longitud del plan: " << plan.size() << endl;
		PintaPlan(plan);
		// ver el plan en el mapa
		VisualizaPlan(origen, plan);
		return true;
	}
	else {
		cout << "No encontrado plan\n";
	}


	return false;
}

//****************************************************************************************************************************
//************************ANCHURA*********************************************************************************************
//****************************************************************************************************************************

bool ComportamientoJugador::pathFinding_Anchura(const estado &origen, const estado &destino, list<Action> &plan) {
	//Borro la lista
	cout << "Calculando plan\n";
	plan.clear();
	set<estado,ComparaEstados> generados; // Lista de Cerrados
	queue<nodo> cola;					  // Lista de Abiertos

  	nodo current;
	current.st = origen;
	current.secuencia.empty();

	cola.push(current);

  	while (!cola.empty() and (current.st.fila!=destino.fila or current.st.columna != destino.columna)){

		cola.pop();
		generados.insert(current.st);

		// Generar descendiente de girar a la derecha
		nodo hijoTurnR = current;
		hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion+1)%4;
		if (generados.find(hijoTurnR.st) == generados.end()){
			hijoTurnR.secuencia.push_back(actTURN_R);
			cola.push(hijoTurnR);
		}

		// Generar descendiente de girar a la izquierda
		nodo hijoTurnL = current;
		hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion+3)%4;
		if (generados.find(hijoTurnL.st) == generados.end()){
			hijoTurnL.secuencia.push_back(actTURN_L);
			cola.push(hijoTurnL);
		}

		// Generar descendiente de avanzar
		nodo hijoForward = current;
		if (!HayObstaculoDelante(hijoForward.st)){
			if (generados.find(hijoForward.st) == generados.end()){
				hijoForward.secuencia.push_back(actFORWARD);
				cola.push(hijoForward);
			}
		}

		current = cola.front();
		while (!cola.empty() and generados.find(current.st) != generados.end()){
			current = cola.front();
			cola.pop();
		}
	}



	cout << "Tamanio generados: " << generados.size() << endl;

  	cout << "Terminada la busqueda\n";

	if (current.st.fila == destino.fila and current.st.columna == destino.columna){
		cout << "Cargando el plan\n";
		plan = current.secuencia;
		cout << "Longitud del plan: " << plan.size() << endl;
		PintaPlan(plan);
		// ver el plan en el mapa
		VisualizaPlan(origen, plan);
		return true;
	}
	else {
		cout << "No encontrado plan\n";
	}


	return false;
}


//****************************************************************************************************************************
//************************COSTO UNIFORME**************************************************************************************
//****************************************************************************************************************************

int ComportamientoJugador::costeCasilla(const char suelo, const bool bik, const bool zap){
	switch (suelo){
		case 'A':
			if(!bik)
				return 100;
			else
				return 10;
		break;
		case 'B': 
			if(!zap)
				return 50;
			else
				return 5;
		break;
		case 'T': 
			return 2;
		break;
		case '?': 
			return 3;
		break;
		default: 
			return 1;
	}
}

class ColaPrio{
private:
	vector<nodo> v;

public:

	vector<nodo>::iterator begin() {
		return v.begin();
	}

	vector<nodo>::iterator end() {
		return v.end();
	}

	vector<nodo>::const_iterator cbegin() const{
		return v.cbegin();
	}

	vector<nodo>::const_iterator cend() const{
		return v.cend();
	}

	bool empty() const{
		return v.empty();
	}

	vector<nodo>::iterator find(const nodo & n){
		auto it = v.begin();
		for(;it != v.end(); ++it){
			if(n.st.orientacion==(*it).st.orientacion&&n.st.fila==(*it).st.fila&&n.st.columna==(*it).st.columna){
				return it;
			}
		}
		return v.end();
	}

	void push(const nodo & n){
		auto it = v.begin();
		for(;it != v.end()&& n.coste > (*it).coste; ++it){}
		v.insert(it,n);
	}

	void pop(){
		v.erase(v.begin());
	}

	void erase(vector<nodo>::iterator it){
		v.erase(it);
	}

	nodo top(){
		return *(v.begin());
	}
};


bool ComportamientoJugador::pathFinding_CostoUniforme(const estado &origen, const estado &destino, list<Action> &plan) {
	//Borro la lista
	cout << "Calculando plan\n";
	plan.clear();
	set<estado,ComparaEstados> generados; 								// Lista de Cerrados
	ColaPrio cola;														// Lista de Abiertos

  	nodo current;
	current.st = origen;
	current.secuencia.empty();
	current.coste = 0;
	current.bikini = tengoBikini;
	current.zapatillas = tengoZapatillas;
	if(mapaResultado[current.st.fila][current.st.columna]=='K'){
		current.bikini = true;
	}
	if(mapaResultado[current.st.fila][current.st.columna]=='D'){
		current.zapatillas = true;
	}

	cola.push(current);

  	while (!cola.empty() and (current.st.fila!=destino.fila or current.st.columna != destino.columna)){

		cola.pop();
		generados.insert(current.st);

		// Generar descendiente de girar a la derecha
		nodo hijoTurnR = current;
		hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion+1)%4;
		if (generados.find(hijoTurnR.st) == generados.end()){
			hijoTurnR.secuencia.push_back(actTURN_R);

			char terreno = mapaResultado[hijoTurnR.st.fila][hijoTurnR.st.columna];
			if(terreno=='K'){
				hijoTurnR.bikini = true;
			}
			if(terreno=='D'){
				hijoTurnR.zapatillas = true;
			}
			hijoTurnR.coste += costeCasilla(terreno,hijoTurnR.bikini,hijoTurnR.zapatillas);

			auto it = cola.find(hijoTurnR);
			if(it==cola.end()){
				cola.push(hijoTurnR);
			}else{
				if((*it).coste > hijoTurnR.coste){
					cola.erase(it);
					cola.push(hijoTurnR);
				}
			}
		}

		// Generar descendiente de girar a la izquierda
		nodo hijoTurnL = current;
		hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion+3)%4;
		if (generados.find(hijoTurnL.st) == generados.end()){
			hijoTurnL.secuencia.push_back(actTURN_L);

			char terreno = mapaResultado[hijoTurnL.st.fila][hijoTurnL.st.columna];
			if(terreno=='K'){
				hijoTurnL.bikini = true;
			}
			if(terreno=='D'){
				hijoTurnL.zapatillas = true;
			}
			hijoTurnL.coste += costeCasilla(terreno,hijoTurnL.bikini,hijoTurnL.zapatillas);

			auto it = cola.find(hijoTurnL);
			if(it==cola.end()){
				cola.push(hijoTurnL);
			}else{
				if((*it).coste > hijoTurnL.coste){
					cola.erase(it);
					cola.push(hijoTurnL);
				}
			}
		}

		// Generar descendiente de avanzar
		nodo hijoForward = current;
		if (!HayObstaculoDelante(hijoForward.st)){
			if (generados.find(hijoForward.st) == generados.end()){
				hijoForward.secuencia.push_back(actFORWARD);

				char terreno = mapaResultado[hijoForward.st.fila][hijoForward.st.columna];
				if(terreno=='K'){
					hijoForward.bikini = true;
				}
				if(terreno=='D'){
					hijoForward.zapatillas = true;
				}
				hijoForward.coste += costeCasilla(terreno,hijoForward.bikini,hijoForward.zapatillas);

				auto it = cola.find(hijoForward);
				if(it==cola.end()){
					cola.push(hijoForward);
				}else{
					if((*it).coste > hijoForward.coste){
						cola.erase(it);
						cola.push(hijoForward);
					}
				}
			}
		}

		if (!cola.empty()){
	    	current = cola.top();
	  	}	
	}

	

	cout << "Tamanio generados: " << generados.size() << endl;

  	cout << "Terminada la busqueda\n";

	if (current.st.fila == destino.fila and current.st.columna == destino.columna){
		cout << "Cargando el plan\n";
		plan = current.secuencia;
		costeFinal = current.coste;
		cout << "Longitud del plan: " << plan.size() << endl;
		PintaPlan(plan);
		// ver el plan en el mapa
		VisualizaPlan(origen, plan);
		return true;
	}
	else {
		cout << "No encontrado plan\n";
	}


	return false;
}



// Sacar por la términal la secuencia del plan obtenido
void ComportamientoJugador::PintaPlan(list<Action> plan) {
	auto it = plan.begin();
	while (it!=plan.end()){
		if (*it == actFORWARD){
			cout << "A ";
		}
		else if (*it == actTURN_R){
			cout << "D ";
		}
		else if (*it == actTURN_L){
			cout << "I ";
		}
		else {
			cout << "- ";
		}
		it++;
	}
	cout << endl;
}



void AnularMatriz(vector<vector<unsigned char> > &m){
	for (int i=0; i<m[0].size(); i++){
		for (int j=0; j<m.size(); j++){
			m[i][j]=0;
		}
	}
}


// Pinta sobre el mapa del juego el plan obtenido
void ComportamientoJugador::VisualizaPlan(const estado &st, const list<Action> &plan){
  AnularMatriz(mapaConPlan);
	estado cst = st;

	auto it = plan.begin();
	while (it!=plan.end()){
		if (*it == actFORWARD){
			switch (cst.orientacion) {
				case 0: cst.fila--; break;
				case 1: cst.columna++; break;
				case 2: cst.fila++; break;
				case 3: cst.columna--; break;
			}
			mapaConPlan[cst.fila][cst.columna]=1;
		}
		else if (*it == actTURN_R){
			cst.orientacion = (cst.orientacion+1)%4;
		}
		else {
			cst.orientacion = (cst.orientacion+3)%4;
		}
		it++;
	}
}



int ComportamientoJugador::interact(Action accion, int valor){
  return false;
}
