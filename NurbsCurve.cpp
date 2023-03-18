#include "NurbsCurve.h"

#define PHI 1.618034f

using namespace std;
using namespace arma;


/*********************************************************/
/*					Constructeur						 */
/*********************************************************/

NurbsCurve::NurbsCurve() {}

NurbsCurve::NurbsCurve(string label, float* color, ControlLine* init_CL)
{
	this->CtrlLn = init_CL;
	this->closedCtrlLn = new ControlLine(*init_CL);
	this->curveColor = color;
	this->Label = label;
	this->curveRank = Curve::getCurveCount();
	this->updateClosedCtrls();
	this->updateKnotVector();
}
									
								
/*********************************************************/
/*					Destructeur							 */
/*********************************************************/
									
NurbsCurve::~NurbsCurve() {}


/*********************************************************/
/*					Méthodes publiques					 */
/*********************************************************/

void NurbsCurve::display2D(bool closed)	//------------------------- LA FONCTION D'AFFICHAGE
{
	if ((this->visible)&&(this->CtrlLn->getSize() > this->degree)) {
		this->closedNurbs = closed;
		if (!closed) this->Curve::display2D();
		else this->display2Dclosed();
		this->Xspan = (float)(1200.0f/(this->Knot.max()-this->Knot.min()+2));
		this->displayKnots();
	}
}

void NurbsCurve::display2Dclosed()
{
	this->updateClosedCtrls();
	this->updateKnotVector();
	rowvec prevPoint = this->computeClosed(0.0f);
	glColor3fv(this->curveColor);
	for (float t=0.001f; t<=1.0f; t+=0.001f) {
		rowvec crntPoint = this->computeClosed(t);
		glBegin(GL_LINES);
			glVertex2f((float)prevPoint(0), (float)prevPoint(1));
			glVertex2f((float)crntPoint(0), (float)crntPoint(1));
		glEnd();
		prevPoint = crntPoint;
	}
	glColor3fv(this->curveColor);
	this->renderBitmapString(144.0f*(this->curveRank)-72.0f,
							 224.0f,
							 GLUT_BITMAP_HELVETICA_18,
							 this->Label.c_str());
}

void NurbsCurve::setWeight(int rowIndex, double newWeight)
{
	this->CtrlLn->setWeight(rowIndex, newWeight);
	this->closedCtrlLn->setWeight(rowIndex, newWeight);
	if (rowIndex<this->degree) this->closedCtrlLn->setWeight(this->CtrlLn->getSize()+rowIndex, newWeight);
}

void NurbsCurve::setNR()
{
	this->CtrlLn->setNR();
	this->closedCtrlLn->setNR();
}

int NurbsCurve::getDegree() const
{
	return this->degree;
}

void NurbsCurve::changeDegree(int delta)	//------------- LA FONCTION DE CHANGEMENT DE DEGRE
{
	this->degree += delta;
	if (this->degree <= 0) this->degree = 1;
	if (this->degree >= this->CtrlLn->getSize()) this->degree = this->CtrlLn->getSize()-1;
	this->updateClosedCtrls();
	this->updateKnotVector();
}

float* NurbsCurve::getKnotVertex2D(int Index) const
{
	return new float[2] {(float)(this->Xspan*(this->Knot[Index]+1)), 50.0f};
}

void NurbsCurve::changeKnotValue(int Index, float nV)
{
	if ((Index>0)&&(Index<this->CtrlLn->getSize() + this->degree)) {
		this->Knot[Index] = (nV/this->Xspan)-1;
		for (int i=0; i < this->Knot.n_elem; i++) this->closedKnot[i] = this->Knot[i];
		double knotGap = this->Knot[Index] - this->Knot[Index-1];
		int conIndex = this->CtrlLn->getSize();
		if (Index < conIndex) {
			this->closedKnot[Index + conIndex] = this->closedKnot[Index + conIndex - 1] + knotGap;
			if((this->closedNurbs)&&(Index<this->degree))
				this->Knot[Index + conIndex] = this->closedKnot[Index + conIndex];
		}
		if (Index > conIndex) {
			this->closedKnot[Index - conIndex] = this->closedKnot[Index - conIndex - 1] + knotGap;
			if((this->closedNurbs)&&(Index>this->CtrlLn->getSize()))
				this->Knot[Index - conIndex] = this->closedKnot[Index - conIndex];
		}
	}
}

int NurbsCurve::getKnotVectorSize()
{
	return this->Knot.n_elem;
}

