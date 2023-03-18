#include "BezierCurve.h"

#define ROUNDZERO 0.0000000000001f

using namespace std;
using namespace arma;


/*********************************************************/
/*					Constructeurs						 */
/*********************************************************/

BezierCurve::BezierCurve(string label, ControlLine* init_CL)
{
	this->CtrlLn = init_CL;
	this->Label = label;
	this->curveRank = Curve::getCurveCount();
}
									
								
/*********************************************************/
/*					Destructeur							 */
/*********************************************************/
									
BezierCurve::~BezierCurve() {}


/*********************************************************/
/*					Méthodes protégées					 */
/*********************************************************/

rowvec BezierCurve::compute(float t)	//----------------- LE POINT DE LA COURBE DE BEZIER POUR LE PARAMETRE t
{	
	int n = this->CtrlLn->getSize();
	vec tBern = ones(n);
	for(int i=0; i<n; i++) tBern(i) = this->Bernstein(i, n-1, t);
	mat BezierPoint = tBern.t()*this->CtrlLn->getMatrix();
	return (rowvec)(BezierPoint.row(0));
}


/*********************************************************/
/*					Méthodes privées					 */
/*********************************************************/

int BezierCurve::Pascal(int i, int n)	//----------------- COEFFICIENTS DU TRIANGLE DE PASCAL
{
	if ((i<0)||(i>n)) return 0;
	else if ((i==0)||(i==n)) return 1;
	else return this->Pascal(i-1, n-1) + this->Pascal(i, n-1);
}

float BezierCurve::Bernstein(int i, int n, float t)	//----- POLYNOMES DE BERNSTEIN
{
	float value {0.0f};
	if ((t>=0.0f) && (t<=1.0f)) value = (float)this->Pascal(i, n)*pow(t,i)*pow(1.0f-t, n-i);
	if (value < ROUNDZERO) value = 0.0f;
	return value;
}
