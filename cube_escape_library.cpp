#define _CRT_SECURE_NO_WARNINGS

#ifdef APPLE
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#else
#include <GL/glew.h>
#include <GL/freeglut.h>
#endif

#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cmath>

using namespace std;

#define PI 3.14159265

// Global variables
int windowWidth = 1600;
int windowHeight = 900;

int scene = 0;

float balloonY = 0.0f;
float balloonMove = 0.02f;
float balloonAngle = 0.0f;

bool purpleBookOpen = false;
bool blueBookOpen = false;
bool pinkBookOpen = false;
bool greenBookOpen = false;

bool instructionOpen = false;
float instructionScale = 1.0f;

string enteredCode = "";
string correctCode = "8383";
bool wrongCode = false;
bool successCode = false;

GLuint startTexture;
GLuint backgroundTexture;
GLuint instructionTexture;

char startImagePath[] = "C://Users//tala2//Downloads//start.bmp";
char backgroundImagePath[] = "C://Users//tala2//Downloads//background.bmp";
char instructionImagePath[] = "C://Users//tala2//Downloads//instruction BMP.bmp";

// Projection setup
void setPixelProjection() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, windowWidth, 0, windowHeight);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void setNormalProjection() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1, 1, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// Load BMP image
GLuint loadBMP(const char* filename) {
    unsigned char header[54];
    unsigned int dataPos, width, height, imageSize;
    unsigned char* data;

    FILE* file = fopen(filename, "rb");

    if (!file) {
        cout << "Image could not be opened: " << filename << endl;
        return 0;
    }

    fread(header, 1, 54, file);

    if (header[0] != 'B' || header[1] != 'M') {
        cout << "Not a BMP file: " << filename << endl;
        fclose(file);
        return 0;
    }

    dataPos = *(int*)&(header[0x0A]);
    imageSize = *(int*)&(header[0x22]);
    width = *(int*)&(header[0x12]);
    height = *(int*)&(header[0x16]);

    if (imageSize == 0)
        imageSize = width * height * 3;

    if (dataPos == 0)
        dataPos = 54;

    data = new unsigned char[imageSize];

    fread(data, 1, imageSize, file);
    fclose(file);

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
        GL_BGR, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    delete[] data;

    return textureID;
}

// Basic drawing functions
void drawRectangle(float x1, float y1, float x2, float y2) {
    glBegin(GL_QUADS);
    glVertex2f(x1, y1);
    glVertex2f(x2, y1);
    glVertex2f(x2, y2);
    glVertex2f(x1, y2);
    glEnd();
}

void drawText(float x, float y, string text) {
    glRasterPos2f(x, y);

    for (char c : text) {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c);
    }
}

void drawBookText(float x, float y, string text, void* font = GLUT_BITMAP_TIMES_ROMAN_24) {
    glColor3f(0, 0, 0);

    for (float dx = -0.003f; dx <= 0.003f; dx += 0.003f) {
        for (float dy = -0.003f; dy <= 0.003f; dy += 0.003f) {
            glRasterPos2f(x + dx, y + dy);

            for (char c : text) {
                glutBitmapCharacter(font, c);
            }
        }
    }

    glColor3f(1, 1, 1);
    glRasterPos2f(x, y);

    for (char c : text) {
        glutBitmapCharacter(font, c);
    }
}

void drawBackground(GLuint texture) {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);
    glColor3f(1, 1, 1);

    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex2f(0, 0);
    glTexCoord2f(1, 0); glVertex2f(windowWidth, 0);
    glTexCoord2f(1, 1); glVertex2f(windowWidth, windowHeight);
    glTexCoord2f(0, 1); glVertex2f(0, windowHeight);
    glEnd();

    glDisable(GL_TEXTURE_2D);
}

// Transparent hitbox
void drawHitBox(float x1, float y1, float x2, float y2) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(1.0f, 0.0f, 0.0f, 0.0f);

    glBegin(GL_QUADS);
    glVertex2f(x1, y1);
    glVertex2f(x2, y1);
    glVertex2f(x2, y2);
    glVertex2f(x1, y2);
    glEnd();

    glDisable(GL_BLEND);
}

