#include <stdlib.h>
#include <math.h>
#include <glut.h>
#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

int windowWidth = 1600;
int windowHeight = 900;

float viewAngHor = 78.53982; // Camera Angle Horizontal

float radZ = 45; // Camera Rotarion Radius Z
float radX = 100; // Camera Rotarion Radius X
float radY = 70; // Camera Rotation Redius Y

int rotateFlag; // Camera Rotation Flag
int zoomFlag; // Camera Zoom Flag
int heightFlag = 0; // Camera Height Flag
int pauseFlag = 0; // Pause Game Flag
int cameraFlag = 1; // Camera Control Flag
int cinematicFlag = 0; // Cinematic Camera Motion Flag
int enterSwitch = 1; // Decieds what the arrow keys navigate 1-Board 2-Buttons
int selectedButton = 1; // Selected HUD Button
int SBtmp = 1; // tmp for saving last selected button
int randomAttackerStart; // Lane Number of the new Attacker
int firstSpawnTime = 1000; // Value is only for the 1st waiting
float attackerSpawnTime = 1000;  // Time between Attacker Spawning
float difficulty = 0.93; // Difficulty increases with more Attackers Spawning

float centerX = 50.0f, centerZ = 25.0f; // Center Position
float eyeX, eyeY = radY, eyeZ; // Eye Position
float tmpCinematicCamera; // Saves position before Cinematic Motion

float rotAnimAng; //Rotation Angle Animation
float fallAnim = 100; //Resource Falling Animation
float bulletAnim = 0; //Defender Bullet Animation

float rotSpeed = 0.02; // Rotation Speed
float zoomSpeed = 1.05; // Zoom Speed
float attackerSpeed = 0.1; // Attacker Walking Speed

int resourcesNum = 50; // Number of Resources Gathered
int attackersNum = 0; // Number of Killed Attackers
int plantPrice = 10; // Plant Price
int defenderPrice = 50; // Defender Price

int winOrLose = 0; // Decides which Game Over message appears

int highlightedTileX = 5; // The Highlight Tile X Position
int highlightedTileZ = 3; // The Highlight Tile Z Position
float hitPoints[5] = {100, 100, 100, 100, 100}; // Places to Hit
int deadLanes[5] = { 1, 1, 1, 1, 1}; // Dead Lanes

int firstCtrl = 0;   // Column Number input for the stupid controls
int secontCtrl = 0;  // Column Number input for the stupid controls

int q = 0; // Never Mind ..... Debug tool

/////////////////////////Helper Methods ///////////////////////

///////////////////////////Classes/////////////////////////////

///////Piece Class////////
class Piece {
protected:
	float positionX, positionZ;
	int boolDead;
public:
	float getPositionX(void) {
		return positionX;
	}
	float getPositionZ(void){
		return positionZ;
	}
	void setBoolDead() {
		boolDead = 1;
	}
	int getBoolDead() {
		return boolDead;
	}
	int getColNum(){
		return (positionX / 10) + 1;
	}
	int getRowNum(){
		return (positionZ / 10) + 1;
	}
};

///////Plant Class////////
class Plant : public Piece {
protected:
	float resourceYposition = 100;
public:
	Plant(float x, float z) {
		positionX = (x * 10) - 5;
		positionZ = (z * 10) - 5;
		boolDead = 0;
		resourceYposition = 100;
	}
	void setPosition(float x, float z) {
		if (boolDead == 0)
		{
			drawPlant(resourceYposition);
			if (resourceYposition < 10){
				resourceYposition = 100;
				resourcesNum += 5;
			}
			resourceYposition -= 0.3;
		}
	}
	void drawResource(float x, float y, float z);
	void drawPlant(float resourceYposition);
};

///////Defender Class////////
class Defender : public Piece {
protected:
	float bulletXposition = 5;
	int integerPositionZ = positionZ;
	bool boolDrawBullet;
public:
		Defender(float x, float z) {
		positionX = (x * 10) - 5;
		positionZ = (z * 10) - 5;
		boolDead = 0;
		bulletXposition = x;
		boolDrawBullet = false;
	}
	int getBulletXposition() {
		return bulletXposition;
	}
	bool getBoolDrawBullet() {
		return boolDrawBullet;
	}
	void setBoolDrawBullet(bool input) {
		boolDrawBullet = input;
	}
	void setPosition(float x, float z) {
		if (boolDead == 0)
		{
			if (bulletXposition > hitPoints[(int(z + 5) / 10) - 1] - positionX) boolDrawBullet = false; else boolDrawBullet = true;
			drawDefender(bulletXposition, boolDrawBullet);
			if (bulletXposition > 100) bulletXposition = 0;
			bulletXposition += 1.5;
		}
	}
	void drawBullet(float x, float y, float z);
	void drawDefender(float bulletXposition, bool drawBullet);
};

///////Attacker Class////////
class Attacker : public Piece {
protected:
	float health;
public:
	Attacker(float x, float z) {
		positionX = x;
		positionZ = (z * 10) - 5;
		boolDead = 0;
		health = 700;
	}
	int getHealth() {
		return health;
	}
	void hit() {
		if (health != 0)
		{
			health -= 20;
		}
	}
	void setPosition(float x, float z) {
		if (boolDead == 0)
		{
			drawAttacker(health);
		}
	}
	void walk() {
		positionX = positionX - attackerSpeed;
	}
	void drawAttacker(float health);
};

///////Resource Class////////
class Resource : public Piece {
protected:
	float resourceYposition = 100;
public:
	Resource() {
		positionX = (rand() % 100);
		positionZ = (rand() % 100) / 2;
	}
	void dropResource() {
		drawResource(positionX, resourceYposition, positionZ);
		if (resourceYposition < 0){
			resourceYposition = 100;
			resourcesNum += 10;
			positionX = (rand() % 100);
			positionZ = (rand() % 100) / 2;
		}
		resourceYposition -= 0.2;
	}
	void drawResource(float x, float y, float z);
};

