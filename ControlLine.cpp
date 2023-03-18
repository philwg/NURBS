#include "ControlLine.h"

using namespace std;
using namespace arma;

/*********************************************************/
/*				Constantes de classe 					 */
/*********************************************************/

const string ControlLine::fileName   = "./Controls/Saved.ctrls";
const float  ControlLine::ptColor[3] = {0.96f, 0.0f, 0.0f};
const float  ControlLine::lnColor[3] = {0.96f, 0.48f, 0.0f};
const double ControlLine::maxWeight  = 16.0;

/*********************************************************/
/*					Constructeur						 */
/*********************************************************/

ControlLine::ControlLine() 
{
	this->readFromFile();
}							

ControlLine::ControlLine(ControlLine const& other)
{
	this->CtrlPts = other.getMatrix();
	this->toDisplay = other.toDisplay;
}

/*********************************************************/
/*					Destructeur 						 */
/*********************************************************/

ControlLine::~ControlLine()
{
	this->CtrlPts.reset();
}


/*********************************************************/
/*					Méthodes publiques					 */
/*********************************************************/

void ControlLine::add(rowvec newPoint)	//----------------- AJOUTE UN POINT AU BOUT DE LA LIGNE
{
	if(!this->CtrlPts.is_empty()) {
		int s = this->getSize();
		this->CtrlPts.resize(s+1,4);
		this->CtrlPts.row(s) = this->vector4Security(newPoint);
	}
	else {
		this->CtrlPts.set_size(1,4);
		this->CtrlPts.row(0) = this->vector4Security(newPoint);
	}
}

void ControlLine::deleteAt(int rowIndex)	//------------- SUPPRIME UN POINT DE LA LIGNE
{
	if(!this->CtrlPts.is_empty()) {
		if ((rowIndex >= 0) && (rowIndex < this->getSize())) {
			this->CtrlPts.shed_row(rowIndex);
		}
	}
}

void ControlLine::insertAt(int rowIndex)	//------------- INSERE UN POINT DANS LA LIGNE
{
	if ((rowIndex >= 0) && (rowIndex < this->getSize()-1)) {
		this->CtrlPts.insert_rows(rowIndex+1, this->vector4Security(this->getMiddle(rowIndex,rowIndex+1)));
	}
}

void ControlLine::moveTo(int rowIndex,rowvec newPoint)	//- MODIFIE UN POINT DE LA LIGNE
{
	if ((!this->CtrlPts.is_empty()) && ((rowIndex >= 0) && (rowIndex < this->getSize()))) {
		this->CtrlPts.row(rowIndex) = this->vector4Security(newPoint);
	}
}

int ControlLine::getSize() const	//--------------------- RENVOIE LE NOMBRE DE POINTS DE LA LIGNE
{
	if (!this->CtrlPts.is_empty()) return (int)this->CtrlPts.n_rows;
	else return 0;
}

rowvec ControlLine::getPointAt(int rowIndex) const	//----- RENVOIE LE POINT COMPLET N°i
{
	return (rowvec)this->CtrlPts.row(rowIndex);
}

mat ControlLine::getMatrix() const	//--------------------- RENVOIE LA MATRICE DES POINTS
{
	return this->CtrlPts;
}

float* ControlLine::getVertex2D(int rowIndex) const	//----- RENVOIE UN VERTEX 2D OPENGL POUR LE POINT N°i
{
	return new float[2] {(float)this->CtrlPts(rowIndex, 0),
						 (float)this->CtrlPts(rowIndex, 1)};
}

float* ControlLine::getWeightVertex2D(int rowIndex) const
{
	double w = this->getPointAt(rowIndex)[3];
	return new float[2] {120.0f+60.0f/(float)(log2(this->maxWeight)/log2((w==0)? 1.0/this->maxWeight : w )),
						 1212.0f-48.0f*(float)(rowIndex+1)};
}

float* ControlLine::getVertex3D(int rowIndex) const	//----- RENVOIE UN VERTEX 3D OPENGL POUR LE POINT N°i
{
	return new float[3] {(float)this->CtrlPts(rowIndex, 0),
						 (float)this->CtrlPts(rowIndex, 1),
						 (float)this->CtrlPts(rowIndex, 2)};
}

double ControlLine::getWeight(int rowIndex) const	//----- RENVOYER LE POIDS DU POINT N°i
{
	if ((!this->CtrlPts.is_empty()) && ((rowIndex >= 0) && (rowIndex < this->getSize()))) {
		return this->CtrlPts(rowIndex, 3);
	}
	else return NAN;	
}

void ControlLine::setWeight(int rowIndex, double newWeight)	// MODIFIER LE POIDS DU POINT N°i
{
	if ((!this->CtrlPts.is_empty()) && ((rowIndex >= 0) && (rowIndex < this->getSize()))) {
		this->CtrlPts(rowIndex, 3) = newWeight;
	}
}

