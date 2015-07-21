//
//  main.cpp
//  SimpleAudioBasedMIRVisualizationTemplate
//
//  Created by Yuan-Yi Fan on 7/17/15.
//  
//

#include <iostream>
// openGL
#if _WIN32
#include <gl/glut.h>
#elif __APPLE__
#include <GLUT/glut.h>
#endif
// portaudio
#include "portaudio.h"
#include <stdio.h>
#include <math.h>
// essentia utility
#include "SimpleEssentiaUtil.h"

// -------------------------------------------- portaudio setup --------------------------------------------

#define SAMPLE_RATE  (44100)
#define FRAMES_PER_BUFFER (512)
#define BUFFER_SIZE 512

int numBytes;
float radius_rms = 0.0f, radius_energy = 0.0f, radius_power = 0.0f, global_scaling = 100.0f;
const float DEG_2_RAD = M_PI/180;
float *sampleBlock, *buffer_to_analyze;

PaStreamParameters inputParameters, outputParameters;
PaStream *stream = NULL;
PaError err;

SimpleEssentiaUtil audioAnalyzer;

// -------------------------------------------- portaudio setup ends --------------------------------------------


// -------------------------------------------- simple openGL setup --------------------------------------------

#define PI 3.1415926
#define WIDTH 640
#define HEIGHT 640

bool animate = true;
bool fullscreen = false;
void custom_init();
void custom_reshape(int, int);
void custom_display();
void draw_waveform();
void init_function();

void custom_init()
{
	glEnable(GL_DEPTH_TEST);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glMatrixMode(GL_PROJECTION);
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable( GL_LINE_SMOOTH );
    glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
	glLoadIdentity();
	gluPerspective(45.0f, (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100000.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glClearColor(0.0, 0.0, 0.0, 0.0);
}

void custom_reshape(int w, int h)
{
	glViewport(0, 0, (GLsizei)WIDTH, (GLsizei)HEIGHT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100000.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void custom_display()
{
    
    if (fullscreen) {
        //glutEnterGameMode();
        glutFullScreen();
    }
    else{
        //glutLeaveGameMode();
        glutReshapeWindow(WIDTH, HEIGHT);
        //glutPositionWindow(0,0);
        fullscreen = false;
    }
    
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPushMatrix();
        gluLookAt(0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    
        draw_waveform();
    
        glPushMatrix();
            glLineWidth(1.0);
            glColor4f(0.3, 0.3, 0.89, 0.5);
            glBegin(GL_LINE_LOOP);
            for (int i=0; i < 360; i++)
            {
                float DEG_IN_RAD = i*DEG_2_RAD;
                glVertex3f(radius_rms*cos(DEG_IN_RAD), radius_rms*sin(DEG_IN_RAD), 0);
            }
            glEnd();
    
            glColor4f(0.3, 0.89, 0.3, 0.5);
            glBegin(GL_LINE_LOOP);
            for (int i=0; i < 360; i++)
            {
                float DEG_IN_RAD = i*DEG_2_RAD;
                glVertex3f(radius_energy*cos(DEG_IN_RAD), radius_energy*sin(DEG_IN_RAD), 0);
            }
            glEnd();
    
            glColor4f(0.89, 0.3, 0.3, 0.5);
            glBegin(GL_LINE_LOOP);
            for (int i=0; i < 360; i++)
            {
                float DEG_IN_RAD = i*DEG_2_RAD;
                glVertex3f(radius_power*cos(DEG_IN_RAD), radius_power*sin(DEG_IN_RAD), 0);
            }
            glEnd();
        glPopMatrix();
	glPopMatrix();
	glFlush();
	glutSwapBuffers();
}

void draw_waveform(){
    glPushMatrix();
    glTranslated(-1.0, 0.0, 0.0);
        glColor4f(0.9, 0.9, 0.9, 0.5);
        glBegin(GL_LINE_STRIP);
            for (int i = 0; i < FRAMES_PER_BUFFER; i++){
                glVertex3f(i/(FRAMES_PER_BUFFER/2.0), buffer_to_analyze[i] , 0);
            }
        glEnd();
    glPopMatrix();
}

void glutKeyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
        case 'a':
            animate = !animate;
            break;
        case 'f':
            fullscreen = !fullscreen;
            break;
        case 'q':
            exit(0);
            break;
        default:
            break;
    }
}

void glutIdle()
{
    if (animate)
    {
        Pa_ReadStream( stream, sampleBlock, FRAMES_PER_BUFFER );
        for (int i = 0; i < FRAMES_PER_BUFFER; i++){
            buffer_to_analyze[i]	= sampleBlock[i];
        }
        audioAnalyzer.analyze(buffer_to_analyze, BUFFER_SIZE);
        // printf("read analyzed result: %f \n", audioAnalyzer.getRms());
        
        radius_rms =    0.2*global_scaling*audioAnalyzer.getRms();
        radius_energy = 0.2*global_scaling*audioAnalyzer.getEnergy();
        radius_power =  3.0*global_scaling*audioAnalyzer.getPower();
    }
    glutPostRedisplay();
}

int custom_window()
{
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Simple Audio Based MIR Visualization Template");
	custom_init();
    glutIdleFunc(&glutIdle);
    glutKeyboardFunc(&glutKeyboard);
	glutReshapeFunc(custom_reshape);
	glutDisplayFunc(custom_display);
	glutMainLoop();
    
    return EXIT_SUCCESS;
}

// -------------------------------------------- simple openGL setup ends --------------------------------------------

int main(int argc, char** argv)
{

    // -------------------------------------------- portaudio setup --------------------------------------------
    
    numBytes = FRAMES_PER_BUFFER * 1 * sizeof(paFloat32) ;
    sampleBlock = (float *) malloc( numBytes );
    buffer_to_analyze =  new float [BUFFER_SIZE];
    
    err = Pa_Initialize();
    
    inputParameters.device = Pa_GetDefaultInputDevice();
    inputParameters.channelCount = 1;
    inputParameters.sampleFormat = paFloat32;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo( inputParameters.device )->defaultLowInputLatency ;
    inputParameters.hostApiSpecificStreamInfo = NULL;
    
    err = Pa_OpenStream(
                        &stream,
                        &inputParameters,
                        NULL,
                        SAMPLE_RATE,
                        FRAMES_PER_BUFFER,
                        paClipOff,
                        NULL,
                        NULL
                        );
    
    err = Pa_StartStream( stream );
    
    // -------------------------------------------- portaudio setup ends --------------------------------------------
    

    // -------------------------------------------- Simple OpenGL setup --------------------------------------------
    
    init_function();
    glutInit(&argc, argv);
    custom_window();
    
    // -------------------------------------------- Simple OpenGL setup ends --------------------------------------------

    return 0;
}

void init_function()
{
    audioAnalyzer.setup(BUFFER_SIZE, SAMPLE_RATE);
    
    for(int i = 0; i < BUFFER_SIZE; i++) {
        sampleBlock[i]= 0.0;
        buffer_to_analyze[i]= 0.0;
    }
}