///////Tile Class////////
class Tile {
protected:
	float rowNum, colNum;
	int state;
	// 0 means Available
	// 1 means Highlighted
	// 2 means Dead
	bool isOccupied;
	int pieceCode;
	// 1 Plant
	// 2 Defender
public:
	Tile() {
		rowNum = colNum = 0;
		state = 0;
		isOccupied = false;
		pieceCode = 0;
	}
	Tile(float x, float z) {
		colNum = x;
		rowNum = z;
		state = 0;
		isOccupied = false;
		pieceCode = 0;
	}
	float getRowNum(void) { return rowNum; }
	float getColNum(void) { return colNum; }
	int getState() { return state; }
	bool getOccupied() { return isOccupied; }
	int getPieceCode() { return pieceCode; }
	void setColNum(int x) { colNum = x; }
	void setRowNum(int z) { rowNum = z; }
	void setState(int input) { state = input; }
	void setOccupied(bool input) { isOccupied = input; }
	void setPieceCode(int input) { pieceCode = input; }
	void drawTile();
};

///////Board Class///////
class Board {
protected:
	Tile board[10][5];
	int highlightFlag = 0;
public:
	Board() {
		for (int i = 0; i < 10; i++)
		{
			for (int j = 0; j < 5; j++)
			{
				board[i][j].setColNum(j);
				board[i][j].setRowNum(i);
				board[i][j].setState(0);
			}
		}
	}
	Tile* getTile(int x, int z) {
		return &board[x - 1][z - 1];
	}
	int getState(int x, int z) {
		return board[x - 1][z - 1].getState();
	}
	void highlight(int x, int z) {
		if (board[x - 1][z - 1].getState() != 2 && highlightFlag == 0) {
			board[x - 1][z - 1].setState(1);
			highlightFlag = 1;
		}
	}
	void dehighlight(int x, int z) {
		if (board[x - 1][z - 1].getState() != 2 && highlightFlag == 1) {
			board[x - 1][z - 1].setState(0);
			highlightFlag = 0;
		}
	}
	void setLaneDead(int z) {
		for (int i = 0; i < 10; i++)
		{
			board[i][z].setState(2);
			deadLanes[z] = 0;
		}
	}
	void drawTiles() {
		glDisable(GL_LIGHTING);
		drawGround();
		for (int i = 0; i < 10; i++)
		{
			for (int j = 0; j < 5; j++)
			{
				 switch (board[i][j].getState()) {
					case 0:
						break;
					case 1:
						board[i][j].drawTile();
						break;
					case 2:
						board[i][j].drawTile();
						break;
					default:
						break;
				}
			}
		}
		glEnable(GL_LIGHTING);
	}
	void drawGround();
};

//////////////////////////Drawing Methods///////////////////////////////
void Plant::drawResource(float x, float y, float z) {
	glPushMatrix();
	glTranslated(x, y, z);
	glRotatef(rotAnimAng, 0, 1, 0);

	glPushMatrix();
	glColor3f(1.0f, 1.0f, 0.0f);
	glutSolidOctahedron();
	glPopMatrix();

	glPushMatrix();
	glColor3f(0.0f, 0.0f, 0.0f);
	glutWireOctahedron();
	glPopMatrix();

	glPopMatrix();
}

void Resource::drawResource(float x, float y, float z) {
	glPushMatrix();
	glTranslated(x, y, z);
	glRotatef(rotAnimAng, 0, 1, 0);

	glPushMatrix();
	glColor3f(1.0f, 1.0f, 0.0f);
	glutSolidOctahedron();
	glPopMatrix();

	glPushMatrix();
	glColor3f(0.0f, 0.0f, 0.0f);
	glutWireOctahedron();
	glPopMatrix();

	glPopMatrix();
}

void Plant::drawPlant(float resourceYposition) {

	glPushMatrix();
	drawResource(positionX, resourceYposition, positionZ);
	glTranslatef(positionX, 10, positionZ);
	glRotatef(rotAnimAng, 0, 1, 0);
	glColor3f(1.0f, 1.0f, 0.0f);
	glScalef(0.9, 0.9, 0.9);
	glutSolidDodecahedron();
	glPopMatrix();


	glPushMatrix();
	glTranslatef(positionX, 10, positionZ);
	glRotatef(rotAnimAng, 0, 1, 0);
	glColor3f(0.0f, 0.0f, 0.0f);
	glScalef(0.9, 0.9, 0.9);
	glutWireDodecahedron();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(positionX, 10, positionZ);
	glRotatef(rotAnimAng, 0, 1, 0);
	float j = 0;
	for (float i = 0; i< 1800; i += 10) {
		glPushMatrix();
		glRotatef(i, 0, 1, 0);
		glTranslatef(0.5, j, 0);
		glColor3f(0.0f, 0.58f, 0.136f);
		glutSolidSphere(0.4, 7, 7);
		glPopMatrix();
		j -= 0.05;
	}
	glPopMatrix();
}

void Defender::drawBullet(float x, float y, float z) {
	glPushMatrix();
	glTranslated(x, y, z);

	glPushMatrix();
	glColor3f(0.0f, 1.0f, 1.0f);
	glutSolidSphere(1, 10, 10);
	glPopMatrix();

	glPopMatrix();
}

void Defender::drawDefender(float bulletXposition, bool withBullet) {

	glPushMatrix();
	if (withBullet) drawBullet(positionX + bulletXposition, 10, positionZ);
	glTranslatef(positionX, 10, positionZ);
	glPushMatrix();
	glColor3f(0.0f, 0.0f, 1.0f);
	glutSolidSphere(1.2, 16, 16);

	glPushMatrix();
	glRotatef(90, 0, 1, 0);
	glColor3f(0.0f, 1.0f, 1.0f);
	GLUquadric* qobj = gluNewQuadric();
	gluQuadricNormals(qobj, GLU_SMOOTH);
	gluCylinder(qobj, 1, 1, 2, 16, 16);

	glPushMatrix();
	glRotatef(90, 1, 0, 0);
	glColor3f(0.0f, 0.0f, 1.0f);
	GLUquadric* qobj3 = gluNewQuadric();
	gluQuadricNormals(qobj3, GLU_SMOOTH);
	gluCylinder(qobj3, 1, 1.2, 10, 16, 16);

	glPopMatrix();
	glPopMatrix();
	glPopMatrix();
	glPopMatrix();
}

