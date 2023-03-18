#include <iostream>
#include <string>
#include <cmath>
#include <armadillo>
#include <GL/glut.h>
#include "BezierCurve.h"
#include "NurbsCurve.h"
#include "MatrixAlphaCurve.h"

#define PT_SIZE 9.0f
#define LINE_WIDTH 2.0f
#define CANCEL 6
#define RESET 7
#define EXIT 8

using namespace std;
using namespace arma;

/*	Définition des variables concernant la fenêtre d'affichage	*/
int dispKnotFrameHeight  {200};
int dispWeightFrameWidth {200};
int dispWdwHeight 		{1200};	
int dispWdwWidth  		{1200};
int dispWdwXpos			  {50};
int dispWdwYpos			  {50};

/*	Definition des variables de glOrtho (projection orthogonale)	*/
int orthoHeight {dispWdwHeight};
int orthoWidth  {dispWdwWidth};
int zOrthoMin 	{-1};
int zOrthoMax 	{ 1};

/*	Couleurs de certains objets	*/
float bgColor[] {0.12f, 0.12f, 0.12f, 1.0f};
float ptColor[] {0.96f, 0.0f,  0.0f};
float lnColor[] {0.96f, 0.48f, 0.0f};

float pickingPrecision {12.0f};
int selected {-1};
int action 	  {0};

ControlLine* CtrlLine;
BezierCurve* Bezier;
MatrixCurve* QuadraticMiddles;
MatrixAlphaCurve* CatmullRom;
MatrixCurve* CubicMiddles;
MatrixAlphaCurve* PwBezier;
MatrixCurve* NaturalSpline;
NurbsCurve* Nurbs;
MatrixAlphaCurve* Hermite;
bool curveClosed {false};


/*********************************************************/
/*             			Prototypes					     */
/*********************************************************/

void initFrameLimits();
bool inCurveFrame(int x, int y);
void display();

int getSelectedPointIndex(int mState, int x, int y, bool NorCP);
void mouse(int button, int state, int x, int y);
void mousemotion(int x, int y);

void initMenu();
void menu(int item);
void keyboard(unsigned char touche, int x, int y);

void idle();
void reshape(int x, int y);
void deleteOnQuit();

/*********************************************************/
/*		  Gestion OpenGL : Affichage et évenements		 */
/*********************************************************/

void initFrameLimits()	//--------------------------------- TRACER LES LIMITES DES ZONES (COURBES, POIDS, NOEUDS)
{
	glColor3f(0.94f, 0.94f, 0.94f);
	glLineWidth(5.0f);
	glBegin(GL_LINES);
		glVertex2f(0.0f,(float)dispKnotFrameHeight);
		glVertex2f((float)dispWdwWidth,(float)dispKnotFrameHeight);
		glVertex2f((float)dispWeightFrameWidth,(float)dispKnotFrameHeight);
		glVertex2f((float)dispWeightFrameWidth,(float)dispWdwHeight);
	glEnd();
}

bool inCurveFrame(int x, int y)	//------------------------- TESTER LA POSITION (ZONE COURBES)
{
	return ((x>(dispWeightFrameWidth)) && (y<dispWdwHeight-dispKnotFrameHeight));
}

