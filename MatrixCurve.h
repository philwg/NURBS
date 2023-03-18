#ifndef DEF_MATRIXCURVE
#define DEF_MATRIXCURVE

#include <string>
#include <cmath>
#include "Curve.h"

class MatrixCurve : public Curve {

	public:
		
		MatrixCurve();
		MatrixCurve(std::string label, std::string mPath, float* color, ControlLine* init_CL);
		MatrixCurve(std::string label, std::string mPath, float* color, ControlLine* init_CL, int step);	

		~MatrixCurve();		//------------------------------------------------- Destructeur

		void display2D();
		void display2D(bool closed);	//----- Tracer la courbe fermée ou non
		virtual	void changeAlpha(int delta);
		
	protected:
	
		std::string matrixPath {""};	//----- Le chemin vers les matrices
		arma::mat pwMatrix;				//----- La matrice
		int pwStep {1};					//----- Le pas pour les enchainements de points de contrôle
			
		arma::rowvec compute(float t);	//----- Renvoie le point associé à la valeur paramétrique t

};

#endif
