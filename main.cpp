#include <GLFW/glfw3.h>
#include <GL/glu.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>
enum DisplayMode {
    WIREFRAME,
    FILLED
};

DisplayMode currentDisplayMode = WIREFRAME;

bool lightEnabled = false;


float rotationX = 0.0f;
float rotationY = 0.0f;
bool rotating = false;
double lastMouseX, lastMouseY;

std::vector<std::string> transformations;
int currentTransformationIndex = -1;

struct Vertex {
    float x, y, z;
};

struct Face {
    int v1, v2, v3;
};

std::vector<Vertex> vertices;
std::vector<Face> faces;

std::vector<Vertex> transformedVertices;
std::vector<Face> transformedFaces;
std::vector<Vertex> previousTransformedVertices;

bool loadOBJ(const char* path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << path << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string type;
        iss >> type;

        if (type == "v") {
            Vertex vertex;
            iss >> vertex.x >> vertex.y >> vertex.z;
            vertices.push_back(vertex);
        } else if (type == "f") {
            Face face;
            std::string v1, v2, v3;
            iss >> v1 >> v2 >> v3;


            face.v1 = std::stoi(v1.substr(0, v1.find('/'))) - 1;
            face.v2 = std::stoi(v2.substr(0, v2.find('/'))) - 1;
            face.v3 = std::stoi(v3.substr(0, v3.find('/'))) - 1;

            faces.push_back(face);
        } else if (type == "s" || type == "t" || type == "x" || type == "y" || type == "z" || type == "c" || type == "e") {
            transformations.push_back(line);
        }
    }

    return true;
}
void scaleModel(float scaleFactor) {
    for (auto& vertex : vertices) {
        vertex.x *= scaleFactor;
        vertex.y *= scaleFactor;
        vertex.z *= scaleFactor;
    }
    for (auto& vertex : transformedVertices) {
        vertex.x *= scaleFactor;
        vertex.y *= scaleFactor;
        vertex.z *= scaleFactor;
    }
    for (auto& vertex : previousTransformedVertices) {
        vertex.x *= scaleFactor;
        vertex.y *= scaleFactor;
        vertex.z *= scaleFactor;
    }
}

void drawModelWireframe(const std::vector<Vertex>& modelVertices, const std::vector<Face>& modelFaces) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBegin(GL_TRIANGLES);
    for (const auto& face : modelFaces) {
        const Vertex& v1 = modelVertices[face.v1];
        const Vertex& v2 = modelVertices[face.v2];
        const Vertex& v3 = modelVertices[face.v3];

        glVertex3f(v1.x, v1.y, v1.z);
        glVertex3f(v2.x, v2.y, v2.z);
        glVertex3f(v3.x, v3.y, v3.z);
    }
    glEnd();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void drawModelFilled(const std::vector<Vertex>& modelVertices, const std::vector<Face>& modelFaces) {
    glBegin(GL_TRIANGLES);
    for (const auto& face : modelFaces) {
        const Vertex& v1 = modelVertices[face.v1];
        const Vertex& v2 = modelVertices[face.v2];
        const Vertex& v3 = modelVertices[face.v3];

        glVertex3f(v1.x, v1.y, v1.z);
        glVertex3f(v2.x, v2.y, v2.z);
        glVertex3f(v3.x, v3.y, v3.z);
    }
    glEnd();

    glColor3f(0.0f, 0.0f, 0.0f); // Preto para as arestas
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBegin(GL_TRIANGLES);
    for (const auto& face : modelFaces) {
        const Vertex& v1 = modelVertices[face.v1];
        const Vertex& v2 = modelVertices[face.v2];
        const Vertex& v3 = modelVertices[face.v3];

        glVertex3f(v1.x, v1.y, v1.z);
        glVertex3f(v2.x, v2.y, v2.z);
        glVertex3f(v3.x, v3.y, v3.z);
    }
    glEnd();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}
void drawModel(const std::vector<Vertex>& modelVertices, const std::vector<Face>& modelFaces) {
    if (currentDisplayMode == WIREFRAME) {
        drawModelWireframe(modelVertices, modelFaces);
    } else if (currentDisplayMode == FILLED) {
        drawModelFilled(modelVertices, modelFaces);
    }
}