// Instruction paper
void drawInstructionPaper() {
    if (!instructionOpen)
        return;

    float centerX = windowWidth / 2.0f;
    float centerY = windowHeight / 2.0f;

    float paperWidth = 350.0f * instructionScale;
    float paperHeight = 250.0f * instructionScale;

    float x1 = centerX - paperWidth / 2.0f;
    float x2 = centerX + paperWidth / 2.0f;
    float y1 = centerY - paperHeight / 2.0f;
    float y2 = centerY + paperHeight / 2.0f;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, instructionTexture);
    glColor3f(1, 1, 1);

    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex2f(x1, y1);
    glTexCoord2f(1, 0); glVertex2f(x2, y1);
    glTexCoord2f(1, 1); glVertex2f(x2, y2);
    glTexCoord2f(0, 1); glVertex2f(x1, y2);
    glEnd();

    glDisable(GL_TEXTURE_2D);
}

// Curved pages
void drawPurpleCurvedPage(float x_start, float width, float y_bottom, float height, bool leftSide) {
    glBegin(GL_QUAD_STRIP);

    for (float i = 0; i <= 1.01f; i += 0.1f) {
        float x = x_start + (i * width);
        float curve = 0.03f * sin(i * PI);

        if (leftSide)
            glColor3f(0.95f - (i * 0.05f), 0.95f - (i * 0.05f), 0.9f);
        else
            glColor3f(0.9f + (i * 0.1f), 0.9f + (i * 0.1f), 0.85f + (i * 0.1f));

        glVertex2f(x, y_bottom + curve);
        glVertex2f(x, y_bottom + height + curve);
    }

    glEnd();
}

void drawBlueCurvedPage(float x_start, float width, float y_bottom, float height, bool leftSide) {
    glBegin(GL_QUAD_STRIP);

    for (float i = 0; i <= 1.01f; i += 0.1f) {
        float x = x_start + (i * width);
        float curve = 0.03f * sin(i * PI);

        if (leftSide)
            glColor3f(0.90f - (i * 0.05f), 0.96f - (i * 0.05f), 1.0f);
        else
            glColor3f(0.9f + (i * 0.1f), 0.9f + (i * 0.1f), 0.85f + (i * 0.1f));

        glVertex2f(x, y_bottom + curve);
        glVertex2f(x, y_bottom + height + curve);
    }

    glEnd();
}

void drawPinkCurvedPage(float x_start, float width, float y_bottom, float height, bool leftSide) {
    glBegin(GL_QUAD_STRIP);

    for (float i = 0; i <= 1.01f; i += 0.1f) {
        float x = x_start + (i * width);
        float curve = 0.03f * sin(i * PI);

        if (leftSide)
            glColor3f(0.95f - (i * 0.05f), 0.95f - (i * 0.05f), 0.88f);
        else
            glColor3f(0.90f + (i * 0.08f), 0.90f + (i * 0.08f), 0.82f + (i * 0.08f));

        glVertex2f(x, y_bottom + curve);
        glVertex2f(x, y_bottom + height + curve);
    }

    glEnd();
}

void drawGreenCurvedPage(float x_start, float width, float y_bottom, float height, bool leftSide) {
    glBegin(GL_QUAD_STRIP);

    for (float i = 0; i <= 1.01f; i += 0.1f) {
        float x = x_start + (i * width);
        float curve = 0.03f * sin(i * PI);

        if (leftSide)
            glColor3f(0.95f - (i * 0.05f), 0.95f - (i * 0.05f), 0.9f);
        else
            glColor3f(0.9f + (i * 0.1f), 0.9f + (i * 0.1f), 0.85f + (i * 0.1f));

        glVertex2f(x, y_bottom + curve);
        glVertex2f(x, y_bottom + height + curve);
    }

    glEnd();
}

// Purple book
void drawClosedPurpleBook() {
    glColor3f(0.65f, 0.65f, 0.65f);
    drawRectangle(-0.45f, -0.58f, 0.48f, 0.58f);

    glColor3f(0.85f, 0.72f, 0.90f);
    drawRectangle(-0.52f, -0.50f, 0.45f, 0.62f);

    glColor3f(0.25f, 0.12f, 0.35f);
    glLineWidth(4);

    glBegin(GL_LINE_LOOP);
    glVertex2f(-0.52f, -0.50f);
    glVertex2f(0.45f, -0.50f);
    glVertex2f(0.45f, 0.62f);
    glVertex2f(-0.52f, 0.62f);
    glEnd();

    drawBookText(-0.28f, 0.22f, "Beware If You Open");
    drawBookText(-0.16f, 0.02f, "The Book");
    drawBookText(-0.30f, -0.22f, "You Can Not Get Back");
    drawBookText(-0.80f, -0.90f, "Press C to open | Press B to go back");
}

