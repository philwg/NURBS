#ifndef DEF_BEZIERCURVE
#define DEF_BEZIERCURVE

#include "Curve.h"

class BezierCurve: public Curve {

	public:

		BezierCurve(std::string label, ControlLine* init_CL); //------- Constructeur à partir d'une ligne de contrôle

		~BezierCurve();		//----------------------------------------- Destructeur

	protected:

		arma::rowvec compute(float t);	//----------------- Le calcul de Bezier
		
	private:
		
		int Pascal(int i, int n);	// -------------------- Générateur des coefficients du triangle de Pascal
		float Bernstein(int i, int n, float t);	//--------- Les polynômes de Bernstein
};

#endif
