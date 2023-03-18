#include "Curve.h"

using namespace std;
using namespace arma;

int Curve::curveCount = 0;

/*********************************************************/
/*					Constructeurs						 */
/*********************************************************/

Curve::Curve()
{
	++curveCount;
}

Curve::Curve(ControlLine* init_CL): CtrlLn(init_CL)
{
	this->curveRank = Curve::getCurveCount();
}
								
									
/*********************************************************/
/*					Destructeur							 */
/*********************************************************/
				
Curve::~Curve()
{
	this->visible = false;
	delete this->CtrlLn;
	delete this->curveColor;
	--curveCount;
}


/*********************************************************/
/*					Méthodes publiques					 */
/*********************************************************/

void Curve::changeVisibility()
{
	if (this->visible) this->visible = false;
	else this->visible = true;
}

void Curve::setVisible()
{
	this->visible = true;
}

bool Curve::isVisible() const
{
	return this->visible;
}

void Curve::display2D()
{
	if ((this->visible)&&(this->CtrlLn->getSize()>0)) {
		rowvec prevPoint = this->compute(0.0f);
		glColor3fv(this->curveColor);
		for (float t=0.001f; t<=1.0f; t+=0.001f) {
			rowvec crntPoint = this->compute(t);
			rowvec delta = crntPoint-prevPoint;
			float dist = sqrt(pow(delta(0),2)+pow(delta(1),2));
			if (dist<12.0f) {
				glBegin(GL_LINES);
					glVertex2f((float)prevPoint(0), (float)prevPoint(1));
					glVertex2f((float)crntPoint(0), (float)crntPoint(1));
				glEnd();
			}	
			prevPoint = crntPoint;
		}
		glColor3fv(this->curveColor);
		this->renderBitmapString(144.0f*(this->curveRank)-72.0f,
								 224.0f,
								 GLUT_BITMAP_HELVETICA_18,
								 this->Label.c_str());
	}
}


/*********************************************************/
/*					Méthodes protégées					 */
/*********************************************************/

void Curve::renderBitmapString(float x, float y, void *font, const char *string) const
{
	const char *c;
	glRasterPos2f(x, y);
	for (c = string; *c != '\0'; c++) {
		glutBitmapCharacter(font, *c);
	}
}

int Curve::getCurveCount()
{
	return curveCount;
}
