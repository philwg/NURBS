#include "MatrixCurve.h"

using namespace std;
using namespace arma;


/*********************************************************/
/*					Constructeurs						 */
/*********************************************************/

MatrixCurve::MatrixCurve() {}

MatrixCurve::MatrixCurve(string label, string mPath, float* color, ControlLine* init_CL)
{
	this->CtrlLn = init_CL;
	this->Label = label;
	this->matrixPath = mPath;
	this->curveColor = color;
	this->pwMatrix.load(this->matrixPath);
	this->curveRank = Curve::getCurveCount();
}

MatrixCurve::MatrixCurve(string label, string mPath, float* color, ControlLine* init_CL, int step)
{
	this->CtrlLn = init_CL;
	this->Label = label;
	this->matrixPath = mPath;
	this->curveColor = color;
	this->pwMatrix.load(this->matrixPath);
	this->pwStep = step;
	this->curveRank = Curve::getCurveCount();
}
									
																		
/*********************************************************/
/*					Destructeur							 */
/*********************************************************/
									
MatrixCurve::~MatrixCurve()
{
	this->pwMatrix.reset();
}


/*********************************************************/
/*				Méthodes publiques						 */
/*********************************************************/

void MatrixCurve::display2D()
{
	if (this->CtrlLn->getSize()>=this->pwMatrix.n_rows)	this->Curve::display2D();
	else if (this->visible) {
		this->changeAlpha(-1);
		this->Curve::display2D();
	}
	if (this->visible) cout << this->pwMatrix << endl;
}

void MatrixCurve::display2D(bool closed)
{
	if (closed) {
		int d = this->CtrlLn->getSize();
		int n = this->pwMatrix.n_rows - this->pwStep;
		for (int i=0; i<n; i++) this->CtrlLn->add(this->CtrlLn->getPointAt(i));
		this->display2D();
		for (int i=0; i<n; i++) this->CtrlLn->deleteAt(d);
	}
	else this->display2D();
}

void MatrixCurve::changeAlpha(int delta) {}


/*********************************************************/
/*				Méthodes protégées						 */
/*********************************************************/

rowvec MatrixCurve::compute(float t)	//----------------- CALCUL DU POINT POUR t
{
	int n = this->CtrlLn->getSize();
	int m = this->pwMatrix.n_rows;
	float pwt = t*ceil(((float)(n-m+1))/((float)this->pwStep));
	vec tVec = ones(m);
	for(int i=0; i<m; i++) tVec(i) = pow(pwt-floor(pwt), m-1-i);
	mat pwPoint = tVec.t()*this->pwMatrix
			 	  *this->CtrlLn->getMatrix().submat(span((int)(floor(pwt)*pwStep),
												    	 (int)(floor(pwt)*pwStep)+m-1),
											   		span(0,2));
	return (rowvec)(pwPoint.row(0));
}
