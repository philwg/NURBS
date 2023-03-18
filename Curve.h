#ifndef DEF_CURVE
#define DEF_CURVE

#include <iostream>
#include <cmath>
#include <armadillo>
#include "ControlLine.h"

class Curve {

	public:

		Curve();					//--------- Constructeur par défaut
		Curve(ControlLine* init_CL);	//----- Constructeur à partir d'une ligne de contrôle

		~Curve();		//--------------------- Destructeur
		
		void changeVisibility();		//----- Bascule Visible/Invisible
		void setVisible();				//----- Rend visibile	
		void display2D();				//----- Tracé de la courbe en mode 2D
		bool isVisible() const;
		
			
	protected:
	
		float* curveColor = new float[3] {0.96f, 0.96f, 0.96f};	//----- La couleur par défaut de la courbe
		ControlLine* CtrlLn;	//------------- La référence vers la ligne des points de contrôle
		bool visible {false};		//--------- Le flag de visibilité de la courbe
		std::string Label {""};		//--------- Le label d'affichage de la courbe
		int curveRank;		
		
		void renderBitmapString(float x, float y, void *font, const char *string) const;
	
		static int getCurveCount();	
		
		virtual arma::rowvec compute(float t) = 0;	//----- Le calcul du point pour la valeur t du paramètre

	private:
	
		static int curveCount;

};

#endif
