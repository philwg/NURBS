#ifndef DEF_NURBSCURVE
#define DEF_NURBSCURVE

#include <string>
#include <cmath>
#include "Curve.h"

class NurbsCurve : public Curve {

	public:
			
		NurbsCurve();										
		NurbsCurve(std::string label, float* color, ControlLine* init_CL);	//----- Constructeur

		~NurbsCurve();	//--------------------------------------------------------- Destructeur
		
		void display2D(bool closed);				//----------------- Affichage de la NURBS
		void display2Dclosed();
		void changeDegree(int delta);				//----- Changement de degré de la NURBS
		float* getKnotVertex2D(int Index) const;	//----- Renvoi pour le Picking sur les noeuds
		void changeKnotValue(int Index, float nV); 	//----- Changement de la valeur du Noeud n°I
		int getKnotVectorSize();
		void updateKnotVector();			//----- Remplissage du vecteur de noeud en mode uniforme
		int getDegree() const;
		void setWeight(int rowIndex, double newWeight);	//- Modifier un poids
		void setNR();	//--------------------------------- Mettre tous les poids à 1		
				
	protected:
	
		int degree {1};		//----------------- Le degré de la NURBS
		float Xspan{1.0f};
		arma::rowvec Knot;	//----------------- Le vecteur de noeuds
		
		arma::rowvec compute(float t);	//----- Renvoie le point associé à la valeur paramétrique t
		int getBaseKnotIndex(float nt, int maxIndex);		//- Retrouver l'index du plus grand noeud qui minore nt
		float BasisFunction(int i, int d, float t);		//- Les fonctions de bases en mode récursif

		arma::rowvec computeClosed(float t);	//----- Renvoie le point associé à la valeur paramétrique t		
		int getBaseClosedKnotIndex(float nt, int maxIndex);	
		float closedBasisFunction(int i, int d, float t);

		void displayKnots();						//----- Affichage des noeuds et des fonctions de base

	private:
		
		arma::rowvec closedKnot;
		ControlLine* closedCtrlLn;
		bool closedNurbs {false};
		
		void updateClosedCtrls();
		
};

#endif