void Attacker::drawAttacker(float health) {

	float HP =  1 - (health/700);

	glPushMatrix();
	glTranslated(positionX, 10, positionZ);
	glScalef(1.3, 1.3, 1.3);

	glPushMatrix();
	glScalef(1.2, 1.2, 1.2);
	glColor3f(HP, 0.0f, 0.0f);
	glutSolidDodecahedron();
	glPopMatrix();

	glPushMatrix();
	glScalef(1.2, 1.2, 1.2);
	glColor3f(1.0f, 0.0f, 0.0f);
	glutWireDodecahedron();
	glPopMatrix();

	glPushMatrix();
	glColor3f(HP, 0.0f, 0.0f);
	glScaled(6.0, 4, 4.0);
	glTranslated(0, -0.7, 0);
	glutSolidCube(1);
	glPopMatrix();

	glPushMatrix();
	glColor3f(1.0f, 0.0f, 0.0f);
	glScaled(6.0, 4, 4.0);
	glTranslated(0, -0.7, 0);
	glutWireCube(1);
	glPopMatrix();

	// 4 Wheels
	glPushMatrix();
	glTranslatef(-1.75, -4.5, 2);
	glColor3f(HP, 0.0f, 0.0f);
	GLUquadric* qobj = gluNewQuadric();
	gluQuadricNormals(qobj, GLU_SMOOTH);
	gluCylinder(qobj, 1.2, 1.2, 1.2, 16, 16);
	glTranslated(0.0, 0.0, 1.2);
	gluDisk(qobj, 0.0, 1.2, 16, 16);
	glTranslated(0.0, 0.0, 0.05);
	glColor3f(1.0f, 0.0f, 0.0f);
	gluDisk(qobj, 0.9, 1.2, 16, 16);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(1.75, -4.5, 2);
	glColor3f(HP, 0.0f, 0.0f);
	GLUquadric* qobj2 = gluNewQuadric();
	gluQuadricNormals(qobj2, GLU_SMOOTH);
	gluCylinder(qobj2, 1.2, 1.2, 1.2, 16, 16);
	glTranslated(0.0, 0.0, 1.2);
	gluDisk(qobj2, 0.0, 1.2, 16, 16);
	glTranslated(0.0, 0.0, 0.05);
	glColor3f(1.0f, 0.0f, 0.0f);
	gluDisk(qobj, 0.9, 1.2, 16, 16);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-1.75, -4.5, -3.2);
	glColor3f(HP, 0.0f, 0.0f);
	GLUquadric* qobj3 = gluNewQuadric();
	gluQuadricNormals(qobj3, GLU_SMOOTH);
	gluCylinder(qobj3, 1.2, 1.2, 1.2, 20, 20);
	glTranslated(0.0, 0.0, 0);
	gluDisk(qobj3, 0.0, 1.2, 16, 16);
	glTranslated(0.0, 0.0, -0.05);
	glColor3f(1.0f, 0.0f, 0.0f);
	gluDisk(qobj, 0.9, 1.2, 16, 16);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(1.75, -4.5, -3.2);
	glColor3f(HP, 0.0f, 0.0f);
	GLUquadric* qobj4 = gluNewQuadric();
	gluQuadricNormals(qobj4, GLU_SMOOTH);
	gluCylinder(qobj4, 1.2, 1.2, 1.2, 16, 16);
	glTranslated(0.0, 0.0, 0);
	gluDisk(qobj4, 0.0, 1.2, 16, 16);
	glTranslated(0.0, 0.0, -0.05);
	glColor3f(1.0f, 0.0f, 0.0f);
	gluDisk(qobj, 0.9, 1.2, 16, 16);
	glPopMatrix();

	//3 Spikes
	for (int i = -1; i < 2; i++)
	{
		glPushMatrix();
		glTranslatef(-3, -2, i);
		glRotatef(-90, 0, 1, 0);
		glColor3f(1.0f, 0.0f, 0.0f);
		glutSolidCone(0.5, 2, 16, 16);
		glPopMatrix();
	}

	glPopMatrix();

}

void Tile::drawTile() {
	float r, g, b;
	if (state == 1)
	{
		r = 0.46; 
		g = 1;
		b = 0.289;
	}
	if (state == 2)
	{
		r = g = b = 0;
	}

	glPushMatrix();
	glColor3f(r, g, b);
	glBegin(GL_QUADS);
	glVertex3f(((rowNum + 1)* 10) - 10, 3, ((colNum + 1) * 10) - 10);
	glVertex3f(((rowNum + 1) * 10) - 10, 3, ((colNum + 1) * 10));
	glVertex3f(((rowNum + 1) * 10), 3, ((colNum + 1) * 10));
	glVertex3f(((rowNum + 1) * 10), 3, ((colNum + 1) * 10) - 10);
	glEnd();
	glPopMatrix();
}

void Board::drawGround() {

	glPushMatrix();
	glColor3f(0.9f, 0.9f, 0.9f);
	glTranslated(50, 1, 25);
	glScaled(100.0, 2, 50.0);
	glutSolidCube(1);
	glPopMatrix();

	for (int i = 0; i < 110; i+= 10)
	{
		glPushMatrix();
		glColor3f(0, 0, 0);
		glLineWidth(1.5);
		glBegin(GL_LINES);
		glVertex3f(i, 3, 0);
		glVertex3f(i, 3, 50);		
		glEnd();
		glLineWidth(1);
		glPopMatrix();
	}

	for (int j = 0; j < 60; j += 10)
	{
		glPushMatrix();
		glColor3f(0, 0, 0);
		glLineWidth(1.5);
		glBegin(GL_LINES);
		glVertex3f(0, 3, j);
		glVertex3f(100, 3, j);
		glEnd();
		glLineWidth(1);
		glPopMatrix();
	}
}

void changeSize(int w, int h) {

	// When a window of Zero width
	if (h == 0)
		h = 1;
	float ratio = w * 1.0 / h;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0, 0, w, h);
	gluPerspective(45.0f, ratio, 0.1f, 1000.0f);
	glMatrixMode(GL_MODELVIEW);
}

void setupLights() {
	GLfloat ambient[] = { 0.7f, 0.7f, 0.7, 1.0f };
	GLfloat diffuse[] = { 0.6f, 0.6f, 0.6, 1.0f };
	GLfloat specular[] = { 1.0f, 1.0f, 1.0, 1.0f };
	GLfloat shininess[] = { 50 };
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, shininess);

	GLfloat lightIntensity[] = { 100, 100, 100, 1.0f };
	GLfloat lightPosition[] = { 50.0f, 50.0f, 100.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, lightIntensity);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightIntensity);
}

