#ifndef DEF_CONTROLLINE
#define DEF_CONTROLLINE

#include <iostream>
#include <string>
#include <armadillo>
#include <cmath>
#include <GL/glut.h>

class ControlLine {

	public:

		ControlLine();											//----- Constructeur à vide
		ControlLine(ControlLine const& other);					//----- Constructeur par copie

		~ControlLine();		//----------------------------- Destructeur

		void add(arma::rowvec newPoint);	//------------------------- Ajouter un point
		void deleteAt(int rowIndex);		//------------------------- Retirer un point
		void insertAt(int rowIndex);		//------------------------- Insérer un point
		void moveTo(int rowIndex, arma::rowvec newPoint);	//--------- Modifier un point
		
		int getSize() const;	//------------------------- Renvoie le nombre de points de contrôle
		
		arma::rowvec getPointAt(int rowIndex) const;	//- Renvoie le point N°i
		arma::mat getMatrix() const;				//----- Renvoie la matrice des points
		float* getVertex2D(int rowIndex) const;		//----- Renvoie un point sous forme OPENGL 2D
		float* getWeightVertex2D(int rowIndex) const;
		float* getVertex3D(int rowIndex) const; 	//----- Renvoie un point sous forme OPENGL 3D

		double getWeight(int rowIndex) const;		//----- Récupérer un poids
		void setWeight(int rowIndex, double newWeight);	//- Modifier un poids
		void setNR();	//--------------------------------- Mettre tous les poids à 1
		
		void changeVisibility();	//--------- La bascule de visibilité		
		void setVisible();			//--------- Rendre visible
		void display2D();			//--------- Tracé de la courbe

		void saveToFile();		//------------- Sauvegarde sur le disque
		void readFromFile();	//------------- Lecture depuis la sauvegarde
		void reset();				//--------- Remise à zéro de la ligne (vidange de la matice)
		
		static const double maxWeight;
		static const float ptColor[3];		//- La couleur des Points
		static const float lnColor[3];		//- La couleur de la ligne
		
	protected:

		arma::mat CtrlPts;		//------------- La matrice des points de contrôle. Chaque ligne comprend 4 nombres
								//              les coordonnées x, y, z ... et le poids w.
		bool toDisplay {false};			//----- Pour gérer l'affichage
		
		// Méthode qui affiche un texte dans la fenêtre OpenGL
		void renderBitmapString(float x, float y, void *font, const char *string);
		
	private:
				
		static const std::string fileName;	//- Le chemin vers le fichier de sauvegarde
					
		// Méthode qui ajuste la dimension d'un vecteur ligne pour qu'il ait 4 colonnes exactement
		arma::rowvec vector4Security(arma::rowvec rv);
		
		// Méthode qui renvoit le milieu de deux points de la liste n°i et n°j
		arma::rowvec getMiddle(int i, int j);
		
		float* getWeightBar2DStartVertex(int rowIndex) const;
		float* getWeightBar2DEndVertex(int rowIndex) const;

};

#endif