void NurbsCurve::updateKnotVector()
{
	this->Knot.ones(this->CtrlLn->getSize() + this->degree +1);
	this->closedKnot.ones(this->CtrlLn->getSize() + 2*this->degree + 1);
	for(int i=0; i<this->Knot.n_elem; i++) this->Knot[i] = i;
	for(int i=0; i<this->closedKnot.n_elem; i++) this->closedKnot[i] = i;
}

/*********************************************************/
/*					Méthodes protégées					 */
/*********************************************************/

rowvec NurbsCurve::compute(float t)	//--------------------- LE POINT DE LA COURBE NURBS POUR LE PARAMETRE t
{	
	vec tBS = zeros(this->degree + 1);
	float nt =   (float)this->Knot(this->degree)
			   + t * (float)(this->Knot(this->CtrlLn->getSize())-this->Knot(this->degree));
	int tIndex = this->getBaseKnotIndex(nt, this->CtrlLn->getSize());
	mat tCtrls = this->CtrlLn->getMatrix().submat(span(tIndex-this->degree, tIndex), span(0,3));
	for(int i=0; i < tBS.n_elem; i++) {
		tBS(i) = this->BasisFunction(tIndex - this->degree + i, this->degree, nt);
		tBS(i) *= this->CtrlLn->getWeight(tIndex - this->degree + i);
	}
	mat NurbsPoint = tBS.t()*tCtrls;
	NurbsPoint /= accu(tBS);
	return (rowvec)(NurbsPoint.row(0));
}

rowvec NurbsCurve::computeClosed(float t)	//------------- LE POINT DE LA COURBE NURBS POUR LE PARAMETRE t
{	
	vec tBS = zeros(this->degree + 1);
	float nt =   (float)this->closedKnot(this->degree)
			   + t * (float)(this->closedKnot(this->closedCtrlLn->getSize())-this->closedKnot(this->degree));
	int tIndex = this->getBaseClosedKnotIndex(nt, this->closedCtrlLn->getSize());
	mat tCtrls = this->closedCtrlLn->getMatrix().submat(span(tIndex-this->degree, tIndex), span(0,3));
	for(int i=0; i < tBS.n_elem; i++) {
		tBS(i) = this->closedBasisFunction(tIndex - this->degree + i, this->degree, nt);
		tBS(i) *= this->closedCtrlLn->getWeight(tIndex - this->degree + i);
	}
	mat NurbsPoint = tBS.t()*tCtrls;
	NurbsPoint /= accu(tBS);
	return (rowvec)(NurbsPoint.row(0));
}

/*********************************************************/
/*					Méthodes privées					 */
/*********************************************************/

int NurbsCurve::getBaseKnotIndex(float nt, int maxIndex) // L'INDEX DU PLUS GRAND NOEUD INFERIEUR A nt
{
	if (nt >= this->Knot(maxIndex)) return maxIndex;
	else return getBaseKnotIndex(nt, maxIndex-1);
}

int NurbsCurve::getBaseClosedKnotIndex(float nt, int maxIndex) // L'INDEX DU PLUS GRAND NOEUD INFERIEUR A nt
{
	if (nt >= this->closedKnot(maxIndex)) return maxIndex;
	else return getBaseClosedKnotIndex(nt, maxIndex-1);
}

void NurbsCurve::updateClosedCtrls()
{
	this->closedCtrlLn->reset();
	for (int i=0; i < this->CtrlLn->getSize(); i++) this->closedCtrlLn->add(this->CtrlLn->getPointAt(i));
	for (int i=0; i < this->degree; i++) this->closedCtrlLn->add(this->CtrlLn->getPointAt(i));
}

float NurbsCurve::BasisFunction(int i, int d, float t) //-- LA FONCTION RECURSIVE DES FONCTIONS DE BASE
{
	if (d == 0) {
		if ((t < this->Knot(i))||(t >= this->Knot(i+1))) return 0.0f;
		else return 1.0f;
	}
	else if (((this->Knot(i+d) - this->Knot(i))!=0.0f)&&((this->Knot(i+d+1)-this->Knot(i+1))!=0.0f))
		return	((t - this->Knot(i)) / (this->Knot(i+d) - this->Knot(i))) * this->BasisFunction(i, d-1, t)
			   +((this->Knot(i+d+1) - t) / (this->Knot(i+d+1) - this->Knot(i+1))) * this->BasisFunction(i+1, d-1, t);
	else if (((this->Knot(i+d) - this->Knot(i))==0.0f)&&((this->Knot(i+d+1)-this->Knot(i+1))==0.0f))
		return 0.0f;
	else if ((this->Knot(i+d) - this->Knot(i))==0.0f)
		return ((this->Knot(i+d+1) - t) / (this->Knot(i+d+1) - this->Knot(i+1))) * this->BasisFunction(i+1, d-1, t);
	else return	((t - this->Knot(i)) / (this->Knot(i+d) - this->Knot(i))) * this->BasisFunction(i, d-1, t);
}

