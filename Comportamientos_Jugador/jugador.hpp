#ifndef COMPORTAMIENTOJUGADOR_H
#define COMPORTAMIENTOJUGADOR_H

#include "comportamientos/comportamiento.hpp"

#include <list>

struct estado {
  int fila;
  int columna;
  int orientacion;
  int coste;
};



class ComportamientoJugador : public Comportamiento {
  public:
    ComportamientoJugador(unsigned int size) : Comportamiento(size) {
      // Inicializar Variables de Estado
      fil = col = 99;
      brujula = 0; // 0: Norte, 1:Este, 2:Sur, 3:Oeste
      destino.fila = -1;
      destino.columna = -1;
      destino.orientacion = -1;
      hayplan=false;
      tengoBikini = false;
      tengoZapatillas =false;
      enMisionBikini = false;
      enMisionZapatillas = false;
      enMisionRecarga = false;
      filDestinoAux = -1;
      colDestinoAux = -1;
      costeFinal = 0;
      numOperaciones = 3000;
      ultimaRecarga = false;
    }
    ComportamientoJugador(std::vector< std::vector< unsigned char> > mapaR) : Comportamiento(mapaR) {
      // Inicializar Variables de Estado
      fil = col = 99;
      brujula = 0; // 0: Norte, 1:Este, 2:Sur, 3:Oeste
      destino.fila = -1;
      destino.columna = -1;
      destino.orientacion = -1;
      hayplan=false;
      tengoBikini = false;
      tengoZapatillas =false;
      enMisionBikini = false;
      enMisionZapatillas = false;
      enMisionRecarga = false;
      filDestinoAux = -1;
      colDestinoAux = -1;
      costeFinal = 0;
      numOperaciones = 3000;
      ultimaRecarga = false;
    }
    ComportamientoJugador(const ComportamientoJugador & comport) : Comportamiento(comport){}
    ~ComportamientoJugador(){}

    Action think(Sensores sensores);
    int interact(Action accion, int valor);
    void VisualizaPlan(const estado &st, const list<Action> &plan);
    ComportamientoJugador * clone(){return new ComportamientoJugador(*this);}

  private:
    // Declarar Variables de Estado
    int fil, col, brujula;
    estado actual, destino;
    list<Action> plan;
    bool hayplan;
    vector<pair<int,int> > casillasRecarga;
    bool tengoBikini, tengoZapatillas, enMisionBikini, enMisionZapatillas, enMisionRecarga;
    char actualLetra;
    int filDestinoAux, colDestinoAux, costeFinal, numOperaciones;
    bool ultimaRecarga;

    // Métodos privados de la clase
    bool pathFinding(int level, const estado &origen, const estado &destino, list<Action> &plan);
    bool pathFinding_Profundidad(const estado &origen, const estado &destino, list<Action> &plan);
    bool pathFinding_Anchura(const estado &origen, const estado &destino, list<Action> &plan);
    bool pathFinding_CostoUniforme(const estado &origen, const estado &destino, list<Action> &plan);

    void rellenarMapa(Sensores sensores);

    void PintaPlan(list<Action> plan);
    bool HayObstaculoDelante(estado &st);
    int costeCasilla(const char suelo, const bool bik, const bool zap);

    void devolverPosicionConoMapa(int orientacion, int posCono, int & fil, int & col);

};

#endif