void draw_axes() {

    float axisLength = 1.0f;
    float arrowSize = 0.05f;


    glColor3f(0.0f, 1.0f, 0.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex3f(-axisLength, 0.0f, 0.0f);
    glVertex3f(axisLength, 0.0f, 0.0f);
    glEnd();


    glBegin(GL_TRIANGLES);
    glVertex3f(axisLength, 0.0f, 0.0f);
    glVertex3f(axisLength - arrowSize, arrowSize, 0.0f);
    glVertex3f(axisLength - arrowSize, -arrowSize, 0.0f);
    glEnd();


    glColor3f(0.0f, 0.0f, 1.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex3f(0.0f, -axisLength, 0.0f);
    glVertex3f(0.0f, axisLength, 0.0f);
    glEnd();


    glBegin(GL_TRIANGLES);
    glVertex3f(0.0f, axisLength, 0.0f);
    glVertex3f(arrowSize, axisLength - arrowSize, 0.0f);
    glVertex3f(-arrowSize, axisLength - arrowSize, 0.0f);
    glEnd();


    glColor3f(1.0f, 0.0f, 0.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex3f(0.0f, 0.0f, -axisLength);
    glVertex3f(0.0f, 0.0f, axisLength);
    glEnd();


    glBegin(GL_TRIANGLES);
    glVertex3f(0.0f, 0.0f, axisLength);
    glVertex3f(arrowSize, 0.0f, axisLength - arrowSize);
    glVertex3f(-arrowSize, 0.0f, axisLength - arrowSize);
    glEnd();
}

void setupProjection(int width, int height) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)width / (double)height, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(1.5, 1.5, 1.5,0.0, 0.0, 0.0,0.0, 1.0, 0.0);
}
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    setupProjection(width, height);
}
void setupLight() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    GLfloat lightPosition[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat lightAmbient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat lightDiffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    GLfloat lightSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
}
void copyModel() {
    transformedVertices = vertices;
    transformedFaces = faces;
    previousTransformedVertices = vertices;
}

void disableLight() {
    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);
}
void applyScale(float sx, float sy, float sz) {
    if (std::isfinite(sx) && std::isfinite(sy) && std::isfinite(sz)) {
        glScalef(sx, sy, sz);
        std::cout << "Scaling: " << sx << ", " << sy << ", " << sz << std::endl;
    }
}
void applyTranslation(float tx, float ty, float tz) {
    glTranslatef(tx, ty, tz);
    std::cout << "Translation: " << tx << ", " << ty << ", " << tz << std::endl;
}
void applyRotationX(float angle) {
    glRotatef(angle, 1.0f, 0.0f, 0.0f);
    std::cout << "Rotation X: " << angle << std::endl;
}
void applyRotationY(float angle) {
    glRotatef(angle, 0.0f, 1.0f, 0.0f);
    std::cout << "Rotation Y: " << angle << std::endl;
}
void applyRotationZ(float angle) {
    glRotatef(angle, 0.0f, 0.0f, 1.0f);
    std::cout << "Rotation Z: " << angle << std::endl;
}
void applyShearing(float shx, float shy, float shz) {
    if (shx != 0) {
        GLfloat shear[] = {
                1.0, 0.0, 0.0, 0.0,
                shy, 1.0, shz, 0.0,
                0.0, 0.0, 1.0, 0.0,
                0.0, 0.0, 0.0, 1.0
        };
        glMultMatrixf(shear);
    } else if (shy != 0) {
        GLfloat shear[] = {
                1.0, shx, 0.0, 0.0,
                0.0, 1.0, 0.0, 0.0,
                0.0, shz, 1.0, 0.0,
                0.0, 0.0, 0.0, 1.0
        };
        glMultMatrixf(shear);
    } else if (shz != 0) {
        GLfloat shear[] = {
                1.0, 0.0, shx, 0.0,
                0.0, 1.0, shy, 0.0,
                0.0, 0.0, 1.0, 0.0,
                0.0, 0.0, 0.0, 1.0
        };
        glMultMatrixf(shear);
    }
    std::cout << "Shearing: " << shx << ", " << shy << ", " << shz << std::endl;
}
void applyReflection(float ex, float ey, float ez) {
    GLfloat reflect[16] = {
            ex == 1 ? -1.0f : 1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, ey == 1 ? -1.0f : 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, ez == 1 ? -1.0f : 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
    };
    glMultMatrixf(reflect);
    std::cout << "Reflection: " << ex << ", " << ey << ", " << ez << std::endl;
}