void drawOpenPurpleBook() {
    glColor3f(0.85f, 0.72f, 0.90f);
    drawRectangle(-0.82f, -0.65f, 0.85f, 0.85f);

    drawPurpleCurvedPage(-0.78f, 0.76f, -0.55f, 1.35f, true);
    drawPurpleCurvedPage(0.02f, 0.76f, -0.55f, 1.35f, false);

    drawBookText(-0.65f, 0.35f, "Mirror Puzzle");
    drawBookText(-0.70f, 0.18f, "Which number");
    drawBookText(-0.70f, 0.05f, "looks the same");
    drawBookText(-0.70f, -0.08f, "in a mirror?");

    glColor3f(0.55f, 0.75f, 0.95f);
    drawRectangle(0.22f, -0.12f, 0.62f, 0.28f);

    drawBookText(0.18f, -0.35f, "6");
    drawBookText(0.35f, -0.35f, "8");
    drawBookText(0.52f, -0.35f, "9");
    drawBookText(0.68f, -0.35f, "3");
    drawBookText(0.42f, -0.50f, "?");
    drawBookText(-0.90f, -0.90f, "Press c to close | Press B to go back");
}

// Blue book
void drawClosedBlueBook() {
    glColor3f(0.75f, 0.75f, 0.75f);
    drawRectangle(-0.45f, -0.58f, 0.48f, 0.58f);

    glColor3f(0.55f, 0.78f, 1.0f);
    drawRectangle(-0.52f, -0.50f, 0.45f, 0.62f);

    glColor3f(0.25f, 0.12f, 0.35f);
    glLineWidth(4);

    glBegin(GL_LINE_LOOP);
    glVertex2f(-0.52f, -0.50f);
    glVertex2f(0.45f, -0.50f);
    glVertex2f(0.45f, 0.62f);
    glVertex2f(-0.52f, 0.62f);
    glEnd();

    drawBookText(-0.22f, 0.22f, "Every Choice");
    drawBookText(-0.24f, 0.02f, "Has A Price...");
    drawBookText(-0.23f, -0.22f, "Open It?");
    drawBookText(-0.80f, -0.90f, "Press C to open | Press B to go back");
}

void drawTree(float x, float y) {
    glColor3f(0.45f, 0.25f, 0.10f);
    drawRectangle(x - 0.03f, y - 0.20f, x + 0.03f, y);

    glColor3f(0.10f, 0.55f, 0.15f);

    glBegin(GL_TRIANGLES);
    glVertex2f(x - 0.15f, y - 0.02f);
    glVertex2f(x + 0.15f, y - 0.02f);
    glVertex2f(x, y + 0.25f);
    glEnd();
}

void drawOpenBlueBook() {
    glColor3f(0.65f, 0.85f, 1.0f);
    drawRectangle(-0.82f, -0.65f, 0.85f, 0.85f);

    drawBlueCurvedPage(-0.78f, 0.76f, -0.55f, 1.35f, true);
    drawBlueCurvedPage(0.02f, 0.76f, -0.55f, 1.35f, false);

    drawBookText(-0.65f, 0.45f, "Counting Puzzle");
    drawBookText(-0.68f, 0.25f, "How many trees");
    drawBookText(-0.68f, 0.10f, "are there?");

    drawTree(0.25f, 0.25f);
    drawTree(0.45f, 0.25f);
    drawTree(0.65f, 0.25f);

    drawBookText(0.18f, -0.35f, "1");
    drawBookText(0.35f, -0.35f, "2");
    drawBookText(0.52f, -0.35f, "3");
    drawBookText(0.68f, -0.35f, "4");
    drawBookText(0.42f, -0.50f, "?");
    drawBookText(-0.90f, -0.90f, "Press c to close | Press B to go back");
}