void renderBitmapString(float x, float y, float z, void *font, char *string) {

		glPushMatrix();
		char *c;
		glRasterPos3f(x, y, z);
		for (c = string; *c != '\0'; c++) {
			glutBitmapCharacter(font, *c);
		}
		glPopMatrix();
}

void setOrthographicProjection() {

	glMatrixMode(GL_PROJECTION);

	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, windowWidth, windowHeight, 0);

	glMatrixMode(GL_MODELVIEW);
}

void restorePerspectiveProjection() {

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
}

void renderButton(float positionX, float positionY, bool selected, int height, int width) {
	int r, g, b;
	if (selected) {
		r = 0.46;
		g = 1;
		b = 0.7;
	}
	else r = g = b = 1;

	glPushMatrix();
	glColor3f(r, g, b);
	glBegin(GL_QUADS);
	glVertex3f(positionX, positionY, 0);
	glVertex3f(positionX + width, positionY, 0);
	glVertex3f(positionX + width, positionY + height, 0);
	glVertex3f(positionX, positionY + height, 0);
	glEnd();
	glPopMatrix();
}

void renderGameOver(int winOrLose) {
	glPushMatrix();

	if (winOrLose == 1) {
		glColor3f(1, 1, 1);
		renderBitmapString((windowWidth / 2) - 40, windowHeight / 2, 0, GLUT_BITMAP_HELVETICA_18, "VICTORY");

		glColor3f(0, 1, 0);
		glBegin(GL_QUADS);
		glVertex3f(0, 0, 0);
		glVertex3f(0, windowHeight, 0);
		glVertex3f(windowWidth, windowHeight, 0);
		glVertex3f(windowWidth, 0, 0);
		glEnd();
	}
	if (winOrLose == 2) {
		glColor3f(1, 1, 1);
		renderBitmapString((windowWidth / 2) - 40, windowHeight / 2, 0, GLUT_BITMAP_HELVETICA_18, "DEFEAT");

		glColor3f(1, 0, 0);
		glBegin(GL_QUADS);
		glVertex3f(0, 0, 0);
		glVertex3f(0, windowHeight, 0);
		glVertex3f(windowWidth, windowHeight, 0);
		glVertex3f(windowWidth, 0, 0);
		glEnd();
	}

	glPopMatrix();
}