void ControlLine::setNR()	//----------------------------- REND L'ENSEMBLE NON-RATIONNEL
{
	if (!this->CtrlPts.is_empty()) {
		for(int i=0; i<this->getSize(); i++) {
			this->CtrlPts(i, 3) = 1;
		}
	}
}

void ControlLine::changeVisibility()	//----------------- SWITCH DE LA VISIBILITE DE LA LIGNE
{
	if (this->toDisplay) this->toDisplay = false;
	else this->toDisplay = true;
}

void ControlLine::setVisible()	//------------------------- ACTIVER LE TRACE DE LA LIGNE DE CONTROLE
{
	this->toDisplay = true;
}

void ControlLine::display2D()	//------------------------- AFFICHAGE DE LA LIGNE DE CONTROLE EN OPENGL
{
	if (this->toDisplay) {						// Si la ligne de contrôle doit être affichée
		char writeBuffer[15] = {'\0'};
		for(int i=0; i<this->getSize(); i++) {	// Pour chaque point qui la définit ...
			sprintf(writeBuffer, "P%d", i);
			glColor3fv(this->ptColor);
			this->renderBitmapString((float)this->getPointAt(i)[0] - 9.0f,	// On affiche son numéro ...
									 (float)this->getPointAt(i)[1] + 9.0f,
									 GLUT_BITMAP_HELVETICA_18,
									 writeBuffer);
			this->renderBitmapString(this->getWeightVertex2D(i)[0] - 9.0f,
			 						 this->getWeightVertex2D(i)[1] + 9.0f,
									 GLUT_BITMAP_HELVETICA_18,
									 writeBuffer);
			glBegin(GL_POINTS);					
				glVertex2fv(this->getVertex2D(i));						// et le point.
				glVertex2fv(this->getWeightVertex2D(i));
			glEnd();
			glColor3fv(this->lnColor);
			sprintf(writeBuffer, "%.2f", this->getPointAt(i)[3]);
			this->renderBitmapString(12.0f, 1212.0f-48.0f*(float)(i+1)-6.0f,
									 GLUT_BITMAP_HELVETICA_18,
									 writeBuffer);
			glBegin(GL_LINES);
				glVertex2fv(this->getWeightBar2DStartVertex(i));
				glVertex2fv(this->getWeightBar2DEndVertex(i));
			glEnd();
			if (i > 0) {						// Si ce n'est pas le premier ...
				glPushAttrib(GL_ENABLE_BIT);
					glLineStipple(4, 0x8888);
					glEnable(GL_LINE_STIPPLE);
					glBegin(GL_LINES);
						glVertex2fv(this->getVertex2D(i-1));	// On le relie en pointillés ...
						glVertex2fv(this->getVertex2D(i));		// avec le précédent.
					glEnd();
				glPopAttrib();
			}
		}
	}
}

void ControlLine::saveToFile()	//------------------------- SAUVEGARDE
{
	this->CtrlPts.save(fileName);
}

void ControlLine::readFromFile()	//--------------------- RESTAURATION DEPUIS LA SAUVEGARDE
{
	this->CtrlPts.load(fileName);
	this->toDisplay = true;
}

void ControlLine::reset()	//----------------------------- EFFACE TOUS LES POINTS DE LA LIGNE
{
	this->CtrlPts.reset();
	this->toDisplay = false;
}

/*********************************************************/
/*					Méthodes protégées					 */
/*********************************************************/

void ControlLine::renderBitmapString(float x, float y, void *font, const char *string) // AFFICHAGE OPENGL
{
	const char *c;
	glRasterPos2f(x, y);
	for (c = string; *c != '\0'; c++) {
		glutBitmapCharacter(font, *c);
	}
}


/*********************************************************/
/*					Méthodes privées					 */
/*********************************************************/

rowvec ControlLine::vector4Security(rowvec rv)	//--------- ASSURE QUE CHAQUE POINT A BIEN 5 COORDONNEES
{
	rowvec res;
	res.zeros(4);
	res(3) = 1.0;
	for(int i=0; i<min(4,(int)rv.size()); i++) res(i) = rv(i);
	return res;
}

rowvec ControlLine::getMiddle(int i, int j)	//------------- RENVOIE LE MILIEU DES DEUX POINTS N°i ET N°j
{
	return (rowvec)((this->getPointAt(i)+this->getPointAt(j))/2);
}

float* ControlLine::getWeightBar2DStartVertex(int rowIndex) const
{
	return new float[2] {60.0f, 1212.0f-48.0f*(float)(rowIndex+1)};
}

float* ControlLine::getWeightBar2DEndVertex(int rowIndex) const
{
	return new float[2] {180.0f, 1212.0f-48.0f*(float)(rowIndex+1)};
}