// Pink book
void drawClosedPinkBook() {
    glColor3f(0.45f, 0.45f, 0.45f);
    drawRectangle(-0.45f, -0.58f, 0.48f, 0.58f);

    glColor3f(1.0f, 0.6f, 0.8f);
    drawRectangle(-0.52f, -0.50f, 0.45f, 0.62f);

    glColor3f(1.0f, 0.2f, 0.60f);
    glLineWidth(4);

    glBegin(GL_LINE_LOOP);
    glVertex2f(-0.52f, -0.50f);
    glVertex2f(0.45f, -0.50f);
    glVertex2f(0.45f, 0.62f);
    glVertex2f(-0.52f, 0.62f);
    glEnd();

    drawBookText(-0.23f, 0.20f, "Puzzle Book");
    drawBookText(-0.35f, 0.00f, "Press C");
    drawBookText(-0.25f, -0.22f, "To Open");
    drawBookText(-0.80f, -0.90f, "Press B to go back");
}

void drawOpenPinkBook() {
    glColor3f(1.0f, 0.6f, 0.8f);
    drawRectangle(-0.82f, -0.65f, 0.85f, 0.85f);

    drawPinkCurvedPage(-0.78f, 0.76f, -0.55f, 1.35f, true);
    drawPinkCurvedPage(0.02f, 0.76f, -0.55f, 1.35f, false);

    drawBookText(-0.65f, 0.35f, "Simple Puzzle");
    drawBookText(-0.65f, 0.15f, "What is 2 * 3 + 6 - 4 = ?");
    drawBookText(0.20f, 0.10f, "THINK CAREFULLY!");
    drawBookText(-0.90f, -0.90f, "Press c to close | Press B to go back");
}

// Green book
void drawDetailedClock() {
    float radius = 0.22f;

    glColor3f(0.85f, 0.85f, 0.85f);

    glBegin(GL_POLYGON);
    for (int i = 0; i < 360; i++) {
        float theta = i * PI / 180.0f;
        glVertex2f(radius * cos(theta), radius * sin(theta));
    }
    glEnd();

    glColor3f(0, 0, 0);
    glLineWidth(2.5f);

    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 360; i++) {
        float theta = i * PI / 180.0f;
        glVertex2f(radius * cos(theta), radius * sin(theta));
    }
    glEnd();

    for (int i = 0; i < 12; i++) {
        float theta = i * 30.0f * PI / 180.0f;

        glBegin(GL_LINES);
        glVertex2f((radius - 0.04f) * cos(theta), (radius - 0.04f) * sin(theta));
        glVertex2f(radius * cos(theta), radius * sin(theta));
        glEnd();
    }

    glLineWidth(2.5f);

    glBegin(GL_LINES);
    glVertex2f(0, 0);
    glVertex2f(-0.16f, 0.1f);
    glEnd();

    glLineWidth(6.0f);

    glBegin(GL_LINES);
    glVertex2f(0, 0);
    glVertex2f(0.14f, 0);
    glEnd();
}

void drawClosedGreenBook() {
    glColor3f(0.65f, 0.65f, 0.65f);
    drawRectangle(-0.45f, -0.58f, 0.48f, 0.58f);

    glColor3f(0.05f, 0.25f, 0.05f);
    drawRectangle(-0.52f, -0.50f, 0.45f, 0.62f);

    glColor3f(0.0f, 0.1f, 0.0f);
    glLineWidth(4);

    glBegin(GL_LINE_LOOP);
    glVertex2f(-0.52f, -0.50f);
    glVertex2f(0.45f, -0.50f);
    glVertex2f(0.45f, 0.62f);
    glVertex2f(-0.52f, 0.62f);
    glEnd();

    drawBookText(-0.18f, 0.15f, "Clock Puzzle");
    drawBookText(-0.80f, -0.90f, "Press C to open | Press B to go back");
}

void drawOpenGreenBook() {
    glColor3f(0.05f, 0.25f, 0.05f);
    drawRectangle(-0.85f, -0.70f, 0.85f, 0.85f);

    drawGreenCurvedPage(-0.78f, 0.76f, -0.55f, 1.35f, true);
    drawGreenCurvedPage(0.02f, 0.76f, -0.55f, 1.35f, false);

    glColor3f(0.2f, 0.2f, 0.2f);
    glLineWidth(3);

    glBegin(GL_LINES);
    glVertex2f(0, -0.58f);
    glVertex2f(0, 0.80f);
    glEnd();

    glPushMatrix();
    glTranslatef(0.40f, 0.15f, 0);
    drawDetailedClock();
    glPopMatrix();

    drawBookText(-0.72f, 0.45f, "The Time Challenge");
    drawBookText(-0.72f, 0.15f, "Where does the short");
    drawBookText(-0.72f, 0.02f, "hand point to?");
    drawBookText(-0.90f, -0.90f, "Press c to close | Press B to go back");
}