void display()	//----------------------------------------- GESTION DE L'AFFICHAGE
{
	/* Effacement de l'image avec la couleur de fond */
	glClearColor(bgColor[0], bgColor[1], bgColor[2], bgColor[3]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	/* Objets à afficher */
	glLoadIdentity();
	initFrameLimits();
	glPushMatrix();
		Bezier->display2D();
		PwBezier->display2D(curveClosed);
		QuadraticMiddles->display2D(curveClosed);
		NaturalSpline->display2D(curveClosed);
		CubicMiddles->display2D(curveClosed);
		CatmullRom->display2D(curveClosed);
		Hermite->display2D(curveClosed);
		Nurbs->display2D(curveClosed);
		CtrlLine->display2D();
	glPopMatrix();
	
	/* Nettoyage */
	glFlush();
	glutSwapBuffers();
}

int getSelectedPointIndex(int mState, int x, int y, bool NorCP)	//----- PICKING D'UN POINT / POIDS
{
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	GLuint selectedVertexNum;
	int vertexNb = (NorCP) ? Nurbs->getKnotVectorSize() : CtrlLine->getSize();
	if ((mState == GLUT_DOWN) && (vertexNb > 0)) {				
		GLuint selectBuf[2 * vertexNb]; 					
		GLuint *pointeurBufferselected;	
		GLint selectedObjectNb; 							
		glSelectBuffer(2 * vertexNb, selectBuf);			
		glRenderMode(GL_SELECT); 							
		glPushMatrix();
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			gluPickMatrix(x, viewport[3] - y, pickingPrecision, pickingPrecision, viewport); 
			glOrtho(0.0f, orthoWidth, 0.0f, orthoHeight, zOrthoMin, zOrthoMax);
			glInitNames();
			glPushName(2);
			glColor3fv(ptColor);
			for (int i=0; i<vertexNb; i++) {
				glLoadName(i);							
				glBegin(GL_POINTS);
					if (NorCP) glVertex2fv(Nurbs->getKnotVertex2D(i));
					else {
						if (action <= 14) glVertex2fv(CtrlLine->getVertex2D(i));
						if (action == 21) glVertex2fv(CtrlLine->getWeightVertex2D(i));
					}
				glEnd();
			}
		glPopMatrix();
		glFlush();
		selectedObjectNb = glRenderMode(GL_RENDER); 
		if (selectedObjectNb) {
			pointeurBufferselected = (GLuint *)selectBuf; 
			pointeurBufferselected += 3;		
			selectedVertexNum = *pointeurBufferselected;
		}
		else selectedVertexNum = -1;
	}
	else selectedVertexNum = -1;
	reshape(viewport[2], viewport[3]);
	return selectedVertexNum;
}

void mouse(int mButton, int mState, int x, int y)	//----- GESTION DU CLIC SOURIS
{
	if (mButton == GLUT_LEFT_BUTTON) {
		switch (action) {

			case 11: 	//--------------------- On ajoute un point de contrôle
				if ((mState == GLUT_DOWN) && inCurveFrame(x,y)) {
					float new_y = (float)(dispWdwHeight - y) * orthoHeight / dispWdwHeight;
					float new_x = (float)x * orthoWidth / dispWdwWidth;
					if (	(new_y >= 10.0)
						 && (new_y <= (float)orthoHeight - 10.0)
						 && (new_x >= 10.0)
						 && (new_x <= (float)orthoWidth - 10.0))
					{
						rowvec newPoint {new_x, new_y, 0.0, 1.0};
						CtrlLine->add(newPoint);
						Nurbs->updateKnotVector();
						CtrlLine->setVisible();
						glutPostRedisplay();
					}
				}
				break;

			case 12: 	//--------------------- On supprime un point de contrôle
				CtrlLine->deleteAt(getSelectedPointIndex(mState, x, y, false));
				Nurbs->updateKnotVector();
				glutPostRedisplay();
				break;
				
			case 13: 	//--------------------- On insère un point de contrôle
				CtrlLine->insertAt(getSelectedPointIndex(mState, x, y, false));
				Nurbs->updateKnotVector();
				glutPostRedisplay();
				break;
				
			case 14:	//--------------------- On sélectionne un point de contrôle pour le déplacer
				selected = getSelectedPointIndex(mState, x, y, false);
				break;
				
			case 21:	//--------------------- On sélectionne un point de contrôle pour le déplacer
				selected = getSelectedPointIndex(mState, x, y, false);
				break;
			
			case 23:
				selected = getSelectedPointIndex(mState, x, y, true);
				break;
							
		}
		if (mState == GLUT_UP) selected = -1;		
	}
}

void mousemotion(int x, int y)	//------------------------- GESTION DU MOUVEMENT DE LA SOURIS
{
	if (selected != -1) {
		float new_x = (float)x * orthoWidth / dispWdwWidth;
		float new_y = (float)(dispWdwHeight - y) * orthoHeight / dispWdwHeight;	
		if (action == 14) { 
			if (new_y < dispKnotFrameHeight+10.0f) new_y = dispKnotFrameHeight + 10.0;
			else if (new_y > orthoHeight - 10.0f) new_y = orthoHeight - 10;
			if (new_x < dispWeightFrameWidth+10.0f) new_x = dispWeightFrameWidth + 10.0;
			else if (new_x > orthoWidth - 10.0f) new_x = orthoWidth - 10.0;
			rowvec newPoint {new_x, new_y};
			CtrlLine->moveTo(selected, newPoint);
			glutPostRedisplay();
		}
		if (action == 21) {
			if (new_x < 60.0f) new_x = 60.0;
			else if (new_x > 180.0f) new_x = 180.0;
			Nurbs->setWeight(selected,(new_x==60.0) ? 0.0 : pow(2,(new_x-120.0)/(60.0/log2(CtrlLine->maxWeight))));
			glutPostRedisplay();
		}
		if (action == 23) {
			if (new_x < Nurbs->getKnotVertex2D(selected-1)[0])
				new_x = Nurbs->getKnotVertex2D(selected-1)[0];
			else if (new_x > Nurbs->getKnotVertex2D(selected + 1)[0])
				new_x = Nurbs->getKnotVertex2D(selected+1)[0];
			Nurbs->changeKnotValue(selected, new_x);
			glutPostRedisplay();
		}
	}
}

void initMenu()	//----------------------------------------- INITIALISATION DU MENU FLOTTANT (CLIC DROIT)
{
	int sm_points = glutCreateMenu(menu);
	glutAddMenuEntry("   Add            ", 11);
	glutAddMenuEntry("   Delete         ", 12);
	glutAddMenuEntry("   Insert         ", 13);
	glutAddMenuEntry("   Move           ", 14);
	glutAddMenuEntry(" ________________ ",  0);
	glutAddMenuEntry("   Display/Hide   ", 15);
	glutAddMenuEntry(" ________________ ",  0);
	glutAddMenuEntry("   Save			", 16);
	glutAddMenuEntry("   Load saved     ", 17);

	int main_menu = glutCreateMenu(menu);
	glutAddSubMenu	("   CONTROLS       ", sm_points);
	glutAddMenuEntry(" ________________ ",  0);
	glutAddMenuEntry("   NURBS          ",  2);
	glutAddMenuEntry("                  ",  0);
	glutAddMenuEntry("   Change Weights ", 21);
	glutAddMenuEntry("   Weights to 1.0 ", 22);
	glutAddMenuEntry("   Move Knots     ", 23);
	glutAddMenuEntry(" ________________ ",  0);
	glutAddMenuEntry("   OTHER CURVES   ",  0);
	glutAddMenuEntry("                  ",  0);
	glutAddMenuEntry("   Bezier         ",  3);
	glutAddMenuEntry("   PW_Bezier      ", 31);
	glutAddMenuEntry("   Catmull-Rom    ", 32);
	glutAddMenuEntry("   Quad. Middles  ", 34);
	glutAddMenuEntry("   Natural Spline ", 33);
	glutAddMenuEntry("   Cubic Middles  ", 35);
	glutAddMenuEntry("   Hermite        ", 36);
	glutAddMenuEntry("                  ",  0);
	glutAddMenuEntry(" ________________ ",  0);
	glutAddMenuEntry("                  ",  0);
	glutAddMenuEntry("   Close curves   ",  4);
	glutAddMenuEntry(" ________________ ",  0);
	glutAddMenuEntry("                  ",  0);
	glutAddMenuEntry("   CANCEL         ",  CANCEL);
	glutAddMenuEntry("   RESET          ",  RESET);
	glutAddMenuEntry("   EXIT           ",  EXIT);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void menu(int item)	//------------------------------------- GESTION DES ACTIONS DU MENU FLOTTANT
{
	switch (item) {

		case 11:			//----------------- Ajouter un point
			action = item;
			glutSetCursor(GLUT_CURSOR_CROSSHAIR);
			break;
		
		case 12:			//----------------- Supprimer un point
			action = item;
			glutSetCursor(GLUT_CURSOR_DESTROY);	
			break;

		case 13:			//----------------- Insérer un point
			action = item;
			glutSetCursor(GLUT_CURSOR_HELP);	
			break;
			
		case 14:			//----------------- Déplacer un point
			action = item;
			glutSetCursor(GLUT_CURSOR_INFO);
			break;
							
		case 15:			//----------------- Afficher/Cacher les points de contrôle
			CtrlLine->changeVisibility();
			glutPostRedisplay();
			break;

		case 16:			//----------------- Sauvegarder dans un fichier
			CtrlLine->saveToFile();
			break;
			
		case 17:			//----------------- Restaurer depuis le fichier de sauvegarde
			CtrlLine->readFromFile();
			CtrlLine->setVisible();
			Nurbs->updateKnotVector();
			glutPostRedisplay();
			break;

		case 2:				//----------------- Afficher/cacher la courbe NURBS
			Nurbs->changeVisibility();
			glutPostRedisplay();
			break;
			
		case 21:			//----------------- Changer des poids
			action = item;
			glutSetCursor(GLUT_CURSOR_INFO);
			break;

		case 22:			//----------------- Remettre tous les poids à 1
			Nurbs->setNR();
			glutPostRedisplay();
			break;
			
		case 23:			//----------------- Selectionner un noeud
			action = item;
			glutSetCursor(GLUT_CURSOR_INFO);
			break;			
						
		case 3:				//----------------- Afficher/Cacher la courbe de Bezier
			Bezier->changeVisibility();
			glutPostRedisplay();
			break;

		case 31:			//----------------- Afficher/cacher la courbe Bezier par morceaux
			PwBezier->changeVisibility();
			glutPostRedisplay();
			break;
	
		case 32:			//----------------- Afficher la Spline de Catmull-Rom
			CatmullRom->changeVisibility();
			glutPostRedisplay();
			break;
			
		case 33:			//----------------- Afficher la courbe BSpline cubique NRU
			NaturalSpline->changeVisibility();
			glutPostRedisplay();
			break;
			
		case 34:			//----------------- Afficher la Spline de Middles
			QuadraticMiddles->changeVisibility();
			glutPostRedisplay();
			break;
			
		case 35:			//----------------- Afficher la Spline de Middles
			CubicMiddles->changeVisibility();
			glutPostRedisplay();
			break;

		case 36:			//----------------- Afficher la Spline de Hermite
			Hermite->changeVisibility();
			glutPostRedisplay();
			break;

		case 4:				//----------------- Fermer les courbes
			if (curveClosed) curveClosed = false;
			else curveClosed = true;
			glutPostRedisplay();
			break;
						
		case CANCEL:		//----------------- Désactiver l'action en cours
			action = 0;
			selected = -1;
			glutSetCursor(GLUT_CURSOR_LEFT_ARROW);
		break;
		
		case RESET:			//----------------- Remettre à zéro
			action = 0;
			selected = -1;
			glutSetCursor(GLUT_CURSOR_LEFT_ARROW);
			CtrlLine->reset();
			Nurbs->updateKnotVector();
			curveClosed = false;
			glutPostRedisplay();
		break;
				
		case EXIT:			//----------------- Quitter
			exit(0);
			break;
			
	}
}

void keyboard(unsigned char touche, int x, int y)	//----- GESTION DES SAISIES CLAVIER
{
	switch(touche) {

		case 'q': 		//--------------------- Quitter
			exit(0);
			break;
				
		case 'A':		//--------------------- Augmenter l'alpha de CatmullRom
			CatmullRom->changeAlpha(1);
			glutPostRedisplay();
			break;
			
		case 'a':		//--------------------- Diminuer l'alpha de CatmullRom 
			CatmullRom->changeAlpha(-1);
			glutPostRedisplay();
			break;
	
		case 'H':		//--------------------- Augmenter l'alpha de Hermite
			Hermite->changeAlpha(1);
			glutPostRedisplay();
			break;
			
		case 'h':		//--------------------- Diminuer l'alpha de Hermite 
			Hermite->changeAlpha(-1);
			glutPostRedisplay();
			break;
	
		case 'B':		//--------------------- Diminuer le degré des Beziers par morceaux
			PwBezier->changeAlpha(1);
			glutPostRedisplay();
			break;
		
		case 'b':		//--------------------- Augmenter le degré des Beziers par morceaux
			PwBezier->changeAlpha(-1);
			glutPostRedisplay();
			break;
			
		case 'D':		//--------------------- Diminuer le degré des Beziers par morceaux
			Nurbs->changeDegree(1);
			glutPostRedisplay();
			break;
		
		case 'd':		//--------------------- Augmenter le degré des Beziers par morceaux
			Nurbs->changeDegree(-1);
			glutPostRedisplay();
			break;			

	}
}

void idle()	//--------------------------------------------- GESTION DES ACTIONS PERMANENTES
{}

void reshape(int width, int height)	//--------------------- GESTION DU REDIMENSIONNEMENT
{
	glViewport(0, 0, width, height); 			
	dispWdwWidth = width;
	dispWdwHeight = height;
	glMatrixMode(GL_PROJECTION); 
	glLoadIdentity(); 
	glOrtho(0.0f, orthoWidth, 0.0f, orthoHeight, zOrthoMin, zOrthoMax); 
	glMatrixMode(GL_MODELVIEW); 
	glLoadIdentity();
}

/*********************************************************/
/*               		main 							 */
/*********************************************************/

int main(int argc, char** argv)
{
	/* Initialisation de glut et creation de la fenêtre */
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowPosition(dispWdwXpos, dispWdwYpos);
	glutInitWindowSize(dispWdwWidth, dispWdwHeight);
	glutCreateWindow("Courbes Parametriques et Splines");

	/* Initialisation des styles */
	glClearColor(bgColor[0], bgColor[1], bgColor[2], bgColor[3]);
	glPointSize(PT_SIZE);
	glLineWidth(LINE_WIDTH);
	glEnable(GL_DEPTH_TEST);
	
	/* Initialisation des élements */
	initMenu();
	CtrlLine = new ControlLine();

	Nurbs = new NurbsCurve("NURBS D/d",
						   new float[3] {0.96f, 0.0f, 0.96f},
						   CtrlLine);
										   
	CatmullRom = new MatrixAlphaCurve("CatmullRom A/a",
									  "./Matrices/CatmullRom/CatmullRom",
									  new int[3] {0, 6, 24},
									  new float[3] {0.48f, 0.96f, 0.96f},
									  CtrlLine);
									  					   
	Hermite = new MatrixAlphaCurve("Hermite H/h",
								   "./Matrices/Hermite/Hermite",
									  new int[3] {0, 12, 24},
									  new float[3] {0.96f, 0.48f, 0.48f},
									  CtrlLine);
									  					   
	QuadraticMiddles = new MatrixCurve("Quad. Middles",
									   "./Matrices/Middles_3.mat",
									   new float[3] {0.48f, 0.96f, 0.48f},
									   CtrlLine);
										
	NaturalSpline = new MatrixCurve("Spline NRU",
									"./Matrices/NaturalSpline.mat",
									new float[3] {0.96f, 0.48f, 0.96f},
									CtrlLine);
										 								  
	CubicMiddles = new MatrixCurve("Cubic Middles",
								   "./Matrices/Middles_4.mat",
								   new float[3] {0.48f, 0.48f, 0.96f},
								   CtrlLine,
								   2);
								   
	PwBezier = new MatrixAlphaCurve("PW_Bezier B/b",
									"./Matrices/Bezier/Bezier",
									new int[3] {2, 3, 25},
									new float[3] {0.96f, 0.96f, 0.0f},
									CtrlLine,
									2);
		
	Bezier = new BezierCurve("Bezier", CtrlLine);
										
	/* Enregistrement des fonctions de rappel */
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutReshapeFunc(reshape);
	glutMouseFunc(mouse);
	glutMotionFunc(mousemotion);
	glutIdleFunc(idle);

	/* Entree dans la boucle principale glut */
	glutMainLoop();
	deleteOnQuit();
	return 0;
}

void deleteOnQuit()
{
	delete CtrlLine;
	delete Bezier;
	delete Hermite;
	delete QuadraticMiddles;
	delete CubicMiddles;
	delete CatmullRom;
	delete PwBezier;
	delete NaturalSpline;
	delete Nurbs;
}
