#include "MatrixAlphaCurve.h"

using namespace std;
using namespace arma;


/*********************************************************/
/*					Constructeurs						 */
/*********************************************************/

MatrixAlphaCurve::MatrixAlphaCurve(string label, string mPath, int* range, float* color, ControlLine* init_CL)
{
	this->CtrlLn = init_CL;
	this->Label = label;
	this->matrixPath = mPath;
	this->curveColor = color;
	this->alphaRange = range;
	this->getPwMatrixFromAlphaPath();
	this->curveRank = Curve::getCurveCount();
}

MatrixAlphaCurve::MatrixAlphaCurve(string label, string mPath, int* range, float* color, ControlLine* init_CL, int step)
{
	this->CtrlLn = init_CL;
	this->Label = label;
	this->matrixPath = mPath;
	this->curveColor = color;
	this->alphaRange = range;
	this->getPwMatrixFromAlphaPath();
	this->pwStep = step;
	this->pwStepFixed = false;
	this->curveRank = Curve::getCurveCount();
}
									
																		
/*********************************************************/
/*					Destructeur							 */
/*********************************************************/
									
MatrixAlphaCurve::~MatrixAlphaCurve()
{
	delete this->alphaRange;
}


/*********************************************************/
/*				Méthodes publiques						 */
/*********************************************************/

void MatrixAlphaCurve::changeAlpha(int delta)	//------------- MODIFICATION DU PARAMETRE DE LA MATRICE
{
	if (this->visible) {
		int newAlpha = this->alphaRange[1] + delta;
		if (newAlpha > alphaRange[2]) newAlpha = alphaRange[2];
		else if (newAlpha < alphaRange[0]) newAlpha = alphaRange[0];		
		this->alphaRange[1] = newAlpha;
		this->getPwMatrixFromAlphaPath();
		if ((!this->pwStepFixed)&&(newAlpha>this->CtrlLn->getSize()+1)) changeAlpha(-delta);	
	}
}


/*********************************************************/
/*				Méthodes privées						 */
/*********************************************************/

void MatrixAlphaCurve::getPwMatrixFromAlphaPath()	//--------- CHARGE LA MATRICE POUR ALPHA COURANT
{
	char mAlpha[2];
	sprintf(mAlpha, "%d", this->alphaRange[1]);
	string fullPath = this->matrixPath;
	fullPath.append("_");
	fullPath.append(mAlpha);
	fullPath.append(".mat");
	this->pwMatrix.load(fullPath);
	if (!this->pwStepFixed) this->pwStep = this->pwMatrix.n_rows-1;
}