// Balloon animation
void drawCircle(float cx, float cy, float r) {
    glBegin(GL_POLYGON);

    for (int i = 0; i < 100; i++) {
        float angle = 2.0f * PI * i / 100;
        float x = r * cos(angle);
        float y = r * sin(angle);

        glVertex2f(cx + x, cy + y);
    }

    glEnd();
}

void drawBalloon() {
    balloonY += balloonMove;

    if (balloonY > 10 || balloonY < -10)
        balloonMove = -balloonMove;

    glPushMatrix();

    glTranslatef(1200, 500 + balloonY, 0);
    glRotatef(balloonAngle, 0, 0, 1);

    glColor3f(1.0f, 0.6f, 0.8f);
    drawCircle(0, 0, 50);

    glColor3f(1, 1, 1);
    glLineWidth(2);

    glBegin(GL_LINES);
    glVertex2f(0, -50);
    glVertex2f(0, -180);
    glEnd();

    glPopMatrix();
}

// Main scenes
void drawStartScene() {
    setPixelProjection();
    drawBackground(startTexture);

    glColor3f(0.9f, 0.45f, 0.15f);
    drawRectangle(650, 380, 950, 500);

    glColor3f(0.25f, 0.15f, 0.08f);
    glLineWidth(4);

    glBegin(GL_LINE_LOOP);
    glVertex2f(650, 380);
    glVertex2f(950, 380);
    glVertex2f(950, 500);
    glVertex2f(650, 500);
    glEnd();

    glColor3f(1, 1, 1);
    drawText(755, 435, "START");
}

void drawLibraryScene() {
    setPixelProjection();
    drawBackground(backgroundTexture);

    drawInstructionPaper();

    drawHitBox(219, 181, 267, 221);
    drawHitBox(1152, 95, 1472, 623);
    drawHitBox(1013, 306, 1026, 391);
    drawHitBox(682, 196, 710, 282);
    drawHitBox(832, 427, 881, 500);
    drawHitBox(787, 90, 811, 171);
}

void drawPurpleBookScene() {
    setPixelProjection();
    drawBackground(startTexture);

    setNormalProjection();

    if (!purpleBookOpen)
        drawClosedPurpleBook();
    else
        drawOpenPurpleBook();
}

void drawBlueBookScene() {
    setPixelProjection();
    drawBackground(startTexture);

    setNormalProjection();

    if (!blueBookOpen)
        drawClosedBlueBook();
    else
        drawOpenBlueBook();
}

void drawPinkBookScene() {
    setPixelProjection();
    drawBackground(startTexture);

    setNormalProjection();

    if (!pinkBookOpen)
        drawClosedPinkBook();
    else
        drawOpenPinkBook();
}

void drawGreenBookScene() {
    setPixelProjection();
    drawBackground(startTexture);

    setNormalProjection();

    if (!greenBookOpen)
        drawClosedGreenBook();
    else
        drawOpenGreenBook();
}

void drawDoorLockScene() {
    setPixelProjection();

    glClearColor(0.15f, 0.10f, 0.08f, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    glColor3f(1, 1, 1);
    drawText(650, 650, "Enter the 4-digit code:");

    glColor3f(0.9f, 0.9f, 0.9f);
    drawRectangle(620, 480, 980, 570);

    glColor3f(0, 0, 0);
    drawText(750, 520, enteredCode);

    glColor3f(1, 1, 1);
    drawText(610, 400, "Press ENTER to check");
    drawText(610, 350, "Press BACKSPACE to delete");
    drawText(610, 300, "Press B to go back");

    if (wrongCode) {
        glColor3f(1, 0, 0);
        drawText(720, 230, "Wrong!");
    }

    if (successCode) {
        glColor3f(0, 1, 0);
        drawText(700, 230, "You Win!");
        drawBalloon();
    }
}

// Display function
void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    if (scene == 0)
        drawStartScene();
    else if (scene == 1)
        drawLibraryScene();
    else if (scene == 2)
        drawPurpleBookScene();
    else if (scene == 3)
        drawPinkBookScene();
    else if (scene == 4)
        drawBlueBookScene();
    else if (scene == 5)
        drawGreenBookScene();
    else if (scene == 6)
        drawDoorLockScene();

    glutSwapBuffers();
}

