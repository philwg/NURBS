#ifndef DEF_MATRIXALPHACURVE
#define DEF_MATRIXALPHACURVE

#include <string>
#include <cmath>
#include "MatrixCurve.h"

class MatrixAlphaCurve : public MatrixCurve {

	public:


		MatrixAlphaCurve(std::string label,		//--------- Constructeurs
						 std::string mPath,
						 int* range,
						 float* color,
						 ControlLine* init_CL);	
						 		
		MatrixAlphaCurve(std::string label,
						 std::string mPath,
						 int* range,
						 float* color,
						 ControlLine* init_CL,
						 int step);

		~MatrixAlphaCurve();		//--------------------- Destructeur
		
		void changeAlpha(int delta);	//----- Modification de alpha en douzièmes par +delta dans [0, 12]

	private:
	
		int* alphaRange = new int[3] {0, 6, 12};	//----- L'intervalle des valeurs de Alpha
		bool pwStepFixed {true};
					
		void getPwMatrixFromAlphaPath(); 			//----- Met à jour la matrice courante.

};

#endif