float NurbsCurve::closedBasisFunction(int i, int d, float t) //-- LA FONCTION RECURSIVE DES FONCTIONS DE BASE
{
	if (d == 0) {
		if ((t < this->closedKnot(i))||(t >= this->closedKnot(i+1))) return 0.0f;
		else return 1.0f;
	}
	else if (((this->closedKnot(i+d) - this->closedKnot(i))!=0.0f)
			 &&((this->closedKnot(i+d+1)-this->closedKnot(i+1))!=0.0f))
		return	((t - this->closedKnot(i)) / (this->closedKnot(i+d) - this->closedKnot(i)))
				* this->closedBasisFunction(i, d-1, t)
			    +((this->closedKnot(i+d+1) - t) / (this->closedKnot(i+d+1) - this->closedKnot(i+1)))
			    * this->closedBasisFunction(i+1, d-1, t);
	else if (((this->closedKnot(i+d) - this->closedKnot(i))==0.0f)
			 &&((this->closedKnot(i+d+1)-this->closedKnot(i+1))==0.0f))
		return 0.0f;
	else if ((this->closedKnot(i+d) - this->closedKnot(i))==0.0f)
		return ((this->closedKnot(i+d+1) - t) / (this->closedKnot(i+d+1) - this->closedKnot(i+1)))
				* this->closedBasisFunction(i+1, d-1, t);
	else return	((t - this->closedKnot(i)) / (this->closedKnot(i+d) - this->closedKnot(i)))
				 * this->closedBasisFunction(i, d-1, t);
}

void NurbsCurve::displayKnots()		//--------------------- AFFICHAGE DES NOEUDS ET DES FONCTIONS DE BASE
{
	char writeBuffer[15] = {'\0'};
	for(int i=0; i < this->Knot.n_elem; i++) {
		sprintf(writeBuffer, "%d", i);
		glColor3fv(this->CtrlLn->ptColor);
		this->renderBitmapString(this->getKnotVertex2D(i)[0] - 9.0f,
								 this->getKnotVertex2D(i)[1] + 9.0f,
								 GLUT_BITMAP_HELVETICA_18,
								 writeBuffer);
		glBegin(GL_POINTS);					
			glVertex2fv(this->getKnotVertex2D(i));
		glEnd();		
		glColor3fv(this->CtrlLn->lnColor);
		sprintf(writeBuffer, "%.1f", (float)(this->Knot(i)));
		this->renderBitmapString(this->getKnotVertex2D(i)[0] - 9.0f,
								 this->getKnotVertex2D(i)[1] - 24.0f,
								 GLUT_BITMAP_HELVETICA_18,
								 writeBuffer);
		if (i > 0) {
			glPushAttrib(GL_ENABLE_BIT);
				glLineStipple(4, 0x8888);
				glEnable(GL_LINE_STIPPLE);
				glBegin(GL_LINES);
					glVertex2fv(this->getKnotVertex2D(i-1));
					glVertex2fv(this->getKnotVertex2D(i));
				glEnd();
			glPopAttrib();
		}
	}
	vec prev_tBS = zeros(this->degree + 1);
	float prev_nt = 0.0f;
	int prev_tIndex = 0;
	for (float t=0.0f; t<=1.0f; t+=0.001f) {
		vec tBS = zeros(this->degree + 1);
		float nt =   (float)this->Knot(this->degree)
			   	   + t * (float)(this->Knot(this->CtrlLn->getSize())-this->Knot(this->degree));
		int tIndex = this->getBaseKnotIndex(nt, this->CtrlLn->getSize());
		for(int i=0; i < tBS.n_elem; i++) {
			tBS[i] = this->BasisFunction(tIndex - this->degree + i, this->degree, nt);
			if ((t>0.0f)&&((tIndex==prev_tIndex)||(this->Knot[tIndex]==this->Knot[prev_tIndex]))) {
				glBegin(GL_LINES);
					glVertex2f(this->Xspan*(prev_nt + 1.0f), 50.0f+120.0f*prev_tBS[i]);
					glVertex2f(this->Xspan*(nt + 1.0f), 50.0f+120.0f*tBS[i]);
				glEnd();
			}
		}
		prev_nt = nt;
		prev_tBS = tBS;
		prev_tIndex = tIndex;
	}
}