// Mouse control
void mouse(int button, int state, int x, int y) {
    if (button != GLUT_LEFT_BUTTON || state != GLUT_DOWN)
        return;

    int newY = windowHeight - y;

    cout << "Mouse x = " << x << " , y = " << newY << endl;

    if (scene == 0) {
        if (x >= 650 && x <= 950 && newY >= 380 && newY <= 500) {
            scene = 1;
        }

        glutPostRedisplay();
        return;
    }

    if (scene == 1) {
        if (x >= 219 && x <= 267 && newY >= 181 && newY <= 221) {
            instructionOpen = true;
            instructionScale = 1.0f;
        }

        else if (x >= 1152 && x <= 1472 && newY >= 95 && newY <= 623) {
            enteredCode = "";
            wrongCode = false;
            successCode = false;
            scene = 6;
        }

        else if (x >= 1013 && x <= 1026 && newY >= 306 && newY <= 391) {
            blueBookOpen = false;
            scene = 4;
        }

        else if (x >= 682 && x <= 710 && newY >= 196 && newY <= 282) {
            pinkBookOpen = false;
            scene = 3;
        }

        else if (x >= 832 && x <= 881 && newY >= 427 && newY <= 500) {
            purpleBookOpen = false;
            scene = 2;
        }

        else if (x >= 787 && x <= 811 && newY >= 90 && newY <= 171) {
            greenBookOpen = false;
            scene = 5;
        }
    }

    glutPostRedisplay();
}

// Keyboard control
void keyboard(unsigned char key, int x, int y) {
    if (scene == 1 && instructionOpen) {
        if (key == 'L' || key == 'l') {
            instructionScale += 0.1f;
        }
        else if (key == 'S' || key == 's') {
            instructionScale -= 0.1f;

            if (instructionScale < 0.3f)
                instructionScale = 0.3f;
        }
        else if (key == 'B' || key == 'b') {
            instructionOpen = false;
        }

        glutPostRedisplay();
        return;
    }

    if (scene == 2) {
        if (key == 'C') purpleBookOpen = true;
        if (key == 'c') purpleBookOpen = false;
    }

    if (scene == 3) {
        if (key == 'C') pinkBookOpen = true;
        if (key == 'c') pinkBookOpen = false;
    }

    if (scene == 4) {
        if (key == 'C') blueBookOpen = true;
        if (key == 'c') blueBookOpen = false;
    }

    if (scene == 5) {
        if (key == 'C') greenBookOpen = true;
        if (key == 'c') greenBookOpen = false;
    }

    if (scene == 6) {
        if (key >= '0' && key <= '9' && enteredCode.length() < 4) {
            enteredCode += key;
        }

        if (key == 8 && enteredCode.length() > 0) {
            enteredCode.pop_back();
        }

        if (key == 13) {
            if (enteredCode == correctCode) {
                successCode = true;
                wrongCode = false;
                enteredCode = "";
            }
            else {
                wrongCode = true;
                successCode = false;
                enteredCode = "";
                scene = 1;
            }
        }
    }

    if (key == 'B' && scene != 0) {
        scene = 1;
    }

    if ((key == 'L' || key == 'l') && scene == 6) {
        balloonAngle += 10.0f;
    }

    glutPostRedisplay();
}

// Window resize
void reshape(int w, int h) {
    windowWidth = w;
    windowHeight = h;
    glViewport(0, 0, w, h);
}

// Initialize textures
void init() {
    glClearColor(0, 0, 0, 1);

    glEnable(GL_TEXTURE_2D);

    startTexture = loadBMP(startImagePath);
    backgroundTexture = loadBMP(backgroundImagePath);
    instructionTexture = loadBMP(instructionImagePath);
}

// Main function
int main(int argc, char** argv) {
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(windowWidth, windowHeight);
    glutInitWindowPosition(50, 50);

    glutCreateWindow("Cube Escape Library");

#ifndef APPLE
    glewInit();
#endif

    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutKeyboardFunc(keyboard);
    glutIdleFunc(display);

    glutMainLoop();

    return 0;
}