void applyTransformations() {
    for (int i = 0; i <= currentTransformationIndex; ++i) {
        std::istringstream iss(transformations[i]);
        std::string type;
        iss >> type;

        if (type == "s") {
            float sx, sy, sz;
            iss >> sx >> sy >> sz;
            applyScale(sx, sy, sz);
        } else if (type == "t") {
            float tx, ty, tz;
            iss >> tx >> ty >> tz;
            applyTranslation(tx, ty, tz);
        } else if (type == "x") {
            float angle;
            iss >> angle;
            applyRotationX(angle);
        } else if (type == "y") {
            float angle;
            iss >> angle;
            applyRotationY(angle);
        } else if (type == "z") {
            float angle;
            iss >> angle;
            applyRotationZ(angle);
        } else if (type == "c") {
            float shx, shy, shz;
            iss >> shx >> shy >> shz;
            applyShearing(shx, shy, shz);
        } else if (type == "e") {
            float ex, ey, ez;
            iss >> ex >> ey >> ez;
            applyReflection(ex, ey, ez);
        }
    }
}
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        if (currentDisplayMode == WIREFRAME) {
            currentDisplayMode = FILLED;
        } else {
            currentDisplayMode = WIREFRAME;
        }
    }
    if (key == GLFW_KEY_L && action == GLFW_PRESS) {
        lightEnabled = !lightEnabled;
        if (lightEnabled) {
            setupLight();
        } else {
            disableLight();
        }
    }
    if ((key == GLFW_KEY_Q || key == GLFW_KEY_ESCAPE) && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        if (currentTransformationIndex < static_cast<int>(transformations.size()) - 1) {
            currentTransformationIndex++;
            previousTransformedVertices = transformedVertices;
        } else {
            currentTransformationIndex = -1;
            copyModel();
        }

    }
}
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        rotating = true;
        glfwGetCursorPos(window, &lastMouseX, &lastMouseY);
    } else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        rotating = false;
    }
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    if (rotating) {
        double dx = xpos - lastMouseX;
        double dy = ypos - lastMouseY;
        rotationX += dy * 0.5f;
        rotationY += dx * 0.5f;
        lastMouseX = xpos;
        lastMouseY = ypos;
    }
}

void printTransformations() {
    std::cout << "Transformations loaded from .obj file:" << std::endl;
    for (const auto& transformation : transformations) {
        std::cout << transformation << std::endl;
    }
}
void setupMaterial(const GLfloat* ambient, const GLfloat* diffuse, const GLfloat* specular, GLfloat shininess) {
    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT, GL_SHININESS, shininess);
}
int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }
    if (!loadOBJ("/home/kegure/CLionProjects/untitled4/DonutMaiara.obj")) {
        return -1;
    }
    copyModel();

    float scaleFactor = 10.0;
    scaleModel(scaleFactor);
    GLFWwindow* window = glfwCreateWindow(640, 480, "Visualizador 3D", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    printTransformations();

    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glfwSetKeyCallback(window, key_callback);

    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    framebuffer_size_callback(window, width, height);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

    GLfloat modelAmbient[] = {0.2f, 0.2f, 0.2f, 1.0f};
    GLfloat modelDiffuse[] = {0.8f, 0.8f, 0.8f, 1.0f};
    GLfloat modelSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat modelShininess = 50.0f;
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        if (lightEnabled) {
            glEnable(GL_LIGHTING);
        } else {
            glDisable(GL_LIGHTING);
        }


        glPushMatrix();
        glRotatef(rotationX, 1.0f, 0.0f, 0.0f);
        glRotatef(rotationY, 0.0f, 1.0f, 0.0f);

        draw_axes();
        setupMaterial(modelAmbient, modelDiffuse, modelSpecular, modelShininess);
        glColor3f(0.0f, 0.0f, 1.0f); // Azul
        drawModel(vertices, faces);

        if (currentTransformationIndex >= 0) {
            glColor3f(0.0f, 1.0f, 0.0f);
            drawModel(previousTransformedVertices, transformedFaces);
        }

        glPushMatrix();
        applyTransformations();
        glColor3f(1.0f, 0.0f, 0.0f);
        drawModel(transformedVertices, transformedFaces);
        glPopMatrix();


        glPopMatrix();



        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
