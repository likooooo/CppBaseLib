/*
 * @Description: 
 * @Version: 1.0
 * @Autor: like
 * @Date: 2021-10-01 23:13:25
 * @LastEditors: like
 * @LastEditTime: 2021-10-03 20:50:43
 */
#include <stdio.h>
#include <iostream>
#define GL_DEBUG
#include <System.Form.GL.hpp>

using namespace System::Form::GL;

void Resized( GLFWwindow* window, int w, int h );
void KeyPressed( GLFWwindow* window, int key, int scancode, int action, int mods );
void MouseClick( GLFWwindow* window, int button, int action, int mods );
void MouseMove( GLFWwindow* window, double x, double y );

GLfloat vertices[] =
{
    -1, -1, -1, -1, -1, 1, -1, 1, 1, -1, 1, -1,
    1, -1, -1, 1, -1, 1, 1, 1, 1, 1, 1, -1,
    -1, -1, -1, -1, -1, 1, 1, -1, 1, 1, -1, -1,
    -1, 1, -1, -1, 1, 1, 1, 1, 1, 1, 1, -1,
    -1, -1, -1, -1, 1, -1, 1, 1, -1, 1, -1, -1,
    -1, -1, 1, -1, 1, 1, 1, 1, 1, 1, -1, 1
};

GLfloat colors[] =
{
    0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1, 0,
    1, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 0,
    0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0,
    0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0,
    0, 0, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0,
    0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1
};

class GL2DWidget : public GLWidget
{
public:
    bool rotate;
    bool drawCube;
    bool drawTraigle;
    GLubyte* pixels;
    GL2DWidget() : GLWidget(300, 200, "2D Visualizer", 
        Resized, KeyPressed, MouseClick, MouseMove)
    {
        rotate      = true;
        drawCube    = true;
        drawTraigle = true;
        pixels      = NULL;
        Viewport::left  = 0;
        Viewport::bottom= 0;
        Viewport::width = GLWidgetInfo::width;
        Viewport::height= GLWidgetInfo::height;
        location= {0, 0, 100};
        focus   = {0, 0, 0};
        normalVector = {0, 1, 0};
        halfRadian = DEGREE_TO_RADIAN(45);
        aspect  = ((float)Viewport::width) / Viewport::height;
        nearLen = 1;
        farLen  = 100;
        glfwGetFramebufferSize(window, &(Viewport::width), &(Viewport::height));
        // Resized(window, Viewport::width, Viewport::height);
        // UpdateOrtho();
        UpdateViewport();
        UpdateCamera();
        UpdateImage(Viewport::width, Viewport::height);
    }
    void DrawFrame() override
    {
        static float alpha = 0;
        glMatrixMode(GL_PROJECTION_MATRIX);
        glLoadIdentity();
        glTranslatef(0,0,-2);
        UpdateOrtho();
        if(rotate)
            glRotatef(alpha, 0, 1, 0);
        DrawPixels();
        if(drawCube)
            DrawCube();
        if(drawTraigle)
            DoTriangle();
        alpha += 0.1f;
    }
    void UpdateImage(int w, int h)
    {
        if(pixels)
        {
            delete []pixels;
        }
        int len = w * h * 3;
        pixels = new GLubyte[w * h * 3];
        int slice = h/255;
        float scala;
        float scalaStep = 1.f/w;
        for(int r = 0, v = 0; r < h; r++)
        {
            scala = 0;
            for(int c = 0; c < w * 3; )
            {
                int v1 = v * scala;
                pixels[r * w * 3 + c++] = v1; 
                pixels[r * w * 3 + c++] = v1;
                pixels[r * w * 3 + c++] = v1; 
                scala += scalaStep;
            }
            if(r > slice)
            {
                v++;
                slice += slice;
            }
        }
        // memset(pixels, 200, len);
    }
    void DrawCube()
    {
        glMatrixMode(GL_MODELVIEW_MATRIX);
        //attempt to rotate cube
        /* We have a color array and a vertex array */
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, vertices);
        glColorPointer(3, GL_FLOAT, 0, colors);
        /* Send data : 24 vertices */
        glDrawArrays(GL_QUADS, 0, 24);
        /* Cleanup states */
        glDisableClientState(GL_COLOR_ARRAY);
        glDisableClientState(GL_VERTEX_ARRAY);
    }
    void DoTriangle()
    {
        glBegin(GL_TRIANGLES);
 
        glColor3f(1.0, 0.0, 0.0);    // Red
        glVertex3f(0.0, 1.0, 0.0);
    
        glColor3f(0.0, 1.0, 0.0);    // Green
        glVertex3f(-1.0, -1.0, 0.0);
    
        glColor3f(0.0, 0.0, 1.0);    // Blue
        glVertex3f(1.0, -1.0, 0.0);
        glEnd();
    }
    void DrawPixels()
    {
        if(NULL == pixels)
        {
            return;
        }
        glDrawPixels(
            Viewport::width, 
            Viewport::height, 
            GL_BGR, 
            GL_UNSIGNED_BYTE, pixels
        );
    }
};

GL2DWidget* w;
Viewport* view;
bool clicked = false;
void Resized( GLFWwindow* window, int w, int h )
{
    view->width = w;
    view->height = h;
    view->UpdateViewport();
    ::w->UpdateImage(w, h);
    ::w->aspect = (float)w/h;
    ::w->farLen = (float)w/2;
    // ::w->focus.x = (float)w/2;
    // ::w->focus.y = (float)h/2;
    ::w->UpdateCamera();
    // ::w->UpdateOrtho();
    printf("Resized : (%d, %d)\n", w, h);
}
void KeyPressed( GLFWwindow* window, int key, int scancode, int action, int mods )
{
    if (key == GLFW_KEY_ESCAPE && mods == 0)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    else if(key == GLFW_KEY_3 && 1 == action)
    {
        w->drawTraigle = !w->drawTraigle;
    }
    else if(key == GLFW_KEY_4 && 1 == action)
    {
        w->drawCube = !w->drawCube;
    }
    else if(key == GLFW_KEY_1 && 1 == action)
    {
        w->rotate = !w->rotate; 
        printf("%s Rotate\n", w->rotate ? "Enable" : "Disable");
    }
    printf("Key : (%d, %d, %d, %d)\n", key, scancode, action, mods);
}
void MouseClick( GLFWwindow* window, int button, int action, int mods )
{
    clicked = !clicked;
    printf("Mouse Click : (%d, %d, %d)\n", button, action, mods);
}
void MouseMove( GLFWwindow* window, double x, double y )
{
    if(!clicked)
    {
        return;
    }
    printf("Mouse Move : (%lf, %lf)\n", x, y);
}

int main()
{
    printf("%s\n", GlInfo::GetGlfwDescription());
    w = new GL2DWidget();
    view = (Viewport*)w;
    w->Run();
    return 0;
}