void renderStats(int selectedButton) {

	//////// Icons on Buttons ////////////
	// Resources
	glPushMatrix();

	glLineWidth(3);
	glColor3f(0, 0, 0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBegin(GL_POLYGON);
	glVertex3f(650, 15, 0);
	glVertex3f(695, 60, 0);
	glVertex3f(650, 105, 0);
	glVertex3f(605, 60, 0);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f(695, 60, 0);
	glVertex3f(605, 60, 0);
	glEnd();
	glLineWidth(1);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glColor3f(1, 1, 0);
	glBegin(GL_QUADS);
	glVertex3f(650, 15, 0);
	glVertex3f(695, 60, 0);
	glVertex3f(650, 105, 0);
	glVertex3f(605, 60, 0);
	glEnd();

	char buffer[8];
	sprintf(buffer, "%d", resourcesNum);
	glColor3f(0, 0, 0);
	renderBitmapString(605, 105, 0, GLUT_BITMAP_HELVETICA_18, buffer);

	glPopMatrix();

	//Plant
	glPushMatrix();
	glLineWidth(3);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glColor3f(0, 0, 0);
	glBegin(GL_POLYGON);
	glVertex3f(750, 13, 0);
	glVertex3f(780, 35, 0);
	glVertex3f(760, 60, 0);
	glVertex3f(740, 60, 0);
	glVertex3f(720, 35, 0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(740, 62, 0);
	glVertex3f(760, 62, 0);
	glVertex3f(760, 105, 0);
	glVertex3f(740, 105, 0);
	glEnd();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glLineWidth(1);

	glColor3f(1, 1, 0);
	glBegin(GL_POLYGON);
	glVertex3f(750, 13, 0);
	glVertex3f(780, 35, 0);
	glVertex3f(760, 60, 0);
	glVertex3f(740, 60, 0);
	glVertex3f(720, 35, 0);
	glEnd();

	glColor3f(0, 1, 0);
	glBegin(GL_POLYGON);
	glVertex3f(740, 62, 0);
	glVertex3f(760, 62, 0);
	glVertex3f(760, 105, 0);
	glVertex3f(740, 105, 0);
	glEnd();

	glColor3f(0, 0.429, 0);
	sprintf(buffer, "%d", plantPrice);
	renderBitmapString(705, 105, 0, GLUT_BITMAP_HELVETICA_18, buffer);

	glPopMatrix();

	//Defender
	glPushMatrix();

	glLineWidth(3);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glColor3f(0, 0, 0);
	glBegin(GL_POLYGON);
	glVertex3f(850, 13, 0);
	glVertex3f(880, 35, 0);
	glVertex3f(860, 60, 0);
	glVertex3f(840, 60, 0);
	glVertex3f(820, 35, 0);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex3f(840, 62, 0);
	glVertex3f(860, 62, 0);
	glVertex3f(860, 105, 0);
	glVertex3f(840, 105, 0);
	glEnd();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glLineWidth(1);

	glColor3f(0, 1, 1);
	glBegin(GL_POLYGON);
	glVertex3f(850, 23, 0);
	glVertex3f(870, 35, 0);
	glVertex3f(860, 50, 0);
	glVertex3f(840, 50, 0);
	glVertex3f(830, 35, 0);
	glEnd();

	glColor3f(0, 0, 1);
	glBegin(GL_POLYGON);
	glVertex3f(850, 13, 0);
	glVertex3f(880, 35, 0);
	glVertex3f(860, 60, 0);
	glVertex3f(840, 60, 0);
	glVertex3f(820, 35, 0);
	glEnd();

	glBegin(GL_POLYGON);
	glVertex3f(840, 62, 0);
	glVertex3f(860, 62, 0);
	glVertex3f(860, 105, 0);
	glVertex3f(840, 105, 0);
	glEnd();

	sprintf(buffer, "%d", defenderPrice);
	renderBitmapString(805, 105, 0, GLUT_BITMAP_HELVETICA_18, buffer);

	glPopMatrix();

	//Attacker
	glPushMatrix();

	glLineWidth(3);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glColor3f(1, 0, 0);
	glBegin(GL_POLYGON);
	glVertex3f(950, 13, 0);
	glVertex3f(980, 35, 0);
	glVertex3f(960, 60, 0);
	glVertex3f(940, 60, 0);
	glVertex3f(920, 35, 0);
	glEnd();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glColor3f(0, 0, 0);
	glBegin(GL_POLYGON);
	glVertex3f(950, 13, 0);
	glVertex3f(980, 35, 0);
	glVertex3f(960, 60, 0);
	glVertex3f(940, 60, 0);
	glVertex3f(920, 35, 0);
	glEnd();

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glColor3f(1, 0, 0);
	glBegin(GL_POLYGON);
	glVertex3f(910, 52, 0);
	glVertex3f(990, 52, 0);
	glVertex3f(970, 95, 0);
	glVertex3f(930, 95, 0);
	glEnd();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glLineWidth(1);

	glColor3f(0, 0, 0);
	glBegin(GL_POLYGON);
	glVertex3f(910, 52, 0);
	glVertex3f(990, 52, 0);
	glVertex3f(970, 95, 0);
	glVertex3f(930, 95, 0);
	glEnd();

	sprintf(buffer, "%d", attackersNum);
	glColor3f(1, 0, 0);
	renderBitmapString(905, 105, 0, GLUT_BITMAP_HELVETICA_18, buffer);

	glPopMatrix();

	for (int j = 1; j < 4; j++)
	{
		glPushMatrix();
		glColor3f(0, 0, 0);
		glLineWidth(3);
		glBegin(GL_LINES);
		glVertex3f(600 + (j * 100), 10, 0);
		glVertex3f(600 + (j * 100), 110, 0);
		glEnd();
		glLineWidth(1);
		glPopMatrix();
	}

	glPushMatrix();
	glColor3f(0, 0, 0);
	renderBitmapString(770, 140, 0, GLUT_BITMAP_HELVETICA_18, "CLEAR");
	glPopMatrix();

	switch (selectedButton) {
	case 0:
		renderButton(700, 10, false, 100, 100);
		renderButton(800, 10, false, 100, 100);
		if (enterSwitch == 2) renderButton(750, 115, false, 40, 100);
		break;
	case 1: 
		renderButton(700, 10, true, 100, 100);
		renderButton(800, 10, false, 100, 100);
		if (enterSwitch == 2) renderButton(750, 115, false, 40, 100);
		break;
	case 2: 
		renderButton(700, 10, false, 100, 100);
		renderButton(800, 10, true, 100, 100);
		if (enterSwitch == 2) renderButton(750, 115, false, 40, 100);
		break;
	case 3:
		renderButton(700, 10, false, 100, 100);
		renderButton(800, 10, false, 100, 100);
		if (enterSwitch == 2) renderButton(750, 115, true, 40, 100);
		break;
	}
	renderButton(600, 10, false, 100, 100);
	renderButton(900, 10, false, 100, 100);
}

void renderControls() {
	
	glPushMatrix();
	glColor3f(0, 1, 0);
	renderBitmapString(100, 810, 0, GLUT_BITMAP_HELVETICA_18, "Camera Controls :");
	if (!cameraFlag) renderBitmapString(260, 810, 0, GLUT_BITMAP_HELVETICA_18, "B - Disable Camera Controls");
	else renderBitmapString(260, 810, 0, GLUT_BITMAP_HELVETICA_18, "B - Enable Camera Controls");
	renderBitmapString(260, 830, 0, GLUT_BITMAP_HELVETICA_18, "V - Cinematic Camera");
	renderBitmapString(260, 850, 0, GLUT_BITMAP_HELVETICA_18, "X - Reset Camera Position");
	if (!cameraFlag) {
		renderBitmapString(260, 870, 0, GLUT_BITMAP_HELVETICA_18, "W/S - Height");
		renderBitmapString(260, 890, 0, GLUT_BITMAP_HELVETICA_18, "Up , Down , Right, Left");
	}

	renderBitmapString(800, 810, 0, GLUT_BITMAP_HELVETICA_18, "Game Controls :");
	renderBitmapString(950, 810, 0, GLUT_BITMAP_HELVETICA_18, "Up , Down , Right, Left");
	if (enterSwitch == 2) renderBitmapString(950, 830, 0, GLUT_BITMAP_HELVETICA_18, "ENTER - Select Plant");
	else renderBitmapString(950, 830, 0, GLUT_BITMAP_HELVETICA_18, "ENTER - Select Tile");
	renderBitmapString(950, 850, 0, GLUT_BITMAP_HELVETICA_18, "P - Pause Game");
	if (enterSwitch == 2) renderBitmapString(950, 870, 0, GLUT_BITMAP_HELVETICA_18, "ESC - Back to Select Tile");
	else renderBitmapString(950, 870, 0, GLUT_BITMAP_HELVETICA_18, "ESC - Exit");
	glPopMatrix();
}

void renderHUD() {
	if (!cinematicFlag) {
		glDisable(GL_LIGHTING);
		setOrthographicProjection();
		glPushMatrix();
		glLoadIdentity();

		renderGameOver(winOrLose);
		renderStats(selectedButton);
		renderControls();

		glPopMatrix();
		restorePerspectiveProjection();
		glEnable(GL_LIGHTING);
	}
}

/////////////////////// Initialization ////////////////////
Board board = Board();
Resource randomResource = Resource();
vector<Plant> plantVector;
vector<Defender> defenderVectorOld;
vector<Defender> defenderVector;
vector<Attacker> Attackers[4];

void Display() {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	eyeX = centerX + radX * cos(viewAngHor * rotSpeed);
	eyeZ = centerZ + radZ * sin(viewAngHor * rotSpeed);
	
	// Don't know why restriction works only here instead of Anim function 
	if (radY < 500) eyeY = radY;
	
	gluLookAt(	eyeX, eyeY, eyeZ,
				centerX, 0, centerZ,
				0.0f, 1.0f, 0.0f);

	//Draw Here
	renderHUD();
	board.drawTiles();
	randomResource.dropResource();

	//Plants and Defenders drawing
	for (int i = 0; i != plantVector.size(); i++) {
		plantVector.at(i).setPosition(plantVector.at(i).getPositionX(), plantVector.at(i).getPositionZ());
	}

	// The Unsolveable Strange Bug
	//for (std::vector<int>::size_type j = 0; j != defenderVectorOld.size(); j++) {
	//	defenderVectorOld.at(j).setPosition(defenderVectorOld.at(j).getPositionX(), defenderVectorOld.at(j).getPositionZ());
	//}

	for (int j = 0; j != defenderVector.size(); j++) {
		defenderVector.at(j).setPosition(defenderVector.at(j).getPositionX(), defenderVector.at(j).getPositionZ());
	}

	//Attackers drawing
	for (int i = 0; i < 5; i++) {
		vector <int> deadAttackers;
		vector <int> deadDefenders;
		vector <int> deadPlants;
		bool tmpDeadLane = false; // indicates if the ith lane is dead

		if (deadLanes[i] == 0) {
			Attackers[i].clear();
		}
		for (int j = 0; j != Attackers[i].size(); j++) {
			Attackers[i].at(j).setPosition(Attackers[i].at(j).getPositionX(), Attackers[i].at(j).getPositionZ());
			Attackers[i].at(j).walk();
			// Setting Hit Points
			hitPoints[i] = Attackers[i].at(0).getPositionX() + 1;
			// Attacker made it to the House
			if (Attackers[i].at(j).getPositionX() < 0) {
				deadAttackers.push_back(j);
				board.setLaneDead(i);
				tmpDeadLane = true;
			}
			// Attacker hitting a Defender or being hit
			for (int k = 0; k != defenderVector.size(); k++) {
				if (tmpDeadLane && defenderVector.at(k).getRowNum() - 1 == i) deadDefenders.push_back(k);
				else {
					if (defenderVector.at(k).getRowNum() == i + 1
						&& defenderVector.at(k).getPositionX() >= Attackers[i].at(j).getPositionX()
						&& defenderVector.at(k).getPositionX() < Attackers[i].at(j).getPositionX() + 5) {

						board.getTile(defenderVector.at(k).getColNum(), defenderVector.at(k).getRowNum())->setOccupied(false);
						board.getTile(defenderVector.at(k).getColNum(), defenderVector.at(k).getRowNum())->setPieceCode(0);
						deadDefenders.push_back(k);
					}
				}
				if (defenderVector.at(k).getRowNum() == i + 1
					&& defenderVector.at(k).getBulletXposition() + defenderVector.at(k).getPositionX() >= Attackers[i].at(j).getPositionX() - 5) {
					if (defenderVector.at(k).getBoolDrawBullet()){
						Attackers[i].at(j).hit();
					}
				}
			}
			// Attacker hitting a Plant
			for (int k = 0; k != plantVector.size(); k++) {
				if (tmpDeadLane && plantVector.at(k).getRowNum() - 1 == i) deadPlants.push_back(k);
				else {
					if (plantVector.at(k).getRowNum() == i + 1
						&& plantVector.at(k).getPositionX() >= Attackers[i].at(j).getPositionX()
						&& plantVector.at(k).getPositionX() < Attackers[i].at(j).getPositionX() + 5) {

						board.getTile(plantVector.at(k).getColNum(), plantVector.at(k).getRowNum())->setOccupied(false);
						board.getTile(plantVector.at(k).getColNum(), plantVector.at(k).getRowNum())->setPieceCode(0);
						deadPlants.push_back(k);
					}
				}
			}
			// Attacker is Dead
			if (Attackers[i].at(j).getHealth() == 0) {
				deadAttackers.push_back(j);
				attackersNum++;
				resourcesNum += 5;
				if (Attackers[i].empty()) hitPoints[i] = 100;
			}
		}
		// Deleting Dead Attackers
		for (int j = 0; j != deadAttackers.size(); j++) {
			Attackers[i].erase(Attackers[i].begin() + deadAttackers.at(j));
		}
		// Deleting Dead Defenders
		for (int j = 0; j != deadDefenders.size(); j++) {
			defenderVector.erase(defenderVector.begin() + deadDefenders.at(j));
		}
		// Deleting Dead Plants
		for (int j = 0; j != deadPlants.size(); j++) {
			plantVector.erase(plantVector.begin() + deadPlants.at(j));
		}
		// Resetting Hit Points if Empty
		if (Attackers[i].empty()) hitPoints[i] = 100;
	}

	//Spawning a New Attacker
	if (firstSpawnTime < 1) {
		firstSpawnTime = attackerSpawnTime;
		attackerSpawnTime = difficulty*attackerSpawnTime;
		int randomLane = ((rand() % 10)) / 2; // Random Number from 0 to 4
		if (deadLanes[randomLane] == 1) Attackers[randomLane].push_back(Attacker(100, randomLane + 1));
	}
	firstSpawnTime--;

	//Checking if Win or Lose
	//Lose Case
	int tmpSum = 0;
	for (int i = 0; i < 5; i++){
		tmpSum += deadLanes[i];
	}
	if (tmpSum < 3) winOrLose = 2;
	//Win Case
	if (attackersNum == 50) winOrLose = 1;


	glutSwapBuffers();
}

void Anim() {

	if (!pauseFlag && !cameraFlag)
	{
		switch (rotateFlag) {
		case 1: viewAngHor += 3;
			break;
		case 2: viewAngHor -= 3;
			break;
		}

		switch (zoomFlag) {
		case 1:
			if (radX < 700 && radZ < 400 && radY < 500)
			{
				radZ = radZ*zoomSpeed;
				radX = radX*zoomSpeed;
				radY = radY*zoomSpeed;
			}
			break;
		case 2:
			if (eyeY > 4 && radZ > 1 && radX > 1)
			{
				radZ = radZ / zoomSpeed;
				radX = radX / zoomSpeed;
				radY = radY / zoomSpeed;
			}
			break;
		default:
			break;
		}

		switch (heightFlag) {
		case 1:
			radY = radY*zoomSpeed;
			break;
		case 2:
			if (eyeY > 4)
			{
				radY = radY / zoomSpeed;
			}
			break;
		default:
			break;
		}
	}
	if (!pauseFlag && cinematicFlag) {
		viewAngHor += 0.5;
		if (viewAngHor > tmpCinematicCamera + 314 )
		{
			cinematicFlag = 0;
			viewAngHor = tmpCinematicCamera;
		}
	}

	if (pauseFlag == 0)
	{
		switch (enterSwitch) {
		case 1:
			board.highlight(highlightedTileX, highlightedTileZ);
			selectedButton = 3;
			break;
		case 2:
			break;
		default:
			break;
		}

		//Animation
		rotAnimAng += 3;

		glutPostRedisplay();
	}
}

/////////////////////////////// Control Methods /////////////////////////////////////////

void Key(unsigned char key, int x, int y) {

	switch (key) {
		case 'w': heightFlag = 1;
			break;
		case 's': heightFlag = 2;
			break;
		case 'p': 
			pauseFlag = !pauseFlag;
			break;
		case 'b':
			cameraFlag = !cameraFlag;
			if (enterSwitch == 0 || enterSwitch == 2) enterSwitch = 1; else enterSwitch = 0;
			break;
		case 'x':
			cinematicFlag = 0;
			viewAngHor = 78.53982;
			radZ = 45;
			radX = 100;
			radY = 70;
			cameraFlag = 1;
			break;
		case 'v': 
			cinematicFlag = 1;
			cameraFlag = 1;
			tmpCinematicCamera = viewAngHor;
			break;
		case 'm':
			resourcesNum = 1000;
			break;
		case 13: 
			switch (enterSwitch) {
			case 1: enterSwitch = 2;
				break;
			case 2: 
				switch (selectedButton) {
				case 1: 
					if (resourcesNum >= plantPrice 
						&& !board.getTile(highlightedTileX, highlightedTileZ)->getOccupied()
						&& board.getTile(highlightedTileX, highlightedTileZ)->getState() != 3
						&& deadLanes[highlightedTileZ - 1] == 1)
					{
						plantVector.push_back(Plant(highlightedTileX, highlightedTileZ));
						board.getTile(highlightedTileX, highlightedTileZ)->setOccupied(true);
						board.getTile(highlightedTileX, highlightedTileZ)->setPieceCode(1);
						resourcesNum -= plantPrice;
					}
					break;
				case 2: 
					if (resourcesNum >= defenderPrice
						&& !board.getTile(highlightedTileX, highlightedTileZ)->getOccupied()
						&& board.getTile(highlightedTileX, highlightedTileZ)->getState() != 3
						&& deadLanes[highlightedTileZ - 1] == 1)
					{
						defenderVector.push_back(Defender(highlightedTileX, highlightedTileZ));
						board.getTile(highlightedTileX, highlightedTileZ)->setOccupied(true);
						board.getTile(highlightedTileX, highlightedTileZ)->setPieceCode(2);
						resourcesNum -= defenderPrice;
					}
					break;
				case 3:
					if (board.getTile(highlightedTileX, highlightedTileZ)->getPieceCode() == 1
						&& board.getTile(highlightedTileX, highlightedTileZ)->getOccupied()
						&& board.getTile(highlightedTileX, highlightedTileZ)->getState() != 3)
					{
						for (int i = 0; i != plantVector.size(); i++) {
							if (plantVector.at(i).getColNum() == highlightedTileX && plantVector.at(i).getRowNum() == highlightedTileZ) {
								plantVector.erase(plantVector.begin() + i);
								board.getTile(highlightedTileX, highlightedTileZ)->setOccupied(false);
								board.getTile(highlightedTileX, highlightedTileZ)->setPieceCode(0);
								break;
							}
						}
						
					}
					if (board.getTile(highlightedTileX, highlightedTileZ)->getPieceCode() == 2
						&& board.getTile(highlightedTileX, highlightedTileZ)->getOccupied()
						&& board.getTile(highlightedTileX, highlightedTileZ)->getState() != 3)
					{
						for (int i = 0; i != defenderVector.size(); i++) {
							if (defenderVector.at(i).getColNum() == highlightedTileX && defenderVector.at(i).getRowNum() == highlightedTileZ) {
								defenderVector.erase(defenderVector.begin() + i);
								board.getTile(highlightedTileX, highlightedTileZ)->setOccupied(false);
								board.getTile(highlightedTileX, highlightedTileZ)->setPieceCode(0);
								break;
							}
						}
					}
					break;
				}
				enterSwitch = 1;
				board.dehighlight(highlightedTileX, highlightedTileZ);
				selectedButton = 0;
				break;
			}
			break;
		case 27:
			switch (enterSwitch) {
				case 1: exit(0);
					break;
				default: 
					enterSwitch = 1;
					board.dehighlight(highlightedTileX, highlightedTileZ);
					selectedButton = 0;
					break;
			}
		case '1': 
			if (secontCtrl == 0) secontCtrl = 1;
			else {
				firstCtrl = 1;
				board.dehighlight(highlightedTileX, highlightedTileX);
				highlightedTileX = firstCtrl;
				highlightedTileZ = secontCtrl;
			}
			break;
		case '2':
			if (secontCtrl == 0) secontCtrl = 2;
			else {
				firstCtrl = 2;
				board.dehighlight(highlightedTileX, highlightedTileX);
				highlightedTileX = firstCtrl;
				highlightedTileZ = secontCtrl;
			}
			break;
		case '3':
			if (secontCtrl == 0) secontCtrl = 3;
			else {
				firstCtrl = 3;
				board.dehighlight(highlightedTileX, highlightedTileX);
				highlightedTileX = firstCtrl;
				highlightedTileZ = secontCtrl;
			}
			break;
		case '4':
			if (secontCtrl == 0) secontCtrl = 4;
			else {
				firstCtrl = 4;
				highlightedTileX = firstCtrl;
				highlightedTileZ = secontCtrl;
			}
			break;
		case '5':
			if (secontCtrl == 0) secontCtrl = 5;
			else  {
				firstCtrl = 5;
				board.dehighlight(highlightedTileX, highlightedTileX);
				highlightedTileX = firstCtrl;
				highlightedTileZ = secontCtrl;
			}
			break;
		case '6':
			firstCtrl = 6;
			break;
		case '7':
			firstCtrl = 7;
			break;
		case '8':
			firstCtrl = 8;
			break;
		case '9':
			firstCtrl = 9;
			break;
		case 'd':
			if (firstCtrl != 0 && secontCtrl != 0
				&& resourcesNum >= defenderPrice
				&& !board.getTile(firstCtrl, secontCtrl)->getOccupied()
				&& board.getTile(firstCtrl, secontCtrl)->getState() != 3
				&& deadLanes[secontCtrl - 1] == 1)
			{
				defenderVector.push_back(Defender(firstCtrl, secontCtrl));
				board.getTile(firstCtrl, secontCtrl)->setOccupied(true);
				board.getTile(firstCtrl, secontCtrl)->setPieceCode(2);
				resourcesNum -= defenderPrice;
				board.dehighlight(firstCtrl, secontCtrl);
				firstCtrl = secontCtrl = 0;
			}
			break;
		case 'r':
			if (firstCtrl != 0 && secontCtrl != 0
				&& resourcesNum >= plantPrice
				&& !board.getTile(firstCtrl, secontCtrl)->getOccupied()
				&& board.getTile(firstCtrl, secontCtrl)->getState() != 3
				&& deadLanes[secontCtrl - 1] == 1)
			{
				plantVector.push_back(Plant(firstCtrl, secontCtrl));
				board.getTile(firstCtrl, secontCtrl)->setOccupied(true);
				board.getTile(firstCtrl, secontCtrl)->setPieceCode(1);
				resourcesNum -= plantPrice;
				board.dehighlight(firstCtrl, secontCtrl);
				firstCtrl = secontCtrl = 0;
			}
			break;
		case 'c':
			if (firstCtrl != 0 && secontCtrl != 0
				&& board.getTile(firstCtrl, secontCtrl)->getPieceCode() == 1
				&& board.getTile(firstCtrl, secontCtrl)->getOccupied()
				&& board.getTile(firstCtrl, secontCtrl)->getState() != 3)
			{
				for (int i = 0; i != plantVector.size(); i++) {
					if (plantVector.at(i).getColNum() == firstCtrl && plantVector.at(i).getRowNum() == secontCtrl) {
						plantVector.erase(plantVector.begin() + i);
						board.getTile(firstCtrl, secontCtrl)->setOccupied(false);
						board.getTile(firstCtrl, secontCtrl)->setPieceCode(0);
						board.dehighlight(firstCtrl, secontCtrl);
						firstCtrl = secontCtrl = 0;
						break;
					}
				}

			}
			if (firstCtrl != 0 && secontCtrl != 0
				&& board.getTile(firstCtrl, secontCtrl)->getPieceCode() == 2
				&& board.getTile(firstCtrl, secontCtrl)->getOccupied()
				&& board.getTile(firstCtrl, secontCtrl)->getState() != 3)
			{
				for (int i = 0; i != defenderVector.size(); i++) {
					if (defenderVector.at(i).getColNum() == firstCtrl && defenderVector.at(i).getRowNum() == secontCtrl) {
						defenderVector.erase(defenderVector.begin() + i);
						board.getTile(firstCtrl, secontCtrl)->setOccupied(false);
						board.getTile(firstCtrl, secontCtrl)->setPieceCode(0);
						board.dehighlight(firstCtrl, secontCtrl);
						firstCtrl = secontCtrl = 0;
						break;
					}
				}
			}
			break;

	}
}

void releaseKey(unsigned char key, int x, int y) {

	switch (key) {
	case 'w':
	case 's': heightFlag = 0;
		break;
	}
}

void pressArrowKey(int key, int xx, int yy) {

	switch (enterSwitch) {
	case 0: 
		switch (key) {
		case GLUT_KEY_LEFT: rotateFlag = 1;
			break;
		case GLUT_KEY_RIGHT: rotateFlag = 2;
			break;
		case GLUT_KEY_UP: zoomFlag = 2;
			break;
		case GLUT_KEY_DOWN: zoomFlag = 1;
			break;
		}
		break;
	case 1: 
		board.dehighlight(highlightedTileX, highlightedTileZ);
		switch (key) {
		case GLUT_KEY_LEFT: 
			highlightedTileX--;
			if (highlightedTileX < 1) highlightedTileX = 10;
			break;
		case GLUT_KEY_RIGHT: 
			highlightedTileX++;
			if (highlightedTileX > 10) highlightedTileX = 1;
			break;
		case GLUT_KEY_UP: 
			highlightedTileZ--;
			if (highlightedTileZ < 1) highlightedTileZ = 5;
			break;
		case GLUT_KEY_DOWN: 
			highlightedTileZ++;
			if (highlightedTileZ > 5) highlightedTileZ = 1;
			break;
		}
		break;
	case 2: 
		if (selectedButton != 3) SBtmp = selectedButton;
		switch (key) {
		case GLUT_KEY_LEFT: selectedButton = SBtmp = 1;
			break;
		case GLUT_KEY_RIGHT: selectedButton = SBtmp = 2;
			break;
		case GLUT_KEY_UP: selectedButton = SBtmp;
			break;
		case GLUT_KEY_DOWN: selectedButton = 3;
			break;
		}
		break;
	}

	
}

void releaseArrowKey(int key, int x, int y) {

	if (enterSwitch == 0) {
		switch (key) {
		case GLUT_KEY_LEFT:
		case GLUT_KEY_RIGHT: rotateFlag = 0;
			break;
		case GLUT_KEY_UP:
		case GLUT_KEY_DOWN: zoomFlag = 0;
			break;
		}
	}
}

int main(int argc, char **argv) {

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(windowWidth, windowHeight);
	glutCreateWindow("Plants Vs. Zombies");

	glutDisplayFunc(Display);
	glutReshapeFunc(changeSize);
	glutIdleFunc(Anim);

	glutKeyboardFunc(Key);
	glutKeyboardUpFunc(releaseKey);
	glutSpecialFunc(pressArrowKey);
	glutIgnoreKeyRepeat(1);
	glutSpecialUpFunc(releaseArrowKey);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);
	glShadeModel(GL_SMOOTH);

	glutMainLoop();
}